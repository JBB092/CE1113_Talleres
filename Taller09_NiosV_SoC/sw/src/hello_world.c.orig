/*
 * Taller 09 - Nios V/m "Hola mundo" sobre el SoC base.
 *
 * Punto de partida del taller. Imprime por la JTAG UART (STDOUT del BSP)
 * y sirve para verificar que el procesador arranca y ejecuta codigo C en
 * simulacion RTL (Questa), sin tarjeta fisica.
 *
 * El trabajo evaluado consiste en AGREGAR, mas abajo, el acceso a dos PIO
 * (uno de entrada y uno de salida) usando punteros directos a sus
 * direcciones (ver system.h del BSP generado), SIN usar las funciones del
 * driver HAL (alt_*) ni las macros IOWR/IORD.
 */

#include <stdio.h>
#include <stdint.h>
#include "system.h"   /* generado por el BSP: define las bases (XXX_BASE) */

int main(void)
{
    for (int i = 0; i < 5; ++i) {
        printf("Hola mundo, aqui el Nios V/m reportandose %d...\n", i);
    }

    /*
     * TODO (evaluado): acceso a los PIO por puntero directo.
     *
     * Tras agregar los PIO en Platform Designer y regenerar el BSP, el
     * archivo system.h del BSP definira sus bases (por ejemplo
     * PIO_OUT_BASE y PIO_IN_BASE). El acceso debe hacerse con punteros
     * volatiles a esas direcciones, por ejemplo:
     *
     *   volatile uint32_t *pio_out = (volatile uint32_t *)PIO_OUT_BASE;
     *   volatile uint32_t *pio_in  = (volatile uint32_t *)PIO_IN_BASE;
     *   *pio_out = 0xA5;                 // escribir el registro de datos
     *   uint32_t v = *pio_in;            // leer el registro de datos
     *   printf("PIO_IN = 0x%08lx\n", (unsigned long)v);
     *
     * No se permite usar IOWR_ALTERA_AVALON_PIO_DATA / IORD_* ni funciones
     * alt_* para este acceso: el objetivo es practicar E/S mapeada a
     * memoria "a mano".
     */

    printf("Fin del programa.\n");
    return 0;
}
