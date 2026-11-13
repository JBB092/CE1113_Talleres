#include <stdio.h>
#include <stdint.h>
#include "system.h"

int main(void)
{
    volatile uint32_t *pio_out = (volatile uint32_t *)PIO_OUT_BASE;
    volatile uint32_t *pio_in  = (volatile uint32_t *)PIO_IN_BASE;
    const uint32_t valores[] = {0x01, 0xA5, 0xFF};

    for (int i = 0; i < 5; ++i) {
        printf("Hola mundo, aqui el Nios V/m reportandose %d...\n", i);
    }

    for (int i = 0; i < 3; ++i) {
        *pio_out = valores[i];
        printf("PIO_OUT <- 0x%02lx\n", (unsigned long)valores[i]);
    }

    uint32_t v = *pio_in;
    printf("PIO_IN  = 0x%08lx\n", (unsigned long)v);

    printf("Fin del programa.\n");
    return 0;
}
