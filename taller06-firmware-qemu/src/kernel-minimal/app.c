#include "devices.h"
#include "clock_config.h"

/* Aplicación: Reloj simple usando Timer
 * 
 * Muestra el tiempo transcurrido desde el inicio.
 * Utiliza el Timer real del hardware (direccion definida en devices.h).
 * Si los bugs estan presentes, el Timer no funcionara correctamente.
 * Cuando los bugs sean solucionados, este contador mostrara valores reales.
 */

/* Extensión del taller:
 * - Parte 1: tres perfiles y medición con referencia independiente.
 * - Parte 2: opción A, TimerProfile_t y timer_apply_profile().
 * - Parte 3: reloj digital configurable.
 * El comentario anterior se conserva de la base del profesor.
 */

#define SAMPLE_COUNT             5u

/* QEMU virt, configuración CLINT estándar:
 * base 0x02000000 + offset mtime 0xBFF8.
 * Solo leemos los 32 bits bajos; no escribimos en este temporizador.
 */
#define PLATFORM_MTIME_LOW_ADDR  0x0200BFF8u
#define PLATFORM_TICKS_PER_US    10u

#define TIMER_TIMEOUT_US         3000000u
#define ONE_SHOT_OBSERVE_US      100000u

#define CLOCK_PRESCALER          999u
#define CLOCK_RELOAD             1000u

typedef struct {
    uint32_t prescaler;
    uint32_t reload;
    uint32_t auto_reload;
} TimerProfile_t;

typedef struct {
    const char *name;
    TimerProfile_t config;
} NamedProfile_t;

typedef struct {
    uint32_t timestamp;
    uint32_t counter;
    uint32_t status_before;
    uint32_t status_after;
    uint32_t elapsed_us;
    uint32_t stopped_ok;
} TimerSample_t;

typedef struct {
    uint32_t hours;
    uint32_t minutes;
    uint32_t seconds;
} ClockTime_t;

static uint32_t platform_ticks(void)
{
    return *(volatile uint32_t *)PLATFORM_MTIME_LOW_ADDR;
}

static uint32_t elapsed_us(uint32_t start, uint32_t end)
{
    /* Restar antes de dividir permite atravesar un desbordamiento.
     * Cada intervalo medido debe ser menor que unos 429 segundos.
     */
    return (end - start) / PLATFORM_TICKS_PER_US;
}

static uint32_t profile_control(const TimerProfile_t *profile)
{
    uint32_t control = TIMER_CTRL_ENABLE;

    if (profile->auto_reload != 0u) {
        control |= TIMER_CTRL_AUTO_RELOAD;
    }

    return control;
}

static uint32_t profile_period_us(const TimerProfile_t *profile)
{
    /* Los perfiles de este taller no desbordan uint32_t. */
    return profile->reload * (profile->prescaler + 1u);
}

static uint32_t timer_read_control(void)
{
    return *(volatile uint32_t *)TIMER_CTRL_ADDR;
}

void timer_apply_profile(const TimerProfile_t *profile)
{
    /* Reiniciar contador y estado antes de aplicar otra configuración. */
    timer_stop();
    timer_reset();
    timer_stop();
    timer_ack_expired();

    timer_set_prescaler(profile->prescaler);
    timer_set_reload(profile->reload);

    *(volatile uint32_t *)TIMER_CTRL_ADDR = profile_control(profile);
}

static uint32_t wait_for_expiration(void)
{
    uint32_t start = platform_ticks();

    while ((timer_get_status() & TIMER_STATUS_EXPIRED) == 0u) {
        if (elapsed_us(start, platform_ticks()) >= TIMER_TIMEOUT_US) {
            return 0u;
        }
    }

    return 1u;
}

static void print_profile_config(const NamedProfile_t *profile)
{
    const TimerProfile_t *config = &profile->config;

    uart_puts("\n=== Perfil ");
    uart_puts(profile->name);
    uart_puts(" ===\n");

    uart_puts("PRESCALER=");
    uart_putdec(config->prescaler);
    uart_puts(" RELOAD=");
    uart_putdec(config->reload);

    uart_puts(" CONTROL=0x");
    uart_puthex(profile_control(config));
    uart_puts(" AUTO_RELOAD=");
    uart_putdec(config->auto_reload);

    uart_puts("\nPeriodo teorico=");
    uart_putdec(profile_period_us(config));
    uart_puts(" us\n");
    uart_puts("COUNTER: ticks del timer educativo.\n");
    uart_puts("T_medido: referencia independiente mtime.\n");
}

static uint32_t capture_expiration(TimerSample_t *sample)
{
    if (wait_for_expiration() == 0u) {
        return 0u;
    }

    sample->timestamp = platform_ticks();

    /* Leer valor actual del Timer */
    sample->counter = timer_read_counter();
    sample->status_before = timer_get_status();

    /* Limpiar la expiración mediante el registro IRQ_ACK. */
    timer_ack_expired();

    sample->status_after = timer_get_status();
    sample->elapsed_us = 0u;
    sample->stopped_ok = 0u;

    return 1u;
}

