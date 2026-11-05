#!/bin/bash
# Ejecuta el firmware compilado en QEMU.

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
WORKSHOP_ROOT="$(cd -- "$SCRIPT_DIR/.." && pwd)"
FIRMWARE="$WORKSHOP_ROOT/build/firmware.elf"
QEMU_BIN="${QEMU_BIN:-qemu-system-riscv32}"

if [ ! -f "$FIRMWARE" ]; then
    echo -e "${RED}Error: firmware no encontrado: $FIRMWARE${NC}"
    echo -e "Primero compilar: ${YELLOW}./scripts/build.sh${NC}"
    exit 1
fi

if ! command -v "$QEMU_BIN" &>/dev/null; then
    echo -e "${RED}Error: QEMU no encontrado: $QEMU_BIN${NC}"
    exit 1
fi

echo -e "${YELLOW}================================${NC}"
echo -e "${YELLOW}Taller 07: Modelado de Hardware${NC}"
echo -e "${YELLOW}================================${NC}"
echo -e "${BLUE}QEMU:     $QEMU_BIN${NC}"
echo -e "${BLUE}Firmware: $FIRMWARE${NC}"
echo -e "${BLUE}Salir:    Ctrl+A  X${NC}\n"

"$QEMU_BIN" \
    -nographic \
    -machine virt \
    -kernel "$FIRMWARE" \
    -bios none \
    -serial stdio \
    -monitor none
