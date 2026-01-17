@echo off
echo === Запуск процесса сборки ===


REM Установка переменных для Qt
set QT_INSTALL="C:\Qt\Qt-6.10.1"
@REM set QT_PLUGIN_PATH="%QT_INSTALL%\plugins"

REM Переход в директорию проекта
set PROJ_DIR=%~dp0
cd /d "%PROJ_DIR%"

if exist "build" (
    rmdir /s /q build
)

mkdir build
cd build

copy "%QT_INSTALL%\bin\Qt6Core.dll" .
copy "%QT_INSTALL%\bin\Qt6Gui.dll" .
copy "%QT_INSTALL%\bin\Qt6Widgets.dll" .

if not exist "platforms" (
    mkdir platforms
    copy "%QT_INSTALL%\plugins\platforms\qwindows.dll" platforms\
)

cmake -G "MinGW Makefiles" ..
mingw32-make

.\qcplot.exe
