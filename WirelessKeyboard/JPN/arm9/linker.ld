OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)

ENTRY(_start)

SECTIONS
{
  . = __entry;
  .text.start : { *(.text.start ) }
  .text       : { *(.text*) }
  .rodata     : { *(.rodata*) }
  .data       : { *(.data*) }
  __bss_start = .;
  .bss        : { *(.bss*) }
  _end = .;
}
