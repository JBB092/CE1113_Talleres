/*
 * Taller 08 - Parte 1: Preemption con hilos de Zephyr
 *
 * Tres hilos de distinta prioridad imprimen periodicamente por UART.
 * El hilo de baja prioridad simula trabajo "pesado" (una espera activa
 * larga) para que sea evidente, en la traza de salida, como los hilos
 * de mayor prioridad lo interrumpen (preemption) en cuanto estan listos
 * para ejecutar.
 *
 * Zephyr usa PRIORIDADES NUMERICAS: un numero MENOR es una prioridad
 * MAYOR. Las prioridades preemptibles van de 0 (la mas alta) hasta
 * CONFIG_NUM_PREEMPT_PRIORITIES-1 (la mas baja).
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#define STACK_SIZE 1024

#define PRIO_ALTA  1
#define PRIO_MEDIA 3
#define PRIO_BAJA  5

K_THREAD_STACK_DEFINE(stack_alta,  STACK_SIZE);
K_THREAD_STACK_DEFINE(stack_media, STACK_SIZE);
K_THREAD_STACK_DEFINE(stack_baja,  STACK_SIZE);

static struct k_thread hilo_alta;
static struct k_thread hilo_media;
static struct k_thread hilo_baja;

static void hilo_alta_fn(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	while (1) {
		printk("[t=%6u ms] ALTA  (prio %d): tick\n",
		       (unsigned int)k_uptime_get(), PRIO_ALTA);
		k_msleep(200);
	}
}

static void hilo_media_fn(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	while (1) {
		printk("[t=%6u ms] MEDIA (prio %d): tick\n",
		       (unsigned int)k_uptime_get(), PRIO_MEDIA);
		k_msleep(500);
	}
}

static void hilo_baja_fn(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	while (1) {
		printk("[t=%6u ms] BAJA  (prio %d): inicio trabajo largo\n",
		       (unsigned int)k_uptime_get(), PRIO_BAJA);

		/* Espera activa (busy-wait) que simula trabajo de CPU real,
		 * sin ceder el procesador voluntariamente. Un hilo de mayor
		 * prioridad que este listo debe poder interrumpirla en
		 * cualquier instante (preemption), no solo en los puntos
		 * donde este hilo llama a k_msleep(). */
		int64_t fin = k_uptime_get() + 1500;
		volatile uint32_t contador = 0;

		while (k_uptime_get() < fin) {
			contador++;
		}

		printk("[t=%6u ms] BAJA  (prio %d): fin trabajo largo (contador=%u)\n",
		       (unsigned int)k_uptime_get(), PRIO_BAJA, contador);

		k_msleep(100);
	}
}

int main(void)
{
	printk("=== Taller 08 - Parte 1: Preemption ===\n\n");

	k_thread_create(&hilo_alta, stack_alta, STACK_SIZE,
			 hilo_alta_fn, NULL, NULL, NULL,
			 PRIO_ALTA, 0, K_NO_WAIT);
	k_thread_name_set(&hilo_alta, "alta");

	k_thread_create(&hilo_media, stack_media, STACK_SIZE,
			 hilo_media_fn, NULL, NULL, NULL,
			 PRIO_MEDIA, 0, K_NO_WAIT);
	k_thread_name_set(&hilo_media, "media");

	k_thread_create(&hilo_baja, stack_baja, STACK_SIZE,
			 hilo_baja_fn, NULL, NULL, NULL,
			 PRIO_BAJA, 0, K_NO_WAIT);
	k_thread_name_set(&hilo_baja, "baja");

	/* main() en Zephyr corre como un hilo mas (prioridad definida por
	 * CONFIG_MAIN_THREAD_PRIORITY); aqui no necesita hacer nada mas. */
	return 0;
}

/*
 * TODO (estudiante, Parte 1):
 *
 * 1. Agregue un cuarto hilo con una prioridad AUN MAS ALTA que
 *    PRIO_ALTA, que imprima cada 50 ms. Observe y documente en el
 *    informe como cambia el entrelazado de la salida serial.
 *
 * 2. Cambie PRIO_BAJA a un valor NEGATIVO (prioridad cooperativa,
 *    ver CONFIG_NUM_COOP_PRIORITIES en prj.conf) y repita el
 *    experimento. Explique, con evidencia de la traza, por que un
 *    hilo cooperativo NO puede ser interrumpido por uno preemptible
 *    de mayor prioridad hasta que el primero ceda el CPU
 *    voluntariamente (k_msleep, k_yield, etc.).
 */
