/*
 * Taller 08 - Parte 2: Inversion de prioridades
 *
 * Tres hilos:
 *   ALTA  (prioridad mas alta): necesita un recurso compartido
 *         protegido por RECURSO. Debe ejecutar rapido y con
 *         latencia acotada.
 *   MEDIA (prioridad intermedia): NO usa el recurso compartido en
 *         absoluto. Simplemente consume CPU de forma periodica
 *         (representa cualquier otra carga del sistema que nada
 *         tiene que ver con el recurso).
 *   BAJA  (prioridad mas baja): toma el recurso compartido y lo
 *         mantiene ocupado por un tiempo ("seccion critica larga").
 *
 * Con RECURSO implementado como SEMAFORO (USE_MUTEX 0): BAJA toma
 * el recurso pero conserva su prioridad baja. Mientras BAJA sostiene
 * el recurso, MEDIA (que no lo necesita) puede preemptarla libremente
 * porque tiene mayor prioridad que BAJA. El resultado es que BAJA
 * tarda mucho en soltar el recurso, y ALTA -- que SI lo necesita y
 * tiene la prioridad mas alta de las tres -- queda bloqueada
 * indirectamente por MEDIA, un hilo con el que ni siquiera interactua
 * directamente. Esto es inversion de prioridades no acotada.
 *
 * Con RECURSO implementado como MUTEX (USE_MUTEX 1): Zephyr aplica
 * herencia de prioridad automaticamente. En cuanto ALTA se bloquea
 * esperando el mutex que sostiene BAJA, BAJA hereda temporalmente la
 * prioridad de ALTA. Mientras dura esa herencia, MEDIA ya no puede
 * preemptar a BAJA (porque su prioridad efectiva ahora es mayor que
 * la de MEDIA), por lo que BAJA termina rapido, libera el mutex, y
 * ALTA es desbloqueada casi de inmediato.
 *
 * Cambie USE_MUTEX entre 0 y 1, recompile y compare las marcas de
 * tiempo de "ALTA: recurso obtenido" en ambas trazas.
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

#ifndef USE_MUTEX
#define USE_MUTEX 0   /* 0 = semaforo (con inversion), 1 = mutex (corregido) */
#endif

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

#if USE_MUTEX
static struct k_mutex recurso;
#define RECURSO_INIT()      k_mutex_init(&recurso)
#define RECURSO_TOMAR()     k_mutex_lock(&recurso, K_FOREVER)
#define RECURSO_SOLTAR()    k_mutex_unlock(&recurso)
#else
static struct k_sem recurso;
#define RECURSO_INIT()      k_sem_init(&recurso, 1, 1)
#define RECURSO_TOMAR()     k_sem_take(&recurso, K_FOREVER)
#define RECURSO_SOLTAR()    k_sem_give(&recurso)
#endif

static void busy_wait_ms(int64_t ms)
{
	int64_t fin = k_uptime_get() + ms;
	volatile uint32_t contador = 0;

	while (k_uptime_get() < fin) {
		contador++;
	}
}

static void hilo_alta_fn(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	/* Deja que BAJA arranque primero y tome el recurso */
	k_msleep(300);

	printk("[t=%6u ms]" CPU_FMT " ALTA : solicitando recurso...\n",
	       (unsigned int)k_uptime_get() CPU_ARG);

	RECURSO_TOMAR();

	printk("[t=%6u ms]" CPU_FMT " ALTA : recurso obtenido\n",
	       (unsigned int)k_uptime_get() CPU_ARG);

	RECURSO_SOLTAR();

	printk("[t=%6u ms]" CPU_FMT " ALTA : recurso liberado, fin\n",
	       (unsigned int)k_uptime_get() CPU_ARG);
}

static void hilo_media_fn(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	/* MEDIA jamas toca 'recurso'. Solo consume CPU repetidamente,
	 * representando carga generica del sistema. */
	while (1) {
		printk("[t=%6u ms]" CPU_FMT " MEDIA: trabajo de CPU (no usa el recurso)\n",
		       (unsigned int)k_uptime_get() CPU_ARG);
		busy_wait_ms(150);
		k_msleep(10);
	}
}

