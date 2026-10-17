OUTPUT_ARCH(m68k)
ENTRY(rtStageBankImage)
SECTIONS {
 .stage 0x100000 : {
  KEEP(*(.stage.header)) *(.text .text.*) *(.rodata .rodata.*)
  . = ALIGN(4); rtStageBankEnd = .;
 }
 .forbidden : { *(.data .data.* .bss .bss.* COMMON) }
 /DISCARD/ : { *(.comment .note* .eh_frame*) }
 ASSERT(SIZEOF(.forbidden)==0,"Stage banks cannot own mutable storage")
 ASSERT(rtStageBankImage==0x100000,"Stage header address changed")
 ASSERT(rtStageBankEnd<=0x400000,"Stage image exceeds3MiB")
}
