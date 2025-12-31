cmake_minimum_required(VERSION 3.10)
project(debmanager VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

include_directories(include)

set(SOURCES
    src/main.cpp
    src/packagemanager.cpp
    src/mainwindow.cpp
    src/operationprogressdialog.cpp
)

set(HEADERS
    include/packagemanager.h
    include/mainwindow.h
    include/operationprogressdialog.h
)

add_executable(debmanager ${SOURCES} ${HEADERS})

target_link_libraries(debmanager Qt5::Widgets)

install(TARGETS debmanager RUNTIME DESTINATION bin)
