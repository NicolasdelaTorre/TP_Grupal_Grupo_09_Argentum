#!/usr/bin/env bash
#
# install.sh - Instala todas las dependencias necesarias para compilar y
# correr el TP (cliente, server y editor) en Debian/Ubuntu (probado en
# Xubuntu 24.04).
#
# Las libs de SDL2 (SDL2, SDL2_image, SDL2_mixer, SDL2_ttf) y SDL2pp se
# compilan desde fuente con FetchContent al hacer `cmake`, pero necesitan
# estas librerias de desarrollo del sistema instaladas de antemano.
#
# Uso:
#   ./install.sh    # instala dependencias y compila el proyecto
#
set -euo pipefail

# ---------------------------------------------------------------------------
# Verificaciones previas
# ---------------------------------------------------------------------------
if ! command -v apt-get >/dev/null 2>&1; then
    echo "ERROR: este instalador solo soporta distros basadas en Debian/Ubuntu (apt)." >&2
    exit 1
fi

# Reejecutar con sudo si no somos root (para poder usar apt-get).
SUDO=""
if [ "$(id -u)" -ne 0 ]; then
    if command -v sudo >/dev/null 2>&1; then
        SUDO="sudo"
    else
        echo "ERROR: se necesitan privilegios de root (instalar sudo o correr como root)." >&2
        exit 1
    fi
fi

echo ">>> Actualizando indice de paquetes..."
$SUDO apt-get update

# ---------------------------------------------------------------------------
# Lista de paquetes
# ---------------------------------------------------------------------------
PACKAGES=(
    # Herramientas de compilacion
    build-essential
    cmake
    git
    pkg-config
    ca-certificates

    # Dependencias de audio/codecs que requieren SDL2_mixer (documentadas en CMakeLists.txt)
    libopus-dev
    libopusfile-dev
    libxmp-dev
    libfluidsynth-dev
    fluidsynth
    libwavpack1
    libwavpack-dev
    wavpack

    # Fuentes (SDL2_ttf)
    libfreetype-dev

    # Backends de video/audio que SDL2 detecta al compilar desde fuente
    libasound2-dev
    libpulse-dev
    libx11-dev
    libxext-dev
    libxrandr-dev
    libxcursor-dev
    libxi-dev
    libxinerama-dev
    libxss-dev
    libwayland-dev
    libxkbcommon-dev
    libegl1-mesa-dev
    libgl1-mesa-dev
    libdrm-dev
    libgbm-dev

    # yaml-cpp (server y editor)
    libyaml-cpp-dev

    # Qt (editor de mapas)
    qt6-base-dev
)

echo ">>> Instalando dependencias..."
$SUDO apt-get install -y --no-install-recommends "${PACKAGES[@]}"

# ---------------------------------------------------------------------------
# Chequeo de version de CMake (el proyecto requiere >= 3.24).
# Si la version del sistema es vieja, agregamos el repo APT de Kitware y
# actualizamos cmake automaticamente.
# ---------------------------------------------------------------------------
REQUIRED="3.24"

cmake_ok() {
    command -v cmake >/dev/null 2>&1 || return 1
    local ver
    ver="$(cmake --version | head -n1 | awk '{print $3}')"
    [ "$(printf '%s\n%s\n' "$REQUIRED" "$ver" | sort -V | head -n1)" = "$REQUIRED" ]
}

if cmake_ok; then
    echo ">>> cmake $(cmake --version | head -n1 | awk '{print $3}') OK (>= $REQUIRED)."
else
    echo ">>> cmake insuficiente (se requiere >= $REQUIRED). Instalando desde Kitware..."

    # Necesitamos estas utilidades para agregar el repo de forma segura.
    $SUDO apt-get install -y --no-install-recommends \
        wget gpg lsb-release software-properties-common

    # Clave GPG de Kitware.
    wget -qO - https://apt.kitware.com/keys/kitware-archive-latest.asc \
        | gpg --dearmor - \
        | $SUDO tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null

    # Repo APT para la distro actual.
    CODENAME="$(lsb_release -cs)"
    echo "deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ ${CODENAME} main" \
        | $SUDO tee /etc/apt/sources.list.d/kitware.list >/dev/null

    $SUDO apt-get update
    $SUDO apt-get install -y --no-install-recommends cmake

    if cmake_ok; then
        echo ">>> cmake actualizado a $(cmake --version | head -n1 | awk '{print $3}')."
    else
        echo "ERROR: no se pudo actualizar cmake a >= $REQUIRED automaticamente." >&2
        echo "       Instalalo manualmente desde https://apt.kitware.com/" >&2
        exit 1
    fi
fi

echo ""
echo ">>> Dependencias instaladas correctamente."

# ---------------------------------------------------------------------------
# Compilacion
# ---------------------------------------------------------------------------
echo ">>> Compilando el proyecto (make compile-debug)..."
make compile-debug
echo ">>> Compilacion finalizada. Binarios en ./build/"
