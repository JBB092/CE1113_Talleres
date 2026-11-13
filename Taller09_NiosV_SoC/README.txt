Nombre: Jose Bernardo Barquero Bonilla
Carne: 2023150476

Bases de los PIO:
  pio_out (8 bits, salida):  0x00040010
  pio_in  (4 bits, entrada): 0x00040020

Reproduccion:
  source scripts/env.sh
  scripts/build-hw.sh
  scripts/build-sw.sh
  scripts/sim-pio.sh
  PIO_IN_VAL=5 scripts/sim-pio.sh
  scripts/sim-pio.sh gui
