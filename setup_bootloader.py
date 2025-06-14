import subprocess
import sys
import os
import shutil

# --- CONFIGURATION ---
TOOLCHAIN_DIR = os.environ.get("TOOLCHAIN_DIR") or r"C:\\Users\\jaide\\Downloads\\i686-elf-tools-windows\\bin"
CC = os.environ.get("CC") or os.path.join(TOOLCHAIN_DIR, "i686-elf-gcc.exe")
LD = os.environ.get("LD") or os.path.join(TOOLCHAIN_DIR, "i686-elf-ld.exe")
OBJDUMP = os.environ.get("OBJDUMP") or os.path.join(TOOLCHAIN_DIR, "i686-elf-objdump.exe")

if not os.path.isfile(CC):
    CC = shutil.which("i686-linux-gnu-gcc") or "i686-linux-gnu-gcc"
if not os.path.isfile(LD):
    LD = shutil.which("i686-linux-gnu-ld") or "i686-linux-gnu-ld"
if not os.path.isfile(OBJDUMP):
    OBJDUMP = shutil.which("i686-linux-gnu-objdump") or "i686-linux-gnu-objdump"
NASM = "nasm"

CDRTOOLS_DIR = os.environ.get("CDRTOOLS_DIR") or r"C:\\Program Files (x86)\\cdrtools"
MKISOFS_EXE = os.environ.get("MKISOFS") or os.path.join(CDRTOOLS_DIR, "mkisofs.exe")
if not os.path.isfile(MKISOFS_EXE):
    MKISOFS_EXE = shutil.which("mkisofs") or "mkisofs"

asm_files = [
    "bootloader.asm",    # boot sector (must remain first, used for bin)
    "kernel.asm",        # kernel entry
    "idt.asm"
]

c_files = [
    "kernel_main.c",
    "vga.c",
    "graphics.c",
    "pmm.c",
    "vmm.c",
    "idt.c",
    "hardware.c",
    "keyboard.c",
    "mouse.c",
    "fabric.c",
    "font8x8.c"
]

tmp_files = []

def check_file(f):
    if not os.path.isfile(f):
        print(f"Error: Required file not found: {f}")
        sys.exit(1)

def run(cmd, **kwargs):
    result = subprocess.run(cmd, capture_output=True, text=True, **kwargs)
    if result.returncode != 0:
        print("Error running:", ' '.join(cmd))
        print(result.stdout)
        print(result.stderr)
        sys.exit(1)
    return result

def assemble(src, out, fmt="bin"):
    print(f"Assembling {src} -> {out} ...")
    fmt_flag = "-f" + fmt
    result = subprocess.run([NASM, fmt_flag, src, "-o", out], capture_output=True, text=True)
    if result.returncode != 0:
        print(result.stdout)
        print(result.stderr)
        sys.exit(1)
    print("OK.")
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

def make_iso(hd_img, iso_out):
    print(f"Creating ISO using: {MKISOFS_EXE}")
    if not os.path.isfile(MKISOFS_EXE):
        print(f"Error: mkisofs.exe not found at {MKISOFS_EXE}!")
        sys.exit(1)
    tmp_dir = "_iso_tmp"
    os.makedirs(tmp_dir, exist_ok=True)
    shutil.copy(hd_img, os.path.join(tmp_dir, hd_img))
    cmd = [
        MKISOFS_EXE,
        "-quiet",
        "-o", iso_out,
        "-b", hd_img,
        tmp_dir
    ]
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        print(result.stdout)
        print(result.stderr)
        shutil.rmtree(tmp_dir)
        sys.exit(1)
    shutil.rmtree(tmp_dir)
    print(f"ISO created: {iso_out}")
    tmp_files.append(iso_out)

def cleanup():
    for f in tmp_files:
        try:
            if os.path.exists(f):
                os.remove(f)
        except Exception as e:
            print(f"Warning: Could not delete {f}: {e}")

def build_kernel(asm_files, c_files, out_bin):
    obj_files = []
    seen_objs = set()

    # Assemble all .asm files except the bootloader (use as kernel objects)
    for idx, asm in enumerate(asm_files):
        check_file(asm)
        base = os.path.splitext(os.path.basename(asm))[0]
        if idx == 0:
            boot_bin = base + ".bin"
            assemble(asm, boot_bin, fmt="bin")
            boot_bin_path = boot_bin
        else:
            obj = f"{base}_stubs.o"
            if obj not in seen_objs:
                print(f"Assembling ASM: {asm} -> {obj}")
                run([NASM, "-felf32", asm, "-o", obj])
                obj_files.append(obj)
                tmp_files.append(obj)
                seen_objs.add(obj)

    # Compile all .c files to .o
    for c in c_files:
        check_file(c)
        base = os.path.splitext(os.path.basename(c))[0]
        c_obj = f"{base}.o"
        if c_obj not in seen_objs:
            print(f"Compiling C source: {c} -> {c_obj}")
            run([CC, "-ffreestanding", "-fno-pie", "-fno-pic", "-m32", "-c", c, "-o", c_obj])
            obj_files.append(c_obj)
            tmp_files.append(c_obj)
            seen_objs.add(c_obj)

    # Link to ELF for debugging (add -e start)
    kernel_bin = "kernel.bin"
    elf_out = kernel_bin.replace(".bin", ".elf")
    print(f"Linking kernel ELF: {elf_out}")
    link_cmd_elf = [LD, "-Ttext", "0x1000", "-m", "elf_i386", "-e", "start", "-nostdlib"] + obj_files + ["-o", elf_out]
    run(link_cmd_elf)
    tmp_files.append(elf_out)

    # Dump debug info from ELF
    print("\n=== KERNEL SYMBOLS (ELF) ===")
    run([OBJDUMP, "-t", elf_out])

    print("\n=== KERNEL SECTIONS (ELF) ===")
    run([OBJDUMP, "-h", elf_out])

    # Convert ELF to raw BIN for booting (add -e start)
    print(f"Converting ELF -> BIN: {elf_out} -> {kernel_bin}")
    link_cmd_bin = [LD, "-Ttext", "0x1000", "-m", "elf_i386", "-e", "start", "--oformat", "binary", "-nostdlib", "-no-pie"] + obj_files + ["-o", kernel_bin]
    run(link_cmd_bin)
    tmp_files.append(kernel_bin)

    return boot_bin_path, kernel_bin

def main():
    iso_out    = "OptrixOS.iso"
    hd_img     = "disk.img"

    # Checks
    for asm in asm_files:
        check_file(asm)
    for c in c_files:
        check_file(c)

    # Build all ASM and C files, get bootloader bin and kernel bin path
    boot_bin, kernel_bin = build_kernel(asm_files, c_files, out_bin="kernel.bin")

    # Build floppy image and ISO
    make_dynamic_img(boot_bin, kernel_bin, hd_img)
    make_iso(hd_img, iso_out)

    # Done, cleanup everything except ISO
    files_to_keep = {iso_out}
    print("\nCleaning up temporary build files...")
    for f in tmp_files:
        if f not in files_to_keep and os.path.exists(f):
            try:
                os.remove(f)
            except Exception as e:
                print(f"Warning: Could not delete {f}: {e}")

    print(f"\nBuild complete! Bootable ISO is '{iso_out}'")

if __name__ == "__main__":
    main()
