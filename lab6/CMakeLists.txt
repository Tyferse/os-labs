if(WIN32)
    set(MINGW_PATH "C:/mingw64/bin")
    set(CMAKE_C_COMPILER "${MINGW_PATH}/gcc.exe")
    set(CMAKE_CXX_COMPILER "${MINGW_PATH}/g++.exe")

    include_directories(include "C:/mingw64/opt/include")
    find_library(LIBSQLITE3 sqlite3 PATHS "C:/mingw64/opt/lib" "C:/mingw64/lib" NO_DEFAULT_PATH)
    if(NOT LIBSQLITE3)
        message(FATAL_ERROR "sqlite3 not found on Windows. Please install it.")
    endif()

    set(CMAKE_PREFIX_PATH "C:/Qt/Qt-6.10.1")
else()
    find_library(LIBSQLITE3 sqlite3 HINTS /usr/lib/x86_64-linux-gnu /usr/lib)
    set(PLATFORM_LIBS ${LIBSQLITE3})
endif()


cmake_minimum_required(VERSION 3.22)
project(qcustomplot-example LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(emulator src/emulator.cpp)

set(SOURCES
    src/temper_logger.cpp
    src/db.cpp
    src/server.cpp
)

add_executable(logger ${SOURCES})

if(WIN32)
    target_compile_definitions(logger PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_link_libraries(logger ${LIBSQLITE3} ws2_32)
else()
    target_link_libraries(logger ${PLATFORM_LIBS})
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Charts Widgets PrintSupport)
qt_standard_project_setup()

add_executable(qcplot src/statistics_gui.cpp)
target_link_libraries(qcplot PRIVATE Qt6::Core Qt6::Charts Qt6::Widgets Qt6::PrintSupport)
