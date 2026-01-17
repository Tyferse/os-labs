cmake_minimum_required(VERSION 3.22)
project(qcustomplot-example LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets PrintSupport)

qt_standard_project_setup()

include_directories(qcustomplot)

# Добавляем файлы QCustomPlot
set(SOURCES
    src/main.cpp
    qcustomplot/qcustomplot.cpp
)

set(HEADERS
    qcustomplot/qcustomplot.h
)

add_executable(qcustomplot-example ${SOURCES} ${HEADERS})

target_link_libraries(qcustomplot-example Qt6::Core Qt6::Widgets Qt6::PrintSupport)

# target_include_directories(qcustomplot-example PRIVATE qcustomplot)
