@echo off
echo === Запуск процесса обновления и сборки ===

@REM git pull

REM Переход в директорию проекта
set PROJ_DIR=%~dp0
cd /d "%PROJ_DIR%"

if exist "build" (
    rmdir /s /q build
)

mkdir build
cd build

cmake -G "MinGW Makefiles" ..
mingw32-make
