# Imagen oficial del proyecto Zephyr: incluye Zephyr SDK (toolchains
# riscv64-zephyr-elf-gcc, etc.), west, CMake, Ninja, dtc y QEMU (usado
# para correr el target "qemu_riscv64" via `west build -t run`, sin
# necesidad de hardware ni OpenOCD/JTAG).
# Se fija a la rama LTS v3.7 (v0.26-branch) por estabilidad.
FROM ghcr.io/zephyrproject-rtos/zephyr-build:v0.26-branch

ARG UID=1000
ARG GID=1000

USER root

# La imagen base ya trae un usuario "user" (home /home/user, grupo
# "plugdev" para acceso a dispositivos USB/JTAG) con un entrypoint.sh
# que vive en /home/user. Se remapea su UID/GID al del host en el
# mismo lugar, sin renombrarlo ni mover su home, para no romper el
# entrypoint de la imagen base.
RUN set -eux; \
    CURRENT_GID=$(id -g user); \
    if [ "$CURRENT_GID" != "$GID" ]; then \
        if getent group "$GID" >/dev/null; then \
            usermod -g "$GID" user; \
        else \
            groupmod -g "$GID" user; \
        fi; \
    fi; \
    if [ "$(id -u user)" != "$UID" ]; then \
        usermod -u "$UID" user; \
    fi; \
    chown -R "$UID:$GID" /home/user

# python3-venv no viene instalado por defecto en la imagen base
RUN apt-get update && apt-get install -y python3-venv && rm -rf /var/lib/apt/lists/*

USER user
WORKDIR /home/user

# --- Workspace de Zephyr (west init + west update), fijado a v3.7-branch (LTS) ---
# Se construye una sola vez dentro de la imagen para que el taller no dependa
# de descargar ~1-2 GB de fuentes durante la sesion de laboratorio.
RUN python3 -m venv /home/user/.venv \
    && . /home/user/.venv/bin/activate \
    && pip install -U pip west \
    && west init -m https://github.com/zephyrproject-rtos/zephyr --mr v3.7-branch zephyrproject \
    && cd zephyrproject \
    && west update --narrow -o=--depth=1 \
    && west zephyr-export \
    && pip install -r zephyr/scripts/requirements.txt

ENV VIRTUAL_ENV=/home/user/.venv
ENV PATH="${VIRTUAL_ENV}/bin:${PATH}"
ENV ZEPHYR_BASE=/home/user/zephyrproject/zephyr

WORKDIR /workspace
