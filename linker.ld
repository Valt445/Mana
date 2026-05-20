ENTRY(_start)

MEMORY
{
  RAM (rwx) : ORIGIN = 0x40000000, LENGTH = 256M
}

SECTIONS
{
  .text : 
  {
    _stext = .;
    *(.text*)
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
  _heap_start = .;
  _heap_max = 0x40000000 + 0x10000000;
}
