#ifndef CLOCK_CONFIG_H
#define CLOCK_CONFIG_H

#define CLOCK_INIT_HH  9
#define CLOCK_INIT_MM  30
#define CLOCK_INIT_SS  0

/* 1: ejecutar los tres perfiles antes del reloj.
 * 0: iniciar directamente las pruebas del reloj y el reloj.
 */
#define APP_RUN_PROFILES  1

#if (CLOCK_INIT_HH < 0) || (CLOCK_INIT_HH > 23)
#error "CLOCK_INIT_HH debe estar entre 0 y 23"
#endif

#if (CLOCK_INIT_MM < 0) || (CLOCK_INIT_MM > 59)
#error "CLOCK_INIT_MM debe estar entre 0 y 59"
#endif

#if (CLOCK_INIT_SS < 0) || (CLOCK_INIT_SS > 59)
#error "CLOCK_INIT_SS debe estar entre 0 y 59"
#endif

#if (APP_RUN_PROFILES != 0) && (APP_RUN_PROFILES != 1)
#error "APP_RUN_PROFILES debe ser 0 o 1"
#endif

#endif