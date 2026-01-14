#!/bin/bash

echo "[SETUP] Starting setup..."

if ! command -v python3 &> /dev/null; then
    echo "[SETUP] Python3 could not be found. Please install Python3 first..."
    exit 1
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

echo "[SETUP] Setting up server certificates..."
./src/Server/setup.sh

echo "[SETUP] Building application..."
make

echo "[SETUP] Installation completed successfully..."
echo "[SETUP] Exiting..."

deactivate

exit 0