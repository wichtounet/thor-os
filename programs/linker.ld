SECTIONS {
	. = 0x400000000;

	.text BLOCK(4096) : ALIGN(4096)
	{
		*(.text)
	}

    . = 0x400400000;

	.data BLOCK(4096) : ALIGN(4096)
	{
		*(.data)
	}

	.bss BLOCK(4096) : ALIGN(4096)
	{
		*(.bss)
	}
}