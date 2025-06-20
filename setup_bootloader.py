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

if not os.path.isfile(CC):
    CC = shutil.which("i686-linux-gnu-gcc") or "i686-linux-gnu-gcc"
if not os.path.isfile(LD):
    LD = shutil.which("i686-linux-gnu-ld") or "i686-linux-gnu-ld"
if not os.path.isfile(OBJDUMP):
    OBJDUMP = shutil.which("i686-linux-gnu-objdump") or "i686-linux-gnu-objdump"

if not shutil.which(CC):
    CC = "gcc"
if not shutil.which(LD):
    LD = "ld"
if not shutil.which(OBJDUMP):
    OBJDUMP = "objdump"

CDRTOOLS_DIR = os.environ.get("CDRTOOLS_DIR") or r"C:\\Program Files (x86)\\cdrtools"
MKISOFS_EXE = os.environ.get("MKISOFS") or os.path.join(CDRTOOLS_DIR, "mkisofs.exe")
if not os.path.isfile(MKISOFS_EXE):
    MKISOFS_EXE = shutil.which("mkisofs") or "mkisofs"

# Only build sources inside the kernel directory
KERNEL_PROJECT_ROOT = "OptrixOS-Kernel"
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
OUTPUT_ISO = os.path.join(SCRIPT_DIR, "OptrixOS.iso")
KERNEL_BIN = "OptrixOS-kernel.bin"
DISK_IMG = "disk.img"
TMP_ISO_DIR = "_iso_tmp"
OBJ_DIR = "_build_obj"

tmp_files = []

def check_file(f):
    if not os.path.isfile(f):
        print(f"Error: Required file not found: {f}")
        sys.exit(1)

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
    # Turns 'kernel/foo/bar.c' -> '_build_obj_kernel_foo_bar.o'
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

def make_dynamic_img(boot_bin, kernel_bin, img_out):
    print("Creating dynamically-sized disk image...")
    boot = open(boot_bin, "rb").read()
    if len(boot) != 512:
        print("Error: Bootloader must be exactly 512 bytes!")
        sys.exit(1)
    kern = open(kernel_bin, "rb").read()
    total = 512 + len(kern)
    min_size = 1474560  # 1.44MB
    img_size = roundup(total, 512)
    if img_size < min_size:
        img_size = min_size
    with open(img_out, "wb") as img:
        img.write(boot)
        img.write(kern)
        img.write(b'\0' * (img_size - total))
    print(f"Disk image ({img_size // 1024} KB) created (kernel+boot: {total} bytes).")
    tmp_files.append(img_out)

def collect_source_files(rootdir):
    asm_files, c_files, h_files = [], [], []
    skip_dirs = {'.git', '_iso_tmp', '_build_obj', '__pycache__'}
    for root, dirs, files in os.walk(rootdir):
        dirs[:] = [d for d in dirs if d not in skip_dirs and not d.startswith('.')]
        for f in files:
            path = os.path.join(root, f)
            if f.endswith('.asm'):
                asm_files.append(path)
            elif f.endswith('.c'):
                c_files.append(path)
            elif f.endswith('.h'):
                h_files.append(path)
    return asm_files, c_files, h_files

# === ROOT FILE EMBEDDING ===
def collect_resource_files():
    res_dir = os.path.join(KERNEL_PROJECT_ROOT, "resources")
    files = []
    for root, _dirs, filenames in os.walk(res_dir):
        for f in filenames:
            if f.startswith('.'):
                continue
            files.append(os.path.join(root, f))
    return files

ROOT_C = os.path.join(KERNEL_PROJECT_ROOT, "src", "root_files.c")
ROOT_H = os.path.join(KERNEL_PROJECT_ROOT, "include", "root_files.h")

