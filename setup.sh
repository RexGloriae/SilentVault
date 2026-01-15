#!/bin/bash

echo "[SETUP] Starting setup..."

if ! command -v make &> /dev/null; then
    echo "[ERROR] make could not be found. Installing make..."
    sudo apt install make
fi

if ! command -v g++ &> /dev/null; then
    echo "[ERROR] g++ could not be found. Installing g++..."
    sudo apt install g++
fi


if ! command -v python3 &> /dev/null; then
    echo "[SETUP] python3 could not be found. Installing python3..."
    sudo apt install python3
fi

python3 - <<'EOF'
import venv
EOF
if [ $? -ne 0]; then
    echo "[SETUP] python3-venv could not be found. Installing python3-venv..."
    sudo apt install python3-venv
fi

if ! command -v pip &> /dev/null; then
    echo "[SETUP] pip could not be found. Installing pip..."
    sudo apt install python3-pip
fi

if ! command -v openssl &> /dev/null; then
    echo "[ERROR] OpenSSL could not be found. Installing OpenSSL..."
    sudo apt install openssl
fi

if ! pkg-config --exists openssl; then
    echo "[ERROR] OpenSSL development headers missing (libssl-dev). Installing missing headers..."
    sudo apt install libssl-dev
fi

if [ ! -d "venv" ]; then
    echo "[SETUP] Creating virtual environment..."
    python3 -m venv venv
else
    echo "[SETUP] Virtual environment already exists..."
fi

echo "[SETUP] Activating virtual environment..."
source ./venv/bin/activate

echo "[SETUP] Upgrading pip..."
pip install --upgrade pip

if [ ! -f requirements.txt ]; then
    echo "[SETUP] requirements.txt not found - get it from https://www.github.com/RexGloriae/SilentVault ..."
    deactivate
    exit 1
fi

echo "[SETUP] Installing dependencies from requirements.txt..."
pip install -r requirements.txt

echo "[SETUP] Creating necessary directories..."
mkdir ./src/Server/bin
mkdir ./src/Server/data
mkdir ./src/Server/crypto
mkdir ./src/Client/bin
mkdir ./src/Client/data
mkdir ./src/Client/tmp

echo "[SETUP] Setting up server certificates..."
touch ./src/Server/crypto/server.crt
touch ./src/Server/crypto/server.key
./src/Server/setup.sh

echo "[SETUP] Building application..."
make

echo "[SETUP] Installation completed successfully..."
echo "[SETUP] Exiting..."

deactivate

exit 0