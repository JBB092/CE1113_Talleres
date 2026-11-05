# Taller 08: Sistemas de Tiempo Real con Zephyr RTOS

CE1113 Sistemas Empotrados — Instituto Tecnológico de Costa Rica

Plataforma objetivo: **QEMU** (`qemu_riscv64`, máquina `virt`, RISC-V 64-bit).
No se requiere hardware físico: los conceptos de este taller (expropiación,
prioridades, inversión de prioridades) son comportamiento del kernel de
Zephyr, idéntico en QEMU y en silicio real.

## Requisitos

- [Docker](https://docs.docker.com/get-docker/)

(No se necesita tarjeta física, USB-JTAG, ni OpenOCD: todo corre dentro del
contenedor sobre QEMU.)

## Inicio rápido

### 1. Construir la imagen Docker

```bash
docker build --build-arg UID=$(id -u) --build-arg GID=$(id -g) -t taller08 .
```

Este paso descarga la imagen oficial de Zephyr (`zephyr-build`, con el Zephyr
SDK, `west` y QEMU ya instalados) e inicializa un workspace de Zephyr fijado
a la rama LTS `v3.7-branch`. Tarda varios minutos (descarga ~1-2 GB) y solo
es necesario hacerlo una vez.

### 2. Iniciar el contenedor

```bash
docker run -it --rm -v "$(pwd)":/workspace taller08
```

No se necesitan banderas de acceso a dispositivos (sin tarjeta física no hay
USB-JTAG ni puerto serial que exponer).

### 3. Compilar y ejecutar la Parte 1 (expropiación)

```bash
cd src/threads_demo
west build -b qemu_riscv64 -p always .
west build -t run
```

`west build -t run` compila (si hace falta) y arranca QEMU directamente con
el ELF generado: no hay paso de "flasheo" como en hardware real.

### 4. Ver la salida

La consola de Zephyr (los `printk` del firmware) se imprime **directamente
en la misma terminal**, igual que con `./scripts/run.sh` en el Taller 06.
Para salir de QEMU: `Ctrl+A` seguido de `X`.

### 5. Compilar y ejecutar la Parte 2 (inversión de prioridades)

```bash
cd ../priority_inversion
west build -b qemu_riscv64 -p always .
west build -t run
```

Edite `USE_MUTEX` en `src/main.c` (0 → 1) para comparar ambos comportamientos,
recompilando (`west build -p always .`) entre cada prueba.

---

## Estructura del repositorio

```
Dockerfile                       Imagen con Zephyr SDK + west + QEMU + workspace v3.7-branch
src/threads_demo/
  CMakeLists.txt
  prj.conf
  src/main.c                     Parte 1: expropiación entre hilos de distinta prioridad
src/priority_inversion/
  CMakeLists.txt
  prj.conf
  src/main.c                     Parte 2: inversión de prioridades (semáforo vs. mutex)
Taller_08_Zephyr_RTOS.pdf        Enunciado del taller (incluye comparación RTOS vs.
                                  bare-metal (Taller 06) vs. Linux (Talleres 04-05))
```

## Board target

Se usa `qemu_riscv64` (máquina `virt` de QEMU, un solo hart RV64GC por
defecto). A diferencia de un SoC comercial, no hay cores heterogéneos ni
periféricos físicos específicos: es la plataforma genérica que Zephyr usa
para desarrollo y pruebas sin hardware. Ver Parte 3 (opcional/exploratoria)
del enunciado para explorar múltiples harts emulados vía `CONFIG_SMP`.

## Verificado

Este taller fue probado de punta a punta con Docker real: la imagen
construye, y ambas aplicaciones (`threads_demo` y `priority_inversion`, en
sus dos modos `USE_MUTEX`) compilan y corren sin errores para
`qemu_riscv64` con el Zephyr SDK v3.7 (LTS), vía `west build -t run`.
