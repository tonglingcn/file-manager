# FileManager Preview

一个基于 Qt6 框架开发的现代化 Linux 文件管理器，具有强大的文件预览功能。

![License](https://img.shields.io/badge/license-GPL--3.0-blue.svg)
![Platform](https://img.shields.io/badge/platform-Linux-lightgrey.svg)
![Qt](https://img.shields.io/badge/Qt-6.x-green.svg)
![CMake](https://img.shields.io/badge/CMake-3.16+-blue.svg)

## ✨ 主要特性

### 🎯 核心功能
- **智能文件浏览**: 支持多种视图模式（图标、列表、树形）
- **多格式预览**: 图片、文本、音视频、PDF、办公文档
- **智能导航**: 面包屑导航、前进后退、快捷访问
- **缩略图系统**: 可控的缩略图生成，兼顾性能和视觉效果
- **自定义界面**: 图标大小、行高、列显示等灵活配置

### 🖼️ 预览功能
| 文件类型 | 支持格式 | 预览特性 |
|---------|---------|----------|
| **图片** | JPG, PNG, GIF, BMP, SVG, WEBP | 鼠标滚轮缩放，自适应窗口显示 |
| **文本** | TXT, CPP, H, PY, JS, HTML, MD, XML | 代码语法高亮，5MB大小限制 |
| **音频** | MP3, WAV, FLAC, OGG, M4A | 完整播放控制，进度条拖拽 |
| **视频** | MP4, AVI, MKV, MOV, WEBM | 内置播放器，音量控制 |
| **PDF** | PDF文档 | 基于Qt PDF模块，支持翻页和缩放 |
| **办公文档** | DOC/DOCX, XLS/XLSX, PPT/PPTX | 通过LibreOffice/WPS转换预览 |

### 🎨 界面特色
- **三栏布局**: 快捷访问 + 文件列表 + 预览区域
- **智能面包屑**: 超过5级自动压缩，支持路径编辑
- **专业图标**: SVG矢量图标，支持高DPI显示
- **现代化设计**: 阴影效果、渐变背景、纸张样式缩略图

## 🚀 快速开始

### 系统要求
- **操作系统**: Linux (Ubuntu 20.04+, Deepin 20+, Debian 11+)
- **编译器**: GCC 9+ 或 Clang 10+ (支持C++17)
- **Qt版本**: Qt 6.2+ (Qt 5.12+ 兼容)

### 安装依赖

#### Ubuntu/Debian
```bash
# 基础依赖
sudo apt update
sudo apt install cmake build-essential qt6-base-dev qt6-multimedia-dev libgl1-mesa-dev

# 可选依赖（用于增强预览功能）
sudo apt install qt6-pdf-dev qt6-webengine-dev libreoffice

# 音视频支持
sudo apt install gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly
```

#### Deepin/UOS
```bash
# 基础依赖
sudo apt update
sudo apt install cmake build-essential qt6base-dev qt6multimedia-dev libgl1-mesa-dev

# 可选依赖
sudo apt install qt6pdf-dev qt6webengine-dev libreoffice
```

### 编译安装

#### 标准编译
```bash
# 克隆或下载项目
cd file-manager

# 创建构建目录
mkdir build && cd build

# 配置项目（启用PDF支持）
cmake .. -DCMAKE_BUILD_TYPE=Release -DENABLE_QT_PDF=ON

# 编译（使用所有CPU核心）
make -j$(nproc)

# 运行
./file_manager
```

#### Debian包构建
```bash
# 使用自动化脚本构建安装包
./build-deb.sh

# 安装生成的包
sudo dpkg -i ../file-manager-preview_1.0.1-1_amd64.deb
sudo apt-get install -f  # 修复依赖关系
```

## 📖 使用指南

### 基本操作
- **双击文件**: 在预览区域打开文件
- **右键菜单**: 复制、剪切、删除、重命名等操作
- **快捷键**:
  - `Ctrl+R`: 刷新当前目录
  - `Ctrl+H`: 显示/隐藏隐藏文件
  - `Backspace`: 返回上级目录
  - `F5`: 刷新

### 视图切换
- **图标视图**: 大图标+缩略图，适合图片浏览
- **列表视图**: 详细信息，适合文件管理
- **树形视图**: 目录树结构，适合深度导航

### 缩略图控制
- **设置路径**: 菜单 → 工具 → 设置
- **性能控制**: 可关闭缩略图生成以提升性能
- **缓存位置**: `~/.cache/file-manager-preview/thumbnails/`

## 🛠️ 技术架构

### 核心技术栈
- **编程语言**: C++17
- **GUI框架**: Qt6 (兼容Qt5)
- **构建系统**: CMake 3.16+
- **多媒体**: Qt6 Multimedia
- **文档处理**: Qt6 PDF + LibreOffice转换

### 模块架构
```
src/
├── MainWindow.cpp/h      # 主窗口（核心控制器）
├── ImageViewer.cpp/h     # 图片预览器
├── TextPreviewer.cpp/h   # 文本预览器
├── MediaViewer.cpp/h     # 媒体播放器
├── PdfSimpleViewer.cpp/h # PDF预览器
├── OfficeConverter.cpp/h # 办公文档转换器
└── OfficeWebViewer.cpp/h # Web预览器（可选）
```

### 特色技术实现

#### 1. 智能缩略图系统
- **异步生成**: 不阻塞主线程
- **缓存机制**: 避免重复生成
- **纸张样式**: 统一的白色背景+阴影效果

#### 2. 多办公软件支持
自动检测并使用系统中最合适的办公软件：
- `unoconv` (最稳定推荐)
- `LibreOffice` (开源免费)
- `WPS Office` (商业软件)
- `OnlyOffice` (开源替代)

#### 3. 异步处理机制
- **QtConcurrent**: Office文档转换不阻塞UI
- **QFutureWatcher**: 监控任务状态
- **错误处理**: 完善的异常捕获和用户提示

## 🔧 配置选项

### CMake配置参数
| 参数 | 默认值 | 说明 |
|------|--------|------|
| `CMAKE_BUILD_TYPE` | `Release` | 构建类型 (Release/Debug) |
| `ENABLE_QT_PDF` | `OFF` | 启用PDF预览功能 |
| `ENABLE_QT_WEBENGINE` | `OFF` | 启用WebEngine预览（实验性） |
| `CMAKE_INSTALL_PREFIX` | `/usr/local` | 安装前缀 |

### 运行时配置
配置文件位置: `~/.config/file-manager-preview/settings.ini`

```ini
[General]
# 界面语言
language=zh_CN

# 默认视图模式
defaultViewMode=1

[Performance]
# 是否生成缩略图
enableThumbnails=true

# 文件大小限制（MB）
maxTextFileSize=5
maxMediaFileSize=100

[Appearance]
# 图标大小
iconSize=64

# 列表行高
listRowHeight=32
```

## 🐛 故障排除

### 常见问题

#### 1. 编译错误：找不到Qt6
```bash
# 确保安装了正确的开发包
sudo apt install qt6-base-dev qt6-multimedia-dev

# 检查Qt版本
qmake6 --version
```

#### 2. 运行时错误：无法播放音频/视频
```bash
# 安装GStreamer插件
sudo apt install gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-libav
```

#### 3. 办公文档无法预览
```bash
# 安装LibreOffice
sudo apt install libreoffice

# 或者unoconv工具
sudo apt install unoconv
```

#### 4. 缩略图显示异常
```bash
# 清理缩略图缓存
rm -rf ~/.cache/file-manager-preview/thumbnails/
```

### 日志调试
```bash
# 启用调试模式运行
./file_manager --debug

# 查看详细日志
tail -f ~/.local/share/file-manager-preview/debug.log
```

## 🤝 贡献指南

### 开发环境设置
```bash
# 克隆项目
git clone <repository-url>
cd file-manager

# 安装开发依赖
sudo apt install qt6-tools-dev qt6-tools-dev-tools

# 使用Qt Creator打开
qt6-creator CMakeLists.txt
```

### 代码规范
- **编码标准**: 遵循Qt编码规范
- **注释**: 使用Doxygen格式注释
- **提交信息**: 使用语义化提交信息

### 添加新的预览格式
1. 在 `src/` 目录下创建新的预览器类
2. 继承 `QWidget` 并实现预览接口
3. 在 `MainWindow.cpp` 中注册新的文件类型
4. 更新CMakeLists.txt添加新文件

## 📄 许可证

本项目采用 **GPL-3.0** 许可证。详见 [LICENSE](LICENSE) 文件。

## 🙏 致谢

- **Qt团队**: 提供优秀的跨平台GUI框架
- **LibreOffice**: 提供办公文档转换支持
- **深度社区**: 提供测试反馈和建议

## 📞 联系方式

- **项目主页**: [GitHub Repository]
- **问题反馈**: [GitHub Issues]
- **邮箱**: [your-email@example.com]

---

**FileManager Preview** - 让文件管理更简单、更高效！