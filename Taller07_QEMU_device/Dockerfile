ARG UID=1000
ARG GID=1000
ARG USERNAME=dev

FROM ubuntu:22.04
ARG UID
ARG GID
ARG USERNAME

ENV DEBIAN_FRONTEND=noninteractive

# --- Dependencias del sistema ---
RUN apt-get update && apt-get install -y \
        git build-essential python3 python3-pip \
        pkg-config libglib2.0-dev libpixman-1-dev \
        ninja-build \
        gcc-riscv64-unknown-elf \
    && rm -rf /var/lib/apt/lists/*

# Ubuntu 22.04: meson 0.61 (QEMU v11 requiere >= 1.5), tomli ausente en Python 3.10
RUN pip3 install meson tomli

# --- Clonar y compilar QEMU (fork con riscv-edu-timer) ---
# El source queda en /opt/qemu-tec para poder recompilar incrementalmente
# con rebuild-device.sh sin reconstruir la imagen completa.
ARG QEMU_REPO=https://gitlab.com/jgonzalez.tec/qemu-tec.git
ARG QEMU_BRANCH=ce1113-taller-07
RUN git clone --depth=1 --branch "$QEMU_BRANCH" "$QEMU_REPO" /opt/qemu-tec \
    && cd /opt/qemu-tec \
    && ./configure --target-list=riscv32-softmmu --prefix=/usr/local \
    && ninja -C build qemu-system-riscv32 \
    && ninja -C build install \
    && chmod -R a+w /opt/qemu-tec/build \
    && chmod a+w /opt/qemu-tec/hw/misc/riscv_edu_matmul.c \
    && chmod a+w /usr/local/bin/qemu-system-riscv32

# --- Crear usuario con UID/GID del host (evita problemas de permisos en el volumen) ---
RUN set -eux; \
    if getent group "$GID" >/dev/null; then \
        EXISTING_GROUP=$(getent group "$GID" | cut -d: -f1); \
    else \
        groupadd -g "$GID" "$USERNAME"; \
        EXISTING_GROUP="$USERNAME"; \
    fi; \
    if id -u "$UID" >/dev/null 2>&1; then \
        EXISTING_USER=$(id -nu "$UID"); \
        usermod -g "$EXISTING_GROUP" "$EXISTING_USER"; \
        if [ "$EXISTING_USER" != "$USERNAME" ]; then \
            usermod -l "$USERNAME" -d "/home/$USERNAME" -m "$EXISTING_USER"; \
        fi; \
    else \
        useradd -m -u "$UID" -g "$EXISTING_GROUP" -s /bin/bash "$USERNAME"; \
    fi

USER ${USERNAME}
WORKDIR /workspace
