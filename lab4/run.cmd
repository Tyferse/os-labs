@echo off
echo === Запуск процесса сборки ===

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
cd ..
build\temper_logger.exe
