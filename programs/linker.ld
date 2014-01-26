ENTRY(main)

SECTIONS {
	. = 0x8000401000;

	.text BLOCK(4096) : ALIGN(4096)
	{
		*(.text)
		*(.text*)
	}

	. = 0x8000602000;

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