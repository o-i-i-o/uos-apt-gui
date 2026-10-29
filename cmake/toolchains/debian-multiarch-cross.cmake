# Debian/Ubuntu multiarch 交叉编译公共设置
#
# 本文件不直接使用，由 linux-<arch>.cmake 预设以下变量后 include：
#   APT_GUI_CROSS_TRIPLET    目标 GNU 三元组，如 aarch64-linux-gnu
#   APT_GUI_CROSS_PROCESSOR  CMake 目标处理器名，如 aarch64

if(NOT APT_GUI_CROSS_TRIPLET OR NOT APT_GUI_CROSS_PROCESSOR)
    message(FATAL_ERROR "请通过 cmake/toolchains/linux-<arch>.cmake 使用本文件")
endif()

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR "${APT_GUI_CROSS_PROCESSOR}")
set(CMAKE_LIBRARY_ARCHITECTURE "${APT_GUI_CROSS_TRIPLET}")

set(CMAKE_C_COMPILER "${APT_GUI_CROSS_TRIPLET}-gcc")
set(CMAKE_CXX_COMPILER "${APT_GUI_CROSS_TRIPLET}-g++")
set(CMAKE_AR "${APT_GUI_CROSS_TRIPLET}-ar")
set(CMAKE_RANLIB "${APT_GUI_CROSS_TRIPLET}-ranlib")
set(CMAKE_STRIP "${APT_GUI_CROSS_TRIPLET}-strip")

# 目标架构的 Qt5 配置位于 multiarch 目录，如 /usr/lib/aarch64-linux-gnu/cmake/Qt5
list(APPEND CMAKE_PREFIX_PATH "/usr/lib/${APT_GUI_CROSS_TRIPLET}/cmake")

# 目标库与头文件同时位于 /usr/<triplet> 与 /usr/lib/<triplet> 两处
set(CMAKE_FIND_ROOT_PATH "/usr/${APT_GUI_CROSS_TRIPLET}" "/usr")

# moc/uic/rcc 必须是宿主架构的可执行文件，因此程序查找不重定向
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)

# 可选：安装 qemu-user-static 后，cmake -DBUILD_TESTING=ON 的 ctest 可直接运行交叉产物
if(NOT DEFINED CMAKE_CROSSCOMPILING_EMULATOR)
    find_program(APT_GUI_CROSS_EMULATOR
        "qemu-${APT_GUI_CROSS_PROCESSOR}-static"
        "qemu-${APT_GUI_CROSS_PROCESSOR}")
    if(APT_GUI_CROSS_EMULATOR)
        set(CMAKE_CROSSCOMPILING_EMULATOR
            "${APT_GUI_CROSS_EMULATOR};-L;/usr/${APT_GUI_CROSS_TRIPLET}")
    endif()
endif()