static uint32_t verify_one_shot_stopped(void)
{
    uint32_t start = platform_ticks();
    uint32_t initial_counter = timer_read_counter();

    /* Observar durante 100 ms, sin detener ni rearmar por software.
     * Es más que los 50 ms configurados para el one-shot.
     */
    do {
        if ((timer_read_control() & TIMER_CTRL_ENABLE) != 0u) {
            return 0u;
        }

        if ((timer_get_status() & TIMER_STATUS_EXPIRED) != 0u) {
            return 0u;
        }

        if (timer_read_counter() != initial_counter) {
            return 0u;
        }
    } while (elapsed_us(start, platform_ticks()) < ONE_SHOT_OBSERVE_US);

    return 1u;
}

static uint32_t collect_periodic(const TimerProfile_t *profile,
                                 TimerSample_t *samples)
{
    timer_apply_profile(profile);

    for (uint32_t index = 0u; index < SAMPLE_COUNT; ++index) {
        if (capture_expiration(&samples[index]) == 0u) {
            timer_stop();
            return 0u;
        }

        if (index != 0u) {
            samples[index].elapsed_us = elapsed_us(
                samples[index - 1u].timestamp,
                samples[index].timestamp);
        }
    }

    timer_stop();
    return 1u;
}

static uint32_t collect_one_shot(const TimerProfile_t *profile,
                                 TimerSample_t *samples)
{
    for (uint32_t index = 0u; index < SAMPLE_COUNT; ++index) {
        timer_apply_profile(profile);

        /* Marca inmediatamente posterior a la activación.
         * La medida incluye latencia de polling y un pequeño desfase
         * entre la escritura de ENABLE y esta lectura.
         */
        uint32_t start = platform_ticks();

        if (capture_expiration(&samples[index]) == 0u) {
            timer_stop();
            return 0u;
        }

        samples[index].elapsed_us = elapsed_us(
            start, samples[index].timestamp);

        /* No llamar a timer_stop() antes de verificar. */
        samples[index].stopped_ok = verify_one_shot_stopped();

        if (samples[index].stopped_ok == 0u) {
            timer_stop();
            return 0u;
        }
    }

    return 1u;
}

static void print_error_us(uint32_t measured, uint32_t theoretical)
{
    uart_puts(" error_us=");

    if (measured >= theoretical) {
        uart_putchar('+');
        uart_putdec(measured - theoretical);
    } else {
        uart_putchar('-');
        uart_putdec(theoretical - measured);
    }
}

static void print_sample(const TimerSample_t *sample,
                         uint32_t index,
                         const TimerProfile_t *profile)
{
    uart_puts("Exp #");
    uart_putdec(index + 1u);
    uart_puts(": COUNTER=");
    uart_putdec(sample->counter);

    uart_puts(" STATUS_antes=0x");
    uart_puthex(sample->status_before);
    uart_puts(" STATUS_despues_ACK=0x");
    uart_puthex(sample->status_after);

    if ((profile->auto_reload != 0u) && (index == 0u)) {
        uart_puts(" T_medido=N/A (primera referencia)");
    } else {
        uart_puts(" T_medido_us=");
        uart_putdec(sample->elapsed_us);
        print_error_us(sample->elapsed_us, profile_period_us(profile));
    }

    if (profile->auto_reload == 0u) {
        uart_puts(" detencion_100ms=");
        uart_puts(sample->stopped_ok != 0u ? "OK" : "FALLO");
    }

    uart_puts("\n");
}

static void print_summary(const TimerProfile_t *profile,
                          const TimerSample_t *samples)
{
    uint32_t first = profile->auto_reload != 0u ? 1u : 0u;
    uint32_t sum = 0u;

    for (uint32_t index = first; index < SAMPLE_COUNT; ++index) {
        sum += samples[index].elapsed_us;
    }

    uint32_t count = SAMPLE_COUNT - first;
    uint32_t average = sum / count;

    uart_puts("RESUMEN: teorico_us=");
    uart_putdec(profile_period_us(profile));
    uart_puts(" promedio_medido_us=");
    uart_putdec(average);
    uart_puts(" mediciones=");
    uart_putdec(count);
    print_error_us(average, profile_period_us(profile));
    uart_puts("\n");
}

static uint32_t run_profile(const NamedProfile_t *profile)
{
    TimerSample_t samples[SAMPLE_COUNT];
    uint32_t success;

    print_profile_config(profile);

    if (profile->config.auto_reload != 0u) {
        success = collect_periodic(&profile->config, samples);
    } else {
        success = collect_one_shot(&profile->config, samples);
    }

    timer_stop();

    if (success == 0u) {
        uart_puts("FALLO: timeout o detencion one-shot incorrecta.\n");
        return 0u;
    }

    /* Imprimir después de capturar para reducir interferencia de UART. */
    for (uint32_t index = 0u; index < SAMPLE_COUNT; ++index) {
        print_sample(&samples[index], index, &profile->config);
    }

    print_summary(&profile->config, samples);
    uart_puts("Fin del perfil ");
    uart_puts(profile->name);
    uart_puts(".\n");

    return 1u;
}

