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
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
KERNEL_PROJECT_ROOT = os.path.join(SCRIPT_DIR, "OptrixOS-Kernel")
OUTPUT_ISO = os.path.join(SCRIPT_DIR, "OptrixOS.iso")
KERNEL_BIN = "OptrixOS-kernel.bin"
DISK_IMG = "ssd.img"
TMP_ISO_DIR = "_iso_tmp"
OBJ_DIR = "_build_obj"
EMBED_HEADER = os.path.join(KERNEL_PROJECT_ROOT, "include", "embedded_resources.h")
EMBED_SOURCE = os.path.join(KERNEL_PROJECT_ROOT, "src", "embedded_resources.c")

VERIFICATION_CONTENT = b"VERIFICATION_OK"

def ensure_verification_file():
    vf = os.path.join(KERNEL_PROJECT_ROOT, "resources", "verification.bin")
    if not os.path.exists(vf) or open(vf, "rb").read() != VERIFICATION_CONTENT:
        os.makedirs(os.path.dirname(vf), exist_ok=True)
        with open(vf, "wb") as fh:
            fh.write(VERIFICATION_CONTENT)

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

def collect_resources():
    resources = []
    if os.path.isdir(RESOURCE_DIR):
        for root, _, files in os.walk(RESOURCE_DIR):
            for name in sorted(files):
                path = os.path.join(root, name)
                with open(path, "rb") as fh:
                    data = fh.read()
                rel = os.path.relpath(path, RESOURCE_DIR).replace("\\", "/")
                resources.append({"name": f"resources/{rel}", "data": data, "size": len(data)})
    return resources

def generate_embedded_sources(resources):
    os.makedirs(os.path.dirname(EMBED_HEADER), exist_ok=True)
    os.makedirs(os.path.dirname(EMBED_SOURCE), exist_ok=True)
    with open(EMBED_HEADER, "w") as fh:
        fh.write("#ifndef EMBEDDED_RESOURCES_H\n#define EMBEDDED_RESOURCES_H\n")
        fh.write("#include <stdint.h>\n\n")
        fh.write("typedef struct { const char* name; const unsigned char* data; uint32_t size; } embedded_resource;\n")
        fh.write(f"#define EMBEDDED_RESOURCE_COUNT {len(resources)}\n")
        fh.write("extern const embedded_resource embedded_resources[EMBEDDED_RESOURCE_COUNT];\n")
        fh.write("#endif\n")

    with open(EMBED_SOURCE, "w") as fc:
        fc.write('#include "embedded_resources.h"\n')
        for idx, r in enumerate(resources):
            array_name = f'resource_data_{idx}'
            bytes_formatted = ','.join(f'0x{b:02x}' for b in r["data"])
            fc.write(f'static const unsigned char {array_name}[] = {{{bytes_formatted}}};\n')
            r['array'] = array_name
        fc.write("const embedded_resource embedded_resources[EMBEDDED_RESOURCE_COUNT] = {\n")
        for r in resources:
            fc.write(f'    {{"{r["name"]}", {r["array"]}, {r["size"]}}},\n')
        fc.write("};\n")
    tmp_files.extend([EMBED_HEADER, EMBED_SOURCE])

