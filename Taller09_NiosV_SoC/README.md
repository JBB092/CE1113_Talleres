# Taller 09: SoC con Nios V/m en Platform Designer (verificado en simulación)

CE1113 Sistemas Empotrados — Instituto Tecnológico de Costa Rica

Construcción de un sistema en chip (SoC) mínimo alrededor del procesador
**Nios V/m** (RISC-V, soft core de Altera) en **Platform Designer**, y su
verificación en **simulación RTL con Questa** — sin tarjeta física. El flujo
puede manejarse desde la GUI de Platform Designer o con los scripts de consola
que se incluyen (ambas vías son equivalentes).

El SoC base que se entrega (`hw/hello.qsys`) contiene:

- reloj/reset (`clock_source`),
- el procesador **Nios V/m** (`intel_niosv_m`),
- una **RAM on-chip** de 256 KB (código + datos),
- una **JTAG UART** como `stdout`.

Sobre esa base se ejecuta un "hola mundo" en C que imprime por la JTAG UART.

## Requisitos (instalación local, sin Docker)

A diferencia de los talleres 06–08, este taller usa herramientas de FPGA que
se instalan de forma nativa:

- **Quartus Prime Lite 25.1std** (soporta Cyclone V / DE1-SoC y Nios V).
- Componente **Nios V / RiscFree** (trae el toolchain `riscv32-unknown-elf` y
  las utilidades `niosv-*`).
- **Questa Altera Starter FPGA Edition** (simulador, incluido con Quartus).
- Licencia de Questa (Starter es gratuita, node-locked). **La variable es
  `SALT_LICENSE_SERVER`**, no `LM_LICENSE_FILE`.

> No hay un procesador "ISS" como en Nios II: la ejecución sin hardware se
> hace por **simulación RTL** del SoC completo en Questa.

## Configuración del entorno

El script `scripts/env.sh` deja listas todas las variables; basta con
`source scripts/env.sh`. Esta sección explica **qué hace y por qué**, por si hay
que ajustarlo a otra instalación o diagnosticar un problema.

### 1. Ubicar la instalación

Las herramientas suelen quedar bajo una raíz como `~/altera_lite/25.1std`. Esa
ruta es `ALTERA_ROOT`. Si su Quartus está en otro lugar, expórtelo antes de
hacer `source`:

```bash
export ALTERA_ROOT=/ruta/a/altera_lite/25.1std
```

### 2. Rutas de las herramientas (`QUARTUS_ROOTDIR` y `PATH`)

```bash
export QUARTUS_ROOTDIR="$ALTERA_ROOT/quartus"
export PATH="$QUARTUS_ROOTDIR/bin:\
$QUARTUS_ROOTDIR/sopc_builder/bin:\
$ALTERA_ROOT/niosv/bin:\
$ALTERA_ROOT/riscfree/toolchain/riscv32-unknown-elf/bin:\
$ALTERA_ROOT/questa_fse/bin:$PATH"
```

Cada ruta aporta:

| Ruta | Herramientas |
|---|---|
| `quartus/bin` | `quartus_sh`, `quartus_pgm` |
| `quartus/sopc_builder/bin` | `qsys-generate`, `qsys-script`, `qsys-edit`, `ip-catalog` |
| `niosv/bin` | `niosv-bsp`, `niosv-app`, `niosv-download`, `elf2hex` |
| `riscfree/toolchain/riscv32-unknown-elf/bin` | compilador RISC-V (`riscv32-unknown-elf-gcc`) |
| `questa_fse/bin` | `vsim` (Questa) |

### 3. Licencia de Questa (SALT, no FLEXlm)

Questa Altera Starter usa licenciamiento **SALT** (Siemens). La variable es
`SALT_LICENSE_SERVER`, apuntando al archivo de licencia:

```bash
export SALT_LICENSE_SERVER="$HOME/.altera.quartus/questa_lic.dat"
```

> Si usa `LM_LICENSE_FILE` (como en versiones viejas de ModelSim) verá el error
> `Invalid license environment. Application closing.` — ese es justamente el
> síntoma de no haber usado `SALT_LICENSE_SERVER`.

La licencia Starter es **node-locked** a la MAC del equipo. Para confirmar que la
suya corresponde, compare la MAC de su máquina con el `HOSTID` del archivo:

```bash
# MACs del equipo
cat /sys/class/net/*/address
# HOSTID dentro de la licencia (debe coincidir con una de las anteriores)
grep -o 'HOSTID=[0-9a-f]*' "$SALT_LICENSE_SERVER"
```

### 4. Verificar que todo responde

```bash
riscv32-unknown-elf-gcc --version   # toolchain RISC-V
qsys-generate --help                # Platform Designer
niosv-bsp --help                    # utilidades Nios V
vsim -version                       # Questa
```

## Flujo rápido

```bash
# 1. Preparar el entorno (ajuste ALTERA_ROOT si su Quartus está en otra ruta)
source scripts/env.sh

# 2. Generar el hardware: RTL + .sopcinfo + testbench de simulación
scripts/build-hw.sh

# 3. Compilar el software: BSP + app + hello.elf + ram.hex
scripts/build-sw.sh

# 4. Simular en Questa (consola): imprime el transcript con el "hola mundo"
scripts/sim.sh

# 4b. Alternativa con GUI, para capturar el waveform:
scripts/sim.sh gui
#   luego, en la consola de Questa:
#     add wave -r /*
#     run 3 ms
```

Salida esperada en el transcript de Questa:

```
# Hola mundo, aqui el Nios V/m reportandose 0...
# ...
# Fin del programa.
```

## Estructura

```
taller09-niosv-soc/
├── main.tex              # enunciado del taller
├── Makefile              # compila el PDF; 'make dist' arma el tar.gz
├── hw/
│   ├── hello.qsys        # SoC base (abrir en Platform Designer)
│   └── build_system.tcl  # script Tcl que genera hello.qsys desde cero
├── sw/src/hello_world.c  # hola mundo (punto de partida del software)
└── scripts/
    ├── env.sh            # exporta rutas y SALT_LICENSE_SERVER
    ├── build-hw.sh       # qsys-generate (síntesis + testbench)
    ├── build-sw.sh       # niosv-bsp + niosv-app + cmake + make
    └── sim.sh            # corre Questa
```

Los directorios `hw/gen/` y `build/` son generados y no se versionan.

## Trabajo evaluado (resumen)

Agregar al SoC **dos PIO** (uno de entrada y uno de salida), conectarlos por
software mediante **punteros directos** a sus direcciones (de `system.h`),
**sin** usar funciones del HAL (`alt_*`) ni las macros `IOWR/IORD`, probar que
funcionan en simulación y entregar una **captura de pantalla** del hola mundo
y de los PIO en funcionamiento. El detalle está en el PDF del enunciado.
