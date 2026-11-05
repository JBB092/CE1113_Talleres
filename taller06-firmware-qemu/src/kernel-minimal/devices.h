/* Definiciones de tipos enteros para bare-metal RISC-V */
typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned short uint16_t;
typedef signed short int16_t;
typedef unsigned int uint32_t;
typedef signed int int32_t;
typedef unsigned long uint64_t;
typedef signed long int64_t;
typedef unsigned long uintptr_t;
typedef signed long intptr_t;

/* Direcciones base de dispositivos */
#define UART_BASE        0x10000000
#define TIMER_BASE       0x10010000

/* Definiciones de dispositivos (offsets) */
#define TIMER_CTRL_ADDR      (TIMER_BASE + 0x00)
#define TIMER_PRESCALER_ADDR (TIMER_BASE + 0x04)
#define TIMER_RELOAD_ADDR    (TIMER_BASE + 0x08)
#define TIMER_CNT_ADDR       (TIMER_BASE + 0x0C)
#define TIMER_STATUS_ADDR    (TIMER_BASE + 0x10)
#define TIMER_IRQ_ACK_ADDR   (TIMER_BASE + 0x14)

#define TIMER_CTRL_ENABLE      (1u << 0)
#define TIMER_CTRL_AUTO_RELOAD (1u << 1)
#define TIMER_CTRL_IRQ_EN      (1u << 2)
#define TIMER_CTRL_RESET       (1u << 3)

#define TIMER_STATUS_EXPIRED   (1u << 0)

/* Prototipos de dispositivos y funciones auxiliares */

/* UART - Comunicación Serial */
void uart_putchar(char c);
void uart_puts(const char *str);
void uart_puthex(uint32_t val);
void uart_putdec(uint32_t val);

/* Timer - Temporización */
uint32_t timer_read_counter(void);
void timer_set_reload(uint32_t value);
uint32_t timer_get_reload(void);
void timer_set_prescaler(uint32_t value);
uint32_t timer_get_status(void);
void timer_ack_expired(void);
void timer_start_periodic(uint32_t reload_value, uint32_t prescaler);
void timer_start_free_run(uint32_t prescaler);
void timer_stop(void);
void timer_reset(void);

/* Utilidades */
void delay_ms(uint32_t ms);
void delay_us(uint32_t us);

/* Aplicación principal */
void app_main(void);
