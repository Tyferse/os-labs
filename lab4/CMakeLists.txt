if(WIN32)
    set(MINGW_PATH "C:/mingw64/bin")
    set(CMAKE_C_COMPILER "${MINGW_PATH}/gcc.exe")
    set(CMAKE_CXX_COMPILER "${MINGW_PATH}/g++.exe")
endif()

cmake_minimum_required(VERSION 3.28)
project(lab CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Debug)

add_executable(temper_logger src/temper_logger.cpp)
add_executable(temper_emulator src/emulator.cpp)

target_include_directories(temper_logger PUBLIC include)
target_include_directories(temper_emulator PUBLIC include)
