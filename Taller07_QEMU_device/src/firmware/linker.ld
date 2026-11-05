/* Linker script para firmware RISC-V mínimo */
/* Define la memoria y las secciones del programa */

MEMORY {
    ROM (rx)  : ORIGIN = 0x80000000, LENGTH = 256K
    RAM (rwx) : ORIGIN = 0x80040000, LENGTH = 256K
}

SECTIONS {
    /* Sección de código (Read-Only) */
    .text : {
        KEEP(*(.text.start))
        *(.text .text.*)
        *(.rodata .rodata.*)
    } > ROM

    /* Sección de datos inicializados */
    .data : {
        *(.data .data.*)
    } > RAM AT > ROM

    /* Sección de datos no inicializados (BSS) */
    .bss : {
        *(.bss .bss.*)
        *(COMMON)
    } > RAM

    /* Stack en RAM (crece hacia arriba desde el final) */
    .stack (NOLOAD) : {
        . = ALIGN(16);
        PROVIDE(_stack_start = .);
        . += 8K;
        PROVIDE(_stack_end = .);
    } > RAM

    /* Heap en RAM */
    .heap (NOLOAD) : {
        PROVIDE(_heap_start = .);
        . += 8K;
        PROVIDE(_heap_end = .);
    } > RAM

    /* Símbolos globales */
    PROVIDE(_text_start = ADDR(.text));
    PROVIDE(_text_end = ADDR(.text) + SIZEOF(.text));
    PROVIDE(_data_start = ADDR(.data));
    PROVIDE(_data_end = ADDR(.data) + SIZEOF(.data));
    PROVIDE(_bss_start = ADDR(.bss));
    PROVIDE(_bss_end = ADDR(.bss) + SIZEOF(.bss));
}
