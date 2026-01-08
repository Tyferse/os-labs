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

add_executable(clonger src/clonger.cpp)
target_include_directories(clonger PUBLIC include)

if(WIN32)
    target_compile_definitions(clonger PRIVATE _WIN32)
else()
    target_link_libraries(clonger rt pthread)
endif()
