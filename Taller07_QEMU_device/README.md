# Taller 07: Modelado de Hardware en QEMU

CE1113 Sistemas Empotrados — Instituto Tecnológico de Costa Rica

## Requisitos

- [Docker](https://docs.docker.com/get-docker/)

## Inicio rápido

### 1. Construir la imagen Docker

```bash
docker build --build-arg UID=$(id -u) --build-arg GID=$(id -g) -t taller07 .
```

Clona el fork `qemu-tec` y compila QEMU con los dispositivos educativos
(`riscv-edu-timer` y `riscv-edu-matmul`). Tarda ~10–15 minutos, solo una vez.

### 2. Iniciar el contenedor

```bash
docker run -it --rm -v "$(pwd)":/workspace taller07
```

### 3. Compilar y ejecutar el firmware de validación

```bash
./scripts/build.sh && ./scripts/run.sh
```

## Ciclo de desarrollo

1. Editar `src/device-skeleton/riscv_edu_matmul.c` (completar los TODO).
2. Dentro del contenedor, aplicar los cambios a QEMU y recompilar (~30 s):
   ```bash
   ./scripts/rebuild-device.sh
   ```
3. Editar `src/firmware/app.c` para agregar casos de prueba.
4. Compilar y ejecutar el firmware:
   ```bash
   ./scripts/build.sh && ./scripts/run.sh
   ```

Repetir los pasos 1–4 hasta que todos los tests pasen.

> **Nota:** `docker build` solo es necesario la primera vez. `rebuild-device.sh`
> recompila incrementalmente sin reconstruir la imagen.

## Estructura del repositorio

```
Dockerfile                       Imagen con QEMU + toolchain RISC-V
scripts/
  build.sh                       Compila el firmware
  run.sh                         Ejecuta en QEMU
  rebuild-device.sh              Recompila QEMU con el device modificado (~30 s)
src/
  device-skeleton/
    riscv_edu_matmul.c           <-- Plantilla del device QEMU (completar)
  firmware/
    app.c                        <-- Firmware de validacion (modificar)
    devices.c / devices.h        Driver MMIO (UART, Timer, Matmul)
    main.c  start.S  linker.ld   Infraestructura bare-metal
Taller_07_QEMU_device.pdf        Enunciado del taller
```