static void hilo_baja_fn(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	printk("[t=%6u ms]" CPU_FMT " BAJA : tomando el recurso\n",
	       (unsigned int)k_uptime_get() CPU_ARG);

	/* R6 (momento 1): prioridad efectiva ANTES de tomar el recurso */
	printk("[t=%6u ms]" CPU_FMT " BAJA : [R6] prioridad ANTES de tomar el recurso = %d\n",
	       (unsigned int)k_uptime_get() CPU_ARG, k_thread_priority_get(k_current_get()));

	RECURSO_TOMAR();

	printk("[t=%6u ms]" CPU_FMT " BAJA : recurso tomado, iniciando seccion critica larga (prioridad efectiva=%d)\n",
	       (unsigned int)k_uptime_get() CPU_ARG, k_thread_priority_get(k_current_get()));

	/* Seccion critica "larga": en un sistema sano deberia tomar un
	 * tiempo acotado y predecible. Aqui se simula con varias rondas
	 * de trabajo para que MEDIA tenga oportunidad de preemptar entre
	 * rondas si BAJA no tiene prioridad heredada. */
	for (int i = 0; i < 10; i++) {
		/* R6 (momento 2): prioridad efectiva DURANTE la seccion critica.
		 * En modo MUTEX, desde la ronda en que ALTA ya esta bloqueada
		 * (t ~ 300 ms) debe verse la prioridad heredada (PRIO_ALTA). */
		printk("[t=%6u ms]" CPU_FMT " BAJA : [R6] ronda %d, prioridad efectiva = %d\n",
		       (unsigned int)k_uptime_get() CPU_ARG, i,
		       k_thread_priority_get(k_current_get()));
		busy_wait_ms(100);
	}

	printk("[t=%6u ms]" CPU_FMT " BAJA : fin seccion critica, liberando recurso\n",
	       (unsigned int)k_uptime_get() CPU_ARG);

	RECURSO_SOLTAR();

	/* R6 (momento 3): prioridad efectiva DESPUES de liberar el recurso */
	printk("[t=%6u ms]" CPU_FMT " BAJA : [R6] prioridad DESPUES de liberar el recurso = %d\n",
	       (unsigned int)k_uptime_get() CPU_ARG, k_thread_priority_get(k_current_get()));
}

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
	printk("=== Taller 08 - Parte 2: Inversion de prioridades (USE_MUTEX=%d) ===\n\n",
	       USE_MUTEX);

#ifdef CONFIG_SMP
	printk("[SMP] CONFIG_MP_MAX_NUM_CPUS=%d, USE_AFINIDAD=%d, main en cpu %d\n\n",
	       CONFIG_MP_MAX_NUM_CPUS, USE_AFINIDAD, (int)arch_curr_cpu()->id);
#endif

	RECURSO_INIT();

	k_thread_create(&hilo_baja, stack_baja, STACK_SIZE,
			 hilo_baja_fn, NULL, NULL, NULL,
			 PRIO_BAJA, 0, INICIO_HILO);
	k_thread_name_set(&hilo_baja, "baja");

	k_thread_create(&hilo_media, stack_media, STACK_SIZE,
			 hilo_media_fn, NULL, NULL, NULL,
			 PRIO_MEDIA, 0, INICIO_HILO);
	k_thread_name_set(&hilo_media, "media");

	k_thread_create(&hilo_alta, stack_alta, STACK_SIZE,
			 hilo_alta_fn, NULL, NULL, NULL,
			 PRIO_ALTA, 0, INICIO_HILO);
	k_thread_name_set(&hilo_alta, "alta");

#if USE_AFINIDAD
	fijar_y_arrancar(&hilo_baja, "baja", 0);
	fijar_y_arrancar(&hilo_media, "media", 0);
	fijar_y_arrancar(&hilo_alta, "alta", 0);
#endif

	return 0;
}

/*
 * TODO (estudiante, Parte 2):
 *
 * 1. Compile y ejecute con USE_MUTEX=0. Mida (con las marcas de
 *    tiempo impresas) cuanto tiempo transcurre entre
 *    "ALTA: solicitando recurso" y "ALTA: recurso obtenido".
 *
 * 2. Cambie a USE_MUTEX=1, recompile y repita la medicion. Compare
 *    ambos tiempos y explique la diferencia citando la herencia de
 *    prioridad de k_mutex.
 *
 * 3. En el modo MUTEX, imprima la prioridad efectiva de BAJA
 *    (k_thread_priority_get) en tres momentos: antes de tomar el
 *    recurso, justo despues de que ALTA se bloquea intentando
 *    tomarlo, y despues de soltarlo. Confirme que sube y luego baja.
 */
