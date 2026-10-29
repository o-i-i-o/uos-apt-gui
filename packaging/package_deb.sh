#!/bin/bash
# DEB 包打包脚本：编译 → 按 CMake 安装规则填充 → 生成 deb
#
# 用法: packaging/package_deb.sh [构建目录]
#   构建目录也可通过 BUILD_DIR 环境变量指定，默认为 <仓库>/build-deb。
#   脚本不依赖当前工作目录。

set -euo pipefail

APP_NAME="apt-gui"
ARCH="$(dpkg --print-architecture)"
MAINTAINER="oi-io <boss@oi-io.cc>"
HOMEPAGE="https://github.com/o-i-i-o/uos-apt-gui"
DEPENDS="apt, polkitd | policykit-1, libc6, libstdc++6, libqt5core5a, libqt5widgets5, libqt5dbus5"

REPO_DIR=$(cd "$(dirname "$0")/.." && pwd)
SOURCE_DIR=$REPO_DIR
BUILD_DIR=${1:-${BUILD_DIR:-$REPO_DIR/build-deb}}

if [ ! -f "$SOURCE_DIR/CMakeLists.txt" ]; then
    echo "错误：未找到 $SOURCE_DIR/CMakeLists.txt" >&2
    exit 1
fi

mkdir -p "$BUILD_DIR"
BUILD_DIR=$(cd "$BUILD_DIR" && pwd)

echo "=== DEB 包打包脚本 ==="
echo "构建目录: $BUILD_DIR"

echo "1. 配置与编译..."
cmake -S "$SOURCE_DIR" -B "$BUILD_DIR" -DCMAKE_INSTALL_PREFIX=/usr >/dev/null
cmake --build "$BUILD_DIR" -j"$(nproc)" >/dev/null

# 版本号唯一来源是 CMakeLists.txt 的 project(VERSION)，这里只读取配置结果
VERSION=$(sed -n 's/^CMAKE_PROJECT_VERSION:STATIC=//p' "$BUILD_DIR/CMakeCache.txt")
if [ -z "$VERSION" ]; then
    echo "错误：无法从 $BUILD_DIR/CMakeCache.txt 读取项目版本" >&2
    exit 1
fi
echo "版本: $VERSION"
DEB_DIR="$BUILD_DIR/${APP_NAME}_${VERSION}_${ARCH}"
OUTPUT_DEB="$BUILD_DIR/${APP_NAME}_${VERSION}_${ARCH}.deb"

echo "2. 安装到打包目录..."
rm -rf "$DEB_DIR"
mkdir -p "$DEB_DIR/DEBIAN"
DESTDIR="$DEB_DIR" cmake --install "$BUILD_DIR" >/dev/null

echo "3. 生成 control..."
INSTALLED_SIZE="$(du -sk "$DEB_DIR" | cut -f1)"
cat > "$DEB_DIR/DEBIAN/control" <<EOF
Package: $APP_NAME
Version: $VERSION
Architecture: $ARCH
Maintainer: $MAINTAINER
Homepage: $HOMEPAGE
Installed-Size: $INSTALLED_SIZE
Depends: $DEPENDS
Section: admin
Priority: optional
Description: DEB package manager with a polkit-authorized helper
 桌面端 DEB 包管理器。安装、卸载、更新等特权操作通过系统总线调用
 apt-helperd，由 polkit 完成授权，客户端无需以 root 运行。
EOF

echo "4. 生成维护脚本..."
cat > "$DEB_DIR/DEBIAN/postinst" <<'EOF'
#!/bin/bash
set -e

if [ -d /run/systemd/system ] && command -v systemctl >/dev/null 2>&1; then
    systemctl daemon-reload >/dev/null 2>&1 || true
fi

if command -v update-desktop-database >/dev/null 2>&1; then
    update-desktop-database -q /usr/share/applications >/dev/null 2>&1 || true
fi

if command -v gtk-update-icon-cache >/dev/null 2>&1; then
    gtk-update-icon-cache -q -t -f /usr/share/icons/hicolor >/dev/null 2>&1 || true
fi

exit 0
EOF

cat > "$DEB_DIR/DEBIAN/postrm" <<'EOF'
#!/bin/bash
set -e

if [ -d /run/systemd/system ] && command -v systemctl >/dev/null 2>&1; then
    systemctl daemon-reload >/dev/null 2>&1 || true
fi

if command -v update-desktop-database >/dev/null 2>&1; then
    update-desktop-database -q /usr/share/applications >/dev/null 2>&1 || true
fi

if command -v gtk-update-icon-cache >/dev/null 2>&1; then
    gtk-update-icon-cache -q -t -f /usr/share/icons/hicolor >/dev/null 2>&1 || true
fi

exit 0
EOF

chmod 0755 "$DEB_DIR/DEBIAN/postinst" "$DEB_DIR/DEBIAN/postrm"

echo "5. 打包..."
dpkg-deb --build --root-owner-group "$DEB_DIR" "$OUTPUT_DEB" >/dev/null
rm -rf "$DEB_DIR"

echo "=== 打包完成：$OUTPUT_DEB ==="
dpkg-deb --info "$OUTPUT_DEB" | sed -n '1,18p'
echo
echo "安装：sudo apt install $OUTPUT_DEB"
echo "卸载：sudo apt remove $APP_NAME"
echo "验证：busctl introspect com.debmanager.Helper /com/debmanager/Helper"
