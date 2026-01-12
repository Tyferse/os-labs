if(WIN32)
    set(MINGW_PATH "C:/mingw64/bin")
    set(CMAKE_C_COMPILER "${MINGW_PATH}/gcc.exe")
    set(CMAKE_CXX_COMPILER "${MINGW_PATH}/g++.exe")

    include_directories(include "C:/mingw64/opt/include")
    find_library(LIBSQLITE3 sqlite3 PATHS "C:/mingw64/opt/lib" "C:/mingw64/lib" NO_DEFAULT_PATH)
    if(NOT LIBSQLITE3)
        message(FATAL_ERROR "sqlite3 not found on Windows. Please install it.")
    endif()
else()
    # find_package(PkgConfig REQUIRED)
    # pkg_check_modules(SQLITE3 REQUIRED sqlite3)
    # find_library(LIBSQLITE3 sqlite3)
    find_library(LIBSQLITE3 sqlite3 HINTS /usr/lib/x86_64-linux-gnu /usr/lib)
    set(PLATFORM_LIBS ${LIBSQLITE3})
    # set(PLATFORM_LIBS ${SQLITE3})
endif()

cmake_minimum_required(VERSION 3.28)
project(lab CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Debug)

include_directories(include)

add_executable(emulator src/emulator.cpp)
# target_include_directories(emulator PRIVATE include)

set(SOURCES
    src/temper_logger.cpp
    src/db.cpp
    src/server.cpp
)

add_executable(logger ${SOURCES})
# target_include_directories(logger PRIVATE include)

if(WIN32)
    target_compile_definitions(logger PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_link_libraries(logger ${LIBSQLITE3} ws2_32)
else()
    target_link_libraries(logger ${PLATFORM_LIBS})
endif()

# add_executable(logger ${SOURCES})
