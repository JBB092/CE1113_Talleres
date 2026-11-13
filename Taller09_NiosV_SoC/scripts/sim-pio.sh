#!/usr/bin/env bash
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
source "$HERE/scripts/env.sh"

MENTOR="$HERE/hw/gen/testbench/mentor"
DUT="${DUT:-/hello_tb/hello_inst}"
VAL="${PIO_IN_VAL:-A}"
T="${RUN_TIME:-6 ms}"

cd "$MENTOR"
rm -rf work ./*_lib transcript 2>/dev/null || true

PRE="set USER_DEFINED_ELAB_OPTIONS {-voptargs=+acc}; source msim_setup.tcl; file_copy; ld"
FIND="echo SIGNALS: [find signals -r /hello_tb/*pio*export*]"
FORCE="force -freeze $DUT/pio_in_export 4'h$VAL 0"

if [ "${1:-}" = "gui" ]; then
    vsim -do "$PRE; $FIND; $FORCE; add wave -radix hexadecimal $DUT/pio_out_export $DUT/pio_in_export; run $T; wave zoom full"
else
    vsim -c -do "$PRE; $FIND; $FORCE; run $T; echo PIO_OUT_FINAL = [examine -hex $DUT/pio_out_export]; quit -f"
fi
