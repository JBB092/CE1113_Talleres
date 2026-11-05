#!/usr/bin/env bash
# Corre la simulacion RTL del SoC en Questa (modo consola) y muestra el
# transcript, donde aparece la salida del printf del Nios V por la JTAG UART.
#
# Requiere:  source scripts/env.sh  y haber corrido build-hw.sh y build-sw.sh
#
# Uso:
#   scripts/sim.sh            # modo consola (headless), imprime el transcript
#   scripts/sim.sh gui        # abre Questa con GUI (para capturar el waveform)
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
source "$HERE/scripts/env.sh"

MENTOR="$HERE/hw/gen/testbench/mentor"
if [ ! -f "$MENTOR/msim_setup.tcl" ]; then
    echo "ERROR: falta el testbench. Corra primero scripts/build-hw.sh" >&2
    exit 1
fi
cd "$MENTOR"
rm -rf work ./*_lib transcript 2>/dev/null || true

if [ "${1:-}" = "gui" ]; then
    # GUI: compila, elabora y deja la simulacion lista para 'add wave' y 'run'.
    vsim -do "source msim_setup.tcl; file_copy; ld"
else
    # Consola: compila, elabora, corre 3 ms y sale.
    vsim -c -do "source msim_setup.tcl; file_copy; ld; run 3 ms; quit -f"
fi
