set(MINGW_PATH "C:/mingw64/bin")
set(CMAKE_C_COMPILER "${MINGW_PATH}/gcc.exe")
set(CMAKE_CXX_COMPILER "${MINGW_PATH}/g++.exe")

cmake_minimum_required(VERSION 3.19)
project(QtTest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_PREFIX_PATH "C:/Qt/Qt-6.10.1")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

add_executable(QtTest main.cpp)

target_link_libraries(QtTest PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
