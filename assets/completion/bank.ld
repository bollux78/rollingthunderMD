SECTIONS {
 . = 0x100000;
 .rodata : { *(.rodata*) }
 .text : { *(.text*) }
 .data : { *(.data*) }
 .bss : { *(.bss*) *(COMMON) }
 /DISCARD/ : { *(.comment) *(.note*) *(.eh_frame*) }
 ASSERT(SIZEOF(.text)==0, "Asset bank contains executable code")
 ASSERT(SIZEOF(.data)==0 && SIZEOF(.bss)==0, "Asset bank is not read-only")
 ASSERT(SIZEOF(.rodata)<=0x80000, "Asset bank exceeds one mapper page")
}
