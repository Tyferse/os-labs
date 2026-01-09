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
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SQLITE3 REQUIRED sqlite3)
endif()

cmake_minimum_required(VERSION 3.28)
project(lab CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Debug)


# find_package(PkgConfig REQUIRED)

# if(WIN32)
#     # For Windows, you may need to provide paths manually
#     find_library(LIBSQLITE3 sqlite3 PATHS "C:\\mingw64\\opt\\lib" "C:\\mingw64\\lib" NO_DEFAULT_PATH)
#     if(NOT LIBSQLITE3)
#         message(FATAL_ERROR "sqlite3 not found on Windows. Please install it.")
#     endif()
# else()
#     find_package(PkgConfig REQUIRED)
#     pkg_check_modules(SQLITE3 REQUIRED sqlite3)
# endif()

# Include directories
# include_directories(include)

# Source files
# set(SOURCES
#     src/temper_logger.cpp
#     src/db.cpp
    # src/server.cpp
# )

# Add Windows-specific definitions
# if(WIN32)
#     target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
# endif()

# Emulator executable
add_executable(emulator src/emulator.cpp)
target_include_directories(emulator PRIVATE include)


set(SOURCES
    src/temper_logger.cpp
    src/db.cpp
    # src/server.cpp
)

add_executable(logger ${SOURCES})

if(WIN32)
    target_link_libraries(logger ${LIBSQLITE3} ws2_32)
else()
    target_link_libraries(logger ${SQLITE3_LIBRARIES})
endif()

# add_executable(db_manager src/db.cpp)
# target_include_directories(db_manager PRIVATE include)
# if(WIN32)
#     target_link_libraries(db_manager ${LIBSQLITE3})
# else()
#     target_link_libraries(db_manager ${SQLITE3_LIBRARIES})
# endif()


# # Executable
# add_executable(logger src/temper_logger.cpp)
# target_include_directories(logger PRIVATE include)

# # Link libraries
# if(WIN32)
#     target_link_libraries(logger ${LIBSQLITE3} ws2_32)
# else()
#     target_link_libraries(logger ${SQLITE3_LIBRARIES})
# endif()

# add_executable(temper_logger src/temper_logger.cpp)
# add_executable(temper_emulator src/emulator.cpp)

# target_include_directories(temper_logger PUBLIC include)
# target_include_directories(temper_emulator PUBLIC include)
