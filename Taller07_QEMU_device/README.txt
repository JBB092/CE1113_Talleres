Nombre: José Bernardo Barquero Bonilla
Carné: 2023150476
Opción elegida en Parte 3: A - Latencia simulada (QEMUTimer, N = 1000 us)

Compilar y ejecutar:
  docker build --build-arg UID=$(id -u) --build-arg GID=$(id -g) -t taller07 .
  docker run -it --rm -v "$(pwd)":/workspace taller07
  ./scripts/rebuild-device.sh
  ./scripts/build.sh && ./scripts/run.sh
