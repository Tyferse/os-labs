if(WIN32)
    set(MINGW_PATH "C:/mingw64/bin")
    set(CMAKE_C_COMPILER "${MINGW_PATH}/gcc.exe")
    set(CMAKE_CXX_COMPILER "${MINGW_PATH}/g++.exe")

    set(CMAKE_PREFIX_PATH "C:/Qt/Qt-6.10.1")

    SET(PROJECT_BUILD_ARGS WIN32)
endif()

# if(WIN32)   
#     SET(PROJECT_BUILD_ARGS WIN32)
# endif()

cmake_minimum_required(VERSION 3.22)
project(plot-widget LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets PrintSupport)

# set(AUTOMOC OFF)
# qt_standard_project_setup()



# add_library(qcustomplot src/qcustomplot.cpp) # include/qcustomplot.h)
# target_link_libraries(qcustomplot Qt6::Core Qt6::Gui Qt6::Widgets Qt6::PrintSupport)
# target_include_directories(qcustomplot PUBLIC include)

# set_target_properties(qcustomplot PROPERTIES
#     AUTOMOC OFF
# )


# add_executable(qcplot src/main.cpp)

# target_link_libraries(qcplot qcustomplot)
# target_link_libraries(qcplot Qt6::Core Qt6::Gui Qt6::Widgets Qt6::PrintSupport)

# target_include_directories(qcplot PUBLIC include)



# set(CMAKE_AUTOMOC OFF)

# Но используем qt_standard_project_setup() для основного приложения
# qt_standard_project_setup()

# set(SOURCES
#     src/main.cpp
#     src/qcustomplot.cpp
# )

add_executable(qcplot main.cpp)

set_target_properties(qcustomplot PROPERTIES
    AUTOMOC OFF
    AUTOUIC OFF
    AUTORCC OFF
)
# set_target_properties(qcplot PROPERTIES
#     AUTOMOC OFF
#     AUTOUIC OFF
# )

target_link_libraries(qcplot PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::PrintSupport)

# target_include_directories(qcplot PRIVATE include)

set_target_properties(qcplot PROPERTIES
    AUTOMOC OFF
    AUTOUIC OFF
)

target_link_libraries(qcplot PRIVATE qcustomplot)
target_compile_definitions(qcplot PRIVATE QCUSTOMPLOT_USE_LIBRARY)
