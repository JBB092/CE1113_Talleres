.section .text.start
.globl _start

_start:
    /* Configurar el stack pointer */
    la sp, _stack_end

    /* Limpiar la sección BSS */
    la t0, _bss_start
    la t1, _bss_end
bss_loop:
    bge t0, t1, bss_done
    sw zero, 0(t0)
    addi t0, t0, 4
    jal zero, bss_loop

bss_done:
    /* Copiar sección .data desde ROM a RAM */
    la t0, _data_start
    la t1, _data_end
    la t2, _text_end

data_loop:
    bge t0, t1, data_done
    lw t3, 0(t2)
    sw t3, 0(t0)
    addi t0, t0, 4
    addi t2, t2, 4
    jal zero, data_loop

data_done:
    /* Saltar a main en C */
    jal ra, main

    /* Si main retorna, entrar en un loop infinito */
    jal zero, .

.section .data
.align 4

.section .bss
.align 4
