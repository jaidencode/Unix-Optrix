ENTRY(kernel_entry)
SECTIONS
{
  . = 0x1000;
  .text : { *(.text*) }
  .rodata : { *(.rodata*) }
  .data : { *(.data*) }
  .bss : { *(.bss*) }
  end = .;
}
