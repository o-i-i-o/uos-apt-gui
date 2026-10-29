cmake_minimum_required(VERSION 3.10)
project(apt-gui VERSION 1.0.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5 COMPONENTS Widgets DBus REQUIRED)

include(CTest)
include_directories(include)

set(SOURCES
    src/main.cpp
    src/packagemanager.cpp
    src/helperclient.cpp
    src/mainwindow.cpp
    src/operationprogressdialog.cpp
    src/common/programpaths.cpp
)

set(HEADERS
    include/packagemanager.h
    include/helperclient.h
    include/mainwindow.h
    include/operationprogressdialog.h
    src/common/programpaths.h
)

add_executable(apt-gui ${SOURCES} ${HEADERS})

target_include_directories(apt-gui PRIVATE src/common)
target_link_libraries(apt-gui Qt5::Widgets Qt5::DBus)
target_compile_definitions(apt-gui PRIVATE APT_GUI_VERSION="${PROJECT_VERSION}")

install(TARGETS apt-gui RUNTIME DESTINATION bin)

set(HELPER_SOURCES
    src/helper/main.cpp
    src/helper/aptbackend.cpp
    src/helper/helperservice.cpp
    src/helper/localpackages.cpp
    src/helper/polkitauthorizer.cpp
    src/common/packageargs.cpp
    src/common/programpaths.cpp
    src/common/proxyconfig.cpp
)

set(HELPER_HEADERS
    src/helper/aptbackend.h
    src/helper/helperservice.h
    src/helper/localpackages.h
    src/helper/polkitauthorizer.h
    src/common/packageargs.h
    src/common/programpaths.h
    src/common/proxyconfig.h
)

add_executable(apt-helperd ${HELPER_SOURCES} ${HELPER_HEADERS})

target_include_directories(apt-helperd PRIVATE src/helper src/common)
target_link_libraries(apt-helperd Qt5::Core Qt5::DBus)
target_compile_definitions(apt-helperd PRIVATE APT_GUI_VERSION="${PROJECT_VERSION}")

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

if(BUILD_TESTING)
    find_package(Qt5 COMPONENTS Test REQUIRED)

    add_executable(packageargs_tests
        tests/test_packageargs.cpp
        src/common/packageargs.cpp
        src/common/programpaths.cpp
        src/common/proxyconfig.cpp
        src/helper/localpackages.cpp
    )
    target_include_directories(packageargs_tests PRIVATE src/common src/helper)
    target_link_libraries(packageargs_tests Qt5::Core)
    add_test(NAME packageargs COMMAND packageargs_tests)

    add_executable(gui_tests
        tests/test_gui.cpp
        src/mainwindow.cpp
        src/packagemanager.cpp
        src/helperclient.cpp
        src/operationprogressdialog.cpp
        src/common/programpaths.cpp
        include/mainwindow.h
        include/packagemanager.h
        include/helperclient.h
        include/operationprogressdialog.h
    )
    target_include_directories(gui_tests PRIVATE include src/common)
    target_link_libraries(gui_tests Qt5::Widgets Qt5::DBus Qt5::Test)
    add_test(NAME gui_interaction COMMAND gui_tests)

    add_executable(helperclient_tests
        tests/test_helperclient.cpp
        src/helperclient.cpp
        include/helperclient.h
    )
    target_include_directories(helperclient_tests PRIVATE include)
    target_link_libraries(helperclient_tests Qt5::DBus Qt5::Test)
    add_test(NAME helperclient COMMAND helperclient_tests)
endif()