def roundup(x, align):
    return ((x + align - 1) // align) * align

def make_disk_with_resources(boot_bin, kernel_bin, img_out, resources):
    print("Creating disk image with embedded resources...")
    boot = open(boot_bin, "rb").read()
    if len(boot) != 512:
        print("Error: Bootloader must be exactly 512 bytes!")
        sys.exit(1)
    kern = open(kernel_bin, "rb").read()
    kern_pad = roundup(len(kern), 512)
    kernel_padded = kern + b"\0" * (kern_pad - len(kern))
    kernel_sectors = kern_pad // 512

    res_data = []
    for r in resources:
        data = bytearray(r["data"])
        size = len(data)
        pad = roundup(size, 512)
        if pad > size:
            data.extend(b"\0" * (pad - size))
        res_data.append({"name": r["name"], "data": bytes(data), "size": size})

    import struct
    ENTRY_STRUCT = "<32sII"
    entry_size = struct.calcsize(ENTRY_STRUCT)

    root_entries = []
    root_bytes = 12 + entry_size * len(res_data)
    root_sectors = roundup(root_bytes, 512) // 512

    kernel_start = 1 + root_sectors
    resource_start = kernel_start + kernel_sectors
    cur_lba = resource_start
    for res in res_data:
        res["lba"] = cur_lba
        cur_lba += len(res["data"]) // 512
        nb = res["name"].encode("ascii", errors="ignore")[:31]
        nb += b"\0" * (32 - len(nb))
        root_entries.append(struct.pack(ENTRY_STRUCT, nb, res["lba"], res["size"]))

    root = struct.pack("<III", len(res_data), root_sectors, kernel_sectors) + b"".join(root_entries)
    root += b"\0" * (root_sectors*512 - len(root))

    total = 512 + root_sectors*512 + len(kernel_padded) + sum(len(r["data"]) for r in res_data)
    min_size = 1474560  # 1.44MB
    img_size = roundup(total, 512)
    if img_size < min_size:
        img_size = min_size

    with open(img_out, "wb") as img:
        img.write(boot)
        img.write(root)
        img.write(kernel_padded)
        for res in res_data:
            img.write(res["data"])
        img.write(b"\0" * (img_size - (512 + len(root) + len(kernel_padded) + sum(len(r["data"]) for r in res_data))))

    print(f"Disk image ({img_size // 1024} KB) created with {len(res_data)} resource(s).")
    tmp_files.append(img_out)
    return res_data

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

# === RESOURCE EMBEDDING ===
# Files within the resources directory are copied directly onto the disk image
# and are no longer converted into C source.
RESOURCE_DIR = os.path.join(KERNEL_PROJECT_ROOT, "resources")

def objcopy_binary(input_path, output_obj):
    if not os.path.exists(input_path):
        print(f"ERROR: Resource not found: {input_path}")
        sys.exit(1)
    # Only rebuild if changed
    if not os.path.exists(output_obj) or os.path.getmtime(output_obj) < os.path.getmtime(input_path):
        print(f"Embedding resource: {input_path} -> {output_obj}")
        result = subprocess.run([
            "objcopy", "-I", "binary", "-O", "elf32-i386", "-B", "i386",
            input_path, output_obj
        ], capture_output=True, text=True)
        if result.returncode != 0:
            print("objcopy failed:", result.stdout, result.stderr)
            sys.exit(1)
        tmp_files.append(output_obj)
    else:
        print(f"Resource already up to date: {output_obj}")


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

    boot_bin = "bootloader.bin"
    assemble(bootloader_src, boot_bin, fmt="bin")

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


    # Resources are stored on the disk image only; they are no longer copied
    # directly onto the ISO file system.

    # Place disk image at ISO root
    if os.path.exists(DISK_IMG):
        shutil.copy(DISK_IMG, os.path.join(tmp_iso_dir, "ssd.img"))


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
        "-b", "ssd.img",
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
    ensure_verification_file()
    resources = collect_resources()
    # Resources are stored on the disk image only. The kernel no longer embeds
    # the raw file data, so generate an empty resource table for compilation.
    generate_embedded_sources([])

    asm_files, c_files, h_files = collect_source_files(KERNEL_PROJECT_ROOT)
    c_files = [f for f in c_files if not f.endswith('scheduler.c')]
    # Ensure disk driver source is present
    if any(f.endswith('disk.c') for f in c_files):
        print('Disk driver source detected')
    else:
        print('Warning: disk.c not found, disk driver missing')
    # Resource files are copied directly to the disk image
    c_files = list(dict.fromkeys(c_files))
    print(f"Found {len(asm_files)} asm, {len(c_files)} c, {len(h_files)} h files.")
    boot_bin, kernel_bin = build_kernel(asm_files, c_files, out_bin=KERNEL_BIN)
    make_disk_with_resources(boot_bin, kernel_bin, DISK_IMG, resources)
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
