cmake_minimum_required(VERSION 3.10)
project(apt-gui VERSION 1.0.1 LANGUAGES CXX)

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

add_executable(apt-gui ${SOURCES} ${HEADERS})

target_link_libraries(apt-gui Qt5::Widgets)

install(TARGETS apt-gui RUNTIME DESTINATION bin)
