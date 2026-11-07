CE1113 Sistemas Empotrados - Taller 08: Sistemas de Tiempo Real con Zephyr RTOS
================================================================================
Nombre: Jose Bernardo Barquero Bonilla
Carne:  2023150476
Parte 3 opcional (SMP): SI fue realizada

--------------------------------------------------------------------------------
1. Entorno (desde la carpeta raiz del taller, donde esta el Dockerfile)
--------------------------------------------------------------------------------
  docker build --build-arg UID=$(id -u) --build-arg GID=$(id -g) -t taller08 .
  docker run -it --rm -v "$(pwd)":/workspace taller08

  Los mensajes de Openbox/Xlib/VNC al iniciar el contenedor son normales
  (los genera la imagen base) y pueden ignorarse; presionar Enter.

--------------------------------------------------------------------------------
2. Como se seleccionan las variantes
--------------------------------------------------------------------------------
  Cada variante se compila en su propio directorio de build (-d) y se
  selecciona con macros pasadas por EXTRA_CFLAGS, sin editar el codigo:

    USE_HILO_CRITICO=1  agrega el cuarto hilo CRIT (prioridad 0, cada 50 ms)
    PRIO_BAJA=-1        hace cooperativo al hilo BAJA
    USE_MUTEX=0|1       semaforo (k_sem) o mutex (k_mutex) en la Parte 2
    USE_AFINIDAD=1      fija todos los hilos al hart 0 (Parte 3, requiere
                        CONFIG_SCHED_CPU_MASK=y)

  Sin macros, el codigo reproduce el comportamiento base del enunciado.
  Ejecutar cualquier variante:   west build -d <carpeta_build> -t run
  Salir de QEMU:                 Ctrl+A y luego X

--------------------------------------------------------------------------------
3. Parte 1 - Expropiacion (src/threads_demo)
--------------------------------------------------------------------------------
  cd /workspace/src/threads_demo

  # R1: firmware base (3 hilos)
  west build -b qemu_riscv64 -p always -d build_p1_base .
  west build -d build_p1_base -t run

  # R2: cuarto hilo CRIT de mayor prioridad
  west build -b qemu_riscv64 -p always -d build_p1_critico . -- -DEXTRA_CFLAGS="-DUSE_HILO_CRITICO=1"
  west build -d build_p1_critico -t run

  # R3: cuarto hilo + BAJA cooperativa (prioridad -1)
  west build -b qemu_riscv64 -p always -d build_p1_coop . -- -DEXTRA_CFLAGS="-DUSE_HILO_CRITICO=1 -DPRIO_BAJA=-1"
  west build -d build_p1_coop -t run

--------------------------------------------------------------------------------
4. Parte 2 - Inversion de prioridades (src/priority_inversion)
--------------------------------------------------------------------------------
  cd /workspace/src/priority_inversion

  # R4-R6: semaforo (con inversion)
  west build -b qemu_riscv64 -p always -d build_p2_sem . -- -DEXTRA_CFLAGS="-DUSE_MUTEX=0"
  west build -d build_p2_sem -t run

  # R4-R6: mutex (con herencia de prioridad)
  west build -b qemu_riscv64 -p always -d build_p2_mutex . -- -DEXTRA_CFLAGS="-DUSE_MUTEX=1"
  west build -d build_p2_mutex -t run

  Las lineas "[R6]" muestran la prioridad efectiva de BAJA antes, durante
  y despues de la seccion critica.

--------------------------------------------------------------------------------
5. Parte 3 (opcional) - SMP
--------------------------------------------------------------------------------
  En los binarios SMP cada linea de la traza muestra [cpu N].
  CONFIG_PRINTK_SYNC=y evita que se mezclen lineas de distintos harts.

  a) Variante oficial de la tarjeta (2 harts):

  B=qemu_riscv64/qemu_virt_riscv64/smp
  cd /workspace/src/threads_demo
  west build -b $B -p always -d build_smp_p1 . -- -DCONFIG_PRINTK_SYNC=y -DEXTRA_CFLAGS="-DUSE_HILO_CRITICO=1"
  west build -b $B -p always -d build_smp_p1_coop . -- -DCONFIG_PRINTK_SYNC=y -DEXTRA_CFLAGS="-DUSE_HILO_CRITICO=1 -DPRIO_BAJA=-1"
  west build -b $B -p always -d build_smp_p1_coop_pin . -- -DCONFIG_PRINTK_SYNC=y -DCONFIG_SCHED_CPU_MASK=y -DEXTRA_CFLAGS="-DUSE_HILO_CRITICO=1 -DPRIO_BAJA=-1 -DUSE_AFINIDAD=1"

  cd /workspace/src/priority_inversion
  west build -b $B -p always -d build_smp_p2_sem . -- -DCONFIG_PRINTK_SYNC=y -DEXTRA_CFLAGS="-DUSE_MUTEX=0"
  west build -b $B -p always -d build_smp_p2_mutex . -- -DCONFIG_PRINTK_SYNC=y -DEXTRA_CFLAGS="-DUSE_MUTEX=1"
  west build -b $B -p always -d build_smp_p2_sem_pin . -- -DCONFIG_PRINTK_SYNC=y -DCONFIG_SCHED_CPU_MASK=y -DEXTRA_CFLAGS="-DUSE_MUTEX=0 -DUSE_AFINIDAD=1"

  b) SMP habilitado por Kconfig sobre qemu_riscv64 (metodo del enunciado, 4 harts):

  cd /workspace/src/threads_demo
  # Intento original: compila, pero el tiempo corre 4x mas lento y solo
  # ejecuta CRIT (causa: CONFIG_QEMU_ICOUNT activo con varios harts)
  west build -b qemu_riscv64 -p always -d build_smp_intento_prjconf . -- -DCONFIG_SMP=y -DCONFIG_MP_MAX_NUM_CPUS=4 -DEXTRA_CFLAGS="-DUSE_HILO_CRITICO=1"
  # Corregido: mismo intento con ICOUNT desactivado (ejecucion exitosa)
  west build -b qemu_riscv64 -p always -d build_smp_intento_noicount . -- -DCONFIG_SMP=y -DCONFIG_MP_MAX_NUM_CPUS=4 -DCONFIG_QEMU_ICOUNT=n -DEXTRA_CFLAGS="-DUSE_HILO_CRITICO=1"

  Ejecutar cualquiera:  west build -d <carpeta_build> -t run
