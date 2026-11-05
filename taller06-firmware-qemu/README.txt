TALLER 06: FIRMWARE BARE-METAL CON QEMU Y RISC-V
CE1113 Sistemas Empotrados

Nombre: Jose Bernardo Barquero Bonilla
Carne: 2023150476

CONSTRUCCION Y EJECUCION

Desde la raiz del proyecto, en el host:

docker build --build-arg UID=$(id -u) --build-arg GID=$(id -g) -t taller06 .
docker run -it --rm -v "$(pwd)":/workspace taller06

Dentro del contenedor:

cd /workspace
./scripts/build.sh
./scripts/run.sh

La aplicacion ejecuta los tres perfiles y luego inicia el reloj.
El reloj funciona indefinidamente. Interrumpir QEMU desde la terminal
para terminar la demostracion.

Para cambios de firmware solo se repiten build.sh y run.sh.
No es necesario reconstruir la imagen Docker.

PARTE 2: OPCION A

Se implementa TimerProfile_t con:
- prescaler
- reload
- auto_reload

timer_apply_profile(const TimerProfile_t *profile) concentra la
configuracion del timer.

NamedProfile_t agrega el nombre utilizado para la salida UART.
run_all_profiles() declara las tres instancias en un arreglo.

Para agregar otro perfil bastaria agregar una entrada al arreglo.
Por ejemplo, un perfil periodico de 250 ms:

{
    .name = "ejemplo-250ms",
    .config = {
        .prescaler = 999u,
        .reload = 250u,
        .auto_reload = 1u
    }
}

No se incorpora esa cuarta entrada en la entrega, porque la Parte 1
solicita exactamente tres perfiles.
