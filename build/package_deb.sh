#!/bin/bash
# DEB 包打包脚本：编译 → 按 CMake 安装规则填充 → 生成 deb

set -euo pipefail

APP_NAME="apt-gui"
VERSION="1.0.2"
ARCH="$(dpkg --print-architecture)"
MAINTAINER="Your Name <your.email@example.com>"
DEPENDS="apt, polkitd | policykit-1, libc6, libstdc++6, libqt5core5a, libqt5widgets5, libqt5dbus5"

BUILD_DIR="$(pwd)"
SOURCE_DIR="$(dirname "$BUILD_DIR")"
DEB_DIR="$BUILD_DIR/${APP_NAME}_${VERSION}_${ARCH}"
OUTPUT_DEB="$BUILD_DIR/${APP_NAME}_${VERSION}_${ARCH}.deb"

if [ ! -f "$SOURCE_DIR/CMakeLists.txt" ]; then
    echo "错误：请在项目的 build 目录中运行本脚本" >&2
    exit 1
fi

echo "=== DEB 包打包脚本 ==="

echo "1. 配置与编译..."
cmake -S "$SOURCE_DIR" -B "$BUILD_DIR" -DCMAKE_INSTALL_PREFIX=/usr >/dev/null
make -C "$BUILD_DIR" -j"$(nproc)" >/dev/null

echo "2. 安装到打包目录..."
rm -rf "$DEB_DIR"
mkdir -p "$DEB_DIR/DEBIAN"
DESTDIR="$DEB_DIR" make -C "$BUILD_DIR" install >/dev/null

echo "3. 生成 control..."
INSTALLED_SIZE="$(du -sk "$DEB_DIR" | cut -f1)"
cat > "$DEB_DIR/DEBIAN/control" <<EOF
Package: $APP_NAME
Version: $VERSION
Architecture: $ARCH
Maintainer: $MAINTAINER
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
