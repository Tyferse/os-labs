if(WIN32)
    set(MINGW_PATH "C:/mingw64/bin")
    set(CMAKE_C_COMPILER "${MINGW_PATH}/gcc.exe")
    set(CMAKE_CXX_COMPILER "${MINGW_PATH}/g++.exe")

    set(CMAKE_PREFIX_PATH "C:/Qt/Qt-6.10.1")
endif()

cmake_minimum_required(VERSION 3.22)
project(qcplot-test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_subdirectory(QCustomPlot-library-2.1.1.1)
add_subdirectory(src)
