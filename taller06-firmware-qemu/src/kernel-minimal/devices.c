#include "devices.h"

/* Definiciones de memoria mapeada de dispositivos (vienen de devices.h) */
#define UART_REG_TX   (UART_BASE + 0x00)
#define UART_REG_RX   (UART_BASE + 0x04)
#define UART_REG_CTRL (UART_BASE + 0x08)
#define UART_REG_STATUS (UART_BASE + 0x0C)

#define TIMER_REG_CTRL      (TIMER_BASE + 0x00)
#define TIMER_REG_PRESCALER (TIMER_BASE + 0x04)
#define TIMER_REG_RELOAD    (TIMER_BASE + 0x08)
#define TIMER_REG_CNT       (TIMER_BASE + 0x0C)
#define TIMER_REG_STATUS    (TIMER_BASE + 0x10)
#define TIMER_REG_IRQ_ACK   (TIMER_BASE + 0x14)

/* Funciones auxiliares para acceso a memoria */
static inline void mmio_write32(uintptr_t addr, uint32_t value) {
    *(volatile uint32_t *)addr = value;
}

static inline uint32_t mmio_read32(uintptr_t addr) {
    return *(volatile uint32_t *)addr;
}

/* ============================================================================
 * UART Functions
 * ============================================================================ */

void uart_putchar(char c) {
    mmio_write32(UART_REG_TX, (uint32_t)c);
}

void uart_puts(const char *str) {
    while (*str) {
        uart_putchar(*str);
        str++;
    }
}

void uart_puthex(uint32_t val) {
    const char *hex = "0123456789abcdef";
    for (int i = 28; i >= 0; i -= 4) {
        uart_putchar(hex[(val >> i) & 0xf]);
    }
}

void uart_putdec(uint32_t val) {
    if (val == 0) {
        uart_putchar('0');
        return;
    }
    
    /* Buffer para almacenar dígitos en orden inverso */
    char buf[10];
    int i = 0;
    
    while (val > 0) {
        buf[i++] = '0' + (val % 10);
        val /= 10;
    }
    
    /* Imprimir dígitos en orden correcto */
    while (i > 0) {
        uart_putchar(buf[--i]);
    }
}

/* ============================================================================
 * Timer Functions
 * ============================================================================ */

uint32_t timer_read_counter(void) {
    return mmio_read32(TIMER_REG_CNT);
}

void timer_set_reload(uint32_t value) {
    mmio_write32(TIMER_REG_RELOAD, value);
}

uint32_t timer_get_reload(void) {
    return mmio_read32(TIMER_REG_RELOAD);
}

void timer_set_prescaler(uint32_t value) {
    mmio_write32(TIMER_REG_PRESCALER, value);
}

uint32_t timer_get_status(void) {
    return mmio_read32(TIMER_REG_STATUS);
}

void timer_ack_expired(void) {
    mmio_write32(TIMER_REG_IRQ_ACK, TIMER_STATUS_EXPIRED);
}

void timer_start_periodic(uint32_t reload_value, uint32_t prescaler) {
    mmio_write32(TIMER_REG_CTRL, 0);
    timer_set_prescaler(prescaler);
    timer_set_reload(reload_value);
    mmio_write32(TIMER_REG_CTRL, TIMER_CTRL_ENABLE | TIMER_CTRL_AUTO_RELOAD);
}

void timer_start_free_run(uint32_t prescaler) {
    mmio_write32(TIMER_REG_CTRL, 0);
    timer_set_prescaler(prescaler);
    timer_set_reload(0);
    mmio_write32(TIMER_REG_CTRL, TIMER_CTRL_ENABLE);
}

void timer_stop(void) {
    mmio_write32(TIMER_REG_CTRL, 0);
}

void timer_reset(void) {
    mmio_write32(TIMER_REG_CTRL, TIMER_CTRL_RESET);
}

/* ============================================================================
 * Delay Functions
 * ============================================================================ */

void delay_us(uint32_t us) {
    uint32_t start = timer_read_counter();
    /* Asumir timer cuenta en microsegundos (1 tick/us) */
    while ((timer_read_counter() - start) < us) {
        /* Busy wait */
    }
}

void delay_ms(uint32_t ms) {
    delay_us(ms * 1000);
}
