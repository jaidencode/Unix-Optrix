import subprocess
import sys
import os
import shutil
import stat

# --- CONFIGURATION ---
TOOLCHAIN_DIR = os.environ.get("TOOLCHAIN_DIR") or r"C:\\Users\\jaide\\Downloads\\i686-elf-tools-windows\\bin"
CC = os.environ.get("CC") or os.path.join(TOOLCHAIN_DIR, "i686-elf-gcc.exe")
LD = os.environ.get("LD") or os.path.join(TOOLCHAIN_DIR, "i686-elf-ld.exe")
OBJDUMP = os.environ.get("OBJDUMP") or os.path.join(TOOLCHAIN_DIR, "i686-elf-objdump.exe")
NASM = "nasm"

CDRTOOLS_DIR = os.environ.get("CDRTOOLS_DIR") or r"C:\\Program Files (x86)\\cdrtools"
MKISOFS_EXE = os.environ.get("MKISOFS") or os.path.join(CDRTOOLS_DIR, "mkisofs.exe")
if not os.path.isfile(MKISOFS_EXE):
    MKISOFS_EXE = shutil.which("mkisofs") or "mkisofs"

KERNEL_PROJECT_ROOT = "OptrixOS-Kernel"
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
OUTPUT_ISO = os.path.join(SCRIPT_DIR, "OptrixOS.iso")
KERNEL_BIN = "OptrixOS-kernel.bin"
TMP_ISO_DIR = "_iso_tmp"
OBJ_DIR = "_build_obj"

BOOT_IMG = "boot.img"   # Boot partition image file (e.g., FAT/ext2)
FS_IMG   = "fs.img"     # File system partition image
OS_IMG   = "os.img"     # OS/user data partition image

tmp_files = []

def run(cmd, **kwargs):
    print(" ".join(str(x) for x in cmd))
    result = subprocess.run(cmd, capture_output=True, text=True, **kwargs)
    if result.returncode != 0:
        print("Error running:", ' '.join(str(x) for x in cmd))
        print(result.stdout)
        print(result.stderr)
        sys.exit(1)
    return result

def ensure_obj_dir():
    if not os.path.exists(OBJ_DIR):
        os.makedirs(OBJ_DIR, exist_ok=True)

def obj_from_src(src):
    no_sep = src.replace(os.sep, "_").replace("/", "_")
    base = os.path.splitext(no_sep)[0]
    return os.path.join(OBJ_DIR, f"{base}.o")

def assemble(src, out, fmt="bin", defines=None):
    print(f"Assembling {src} -> {out} ...")
    fmt_flag = "-f" + fmt
    cmd = [NASM, fmt_flag]
    if defines:
        for k, v in defines.items():
            cmd.append(f"-D{k}={v}")
    cmd += [src, "-o", out]
    run(cmd)
    tmp_files.append(out)

def compile_c(src, out):
    run([
        CC,
        "-ffreestanding",
        "-fno-pie",
        "-fno-pic",
        "-m32",
        "-c", src,
        "-o", out,
        "-Iinclude",
        "-IOptrixOS-Kernel/include"
    ])
    tmp_files.append(out)

