ENTRY(_start)

MEMORY
{
  RAM (rwx) : ORIGIN = 0x40000000, LENGTH = 256M
}

SECTIONS
{
  .text :
  {
    . = ALIGN(2048);        /* Force 2 KB alignment for the vector table */
    _vectors_start = .;
    *(.vectors)             /* Put your .vectors section here */
    _vectors_end = .;

    _stext = .;
    *(.text*)               /* All other code follows */
    _etext = .;
  } > RAM

  .data :
  {
    _sdata = .;
    *(.data*)
    _edata = .;
  } > RAM

  .bss :
  {
    *(.bss*)
  } > RAM
  . = ALIGN(4096);
  _heap_start = .;
  _heap_max = 0x40000000 + 0x10000000;
}