def generate_root_files():
    entries = []
    files = collect_resource_files()
    for f in files:
        if not os.path.isfile(f):
            continue
        rel = os.path.relpath(f, KERNEL_PROJECT_ROOT).replace("\\", "/")
        with open(f, "r", errors="ignore") as fh:
            data = fh.read().replace("\\", "\\\\").replace("\"", "\\\"").replace("\n", "\\n")
        entries.append((rel, data))
    with open(ROOT_H, "w") as h:
        h.write("#ifndef ROOT_FILES_H\n#define ROOT_FILES_H\n")
        h.write("typedef struct { const char* path; const char* data; } root_file;\n")
        h.write("extern const int root_files_count;\n")
        h.write("extern const root_file root_files[];\n")
        h.write("#endif\n")
    with open(ROOT_C, "w") as c:
        c.write('#include "root_files.h"\n')
        c.write("const root_file root_files[] = {\n")
        for name, data in entries:
            c.write(f'    {{"{name}", "{data}"}},\n')
        c.write("};\n")
        c.write(f"const int root_files_count = {len(entries)};\n")
    return ROOT_C



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
    # Always check for kernel.ld in project root or current dir
    linker_script = os.path.join("OptrixOS-Kernel", "kernel.ld")
    if not os.path.exists(linker_script):
        linker_script = "kernel.ld"
    check_file(linker_script)
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

def on_rm_error(func, path, exc_info):
    # Make file writable and retry (Windows-safe)
    try:
        os.chmod(path, stat.S_IWRITE)
        func(path)
    except Exception as e:
        print(f"Failed to delete {path}: {e}")

def ignore_git(dir, files):
    # Don't copy .git or any hidden folders/files
    return [f for f in files if f == ".git" or f.startswith('.')]

def copy_tree_to_iso(tmp_iso_dir, proj_root):
    """Create the ISO file tree with only the kernel folder and disk image."""
    print("Copying project files to ISO structure...")
    if os.path.exists(tmp_iso_dir):
        shutil.rmtree(tmp_iso_dir, onerror=on_rm_error)
    os.makedirs(tmp_iso_dir, exist_ok=True)

    # Copy the entire kernel folder
    kernel_dest = os.path.join(tmp_iso_dir, os.path.basename(proj_root))
    shutil.copytree(proj_root, kernel_dest, ignore=ignore_git, dirs_exist_ok=True)

    # Place disk image at ISO root
    if os.path.exists(DISK_IMG):
        shutil.copy(DISK_IMG, os.path.join(tmp_iso_dir, "disk.img"))


def make_iso_with_tree(tmp_iso_dir, iso_out):
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
        "-b", "disk.img",
        "-R", "-J", "-l",
        tmp_iso_dir
    ]
    run(cmd)
    # Forcibly copy ISO to script's dir if not already there
    script_dir = os.path.dirname(os.path.abspath(__file__))
    dest_iso = os.path.join(script_dir, "OptrixOS.iso")
    if os.path.abspath(iso_out) != dest_iso:
        if os.path.exists(dest_iso):
            os.remove(dest_iso)
        shutil.copyfile(iso_out, dest_iso)
        print(f"ISO forcibly copied to: {dest_iso}")
    print(f"ISO created: {dest_iso}")
    # DO NOT add the ISO to tmp_files here

def cleanup():
    for f in tmp_files:
        try:
            # Never touch any .iso file, ever
            if os.path.exists(f) and not f.lower().endswith(".iso"):
                os.remove(f)
        except Exception as e:
            print(f"Warning: Could not delete {f}: {e}")
    if os.path.exists(OBJ_DIR):
        shutil.rmtree(OBJ_DIR, onerror=on_rm_error)

def main():
    print("Collecting all project source files...")
    asm_files, c_files, h_files = collect_source_files(KERNEL_PROJECT_ROOT)
    # Exclude the old scheduler from builds
    c_files = [f for f in c_files if not f.endswith('scheduler.c')]
    root_c = generate_root_files()
    if root_c and root_c not in c_files:
        c_files.append(root_c)
    c_files = list(dict.fromkeys(c_files))
    print(f"Found {len(asm_files)} asm, {len(c_files)} c, {len(h_files)} h files.")
    boot_bin, kernel_bin = build_kernel(asm_files, c_files, out_bin=KERNEL_BIN)
    make_dynamic_img(boot_bin, kernel_bin, DISK_IMG)
    copy_tree_to_iso(TMP_ISO_DIR, KERNEL_PROJECT_ROOT)
    make_iso_with_tree(TMP_ISO_DIR, OUTPUT_ISO)

    print("\nCleaning up temporary build files... (ISO is NEVER deleted)")
    for f in tmp_files:
        try:
            # Never touch any .iso file, ever
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
