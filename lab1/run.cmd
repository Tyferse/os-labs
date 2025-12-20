@echo off
echo === Запуск процесса обновления и сборки ===

git pull

if not exist "build" mkdir build
cd build

cmake ..
mingw32-make
pause
