cmake_minimum_required(VERSION 3.16)
project(FileManagerPreview LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt MOC for signals/slots in Q_OBJECT classes
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTORCC ON)

# Try Qt6 first, then fallback to Qt5
find_package(Qt6 COMPONENTS Widgets)
if (Qt6_FOUND)
    set(QT_VERSION 6)
    set(QT_LIBS Qt6::Widgets)
    find_package(Qt6 COMPONENTS Concurrent QUIET)
    if (TARGET Qt6::Concurrent)
        list(APPEND QT_LIBS Qt6::Concurrent)
    endif()
    
    # QtMultimedia for audio/video preview
    find_package(Qt6 COMPONENTS Multimedia MultimediaWidgets QUIET)
    if (TARGET Qt6::Multimedia AND TARGET Qt6::MultimediaWidgets)
        list(APPEND QT_LIBS Qt6::Multimedia Qt6::MultimediaWidgets)
        set(HAVE_QT_MULTIMEDIA ON)
    endif()

    # Optional PDF support (off by default to avoid broken system packages)
    option(ENABLE_QT_PDF "Enable Qt Pdf module for PDF preview" OFF)
    if (ENABLE_QT_PDF)
        # Prefer core QtPdf module to avoid PdfWidgets config issues
        find_package(Qt6 COMPONENTS Pdf QUIET)
        if (TARGET Qt6::Pdf)
            set(HAVE_QT_PDF_CORE ON)
            list(APPEND QT_LIBS Qt6::Pdf)
        endif()
    endif()
    
    # Optional WebEngine support for office document preview
    option(ENABLE_QT_WEBENGINE "Enable Qt WebEngine for office document preview" OFF)
    if (ENABLE_QT_WEBENGINE)
        find_package(Qt6 COMPONENTS WebEngineWidgets QUIET)
        if (TARGET Qt6::WebEngineWidgets)
            set(HAVE_QT_WEBENGINE ON)
            list(APPEND QT_LIBS Qt6::WebEngineWidgets)
        endif()
    endif()
else()
    find_package(Qt5 COMPONENTS Widgets REQUIRED)
    set(QT_VERSION 5)
    set(QT_LIBS Qt5::Widgets)
    
    # QtMultimedia for Qt5
    find_package(Qt5 COMPONENTS Multimedia MultimediaWidgets QUIET)
    if (TARGET Qt5::Multimedia AND TARGET Qt5::MultimediaWidgets)
        list(APPEND QT_LIBS Qt5::Multimedia Qt5::MultimediaWidgets)
        set(HAVE_QT_MULTIMEDIA ON)
    endif()
endif()

add_executable(file_manager
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/ImageViewer.cpp
    src/ImageViewer.h
    src/TextPreviewer.cpp
    src/TextPreviewer.h
    src/MediaViewer.cpp
    src/MediaViewer.h
    src/OfficeConverter.cpp
    src/OfficeConverter.h
    resources/resources.qrc
)

# Conditionally add PDF viewer sources
if (HAVE_QT_PDF_CORE)
    target_sources(file_manager PRIVATE
        src/PdfSimpleViewer.cpp
        src/PdfSimpleViewer.h
    )
    target_compile_definitions(file_manager PRIVATE HAVE_QT_PDF_CORE=1)
endif()

# Conditionally add WebEngine office viewer sources
if (HAVE_QT_WEBENGINE)
    target_sources(file_manager PRIVATE
        src/OfficeWebViewer.cpp
        src/OfficeWebViewer.h
    )
    target_compile_definitions(file_manager PRIVATE HAVE_QT_WEBENGINE=1)
endif()

# If PdfWidgets also available, you could alternatively use it
if (HAVE_QT_PDF_WIDGETS)
    target_sources(file_manager PRIVATE
        src/PdfViewer.cpp
        src/PdfViewer.h
    )
    target_compile_definitions(file_manager PRIVATE HAVE_QT_PDF_WIDGETS=1)
endif()

# Link libraries
target_link_libraries(file_manager PRIVATE ${QT_LIBS})

if (QT_VERSION EQUAL 6)
    qt_finalize_executable(file_manager)
endif()

# 安装规则
install(TARGETS file_manager
    RUNTIME DESTINATION bin
)

# 安装桌面文件和图标（如果不是通过 debian/rules 安装）
if(NOT DEFINED DEBIAN_PACKAGE)
    install(FILES debian/file-manager-preview.desktop
        DESTINATION share/applications
    )
    install(FILES resources/icons/app-logo.svg
        DESTINATION share/icons/hicolor/scalable/apps
        RENAME file-manager-preview.svg
    )
endif()
