# Taller 06: Firmware Bare-Metal con QEMU y RISC-V

CE1113 Sistemas Empotrados — Instituto Tecnológico de Costa Rica

## Requisitos

- [Docker](https://docs.docker.com/get-docker/)

No se requiere instalar QEMU ni el toolchain RISC-V localmente; todo corre dentro del contenedor.

## Inicio rápido

### 1. Construir la imagen Docker

```bash
docker build -t taller06 .
```

Este paso clona el fork de QEMU (`qemu-tec`), compila el binario `qemu-system-riscv32` con el
device `riscv-edu-timer` integrado, e instala el toolchain RISC-V. **Solo es necesario hacerlo
una vez** (o cuando cambie el Dockerfile). Tarda aproximadamente 10–15 minutos.

Para pasar un UID/GID distinto al predeterminado (1000):

```bash
docker build --build-arg UID=$(id -u) --build-arg GID=$(id -g) -t taller06 .
```

### 2. Iniciar el contenedor

```bash
docker run -it --rm -v "$(pwd)":/workspace taller06
```

El directorio actual queda montado en `/workspace` dentro del contenedor.

### 3. Compilar el firmware

Dentro del contenedor:

```bash
./scripts/build.sh
```

### 4. Ejecutar en QEMU

```bash
./scripts/run.sh
```

La salida serial del firmware aparece en la terminal.
Para salir de QEMU: **Ctrl+A** seguido de **X**.

---

## Ciclo de desarrollo

Editar `src/kernel-minimal/app.c`, luego desde dentro del contenedor:

```bash
./scripts/build.sh && ./scripts/run.sh
```

No es necesario reiniciar el contenedor entre compilaciones.

## Estructura del repositorio

```
Dockerfile                  Imagen con QEMU+toolchain RISC-V
scripts/
  build.sh                  Compila el firmware
  run.sh                    Ejecuta en QEMU
src/kernel-minimal/
  app.c                     <-- Archivo principal a modificar
  devices.c / devices.h     Driver MMIO del UART y Timer
  main.c                    Inicialización y punto de entrada
  start.S                   Bootstrap en ensamblador RISC-V
  linker.ld                 Layout de memoria
main.pdf                    Enunciado del taller
```

## Modificar el device QEMU (solo desarrollo)

El timer custom (`riscv-edu-timer`) está implementado en el fork
[`qemu-tec`](https://gitlab.com/jgonzalez.tec/qemu-tec.git), en
`hw/misc/riscv_edu_timer.c`. Para modificarlo, editar el archivo en
el fork y reconstruir la imagen Docker:

```bash
docker build --no-cache -t taller06 .
```
