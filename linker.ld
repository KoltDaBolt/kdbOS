ENTRY(loader)

SECTIONS {
    . = 0x00100000;

    .multiboot ALIGN(4) :
    {
        *(.multiboot)
    }

    . = ALIGN(0x1000);
    _text_start = .;
    .text  :
    {
        *(.text*)
    }

    . = ALIGN(0x1000);
    _rodata_start = .;
    .rodata :
    {
        *(.rodata*)
    }

    . = ALIGN(0x1000);
    _data_start = .;
    .data :
    {
        *(.data)
    }

    . = ALIGN(0x1000);
    _bss_start = .;
    .bss :
    {
        *(COMMON)
        *(.bss)
    }

    . = ALIGN(0x1000);
    _kernel_end = .;
}
