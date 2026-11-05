/* Punto de entrada principal del firmware
 * 
 * Esta función es llamada por start.S después de inicializar
 * memoria, stack y BSS. Su única responsabilidad es inicializar
 * subsistemas y llamar a la aplicación principal.
 */

#include "devices.h"

int main(void) {
    /* Inicializar dispositivos usando direcciones de devices.h */
    timer_reset();
    timer_start_free_run(0u); /* contador libre, 1 tick = 1 us */
    
    /* Llamar a la aplicación principal */
    app_main();
    
    /* Nunca debería llegar aquí */
    while (1) {
        __asm__ volatile("wfi");  /* Wait for interrupt */
    }
    
    return 0;
}
