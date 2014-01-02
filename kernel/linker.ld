ENTRY(kernel_main)

SECTIONS
{
    /* This is where the bootloader will load the code */
	. = 0x6000;

    .text_16 BLOCK(512) : ALIGN(512)
	{
		boot_16_64.o(.text)
	}

    .rodata_16 BLOCK(512) : ALIGN(512)
    {
		boot_16_64.o(.rodata)
    }

    .bss_16 BLOCK(512) : ALIGN(512)
    {
		boot_16_64.o(.bss)
    }

    .data_16 BLOCK(512) : ALIGN(512)
    {
		boot_16_64.o(.data)
    }

    .text_32 BLOCK(512) : ALIGN(512)
	{
		boot_32_64.o(.text)
	}

    /* 4K of code */
	.text BLOCK(1K) : ALIGN(1K)
	{
		*(.text)
        *(.text.*)
	}

	/* 4K of Read-only data */
	.rodata BLOCK(1K) : ALIGN(1K)
	{
		*(.rodata)
	}

	/* 4K of Read-write initialized data. */
	.data BLOCK(1K) : ALIGN(1K)
	{
		*(.data)
	}

	/* 4K of Read-write uninitialized data. */
	.bss BLOCK(1K) : ALIGN(1K)
	{
		*(.bss)
	}
}