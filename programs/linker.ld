ENTRY(main)

SECTIONS {
	. = 0x400401000;

	.text BLOCK(4096) : ALIGN(4096)
	{
		*(.text)
	}

	. = 0x400601000;

	.data BLOCK(4096) : ALIGN(4096)
	{
		*(.data)
	}

	.bss BLOCK(4096) : ALIGN(4096)
	{
		*(.bss)
	}
}