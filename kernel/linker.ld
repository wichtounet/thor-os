ENTRY(_Z11kernel_mainv)

SECTIONS
{
    /* This is where the bootloader will load the code */
    . = 0x100000;

    /* 4K of code */
    .text : ALIGN(0x1000)
    {
        *(.start)
        *(.text)
        *(.text.*)
    }

    /* 4K of Read-only data */
    .rodata : ALIGN(0x1000)
    {
        *(.rodata)
    }

    /* 4K of Read-write initialized data. */
    .data : ALIGN(0x1000)
    {
        *(.data)
    }

    /* 4K of Read-write uninitialized data. */
    .bss : ALIGN(0x1000)
    {
        *(.bss)
    }
}
