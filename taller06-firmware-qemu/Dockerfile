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
ARG QEMU_REPO=https://gitlab.com/jgonzalez.tec/qemu-tec.git
RUN git clone --depth=1 "$QEMU_REPO" /tmp/qemu-tec \
    && cd /tmp/qemu-tec \
    && ./configure --target-list=riscv32-softmmu --prefix=/usr/local \
    && ninja -C build qemu-system-riscv32 \
    && ninja -C build install \
    && rm -rf /tmp/qemu-tec

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
