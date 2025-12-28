#!/bin/bash

set -e
echo "=== Запуск процесса обновления и сборки ==="

# Переход в директорию проекта
PROJ_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$PROJ_DIR"


if [ -e "build" ]; then
    rm -rf build
fi
mkdir build
cd build

cmake ..
make
./test sleep 4