def roundup(x, align):
    return ((x + align - 1) // align) * align

def build_kernel(asm_files, c_files, out_bin):
    ensure_obj_dir()
    obj_files = []
    bootloader_src = None

    for asm in asm_files:
        obj = obj_from_src(asm)
        base = os.path.splitext(os.path.basename(asm))[0]
        if "bootloader" in base:
            bootloader_src = asm
        else:
            run([NASM, "-felf32", asm, "-o", obj])
            obj_files.append(obj)
            tmp_files.append(obj)
    for c in c_files:
        c_obj = obj_from_src(c)
        compile_c(c, c_obj)
        obj_files.append(c_obj)
    linker_script = os.path.join("OptrixOS-Kernel", "kernel.ld")
    if not os.path.exists(linker_script):
        linker_script = "kernel.ld"
    if not os.path.isfile(linker_script):
        print(f"Error: Required file not found: {linker_script}")
        sys.exit(1)
    elf_out = out_bin.replace(".bin", ".elf")
    print(f"Linking kernel ELF: {elf_out}")
    link_cmd_elf = [
        LD, "-T", linker_script, "-m", "elf_i386", "-nostdlib"
    ] + obj_files + ["-o", elf_out]
    run(link_cmd_elf)
    tmp_files.append(elf_out)
    print(f"Converting ELF -> BIN: {elf_out} -> {out_bin}")
    link_cmd_bin = [
        LD, "-T", linker_script, "-m", "elf_i386", "--oformat", "binary", "-nostdlib"
    ] + obj_files + ["-o", out_bin]
    run(link_cmd_bin)
    tmp_files.append(out_bin)
    kernel_bytes = os.path.getsize(out_bin)
    sectors = roundup(kernel_bytes, 512) // 512
    boot_bin = "bootloader.bin"
    assemble(bootloader_src, boot_bin, fmt="bin", defines={"KERNEL_SECTORS": sectors})
    return boot_bin, out_bin

def pad_bootloader_for_no_emul(bootloader_bin, out_path="bootloader_padded.bin"):
    """Pads bootloader to multiple of 2048 bytes for El Torito 'no emulation' mode."""
    with open(bootloader_bin, "rb") as f:
        data = f.read()
    sectors = (len(data) + 2047) // 2048
    padded = data.ljust(sectors * 2048, b"\0")
    with open(out_path, "wb") as f:
        f.write(padded)
    return out_path, sectors

def make_partition_img(filename, mb, fs="fat", label="PART"):
    """Creates a blank partition image of specified size and type."""
    size = mb * 1024 * 1024
    print(f"Creating {fs.upper()} partition image: {filename} ({mb}MB)...")
    with open(filename, "wb") as f:
        f.truncate(size)
    if fs == "fat":
        try:
            from pyfatfs.PyFat import PyFat
            pf = PyFat()
            pf.mkfs(filename, PyFat.FAT_TYPE_FAT16, size=size, sector_size=512, label=label)
            pf.close()
        except Exception:
            print("pyfatfs not found or error occurred; partition image will be blank.")
    elif fs == "ext2":
        if shutil.which("mkfs.ext2"):
            run(["mkfs.ext2", "-F", filename])
        else:
            print("mkfs.ext2 not found; partition image will be blank ext2.")
    else:
        print("Unknown filesystem type, leaving blank image.")
    tmp_files.append(filename)

def copy_tree_to_iso(tmp_iso_dir, proj_root, boot_img_name, kernel_bin):
    print("Copying project files to ISO structure...")
    if os.path.exists(tmp_iso_dir):
        shutil.rmtree(tmp_iso_dir, onerror=on_rm_error)
    os.makedirs(tmp_iso_dir, exist_ok=True)
    kernel_dest = os.path.join(tmp_iso_dir, os.path.basename(proj_root))
    shutil.copytree(proj_root, kernel_dest, dirs_exist_ok=True)
    shutil.copy(boot_img_name, os.path.join(tmp_iso_dir, os.path.basename(boot_img_name)))
    shutil.copy(kernel_bin, os.path.join(tmp_iso_dir, os.path.basename(kernel_bin)))
    # Add all partition images
    for img in [BOOT_IMG, FS_IMG, OS_IMG]:
        if os.path.exists(img):
            shutil.copy(img, os.path.join(tmp_iso_dir, os.path.basename(img)))

def make_iso_with_tree(tmp_iso_dir, iso_out, boot_img_name, boot_sectors):
    print(f"Creating ISO using: {MKISOFS_EXE}")
    print(f"ISO should be written to: {iso_out}")
    if not os.path.isfile(MKISOFS_EXE):
        print(f"Error: mkisofs.exe not found at {MKISOFS_EXE}!")
        sys.exit(1)
    if os.path.exists(iso_out):
        os.remove(iso_out)
    cmd = [
        MKISOFS_EXE,
        "-quiet",
        "-o", iso_out,
        "-b", os.path.basename(boot_img_name),
        "-no-emul-boot",
        "-boot-load-size", str(boot_sectors),
        "-boot-info-table",
        "-R", "-J", "-l",
        tmp_iso_dir
    ]
    run(cmd)
    script_dir = os.path.dirname(os.path.abspath(__file__))
    dest_iso = os.path.join(script_dir, "OptrixOS.iso")
    if os.path.abspath(iso_out) != dest_iso:
        if os.path.exists(dest_iso):
            os.remove(dest_iso)
        shutil.copyfile(iso_out, dest_iso)
        print(f"ISO forcibly copied to: {dest_iso}")
    print(f"ISO created: {dest_iso}")

def on_rm_error(func, path, exc_info):
    try:
        os.chmod(path, stat.S_IWRITE)
        func(path)
    except Exception as e:
        print(f"Failed to delete {path}: {e}")

def cleanup():
    for f in tmp_files:
        try:
            if os.path.exists(f) and not f.lower().endswith(".iso"):
                os.remove(f)
        except Exception as e:
            print(f"Warning: Could not delete {f}: {e}")
    if os.path.exists(OBJ_DIR):
        shutil.rmtree(OBJ_DIR, onerror=on_rm_error)

def main():
    print("Collecting all project source files...")
    asm_files, c_files, h_files = [], [], []
    for root, dirs, files in os.walk(KERNEL_PROJECT_ROOT):
        for f in files:
            path = os.path.join(root, f)
            if f.endswith('.asm'):
                asm_files.append(path)
            elif f.endswith('.c'):
                c_files.append(path)
            elif f.endswith('.h'):
                h_files.append(path)
    c_files = [f for f in c_files if not f.endswith('scheduler.c')]
    print(f"Found {len(asm_files)} asm, {len(c_files)} c, {len(h_files)} h files.")

    boot_bin, kernel_bin = build_kernel(asm_files, c_files, out_bin=KERNEL_BIN)

    # -- El Torito No-Emulation mode: pad bootloader
    boot_img_name, boot_sectors = pad_bootloader_for_no_emul(boot_bin)

    # -- Create partition images (sizes in MB, adjust as needed)
    make_partition_img(BOOT_IMG, 64, fs="fat", label="BOOT")
    make_partition_img(FS_IMG, 256, fs="ext2", label="FS")
    make_partition_img(OS_IMG, 512, fs="ext2", label="OS")

    # -- Build ISO structure and copy files
    copy_tree_to_iso(TMP_ISO_DIR, KERNEL_PROJECT_ROOT, boot_img_name, kernel_bin)

    # -- Create ISO with no emulation, includes all partition images as files
    make_iso_with_tree(TMP_ISO_DIR, OUTPUT_ISO, boot_img_name, boot_sectors)

    print("\nCleaning up temporary build files... (ISO is NEVER deleted)")
    for f in tmp_files:
        try:
            if os.path.exists(f) and not f.lower().endswith(".iso"):
                os.remove(f)
        except Exception as e:
            print(f"Warning: Could not delete {f}: {e}")
    if os.path.exists(TMP_ISO_DIR):
        shutil.rmtree(TMP_ISO_DIR, onerror=on_rm_error)
    cleanup()
    print(f"\nBuild complete! Bootable ISO is at: {OUTPUT_ISO}")

if __name__ == "__main__":
    main()
