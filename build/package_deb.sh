#!/bin/bash

# DEB包打包脚本
# 用于将debmanager二进制文件打包为deb包

set -e

echo "=== DEB包打包脚本 ==="

# 基本配置
APP_NAME="apt-gui"
VERSION="1.0.2"
ARCH="all"
MAINTAINER="Your Name <your.email@example.com>"
DESCRIPTION="A simple DEB package manager for Linux"
DEPENDS="apt, policykit-1"

# 工作目录
BUILD_DIR=$(pwd)
SOURCES_DIR=$(dirname $BUILD_DIR)
DEB_DIR="$BUILD_DIR/${APP_NAME}_${VERSION}_${ARCH}"

# 检查二进制文件是否存在
if [ ! -f "$BUILD_DIR/$APP_NAME" ]; then
    echo "错误：二进制文件 $BUILD_DIR/$APP_NAME 不存在"
    echo "请先编译项目：cd $BUILD_DIR && make"
    exit 1
fi

echo "1. 创建DEB包结构..."
# 创建基本目录结构
mkdir -p "$DEB_DIR/DEBIAN"
mkdir -p "$DEB_DIR/usr/bin"
mkdir -p "$DEB_DIR/usr/share/applications"
mkdir -p "$DEB_DIR/usr/share/icons/hicolor/64x64/apps"

# 创建DEBIAN/control文件
echo "2. 创建control文件..."
cat > "$DEB_DIR/DEBIAN/control" <<EOF
Package: $APP_NAME
Version: $VERSION
Architecture: $ARCH
Maintainer: $MAINTAINER
Description: $DESCRIPTION
Depends: $DEPENDS
Section: utils
Priority: optional
EOF

# 创建postinst脚本
echo "3. 创建postinst脚本..."
cat > "$DEB_DIR/DEBIAN/postinst" <<EOF
#!/bin/bash

set -e

# 更新图标缓存
if which update-icon-caches >/dev/null 2>&1; then
    update-icon-caches /usr/share/icons/hicolor/*/apps
fi

# 更新桌面数据库
if which update-desktop-database >/dev/null 2>&1; then
    update-desktop-database -q
fi

exit 0
EOF

chmod +x "$DEB_DIR/DEBIAN/postinst"

# 创建prerm脚本
echo "4. 创建prerm脚本..."
cat > "$DEB_DIR/DEBIAN/prerm" <<EOF
#!/bin/bash

set -e

exit 0
EOF

chmod +x "$DEB_DIR/DEBIAN/prerm"

# 复制二进制文件
echo "5. 复制二进制文件..."
cp "$BUILD_DIR/$APP_NAME" "$DEB_DIR/usr/bin/"
chmod +x "$DEB_DIR/usr/bin/$APP_NAME"

# 创建图标文件（使用默认图标，实际使用时应替换为真实图标）
echo "6. 创建图标文件..."
cat > "$DEB_DIR/usr/share/icons/hicolor/64x64/apps/$APP_NAME.png" <<EOF
89504E470D0A1A0A0000000D4948445200000040000000400806000000A4F08F14000000017352474200AECE1CE90000000467414D410000B18F0BFC6105000000206348524D00007A26000080840000FA00000080E8000075300000EA6000003A98000017709CBA513F0000003949444154388D6360000000020001E2215032D0000009100012540410A0000003249454E44AE426082
EOF

# 创建.desktop快捷方式
echo "7. 创建.desktop快捷方式..."
cat > "$DEB_DIR/usr/share/applications/$APP_NAME.desktop" <<EOF
[Desktop Entry]
Version=1.0
Type=Application
Name=DEB包管理器
GenericName=包管理器
Comment=管理DEB包的工具
Exec=$APP_NAME
Icon=$APP_NAME
Terminal=false
Categories=System;PackageManager;
StartupNotify=true
EOF

# 打包为DEB包
echo "8. 生成DEB包..."
dpkg-deb --build "$DEB_DIR"

# 清理临时文件
echo "9. 清理临时文件..."
rm -rf "$DEB_DIR"

echo "=== 打包完成！ ==="
echo "DEB包已生成：$BUILD_DIR/${APP_NAME}_${VERSION}_${ARCH}.deb"
echo ""
echo "安装命令：sudo dpkg -i $BUILD_DIR/${APP_NAME}_${VERSION}_${ARCH}.deb"
echo "卸载命令：sudo dpkg -r $APP_NAME"
echo ""
echo "使用说明："
echo "  1. 安装后，可在应用程序菜单中找到\"DEB包管理器\""
echo "  2. 或直接在终端中运行：$APP_NAME"
echo ""
echo "=== 脚本结束 ==="
