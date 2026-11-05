#!/usr/bin/env bash
# Entorno de herramientas para el Taller 09 (Nios V + Platform Designer + Questa).
#
# Uso:  source scripts/env.sh
#
# Ajuste ALTERA_ROOT si su instalacion de Quartus/Altera esta en otra ruta.
# Ajuste SALT_LICENSE_SERVER si su licencia de Questa esta en otro lugar.

: "${ALTERA_ROOT:=$HOME/altera_lite/25.1std}"

if [ ! -d "$ALTERA_ROOT/quartus" ]; then
    echo "ERROR: no encuentro Quartus en '$ALTERA_ROOT/quartus'." >&2
    echo "       Exporte ALTERA_ROOT apuntando a su instalacion, p. ej.:" >&2
    echo "       export ALTERA_ROOT=/ruta/a/altera_lite/25.1std" >&2
    return 1 2>/dev/null || exit 1
fi

export QUARTUS_ROOTDIR="$ALTERA_ROOT/quartus"
export PATH="$QUARTUS_ROOTDIR/bin:$QUARTUS_ROOTDIR/sopc_builder/bin:$ALTERA_ROOT/niosv/bin:$ALTERA_ROOT/riscfree/toolchain/riscv32-unknown-elf/bin:$ALTERA_ROOT/questa_fse/bin:$PATH"

# Questa Altera Starter usa licenciamiento SALT (no FLEXlm): la variable es
# SALT_LICENSE_SERVER, NO LM_LICENSE_FILE.
: "${SALT_LICENSE_SERVER:=$HOME/.altera.quartus/questa_lic.dat}"
export SALT_LICENSE_SERVER

echo "Entorno Nios V listo:"
echo "  ALTERA_ROOT         = $ALTERA_ROOT"
echo "  SALT_LICENSE_SERVER = $SALT_LICENSE_SERVER"
