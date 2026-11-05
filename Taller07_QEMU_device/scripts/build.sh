#!/bin/bash
# Compila el firmware RISC-V bare-metal.

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
WORKSHOP_ROOT="$(cd -- "$SCRIPT_DIR/.." && pwd)"
FIRMWARE_DIR="$WORKSHOP_ROOT/src/firmware"
BUILD_DIR="$WORKSHOP_ROOT/build"

echo -e "${YELLOW}================================${NC}"
echo -e "${YELLOW}Firmware Build Script${NC}"
echo -e "${YELLOW}Taller 07: Modelado de Hardware${NC}"
echo -e "${YELLOW}================================${NC}\n"

mkdir -p "$BUILD_DIR"

echo -e "${YELLOW}Compilando firmware...${NC}"
cd "$FIRMWARE_DIR"
make clean
make

cp firmware.elf "$BUILD_DIR/"
cp firmware.bin "$BUILD_DIR/"
cp firmware.map "$BUILD_DIR/"

echo -e "${GREEN}Firmware compilado exitosamente${NC}"
echo -e "  - build/firmware.elf"
echo -e "  - build/firmware.bin"
echo -e "  - build/firmware.map\n"

echo -e "Para ejecutar:"
echo -e "   ${YELLOW}./scripts/run.sh${NC}"
