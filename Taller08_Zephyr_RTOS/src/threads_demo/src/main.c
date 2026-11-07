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

/* ---- Parte 3 (SMP) --------------------------------------------------
 * Solo afecta binarios compilados con CONFIG_SMP=y. En un solo hart
 * (Partes 1 y 2) CPU_FMT/CPU_ARG quedan vacios y la salida es identica.
 */
#ifdef CONFIG_SMP
#define CPU_FMT " [cpu %d]"
#define CPU_ARG , (int)arch_curr_cpu()->id
#else
#define CPU_FMT ""
#define CPU_ARG
#endif

/* Afinidad: 1 = fijar TODOS los hilos al hart 0 (requiere CONFIG_SCHED_CPU_MASK=y) */
#ifndef USE_AFINIDAD
#define USE_AFINIDAD 0
#endif
#if USE_AFINIDAD && !defined(CONFIG_SCHED_CPU_MASK)
#error "USE_AFINIDAD=1 requiere CONFIG_SCHED_CPU_MASK=y"
#endif

/* Con afinidad, los hilos se crean detenidos para poder fijarlos antes de arrancar */
#if USE_AFINIDAD
#define INICIO_HILO K_FOREVER
#else
#define INICIO_HILO K_NO_WAIT
#endif

#define STACK_SIZE 1024

#define PRIO_ALTA  1
#define PRIO_MEDIA 3
#ifndef PRIO_BAJA
#define PRIO_BAJA  5   /* R3: compilar con -DPRIO_BAJA=-1 (cooperativa) */
#endif

/* R2: cuarto hilo con prioridad MAYOR que PRIO_ALTA (numero menor) */
#ifndef USE_HILO_CRITICO
#define USE_HILO_CRITICO 0   /* 0 = firmware base (R1), 1 = con cuarto hilo (R2/R3) */
#endif
#define PRIO_CRITICA 0

K_THREAD_STACK_DEFINE(stack_alta,  STACK_SIZE);
K_THREAD_STACK_DEFINE(stack_media, STACK_SIZE);
K_THREAD_STACK_DEFINE(stack_baja,  STACK_SIZE);

static struct k_thread hilo_alta;
static struct k_thread hilo_media;
static struct k_thread hilo_baja;

#if USE_HILO_CRITICO
K_THREAD_STACK_DEFINE(stack_critica, STACK_SIZE);
static struct k_thread hilo_critica;
#endif

static void hilo_alta_fn(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	while (1) {
		printk("[t=%6u ms]" CPU_FMT " ALTA  (prio %d): tick\n",
		       (unsigned int)k_uptime_get() CPU_ARG, PRIO_ALTA);
		k_msleep(200);
	}
}

static void hilo_media_fn(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	while (1) {
		printk("[t=%6u ms]" CPU_FMT " MEDIA (prio %d): tick\n",
		       (unsigned int)k_uptime_get() CPU_ARG, PRIO_MEDIA);
		k_msleep(500);
	}
}

static void hilo_baja_fn(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	while (1) {
		printk("[t=%6u ms]" CPU_FMT " BAJA  (prio %d): inicio trabajo largo\n",
		       (unsigned int)k_uptime_get() CPU_ARG, PRIO_BAJA);

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

		printk("[t=%6u ms]" CPU_FMT " BAJA  (prio %d): fin trabajo largo (contador=%u)\n",
		       (unsigned int)k_uptime_get() CPU_ARG, PRIO_BAJA, contador);

		k_msleep(100);
	}
}

#if USE_HILO_CRITICO
/* R2: hilo de prioridad mas alta que ALTA, imprime cada 50 ms */
static void hilo_critica_fn(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	while (1) {
		printk("[t=%6u ms]" CPU_FMT " CRIT  (prio %d): tick\n",
		       (unsigned int)k_uptime_get() CPU_ARG, PRIO_CRITICA);
		k_msleep(50);
	}
}
#endif

#if USE_AFINIDAD
/* Parte 3: fija un hilo (aun detenido) a un hart y luego lo arranca */
static void fijar_y_arrancar(struct k_thread *t, const char *nombre, int cpu)
{
	int ret = k_thread_cpu_pin(t, cpu);

	printk("[SMP] k_thread_cpu_pin(%s, cpu %d) = %d\n", nombre, cpu, ret);
	k_thread_start(t);
}
#endif

int main(void)
{
	printk("=== Taller 08 - Parte 1: Preemption ===\n\n");

	/* Muestra con que variante se compilo este binario */
	printk("Config: USE_HILO_CRITICO=%d, PRIO_BAJA=%d (%s)\n\n",
	       USE_HILO_CRITICO, PRIO_BAJA,
	       (PRIO_BAJA < 0) ? "cooperativa" : "expropiativa");

#ifdef CONFIG_SMP
	printk("[SMP] CONFIG_MP_MAX_NUM_CPUS=%d, USE_AFINIDAD=%d, main en cpu %d\n\n",
	       CONFIG_MP_MAX_NUM_CPUS, USE_AFINIDAD, (int)arch_curr_cpu()->id);
#endif

#if USE_HILO_CRITICO
	k_thread_create(&hilo_critica, stack_critica, STACK_SIZE,
			 hilo_critica_fn, NULL, NULL, NULL,
			 PRIO_CRITICA, 0, INICIO_HILO);
	k_thread_name_set(&hilo_critica, "critica");
#endif

	k_thread_create(&hilo_alta, stack_alta, STACK_SIZE,
			 hilo_alta_fn, NULL, NULL, NULL,
			 PRIO_ALTA, 0, INICIO_HILO);
	k_thread_name_set(&hilo_alta, "alta");

	k_thread_create(&hilo_media, stack_media, STACK_SIZE,
			 hilo_media_fn, NULL, NULL, NULL,
			 PRIO_MEDIA, 0, INICIO_HILO);
	k_thread_name_set(&hilo_media, "media");

	k_thread_create(&hilo_baja, stack_baja, STACK_SIZE,
			 hilo_baja_fn, NULL, NULL, NULL,
			 PRIO_BAJA, 0, INICIO_HILO);
	k_thread_name_set(&hilo_baja, "baja");

#if USE_AFINIDAD
#if USE_HILO_CRITICO
	fijar_y_arrancar(&hilo_critica, "critica", 0);
#endif
	fijar_y_arrancar(&hilo_alta, "alta", 0);
	fijar_y_arrancar(&hilo_media, "media", 0);
	fijar_y_arrancar(&hilo_baja, "baja", 0);
#endif

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
