if(WIN32)
    set(MINGW_PATH "C:/mingw64/bin")
    set(CMAKE_C_COMPILER "${MINGW_PATH}/gcc.exe")
    set(CMAKE_CXX_COMPILER "${MINGW_PATH}/g++.exe")

    set(CMAKE_PREFIX_PATH "C:/Qt/Qt-6.10.1")
endif()

cmake_minimum_required(VERSION 3.22)
project(qcustomplot-example LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Charts Widgets PrintSupport)
qt_standard_project_setup()

add_executable(qcplot src/statistics.cpp)
target_link_libraries(qcplot PRIVATE Qt6::Core Qt6::Charts Qt6::Widgets Qt6::PrintSupport)
