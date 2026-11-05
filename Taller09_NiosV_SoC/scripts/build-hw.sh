#!/usr/bin/env bash
# Genera el hardware del SoC a partir de hw/hello.qsys:
#   - RTL de sintesis + hello.sopcinfo (para el BSP)
#   - testbench de simulacion (Verilog) para Questa
#
# Requiere haber hecho antes:  source scripts/env.sh
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
source "$HERE/scripts/env.sh"

cd "$HERE/hw"

# Si modifico el sistema con el script Tcl (en vez de la GUI), regenere el
# .qsys descomentando la linea siguiente:
# qsys-script --script=build_system.tcl

echo ">> Generando RTL de sintesis + .sopcinfo ..."
qsys-generate hello.qsys --synthesis=VERILOG     --output-directory="$HERE/hw/gen"

echo ">> Generando testbench de simulacion (Verilog) ..."
qsys-generate hello.qsys --testbench=STANDARD --testbench-simulation=VERILOG \
              --output-directory="$HERE/hw/gen"

echo ">> Listo. .sopcinfo en: $HERE/hw/hello.sopcinfo"
