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

add_library(bg_runner src/bg_runner.cpp)
target_include_directories(bg_runner PUBLIC include)

add_executable(test tests/test.cpp)
target_link_libraries(test bg_runner)


if(WIN32)
    target_compile_definitions(bg_runner PRIVATE _WIN32)
endif()
