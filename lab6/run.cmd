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

@REM copy "%QT_INSTALL%\bin\Qt6Core.dll" .
@REM copy "%QT_INSTALL%\bin\Qt6Gui.dll" .
@REM copy "%QT_INSTALL%\bin\Qt6Widgets.dll" .
@REM copy "%QT_INSTALL%\bin\libgcc_s_seh-1.dll" .
@REM copy "%QT_INSTALL%\bin\libstdc++-6.dll" .
@REM copy "%QT_INSTALL%libwinpthread-1.dll" .
@REM copy "%QT_INSTALL%\bin\Qt6Charts.dll" .

@REM if not exist "platforms" (
@REM     mkdir platforms
@REM     copy "%QT_INSTALL%\plugins\platforms\qwindows.dll" platforms\
@REM )

cmake -G "MinGW Makefiles" ..
mingw32-make

"C:\Qt\Qt-6.10.1\bin\windeployqt6.exe" qcplot.exe
