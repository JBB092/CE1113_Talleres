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
#define UART_BASE        0x10000000u
#define TIMER_BASE       0x10010000u
#define MATMUL_BASE      0x10011000u

/* -------------------------------------------------------------------------
 * Timer (riscv-edu-timer) - Taller 06
 * ---------------------------------------------------------------------- */
#define TIMER_CTRL_ADDR      (TIMER_BASE + 0x00u)
#define TIMER_PRESCALER_ADDR (TIMER_BASE + 0x04u)
#define TIMER_RELOAD_ADDR    (TIMER_BASE + 0x08u)
#define TIMER_CNT_ADDR       (TIMER_BASE + 0x0Cu)
#define TIMER_STATUS_ADDR    (TIMER_BASE + 0x10u)
#define TIMER_IRQ_ACK_ADDR   (TIMER_BASE + 0x14u)

#define TIMER_CTRL_ENABLE      (1u << 0)
#define TIMER_CTRL_AUTO_RELOAD (1u << 1)
#define TIMER_CTRL_IRQ_EN      (1u << 2)
#define TIMER_CTRL_RESET       (1u << 3)
#define TIMER_STATUS_EXPIRED   (1u << 0)

/* -------------------------------------------------------------------------
 * Acelerador de multiplicación de matrices 4×4 (riscv-edu-matmul)
 *
 * Elemento [i][j] de la matriz A → offset REG_A_BASE + (i*4+j)*4
 * Elemento [i][j] de la matriz B → offset REG_B_BASE + (i*4+j)*4
 * Elemento [i][j] del resultado C → offset REG_C_BASE + (i*4+j)*4
 * ---------------------------------------------------------------------- */
#define MATMUL_REG_A(i,j)   (MATMUL_BASE + 0x00u + ((i)*4u+(j))*4u)
#define MATMUL_REG_B(i,j)   (MATMUL_BASE + 0x40u + ((i)*4u+(j))*4u)
#define MATMUL_REG_C(i,j)   (MATMUL_BASE + 0x80u + ((i)*4u+(j))*4u)
#define MATMUL_REG_CONTROL  (MATMUL_BASE + 0xC0u)
#define MATMUL_REG_STATUS   (MATMUL_BASE + 0xC4u)

#define MATMUL_CTRL_START   (1u << 0)
#define MATMUL_STATUS_DONE  (1u << 0)
#define MATMUL_STATUS_BUSY  (1u << 1)

#define MAT_N 4u

/* -------------------------------------------------------------------------
 * Prototipos
 * ---------------------------------------------------------------------- */

/* UART */
void uart_putchar(char c);
void uart_puts(const char *s);
void uart_puthex(uint32_t val);
void uart_putdec(uint32_t val);

/* Timer */
uint32_t timer_read_counter(void);
void timer_start_periodic(uint32_t reload, uint32_t prescaler);
void timer_start_free_run(uint32_t prescaler);
void timer_stop(void);
void timer_reset(void);
uint32_t timer_get_status(void);
void timer_ack_expired(void);

/* Multiplicador de matrices */
void matmul_load_a(const uint8_t a[MAT_N][MAT_N]);
void matmul_load_b(const uint8_t b[MAT_N][MAT_N]);
void matmul_start(void);
uint32_t matmul_wait_done(void);
void matmul_read_c(uint32_t c[MAT_N][MAT_N]);

/* Referencia SW (para verificación) */
void matmul_sw(const uint8_t a[MAT_N][MAT_N], const uint8_t b[MAT_N][MAT_N],
               uint32_t c[MAT_N][MAT_N]);

/* Aplicación principal */
void app_main(void);
