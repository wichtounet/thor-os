ENTRY(main)

SECTIONS {
	. = 0x8000401000;

	.text BLOCK(4096) : ALIGN(4096)
	{
		*(.text)
	}

	. = 0x8000601000;

	.data BLOCK(4096) : ALIGN(4096)
	{
		*(.data)
	}

	.bss BLOCK(4096) : ALIGN(4096)
	{
		*(.bss)
	}
}