static uint32_t run_all_profiles(void)
{
    const NamedProfile_t profiles[] = {
        {
            .name = "rapido",
            .config = { .prescaler = 0u, .reload = 10000u,
                        .auto_reload = 1u }
        },
        {
            .name = "lento",
            .config = { .prescaler = 999u, .reload = 500u,
                        .auto_reload = 1u }
        },
        {
            .name = "one-shot",
            .config = { .prescaler = 0u, .reload = 50000u,
                        .auto_reload = 0u }
        }
    };

    const uint32_t count = sizeof(profiles) / sizeof(profiles[0]);

    for (uint32_t index = 0u; index < count; ++index) {
        if (run_profile(&profiles[index]) == 0u) {
            return 0u;
        }
    }

    return 1u;
}

static void clock_advance(ClockTime_t *clock)
{
    ++clock->seconds;

    if (clock->seconds == 60u) {
        clock->seconds = 0u;
        ++clock->minutes;
    }

    if (clock->minutes == 60u) {
        clock->minutes = 0u;
        ++clock->hours;
    }

    if (clock->hours == 24u) {
        clock->hours = 0u;
    }
}

static void uart_put_two_digits(uint32_t value)
{
    uart_putchar((char)('0' + value / 10u));
    uart_putchar((char)('0' + value % 10u));
}

static void clock_print(const ClockTime_t *clock)
{
    uart_put_two_digits(clock->hours);
    uart_putchar(':');
    uart_put_two_digits(clock->minutes);
    uart_putchar(':');
    uart_put_two_digits(clock->seconds);
    uart_putchar('\n');
}

static uint32_t clock_test_case(ClockTime_t initial,
                                ClockTime_t expected)
{
    clock_advance(&initial);

    return initial.hours == expected.hours
        && initial.minutes == expected.minutes
        && initial.seconds == expected.seconds;
}

static uint32_t clock_self_test(void)
{
    uint32_t minute_ok = clock_test_case(
        (ClockTime_t){9u, 30u, 59u}, (ClockTime_t){9u, 31u, 0u});

    uint32_t hour_ok = clock_test_case(
        (ClockTime_t){9u, 59u, 59u}, (ClockTime_t){10u, 0u, 0u});

    uint32_t day_ok = clock_test_case(
        (ClockTime_t){23u, 59u, 59u}, (ClockTime_t){0u, 0u, 0u});

    uart_puts("\nPrueba cambio de minuto: ");
    uart_puts(minute_ok != 0u ? "OK\n" : "FALLO\n");
    uart_puts("Prueba cambio de hora: ");
    uart_puts(hour_ok != 0u ? "OK\n" : "FALLO\n");
    uart_puts("Prueba cambio de dia: ");
    uart_puts(day_ok != 0u ? "OK\n" : "FALLO\n");

    return minute_ok && hour_ok && day_ok;
}

static void run_clock(void)
{
    ClockTime_t clock = {
        .hours = CLOCK_INIT_HH,
        .minutes = CLOCK_INIT_MM,
        .seconds = CLOCK_INIT_SS
    };

    const TimerProfile_t profile = {
        .prescaler = CLOCK_PRESCALER,
        .reload = CLOCK_RELOAD,
        .auto_reload = 1u
    };

    uart_puts("\n=== Reloj digital ===\n");
    uart_puts("PRESCALER=999 RELOAD=1000 CONTROL=0x00000003\n");
    uart_puts("Periodo teorico=1000000 us\n");
    clock_print(&clock);

    timer_apply_profile(&profile);

    while (1) {
        /* El reloj avanza por expiraciones del timer educativo. */
        while ((timer_get_status() & TIMER_STATUS_EXPIRED) == 0u) {
            /* Esperar el siguiente segundo. */
        }

        timer_ack_expired();
        clock_advance(&clock);
        clock_print(&clock);
    }
}

void app_main(void) {
    uart_puts("===========================================\n");
    uart_puts("Taller 06: Firmware Timer Application\n");
    uart_puts("===========================================\n\n");
    
    uart_puts("Firmware iniciado correctamente\n");
    uart_puts("Arquitectura: RISC-V 32-bit (RV32I)\n");
    uart_puts("Iniciando demostración de Timer...\n");
    uart_puts("===========================================\n\n");
    
    uart_puts("Hello from bare-metal RISC-V!\n");

    /* Detener el conteo libre iniciado por main(). */
    timer_stop();

    if (APP_RUN_PROFILES != 0) {
        if (run_all_profiles() == 0u) {
            uart_puts("Revisar el fallo antes de continuar.\n");
            return;
        }

        uart_puts("\nSecuencia de perfiles terminada.\n");
        uart_puts("Timer detenido.\n");
    }

    if (clock_self_test() == 0u) {
        uart_puts("FALLO en las pruebas del reloj.\n");
        return;
    }

    run_clock();
}