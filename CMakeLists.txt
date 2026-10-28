cmake_minimum_required(VERSION 3.10)
project(apt-gui VERSION 1.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5 COMPONENTS Widgets DBus REQUIRED)

include_directories(include)

set(SOURCES
    src/main.cpp
    src/packagemanager.cpp
    src/helperclient.cpp
    src/mainwindow.cpp
    src/operationprogressdialog.cpp
)

set(HEADERS
    include/packagemanager.h
    include/helperclient.h
    include/mainwindow.h
    include/operationprogressdialog.h
)

add_executable(apt-gui ${SOURCES} ${HEADERS})

target_link_libraries(apt-gui Qt5::Widgets Qt5::DBus)

install(TARGETS apt-gui RUNTIME DESTINATION bin)

set(HELPER_SOURCES
    src/helper/main.cpp
    src/helper/aptbackend.cpp
    src/helper/helperservice.cpp
    src/helper/polkitauthorizer.cpp
    src/common/packageargs.cpp
)

set(HELPER_HEADERS
    src/helper/aptbackend.h
    src/helper/helperservice.h
    src/helper/polkitauthorizer.h
    src/common/packageargs.h
)

add_executable(apt-helperd ${HELPER_SOURCES} ${HELPER_HEADERS})

target_include_directories(apt-helperd PRIVATE src/helper src/common)
target_link_libraries(apt-helperd Qt5::Core Qt5::DBus)

install(TARGETS apt-helperd RUNTIME DESTINATION lib/apt-gui)

install(FILES packaging/com.debmanager.Helper.conf
        DESTINATION share/dbus-1/system.d)
install(FILES packaging/com.debmanager.Helper.dbus-service
        DESTINATION share/dbus-1/system-services
        RENAME com.debmanager.Helper.service)
install(FILES packaging/com.debmanager.Helper.service
        DESTINATION lib/systemd/system)
install(FILES packaging/com.debmanager.helper.policy
        DESTINATION share/polkit-1/actions)
install(FILES packaging/apt-gui.desktop
        DESTINATION share/applications)
install(FILES packaging/apt-gui.svg
        DESTINATION share/icons/hicolor/scalable/apps)
