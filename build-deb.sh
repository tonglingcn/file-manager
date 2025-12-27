#!/bin/bash

# Debian 打包脚本
# 使用方法: ./build-deb.sh

set -e

echo "========================================="
echo "  文件管理器预览 - Debian 打包工具"
echo "========================================="
echo ""

# 检查必要的工具
echo "检查依赖工具..."
for cmd in dpkg-buildpackage debuild; do
    if ! command -v $cmd &> /dev/null; then
        echo "错误: 未找到 $cmd，请安装: sudo apt install devscripts build-essential"
        exit 1
    fi
done

# 检查构建依赖
echo "检查构建依赖..."
if ! dpkg-checkbuilddeps 2>/dev/null; then
    echo ""
    echo "缺少构建依赖，尝试安装..."
    sudo apt-get update
    sudo apt-get install -y \
        debhelper \
        cmake \
        qt6-base-dev \
        qt6-multimedia-dev \
        libgl1-mesa-dev \
        libqt6core6 \
        libqt6gui6 \
        libqt6widgets6 \
        libqt6multimedia6 \
        libqt6multimediawidgets6
    
    # 尝试安装 Qt PDF（可选）
    sudo apt-get install -y libqt6pdf6-dev libqt6pdf6 || echo "警告: Qt PDF 模块未安装，PDF 预览功能将不可用"
fi

echo ""
echo "开始构建 Debian 包..."
echo ""

# 清理旧的构建文件
rm -rf build
rm -f ../file-manager-preview_*.deb
rm -f ../file-manager-preview_*.changes
rm -f ../file-manager-preview_*.buildinfo

# 构建包
dpkg-buildpackage -us -uc -b

echo ""
echo "========================================="
echo "  构建完成！"
echo "========================================="
echo ""
echo "生成的包文件："
ls -lh ../file-manager-preview_*.deb

echo ""
echo "安装方法："
echo "  sudo dpkg -i ../file-manager-preview_*.deb"
echo "  sudo apt-get install -f  # 如果有依赖问题"
echo ""
