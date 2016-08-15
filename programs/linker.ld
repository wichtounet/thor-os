ENTRY(_start)

SECTIONS {
	. = 0x8000400000;

	.text BLOCK(4096) : ALIGN(4096)
	{
        *(.start)
		*(.text)
		*(.text*)
	}

	. = 0x8000600000;

	.data BLOCK(4096) : ALIGN(4096)
	{
		*(.data)
	}

	.rodata BLOCK(4096) : ALIGN(4096)
	{
		*(.rodata)
		*(.rodata*)
	}

	.bss BLOCK(4096) : ALIGN(4096)
	{
		*(.bss)
	}
}
