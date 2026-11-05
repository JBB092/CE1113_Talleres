#include "devices.h"

/* Acceso MMIO genérico */
static inline void mmio_write32(uintptr_t addr, uint32_t v) {
    *(volatile uint32_t *)addr = v;
}
static inline uint32_t mmio_read32(uintptr_t addr) {
    return *(volatile uint32_t *)addr;
}

/* ============================================================================
 * UART
 * ========================================================================= */
void uart_putchar(char c) { mmio_write32(UART_BASE, (uint32_t)c); }

void uart_puts(const char *s) { while (*s) uart_putchar(*s++); }

void uart_puthex(uint32_t val) {
    const char *h = "0123456789abcdef";
    for (int i = 28; i >= 0; i -= 4)
        uart_putchar(h[(val >> i) & 0xf]);
}

void uart_putdec(uint32_t val) {
    if (val == 0) { uart_putchar('0'); return; }
    char buf[10]; int i = 0;
    while (val > 0) { buf[i++] = '0' + (val % 10); val /= 10; }
    while (i > 0) uart_putchar(buf[--i]);
}

/* ============================================================================
 * Timer
 * ========================================================================= */
uint32_t timer_read_counter(void) { return mmio_read32(TIMER_CNT_ADDR); }

void timer_start_periodic(uint32_t reload, uint32_t prescaler) {
    mmio_write32(TIMER_CTRL_ADDR, 0);
    mmio_write32(TIMER_PRESCALER_ADDR, prescaler);
    mmio_write32(TIMER_RELOAD_ADDR, reload);
    mmio_write32(TIMER_CTRL_ADDR, TIMER_CTRL_ENABLE | TIMER_CTRL_AUTO_RELOAD);
}

void timer_start_free_run(uint32_t prescaler) {
    mmio_write32(TIMER_CTRL_ADDR, 0);
    mmio_write32(TIMER_PRESCALER_ADDR, prescaler);
    mmio_write32(TIMER_RELOAD_ADDR, 0);
    mmio_write32(TIMER_CTRL_ADDR, TIMER_CTRL_ENABLE);
}

void timer_stop(void)  { mmio_write32(TIMER_CTRL_ADDR, 0); }
void timer_reset(void) { mmio_write32(TIMER_CTRL_ADDR, TIMER_CTRL_RESET); }
uint32_t timer_get_status(void)  { return mmio_read32(TIMER_STATUS_ADDR); }
void timer_ack_expired(void) {
    mmio_write32(TIMER_IRQ_ACK_ADDR, TIMER_STATUS_EXPIRED);
}

/* ============================================================================
 * Acelerador de matrices — driver MMIO
 * ========================================================================= */
void matmul_load_a(const uint8_t a[MAT_N][MAT_N]) {
    uint32_t i, j;
    for (i = 0; i < MAT_N; i++)
        for (j = 0; j < MAT_N; j++)
            mmio_write32(MATMUL_REG_A(i, j), (uint32_t)a[i][j]);
}

void matmul_load_b(const uint8_t b[MAT_N][MAT_N]) {
    uint32_t i, j;
    for (i = 0; i < MAT_N; i++)
        for (j = 0; j < MAT_N; j++)
            mmio_write32(MATMUL_REG_B(i, j), (uint32_t)b[i][j]);
}

void matmul_start(void) {
    mmio_write32(MATMUL_REG_CONTROL, MATMUL_CTRL_START);
}

uint32_t matmul_wait_done(void) {
    uint32_t status;
    do {
        status = mmio_read32(MATMUL_REG_STATUS);
    } while (!(status & MATMUL_STATUS_DONE));
    return status;
}

void matmul_read_c(uint32_t c[MAT_N][MAT_N]) {
    uint32_t i, j;
    for (i = 0; i < MAT_N; i++)
        for (j = 0; j < MAT_N; j++)
            c[i][j] = mmio_read32(MATMUL_REG_C(i, j));
}

/* ============================================================================
 * Referencia software — cómputo equivalente en firmware (golden reference)
 * ========================================================================= */
void matmul_sw(const uint8_t a[MAT_N][MAT_N], const uint8_t b[MAT_N][MAT_N],
               uint32_t c[MAT_N][MAT_N]) {
    uint32_t i, j, k;
    for (i = 0; i < MAT_N; i++) {
        for (j = 0; j < MAT_N; j++) {
            c[i][j] = 0;
            for (k = 0; k < MAT_N; k++)
                c[i][j] += (uint32_t)a[i][k] * (uint32_t)b[k][j];
        }
    }
}
