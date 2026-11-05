#!/bin/bash
# Copia el skeleton del device al source de QEMU y recompila incrementalmente.
# Solo recompila riscv_edu_matmul.c + reenlaza qemu-system-riscv32 (~30 s).
# Ejecutar dentro del contenedor despues de modificar src/device-skeleton/riscv_edu_matmul.c

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
WORKSPACE="$(cd -- "$SCRIPT_DIR/.." && pwd)"
DEVICE_SRC="$WORKSPACE/src/device-skeleton/riscv_edu_matmul.c"
QEMU_DEVICE="/opt/qemu-tec/hw/misc/riscv_edu_matmul.c"
QEMU_BUILD="/opt/qemu-tec/build"

if [ ! -f "$DEVICE_SRC" ]; then
    echo -e "${RED}Error: no se encontro el skeleton: $DEVICE_SRC${NC}"
    exit 1
fi

echo -e "${YELLOW}================================${NC}"
echo -e "${YELLOW}Rebuild: riscv-edu-matmul${NC}"
echo -e "${YELLOW}================================${NC}"

echo -e "${BLUE}Copiando skeleton a QEMU source...${NC}"
# rm -f "$QEMU_DEVICE"
cp "$DEVICE_SRC" "$QEMU_DEVICE"

echo -e "${BLUE}Recompilando QEMU (incremental)...${NC}"
ninja -C "$QEMU_BUILD" qemu-system-riscv32

echo -e "${BLUE}Instalando binario...${NC}"
cp "$QEMU_BUILD/qemu-system-riscv32" /usr/local/bin/qemu-system-riscv32

echo -e "${GREEN}Listo. Ejecutar: ./scripts/build.sh && ./scripts/run.sh${NC}"
