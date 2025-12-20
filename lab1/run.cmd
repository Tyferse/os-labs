@echo off
echo === Запуск процесса обновления и сборки ===

git pull

REM Переход в директорию проекта
set PROJ_DIR=%~dp0
cd /d "%PROJ_DIR%"

if not exist "build" mkdir build
cd build

cmake ..
mingw32-make
pause
