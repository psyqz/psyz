OUTPUT_FORMAT("elf32-tradlittlemips", "elf32-tradbigmips", "elf32-tradlittlemips")
OUTPUT_ARCH(mips)
SECTIONS
{
  PROVIDE (__executable_start = SEGMENT_START("text-segment", 0x80000000));
  . = SEGMENT_START("text-segment", 0x80000000) + SIZEOF_HEADERS;

  . = ALIGN (0x100);
  .text           : { *(.text .stub .text.* .gnu.linkonce.t.*) }

  . = ALIGN (0x100);
  .rodata         : { *(.rodata) }

  . = ALIGN (0x100);
  .data           : { *(.data .data.* .gnu.linkonce.d.*) }
  
  . = ALIGN (0x100);
  .sbss           : { *(.sbss .sbss.* .gnu.linkonce.sb.*) }
  .bss            :
  {
   *(.bss .bss.* .gnu.linkonce.b.*)
   *(COMMON)
   /* Align here to ensure that the .bss section occupies space up to
      _end.  Align after .bss to ensure correct alignment even if the
      .bss section disappears because there are no input sections. */
   . = ALIGN(. != 0 ? 32 / 8 : 1);
  }
  /* Stabs debugging sections.  */
  .comment       0 : { *(.comment) }
  /DISCARD/ : { *(.note.GNU-stack) *(.gnu_debuglink) *(.gnu.lto_*) }
}
