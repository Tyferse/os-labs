#!/bin/bash

set -e
echo "=== Запуск процесса обновления и сборки ==="

git pull

# Переход в директорию проекта
PROJ_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$PROJ_DIR"

if [ ! -d "build" ]; then
    mkdir build
fi
cd build

cmake ..
make
