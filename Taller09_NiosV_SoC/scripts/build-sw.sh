#!/usr/bin/env bash
# Compila el software para el Nios V/m:
#   - BSP HAL a partir de hw/hello.sopcinfo
#   - aplicacion (proyecto CMake) a partir de sw/src/
#   - enlaza hello.elf y genera ram.hex (elf2hex), que es la imagen con que
#     se inicializa la RAM on-chip en simulacion.
#
# Requiere haber corrido antes:  source scripts/env.sh  y  scripts/build-hw.sh
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
source "$HERE/scripts/env.sh"
cd "$HERE"

SOPC="$HERE/hw/hello.sopcinfo"
if [ ! -f "$SOPC" ]; then
    echo "ERROR: falta $SOPC. Corra primero scripts/build-hw.sh" >&2
    exit 1
fi

# Partimos de cero para que el BSP refleje cambios del .qsys (p. ej. PIO nuevos).
rm -rf "$HERE/build"
mkdir -p "$HERE/build"

echo ">> Generando BSP (HAL) ..."
niosv-bsp --create --type=hal --sopcinfo="$SOPC" "$HERE/build/bsp/settings.bsp"

echo ">> Generando proyecto de aplicacion (CMake) ..."
niosv-app --app-dir="$HERE/build/app" --bsp-dir="$HERE/build/bsp" \
          --srcs="$HERE/sw/src" --elf-name=hello.elf

echo ">> Compilando ..."
cmake -G "Unix Makefiles" -S "$HERE/build/app" -B "$HERE/build/app/build" >/dev/null
make -C "$HERE/build/app/build"

# La RAM on-chip en simulacion lee "ram.hex" desde el directorio donde corre
# vsim (hw/gen/testbench/mentor). Copiamos la imagen recien generada ahi.
MENTOR="$HERE/hw/gen/testbench/mentor"
if [ -d "$MENTOR" ]; then
    cp "$HERE/build/app/build/ram.hex" "$MENTOR/ram.hex"
    echo ">> ram.hex copiado a $MENTOR"
fi

echo ">> Listo. ELF en: $HERE/build/app/build/hello.elf"
