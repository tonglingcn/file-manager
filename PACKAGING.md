# Debian 打包说明

## 快速开始

### 1. 安装打包工具

```bash
sudo apt install devscripts build-essential debhelper
```

### 2. 安装构建依赖

```bash
sudo apt install cmake qt6-base-dev qt6-multimedia-dev libgl1-mesa-dev

# 可选：PDF 支持
sudo apt install libqt6pdf6-dev
```

### 3. 构建 Debian 包

```bash
./build-deb.sh
```

或者手动构建：

```bash
dpkg-buildpackage -us -uc -b
```

### 4. 安装

```bash
sudo dpkg -i ../file-manager-preview_1.0.1-1_amd64.deb
sudo apt-get install -f  # 自动修复依赖
```

## 文件结构

```
debian/
├── changelog          # 版本变更日志
├── compat            # debhelper 兼容级别
├── control           # 包信息和依赖
├── copyright         # 版权信息
├── rules             # 构建规则
├── install           # 安装文件列表
├── source/
│   └── format        # 源码包格式
└── file-manager-preview.desktop  # 桌面快捷方式
```

## 修改包信息

### 更改维护者信息

编辑 `debian/control` 和 `debian/changelog`：

```bash
# 修改维护者
Maintainer: 克亮 <315707022@qq.com>

# 修改主页
Homepage: https://github.com/tonglingcn/file-manager-preview
```

### 更新版本号

```bash
dch -v 1.0.1-1  # 创建新版本
```

## 依赖说明

### 依赖分析

使用 `ldd` 命令可以查看编译后二进制文件的实际依赖：

```bash
cd build
ldd file_manager
```

主要依赖库包括：
- **Qt6 核心模块**：Core, Gui, Widgets
- **Qt6 多媒体**：Multimedia, MultimediaWidgets（音视频播放）
- **Qt6 PDF**：Pdf（PDF 文档预览）
- **系统库**：自动通过 ${shlibs:Depends} 解析

### 构建依赖 (Build-Depends)
- `debhelper-compat (= 13)` - Debian 打包工具
- `cmake (>= 3.16)` - 构建系统
- `qt6-base-dev` - Qt6 基础开发库
- `qt6-multimedia-dev` - Qt6 多媒体库
- `libqt6pdf6-dev` - Qt6 PDF 库（可选）
- `libgl1-mesa-dev` - OpenGL 库

### 运行依赖 (Depends)
- `libqt6core6` - Qt6 核心库
- `libqt6gui6` - Qt6 GUI 库
- `libqt6widgets6` - Qt6 Widgets 库
- `libqt6multimedia6` - Qt6 多媒体核心库
- `libqt6multimediawidgets6` - Qt6 多媒体 Widgets
- `libqt6pdf6` - Qt6 PDF 预览库
- `qt6-qpa-plugins` - Qt6 平台插件

注意：`${shlibs:Depends}` 会自动解析系统库依赖（如 libstdc++6, libc6 等）

### 推荐依赖 (Recommends)
- `wps-office` - WPS Office（推荐，中国市场常用）
- `libreoffice` - LibreOffice（开源免费）
- `onlyoffice-desktopeditors` - OnlyOffice（开源免费）

注意：办公文档预览功能需要安装以上任一办公软件

### 建议依赖 (Suggests)
- `libreoffice-writer` - LibreOffice Writer 组件
- `wps-office-wps` - WPS 文字处理组件

## 测试包

### 检查包内容

```bash
dpkg -c ../file-manager-preview_1.0.1-1_amd64.deb
```

### 检查包信息

```bash
dpkg -I ../file-manager-preview_1.0.1-1_amd64.deb
```

### Lintian 检查

```bash
lintian ../file-manager-preview_1.0.1-1_amd64.deb
```

## 卸载

```bash
sudo apt remove file-manager-preview
```

## 清理构建文件

```bash
rm -rf build
rm -f ../file-manager-preview_*
debian/rules clean
```

## 常见问题

### Q: 构建失败，提示缺少依赖？
A: 运行 `sudo apt-get build-dep .` 或手动安装缺少的包

### Q: PDF 预览不工作？
A: 确保安装了 `libqt6pdf6-dev` 并重新构建

### Q: 办公文档预览不工作？
A: 安装以下任一办公软件：
```bash
# WPS Office（推荐，中国市场常用）
sudo apt install wps-office

# LibreOffice（开源免费）
sudo apt install libreoffice

# OnlyOffice（开源免费）
sudo apt install onlyoffice-desktopeditors
```

程序会自动检测已安装的办公软件并使用。

### Q: 如何为其他发行版打包？
A: 可以使用 `alien` 转换为 RPM：
```bash
sudo apt install alien
sudo alien -r file-manager-preview_1.0.1-1_amd64.deb
```

## 发布到 PPA

如果要发布到 Ubuntu PPA：

```bash
# 1. 签名包
debuild -S -sa

# 2. 上传到 PPA
dput ppa:your-ppa-name file-manager-preview_1.0.1-1_source.changes
```

## 许可证

本项目采用 GPL-3+ 许可证。详见 `debian/copyright`。
