#include "devices.h"

/*
 * Matrices de prueba fijas.
 * A es la identidad escaleada (diagonal = 2); B tiene valores incremetales.
 * El resultado esperado es simplemente 2*B (fácil de verificar a mano).
 *
 * A:                B:
 *  2 0 0 0          1  2  3  4
 *  0 2 0 0          5  6  7  8
 *  0 0 2 0          9 10 11 12
 *  0 0 0 2         13 14 15 16
 *
 * C = A*B:
 *  2  4  6  8
 * 10 12 14 16
 * 18 20 22 24
 * 26 28 30 32
 */
static const uint8_t TEST_A[MAT_N][MAT_N] = {
    {2, 0, 0, 0},
    {0, 2, 0, 0},
    {0, 0, 2, 0},
    {0, 0, 0, 2},
};

static const uint8_t TEST_B[MAT_N][MAT_N] = {
    { 1,  2,  3,  4},
    { 5,  6,  7,  8},
    { 9, 10, 11, 12},
    {13, 14, 15, 16},
};

static const uint8_t TEST2_A[MAT_N][MAT_N] = {
    {3, 1, 4, 1},
    {5, 9, 2, 6},
    {5, 3, 5, 8},
    {9, 7, 9, 3},
};

static const uint8_t TEST2_B[MAT_N][MAT_N] = {
    {2, 7, 1, 8},
    {2, 8, 1, 8},
    {2, 8, 4, 5},
    {9, 0, 4, 5},
};

static void print_matrix(const char *name, uint32_t m[MAT_N][MAT_N]) {
    uint32_t i, j;
    uart_puts(name);
    uart_puts(":\n");
    for (i = 0; i < MAT_N; i++) {
        for (j = 0; j < MAT_N; j++) {
            uart_puts("  ");
            uart_putdec(m[i][j]);
        }
        uart_puts("\n");
    }
}

static void run_test(const uint8_t a[MAT_N][MAT_N], const uint8_t b[MAT_N][MAT_N]) {
    uint32_t hw_c[MAT_N][MAT_N];
    uint32_t sw_c[MAT_N][MAT_N];
    uint32_t i, j, errors;
    uint32_t t0, t1;

    /* --- Cómputo hardware --- */
    uart_puts("[HW] Cargando matrices en el acelerador...\n");
    matmul_load_a(a);
    matmul_load_b(b);

    uart_puts("[HW] Disparando cómputo (START)...\n");
    t0 = timer_read_counter();
    matmul_start();

    uart_puts("[HW] Esperando DONE...\n");
    matmul_wait_done();
    t1 = timer_read_counter();

    uart_puts("[HW] Tiempo de espera START->DONE: ");
    uart_putdec(t1 - t0);
    uart_puts(" us\n");

    uart_puts("[HW] Leyendo resultado C...\n");
    matmul_read_c(hw_c);
    print_matrix("[HW] C (hardware)", hw_c);

    /* --- Cómputo software (referencia) --- */
    uart_puts("\n[SW] Calculando referencia en firmware...\n");
    matmul_sw(a, b, sw_c);
    print_matrix("[SW] C (software)", sw_c);

    /* --- Verificación --- */
    uart_puts("\n[TEST] Comparando resultados...\n");
    errors = 0;
    for (i = 0; i < MAT_N; i++) {
        for (j = 0; j < MAT_N; j++) {
            if (hw_c[i][j] != sw_c[i][j]) {
                uart_puts("  MISMATCH en C[");
                uart_putdec(i);
                uart_puts("][");
                uart_putdec(j);
                uart_puts("]: HW=");
                uart_putdec(hw_c[i][j]);
                uart_puts(" SW=");
                uart_putdec(sw_c[i][j]);
                uart_puts("\n");
                errors++;
            }
        }
    }

    if (errors == 0) {
        uart_puts("[TEST] PASS -- todos los elementos coinciden.\n");
    } else {
        uart_puts("[TEST] FAIL — ");
        uart_putdec(errors);
        uart_puts(" elemento(s) incorrecto(s).\n");
        uart_puts("       Verifique su implementacion de matmul_compute().\n");
    }
}

void app_main(void) {
    uart_puts("=== Taller 07: Acelerador de Matrices 4x4 ===\n\n");

    uart_puts("--- Caso de prueba 1 ---\n");
    run_test(TEST_A, TEST_B);

    uart_puts("\n--- Caso de prueba 2 ---\n");
    run_test(TEST2_A, TEST2_B);

    uart_puts("\n=== Fin ===\n");
}