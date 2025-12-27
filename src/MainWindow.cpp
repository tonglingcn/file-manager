#include "MainWindow.h"
#include "ImageViewer.h"
#include "TextPreviewer.h"
#include "MediaViewer.h"

#ifdef HAVE_QT_PDF_CORE
#include "PdfSimpleViewer.h"
#endif
#ifdef HAVE_QT_WEBENGINE
#include "OfficeWebViewer.h"
#endif

#include <QFileSystemModel>
#include <QTreeView>
#include <QTableView>
#include <QListView>
#include <QListWidget>
#include <QSplitter>
#include <QStackedWidget>
#include <QLabel>
#include <QSizePolicy>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QDir>
#include <QFileInfo>
#include <QImageReader>
#include <QFile>
#include <QMessageBox>
#include <QStatusBar>
#include <QFileIconProvider>
#include <QStandardPaths>
#include <QDateTime>
#include <QToolBar>
#include <QToolButton>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QStorageInfo>
#include <QMenu>
#include <QAction>
#include <QDialog>
#include <QFrame>
#include <QPixmap>
#include <QTimer>
#include <QApplication>
#include <QClipboard>
#include "OfficeConverter.h"
#include <QtConcurrent>
#include <QFutureWatcher>
#include <QImageReader>
#include <QPainter>
#include <QPolygonF>
#include <QFont>
#include <QFile>
#include <QTextStream>
#ifdef HAVE_QT_PDF_CORE
#include <QtPdf/QPdfDocument>
#endif
#include <QPushButton>
#include <QWidgetAction>

// ThumbnailIconProvider 实现
bool ThumbnailIconProvider::isImageFile(const QString &filePath) {
    const QByteArray suffix = QFileInfo(filePath).suffix().toLower().toUtf8();
    const QList<QByteArray> fmts = QImageReader::supportedImageFormats();
    return fmts.contains(suffix);
}

QIcon ThumbnailIconProvider::icon(const QFileInfo &info) const {
    // 如果缩略图功能被禁用，直接使用默认图标
    if (!m_enableThumbnails || !*m_enableThumbnails) {
        return QFileIconProvider::icon(info);
    }
    
    const QString filePath = info.absoluteFilePath();
    
    // 对于图片文件，生成缩略图
    if (info.isFile() && isImageFile(filePath)) {
        // 检查缓存
        if (m_thumbnailCache.contains(filePath)) {
            return m_thumbnailCache.value(filePath);
        }
        
        // 生成缩略图
        QPixmap originalPixmap(filePath);
        if (!originalPixmap.isNull()) {
            // 创建纸张样式的背景 (类似文档图标)
            QPixmap documentIcon(80, 96); // 文档比例 (4:3 -> 5:6)
            documentIcon.fill(Qt::transparent);
            
            QPainter painter(&documentIcon);
            painter.setRenderHint(QPainter::Antialiasing);
            
            // 绘制纸张背景
            QRectF docRect(4, 4, 72, 88);
            
            // 绘制阴影
            painter.setBrush(QColor(0, 0, 0, 40));
            painter.setPen(Qt::NoPen);
            painter.drawRoundedRect(docRect.translated(2, 2), 4, 4);
            
            // 绘制白色纸张背景
            painter.setBrush(Qt::white);
            painter.setPen(QPen(QColor(200, 200, 200), 1));
            painter.drawRoundedRect(docRect, 4, 4);
            
            // 绘制右上角折叠效果
            QPolygonF foldTriangle;
            foldTriangle << QPointF(docRect.right() - 12, docRect.top())
                        << QPointF(docRect.right(), docRect.top() + 12)
                        << QPointF(docRect.right(), docRect.top());
            painter.setBrush(QColor(230, 230, 230));
            painter.setPen(QPen(QColor(200, 200, 200), 1));
            painter.drawPolygon(foldTriangle);
            
            // 在纸张内部绘制缩略图内容
            QRectF contentRect = docRect.adjusted(6, 8, -6, -8);
            
            // 缩放图片以适应内容区域
            QPixmap scaledPixmap = originalPixmap.scaled(
                contentRect.size().toSize(), 
                Qt::KeepAspectRatio, 
                Qt::SmoothTransformation
            );
            
            // 居中绘制缩略图
            QPointF imagePos = contentRect.center() - QRectF(scaledPixmap.rect()).center();
            painter.drawPixmap(imagePos, scaledPixmap);
            
            QIcon thumbnailIcon(documentIcon);
            
            // 缓存缩略图
            m_thumbnailCache.insert(filePath, thumbnailIcon);
            return thumbnailIcon;
        }
    }
    
    // 对于PDF文件，尝试生成真实内容预览
    if (info.isFile() && info.suffix().toLower() == "pdf") {
        // 检查缓存
        QString cacheKey = filePath + "_pdf_real";
        if (m_thumbnailCache.contains(cacheKey)) {
            return m_thumbnailCache.value(cacheKey);
        }
        
#ifdef HAVE_QT_PDF_CORE
        // 尝试使用Qt PDF模块生成真实预览
        QPdfDocument pdfDoc;
        if (pdfDoc.load(filePath) == QPdfDocument::Error::None && pdfDoc.pageCount() > 0) {
            // 渲染第一页
            QSizeF pageSize = pdfDoc.pagePointSize(0);
            qreal scale = qMin(60.0 / pageSize.width(), 72.0 / pageSize.height());
            QSize renderSize = (pageSize * scale).toSize();
            
            QImage pdfImage = pdfDoc.render(0, renderSize);
            
            if (!pdfImage.isNull()) {
                // 创建文档样式背景
                QPixmap pdfIcon(80, 96);
                pdfIcon.fill(Qt::transparent);
                
                QPainter painter(&pdfIcon);
                painter.setRenderHint(QPainter::Antialiasing);
                
                QRectF docRect(4, 4, 72, 88);
                
                // 绘制阴影
                painter.setBrush(QColor(0, 0, 0, 40));
                painter.setPen(Qt::NoPen);
                painter.drawRoundedRect(docRect.translated(2, 2), 4, 4);
                
                // 绘制白色纸张背景
                painter.setBrush(Qt::white);
                painter.setPen(QPen(QColor(200, 200, 200), 1));
                painter.drawRoundedRect(docRect, 4, 4);
                
                // 绘制右上角折叠效果
                QPolygonF foldTriangle;
                foldTriangle << QPointF(docRect.right() - 12, docRect.top())
                            << QPointF(docRect.right(), docRect.top() + 12)
                            << QPointF(docRect.right(), docRect.top());
                painter.setBrush(QColor(230, 230, 230));
                painter.setPen(QPen(QColor(200, 200, 200), 1));
                painter.drawPolygon(foldTriangle);
                
                // 在纸张内部绘制PDF内容
                QRectF contentRect = docRect.adjusted(6, 8, -6, -12);
                QPixmap pdfPixmap = QPixmap::fromImage(pdfImage.scaled(
                    contentRect.size().toSize(), 
                    Qt::KeepAspectRatio, 
                    Qt::SmoothTransformation
                ));
                
                // 居中绘制PDF内容
                QPointF imagePos = contentRect.center() - QRectF(pdfPixmap.rect()).center();
                painter.drawPixmap(imagePos, pdfPixmap);
                
                // 绘制PDF标识
                painter.setFont(QFont("Arial", 7, QFont::Bold));
                painter.setPen(QColor(220, 53, 69));
                painter.drawText(docRect.adjusted(6, docRect.height() - 16, -6, -4), 
                                Qt::AlignLeft | Qt::AlignBottom, "PDF");
                
                QIcon pdfIconResult(pdfIcon);
                m_thumbnailCache.insert(cacheKey, pdfIconResult);
                return pdfIconResult;
            }
        }
#endif
        
        // 如果PDF预览失败，使用模拟内容
        QPixmap pdfIcon(80, 96);
        pdfIcon.fill(Qt::transparent);
        
        QPainter painter(&pdfIcon);
        painter.setRenderHint(QPainter::Antialiasing);
        
        QRectF docRect(4, 4, 72, 88);
        
        // 绘制阴影
        painter.setBrush(QColor(0, 0, 0, 40));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(docRect.translated(2, 2), 4, 4);
        
        // 绘制白色纸张背景
        painter.setBrush(Qt::white);
        painter.setPen(QPen(QColor(200, 200, 200), 1));
        painter.drawRoundedRect(docRect, 4, 4);
        
        // 绘制右上角折叠效果
        QPolygonF foldTriangle;
        foldTriangle << QPointF(docRect.right() - 12, docRect.top())
                    << QPointF(docRect.right(), docRect.top() + 12)
                    << QPointF(docRect.right(), docRect.top());
        painter.setBrush(QColor(230, 230, 230));
        painter.setPen(QPen(QColor(200, 200, 200), 1));
        painter.drawPolygon(foldTriangle);
        
        // 绘制PDF内容样式（模拟文本行）
        painter.setPen(QPen(QColor(100, 100, 100), 1));
        QRectF contentRect = docRect.adjusted(8, 12, -8, -8);
        
        // 绘制标题区域
        painter.setBrush(QColor(220, 220, 220));
        painter.drawRect(contentRect.left(), contentRect.top(), contentRect.width(), 8);
        
        // 绘制文本行
        for (int i = 0; i < 8; ++i) {
            float y = contentRect.top() + 16 + i * 6;
            float width = contentRect.width() * (0.7 + (i % 3) * 0.1);
            painter.drawLine(contentRect.left(), y, contentRect.left() + width, y);
        }
        
        // 绘制PDF标识
        painter.setFont(QFont("Arial", 8, QFont::Bold));
        painter.setPen(QColor(220, 53, 69)); // PDF红色
        painter.drawText(contentRect.adjusted(0, contentRect.height() - 16, 0, 0), 
                        Qt::AlignLeft | Qt::AlignBottom, "PDF");
        
        QIcon pdfIconResult(pdfIcon);
        m_thumbnailCache.insert(cacheKey, pdfIconResult);
        return pdfIconResult;
    }
    
    // 对于文本文件，生成真实内容预览
    if (info.isFile()) {
        QString ext = info.suffix().toLower();
        QStringList textTypes = {"txt", "md", "cpp", "h", "hpp", "c", "py", "js", "ts", "html", "css", "xml", "json", "yml", "yaml", "ini", "conf", "log", "sh", "bat"};
        
        if (textTypes.contains(ext)) {
            QString cacheKey = filePath + "_text_real";
            if (m_thumbnailCache.contains(cacheKey)) {
                return m_thumbnailCache.value(cacheKey);
            }
            
            // 读取文件内容
            QFile file(filePath);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text) && file.size() < 1024 * 1024) { // 限制1MB
                QTextStream stream(&file);
                // Qt6 中不再需要 setCodec，默认使用 UTF-8
                QString content = stream.read(2000); // 读取前2000个字符
                file.close();
                
                if (!content.isEmpty()) {
                    // 创建文档样式背景
                    QPixmap textIcon(80, 96);
                    textIcon.fill(Qt::transparent);
                    
                    QPainter painter(&textIcon);
                    painter.setRenderHint(QPainter::Antialiasing);
                    painter.setRenderHint(QPainter::TextAntialiasing);
                    
                    QRectF docRect(4, 4, 72, 88);
                    
                    // 绘制阴影
                    painter.setBrush(QColor(0, 0, 0, 40));
                    painter.setPen(Qt::NoPen);
                    painter.drawRoundedRect(docRect.translated(2, 2), 4, 4);
                    
                    // 绘制白色纸张背景
                    painter.setBrush(Qt::white);
                    painter.setPen(QPen(QColor(200, 200, 200), 1));
                    painter.drawRoundedRect(docRect, 4, 4);
                    
                    // 绘制右上角折叠效果
                    QPolygonF foldTriangle;
                    foldTriangle << QPointF(docRect.right() - 12, docRect.top())
                                << QPointF(docRect.right(), docRect.top() + 12)
                                << QPointF(docRect.right(), docRect.top());
                    painter.setBrush(QColor(230, 230, 230));
                    painter.setPen(QPen(QColor(200, 200, 200), 1));
                    painter.drawPolygon(foldTriangle);
                    
                    // 绘制真实文本内容
                    QRectF contentRect = docRect.adjusted(6, 8, -6, -12);
                    
                    // 设置字体
                    QFont font("Consolas", 5); // 使用等宽字体
                    if (!QFontInfo(font).exactMatch()) {
                        font = QFont("Courier New", 5);
                    }
                    if (!QFontInfo(font).exactMatch()) {
                        font = QFont("monospace", 5);
                    }
                    painter.setFont(font);
                    
                    // 设置文本颜色
                    painter.setPen(QColor(60, 60, 60));
                    
                    // 分行显示文本
                    QStringList lines = content.split('\n');
                    int maxLines = qMin(lines.size(), 12); // 最多显示12行
                    
                    for (int i = 0; i < maxLines; ++i) {
                        QString line = lines[i];
                        if (line.length() > 15) {
                            line = line.left(15) + "...";
                        }
                        
                        QRectF lineRect(contentRect.left(), 
                                       contentRect.top() + i * 6, 
                                       contentRect.width(), 6);
                        
                        painter.drawText(lineRect, Qt::AlignLeft | Qt::AlignTop, line);
                    }
                    
                    // 绘制文件类型标识
                    painter.setFont(QFont("Arial", 7, QFont::Bold));
                    painter.setPen(QColor(70, 130, 180));
                    painter.drawText(docRect.adjusted(6, docRect.height() - 16, -6, -4), 
                                    Qt::AlignLeft | Qt::AlignBottom, ext.toUpper());
                    
                    QIcon textIconResult(textIcon);
                    m_thumbnailCache.insert(cacheKey, textIconResult);
                    return textIconResult;
                }
            }
        }
    }
    
    // 对于其他办公文档类型，创建通用文档图标
    if (info.isFile()) {
        QString ext = info.suffix().toLower();
        QStringList docTypes = {"doc", "docx", "txt", "rtf", "odt"};
        QStringList spreadsheetTypes = {"xls", "xlsx", "ods", "csv"};
        QStringList presentationTypes = {"ppt", "pptx", "odp"};
        
        if (docTypes.contains(ext) || spreadsheetTypes.contains(ext) || presentationTypes.contains(ext)) {
            QString cacheKey = filePath + "_doc_" + ext;
            if (m_thumbnailCache.contains(cacheKey)) {
                return m_thumbnailCache.value(cacheKey);
            }
            
            QPixmap docIcon(80, 96);
            docIcon.fill(Qt::transparent);
            
            QPainter painter(&docIcon);
            painter.setRenderHint(QPainter::Antialiasing);
            
            QRectF docRect(4, 4, 72, 88);
            
            // 绘制阴影
            painter.setBrush(QColor(0, 0, 0, 40));
            painter.setPen(Qt::NoPen);
            painter.drawRoundedRect(docRect.translated(2, 2), 4, 4);
            
            // 绘制白色纸张背景
            painter.setBrush(Qt::white);
            painter.setPen(QPen(QColor(200, 200, 200), 1));
            painter.drawRoundedRect(docRect, 4, 4);
            
            // 绘制右上角折叠效果
            QPolygonF foldTriangle;
            foldTriangle << QPointF(docRect.right() - 12, docRect.top())
                        << QPointF(docRect.right(), docRect.top() + 12)
                        << QPointF(docRect.right(), docRect.top());
            painter.setBrush(QColor(230, 230, 230));
            painter.setPen(QPen(QColor(200, 200, 200), 1));
            painter.drawPolygon(foldTriangle);
            
            // 根据文件类型绘制不同的内容
            QRectF contentRect = docRect.adjusted(8, 12, -8, -8);
            
            if (docTypes.contains(ext)) {
                // 文档类型 - 绘制文本行
                painter.setPen(QPen(QColor(100, 100, 100), 1));
                for (int i = 0; i < 10; ++i) {
                    float y = contentRect.top() + i * 6;
                    float width = contentRect.width() * (0.8 + (i % 3) * 0.1);
                    painter.drawLine(contentRect.left(), y, contentRect.left() + width, y);
                }
            } else if (spreadsheetTypes.contains(ext)) {
                // 表格类型 - 绘制网格
                painter.setPen(QPen(QColor(150, 150, 150), 1));
                // 绘制网格
                for (int i = 0; i <= 4; ++i) {
                    float x = contentRect.left() + i * (contentRect.width() / 4);
                    painter.drawLine(x, contentRect.top(), x, contentRect.bottom());
                }
                for (int i = 0; i <= 6; ++i) {
                    float y = contentRect.top() + i * (contentRect.height() / 6);
                    painter.drawLine(contentRect.left(), y, contentRect.right(), y);
                }
            } else if (presentationTypes.contains(ext)) {
                // 演示文稿类型 - 绘制幻灯片样式
                painter.setBrush(QColor(240, 240, 240));
                painter.setPen(QPen(QColor(200, 200, 200), 1));
                QRectF slideRect = contentRect.adjusted(4, 4, -4, -20);
                painter.drawRect(slideRect);
                
                // 绘制标题区域
                painter.setBrush(QColor(220, 220, 220));
                painter.drawRect(slideRect.left(), slideRect.top(), slideRect.width(), 12);
            }
            
            // 绘制文件类型标识
            painter.setFont(QFont("Arial", 7, QFont::Bold));
            painter.setPen(QColor(70, 130, 180));
            painter.drawText(contentRect.adjusted(0, contentRect.height() - 12, 0, 0), 
                            Qt::AlignLeft | Qt::AlignBottom, ext.toUpper());
            
            QIcon docIconResult(docIcon);
            m_thumbnailCache.insert(cacheKey, docIconResult);
            return docIconResult;
        }
    }
    
    // 对于非文档文件，使用默认图标
    return QFileIconProvider::icon(info);
}

static bool isImageFile(const QString &filePath) {
    const QByteArray suffix = QFileInfo(filePath).suffix().toLower().toUtf8();
    const QList<QByteArray> fmts = QImageReader::supportedImageFormats();
    return fmts.contains(suffix);
}

static bool isTextLikeFile(const QString &filePath) {
    // Simple heuristic by extension
    static const QStringList exts = {
        "txt","md","cpp","c","h","hpp","cc","py","js","ts","json","yml","yaml","xml","html","css","ini","conf","log","sh","bat","cmake","pro"
    };
    const QString ext = QFileInfo(filePath).suffix().toLower();
    return exts.contains(ext);
}

static bool isPdfFile(const QString &filePath) {
    return QFileInfo(filePath).suffix().compare("pdf", Qt::CaseInsensitive) == 0;
}

static bool isOfficeFile(const QString &filePath) {
    static const QStringList exts = {"doc","docx","xls","xlsx","ppt","pptx","odt","ods","odp","rtf"};
    const QString ext = QFileInfo(filePath).suffix().toLower();
    return exts.contains(ext);
}

static bool isAudioFile(const QString &filePath) {
    static const QStringList exts = {"mp3","wav","flac","aac","ogg","m4a","wma","ape","opus"};
    const QString ext = QFileInfo(filePath).suffix().toLower();
    return exts.contains(ext);
}

static bool isVideoFile(const QString &filePath) {
    static const QStringList exts = {"mp4","avi","mkv","mov","wmv","flv","webm","m4v","mpg","mpeg","3gp"};
    const QString ext = QFileInfo(filePath).suffix().toLower();
    return exts.contains(ext);
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setupUI();
}

MainWindow::~MainWindow() {
    delete m_iconProvider;
}

void MainWindow::setupUI() {
    // 主容器
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    auto *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // 顶部工具栏
    setupToolbar();
    mainLayout->addWidget(m_breadcrumbArea);
    
    // 三列分栏
    auto *mainSplitter = new QSplitter(Qt::Horizontal, centralWidget);
    mainLayout->addWidget(mainSplitter, 1);

    // 左侧：快捷访问栏
    m_shortcuts = new QListWidget(mainSplitter);
    m_shortcuts->setMaximumWidth(220);  // 稍微增加最大宽度
    m_shortcuts->setMinimumWidth(160);  // 增加最小宽度，防止过度收缩
    m_shortcuts->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    
    // 添加常用目录
    auto addShortcut = [this](const QString &iconPath, const QString &name, const QString &path) {
        QIcon icon(iconPath);
        auto *item = new QListWidgetItem(icon, name);
        item->setData(Qt::UserRole, path);
        m_shortcuts->addItem(item);
    };
    

    addShortcut(":/icons/icons/home.svg", tr("主目录"), QDir::homePath());
    addShortcut(":/icons/icons/desktop.svg", tr("桌面"), QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
    addShortcut(":/icons/icons/documents.svg", tr("文档"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    addShortcut(":/icons/icons/download.svg", tr("下载"), QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
    addShortcut(":/icons/icons/pictures.svg", tr("图片"), QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
    addShortcut(":/icons/icons/music.svg", tr("音乐"), QStandardPaths::writableLocation(QStandardPaths::MusicLocation));
    addShortcut(":/icons/icons/video.svg", tr("视频"), QStandardPaths::writableLocation(QStandardPaths::MoviesLocation));
    m_shortcuts->addItem("");  // 分隔符
    addShortcut(":/icons/icons/computer.svg", tr("计算机"), QDir::rootPath());
    
    // 添加磁盘列表
    struct DiskInfo {
        QString path;
        qint64 size;
        bool isSystem;
        QString customName; // 保留用户自定义卷名
    };
    QList<DiskInfo> disks;
    for (const QStorageInfo &storage : QStorageInfo::mountedVolumes()) {
        if (!storage.isValid() || !storage.isReady()) continue;
        const QString root = storage.rootPath();
        const qint64 total = storage.bytesTotal();
        // 过滤掉系统临时挂载、只读小分区等以及 /boot、/persistent、/opt、/root、/var
        if (root.startsWith("/dev/loop") || root.startsWith("/run") || root.startsWith("/sys") || root.startsWith("/proc") || root.startsWith("/tmp") || root.startsWith("/boot")
            || root.startsWith("/persistent") || root.startsWith("/opt") || root.startsWith("/root") || root.startsWith("/var") || total < 1024*1024*1024) continue;
        bool isSystem = (root == "/");
        // 优先使用用户自定义卷名，若displayName非空且不等于root
        QString customName = storage.displayName();
        if (customName.isEmpty() || customName == root || customName == "_dde_data") customName.clear();
        // /home 也视为数据盘，除非容量很小才视作系统盘
        if (!isSystem && (total <= 64ULL*1024*1024*1024)) isSystem = true;
        disks.append({root, total, isSystem, customName});
    }
    // 按系统/非系统分类，再按容量排序；数据盘自定义名称优先显示
    std::sort(disks.begin(), disks.end(), [](const DiskInfo &a, const DiskInfo &b) {
        if (a.isSystem != b.isSystem) return a.isSystem > b.isSystem;
        if (!a.customName.isEmpty() && b.customName.isEmpty()) return true;
        if (a.customName.isEmpty() && !b.customName.isEmpty()) return false;
        return b.size < a.size;
    });
    auto formatGB = [](qint64 bytes) {
        return QString::number(bytes / (1024.0 * 1024.0 * 1024.0), 'f', 0) + "GB";
    };
    int sysIdx = 1, dataIdx = 1;
    for (const auto &d : disks) {
        QString name;
        if (!d.customName.isEmpty()) {
            name = d.customName + " (" + formatGB(d.size) + ")";
        } else if (d.isSystem) {
            name = tr("系统盘%1 (%2)").arg(sysIdx++).arg(formatGB(d.size));
        } else {
            name = tr("数据盘%1 (%2)").arg(dataIdx++).arg(formatGB(d.size));
        }
        QString diskIcon = d.isSystem ? ":/icons/icons/system-disk.svg" : ":/icons/icons/harddisk.svg";
        addShortcut(diskIcon, name, d.path);
        qDebug() << "Adding disk entry:" << name << "=>" << d.path;
    }
    
    connect(m_shortcuts, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        const QString path = item->data(Qt::UserRole).toString();
        if (!path.isEmpty() && QDir(path).exists()) {
            navigateToPath(path);
        }
    });

    // 中间：文件列表
    m_model = new CustomFileSystemModel(this);
    m_model->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files);
    
    // 创建并设置自定义图标提供器（支持缩略图开关）
    m_iconProvider = new ThumbnailIconProvider(&m_showThumbnails);
    m_model->setIconProvider(m_iconProvider);
    
    const QString rootPath = QDir::currentPath();
    m_model->setRootPath(rootPath);

    // 创建文件视图堆叠窗口
    m_fileViewStack = new QStackedWidget(mainSplitter);
    m_fileViewStack->setMinimumWidth(400);  // 设置最小宽度，防止过度收缩
    m_fileViewStack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    // 表格视图
    m_tableView = new QTableView();
    m_tableView->setModel(m_model);
    m_tableView->setRootIndex(m_model->index(rootPath));
    m_tableView->setSortingEnabled(true);
    m_tableView->sortByColumn(0, Qt::AscendingOrder);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setShowGrid(false);
    m_tableView->verticalHeader()->setVisible(false);
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    m_tableView->horizontalHeader()->setSectionsClickable(true);
    m_tableView->horizontalHeader()->setSortIndicatorShown(true);
    m_tableView->setColumnWidth(0, 250);
    m_tableView->setIconSize(QSize(24, 24));  // 默认图标大小
    
    // 列表视图
    m_listView = new QListView();
    m_listView->setModel(m_model);
    m_listView->setRootIndex(m_model->index(rootPath));
    m_listView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_listView->setIconSize(QSize(32, 32));
    m_listView->setViewMode(QListView::IconMode);
    m_listView->setGridSize(QSize(120, 120));
    m_listView->setResizeMode(QListView::Adjust);
    m_listView->setWordWrap(true);
    m_listView->setSpacing(10);
    
    // 树形视图
    m_treeView = new QTreeView();
    m_treeView->setModel(m_model);
    m_treeView->setRootIndex(m_model->index(rootPath));
    m_treeView->setSortingEnabled(true);
    m_treeView->sortByColumn(0, Qt::AscendingOrder);
    m_treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_treeView->setAlternatingRowColors(true);
    m_treeView->setHeaderHidden(false);
    m_treeView->setIconSize(QSize(20, 20));
    m_treeView->setAnimated(true);
    m_treeView->setExpandsOnDoubleClick(false);
    m_treeView->setColumnWidth(0, 250);
    
    // 添加到堆叠窗口
    m_fileViewStack->addWidget(m_tableView);
    m_fileViewStack->addWidget(m_listView);
    m_fileViewStack->addWidget(m_treeView);
    
    // 默认显示表格视图
    m_fileViewStack->setCurrentWidget(m_tableView);
    
    // 启用右键菜单
    m_tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_listView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    
    connect(m_tableView, &QTableView::customContextMenuRequested, this, &MainWindow::showContextMenu);
    connect(m_listView, &QListView::customContextMenuRequested, this, &MainWindow::showContextMenu);
    connect(m_treeView, &QTreeView::customContextMenuRequested, this, &MainWindow::showContextMenu);
    
    // 单击事件
    connect(m_tableView, &QTableView::clicked, this, &MainWindow::onItemClicked);
    connect(m_listView, &QListView::clicked, this, &MainWindow::onItemClicked);
    connect(m_treeView, &QTreeView::clicked, this, &MainWindow::onItemClicked);
    // 双击事件处理
    connect(m_tableView, &QTableView::doubleClicked, this, [this](const QModelIndex &index) {
        if (!index.isValid()) return;
        const QFileInfo info = m_model->fileInfo(index);
        if (info.isDir()) {
            navigateToPath(info.absoluteFilePath());
        }
    });
    
    connect(m_listView, &QListView::doubleClicked, this, [this](const QModelIndex &index) {
        if (!index.isValid()) return;
        const QFileInfo info = m_model->fileInfo(index);
        if (info.isDir()) {
            navigateToPath(info.absoluteFilePath());
        }
    });
    
    connect(m_treeView, &QTreeView::doubleClicked, this, [this](const QModelIndex &index) {
        if (!index.isValid()) return;
        const QFileInfo info = m_model->fileInfo(index);
        if (info.isDir()) {
            navigateToPath(info.absoluteFilePath());
        }
    });
    


    // 右侧：预览/详情面板
    QWidget *rightPanel = new QWidget(mainSplitter);
    rightPanel->setMinimumWidth(250);  // 设置最小宽度
    rightPanel->setMaximumWidth(800);  // 增加最大宽度到800px，允许更大的预览区域
    rightPanel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    auto *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(6,6,6,6);

    m_stack = new QStackedWidget(rightPanel);
    rightLayout->addWidget(m_stack, 1);

    m_imageViewer = new ImageViewer(m_stack);
    m_textViewer = new TextPreviewer(m_stack);
    m_mediaViewer = new MediaViewer(m_stack);

#ifdef HAVE_QT_PDF_CORE
    m_pdfCoreViewer = new PdfSimpleViewer(m_stack);
#endif
#ifdef HAVE_QT_WEBENGINE
    m_officeWebViewer = new OfficeWebViewer(m_stack);
#endif

    // 详情面板 - 移除固定背景色，使用系统主题
    m_detailsPanel = new QWidget(m_stack);
    auto *detailsLayout = new QVBoxLayout(m_detailsPanel);
    detailsLayout->setAlignment(Qt::AlignTop);
    detailsLayout->setContentsMargins(15, 15, 15, 15);
    detailsLayout->setSpacing(5);
    
    // 大图标
    m_detailIcon = new QLabel(m_detailsPanel);
    m_detailIcon->setAlignment(Qt::AlignCenter);
    m_detailIcon->setMinimumHeight(150);
    detailsLayout->addWidget(m_detailIcon);
    
    m_infoLabel = new QLabel(tr("请选择文件进行预览"), m_stack);
    m_infoLabel->setAlignment(Qt::AlignCenter);

    m_stack->addWidget(m_infoLabel);
    m_stack->addWidget(m_imageViewer);
    m_stack->addWidget(m_textViewer);
    m_stack->addWidget(m_mediaViewer);
    m_stack->addWidget(m_detailsPanel);

#ifdef HAVE_QT_PDF_CORE
    m_stack->addWidget(m_pdfCoreViewer);
#endif
#ifdef HAVE_QT_WEBENGINE
    m_stack->addWidget(m_officeWebViewer);
#endif

    mainSplitter->addWidget(m_shortcuts);
    mainSplitter->addWidget(m_fileViewStack);
    mainSplitter->addWidget(rightPanel);
    
    // 设置各部分的拉伸因子
    mainSplitter->setStretchFactor(0, 0);  // 左侧快捷访问栏不拉伸
    mainSplitter->setStretchFactor(1, 3);  // 中间文件列表占据更多空间
    mainSplitter->setStretchFactor(2, 1);  // 右侧预览区域占据较少空间
    
    // 设置初始大小比例，避免出现空白区域
    QList<int> sizes;
    sizes << 180 << 630 << 270;  // 左侧180px，中间630px，右侧270px（完美尺寸）
    mainSplitter->setSizes(sizes);
    
    // 设置最小宽度，防止面板被完全隐藏
    mainSplitter->setChildrenCollapsible(false);  // 防止子部件被完全折叠

    // 初始化导航
    m_currentPath = rootPath;
    navigateToPath(rootPath);
}

void MainWindow::setupToolbar() {
    // 创建面包屑导航区域
    m_breadcrumbArea = new QScrollArea(this);
    m_breadcrumbArea->setMaximumHeight(50);
    m_breadcrumbArea->setWidgetResizable(true);
    m_breadcrumbArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_breadcrumbArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_breadcrumbArea->setFrameShape(QFrame::NoFrame);
    
    QWidget *toolbarWidget = new QWidget();
    auto *toolbarLayout = new QHBoxLayout(toolbarWidget);
    toolbarLayout->setContentsMargins(5, 5, 5, 5);
    
    // 后退按钮
    m_btnBack = new QToolButton(toolbarWidget);
    m_btnBack->setText("◀");
    m_btnBack->setToolTip(tr("后退"));
    connect(m_btnBack, &QToolButton::clicked, this, &MainWindow::goBack);
    toolbarLayout->addWidget(m_btnBack);
    
    // 前进按钮
    m_btnForward = new QToolButton(toolbarWidget);
    m_btnForward->setText("▶");
    m_btnForward->setToolTip(tr("前进"));
    connect(m_btnForward, &QToolButton::clicked, this, &MainWindow::goForward);
    toolbarLayout->addWidget(m_btnForward);
    
    // 向上按钮
    m_btnUp = new QToolButton(toolbarWidget);
    m_btnUp->setText("▲");
    m_btnUp->setToolTip(tr("上级目录"));
    connect(m_btnUp, &QToolButton::clicked, this, &MainWindow::goUp);
    toolbarLayout->addWidget(m_btnUp);
    
    // 地址栏切换按钮
    m_btnToggleAddressBar = new QToolButton(toolbarWidget);
    m_btnToggleAddressBar->setText("|||");
    m_btnToggleAddressBar->setToolTip(tr("切换地址栏模式"));
    connect(m_btnToggleAddressBar, &QToolButton::clicked, this, &MainWindow::toggleAddressBarMode);
    toolbarLayout->addWidget(m_btnToggleAddressBar);
    
    // 缩略图开关按钮 - 移动到导航按钮组后面
    m_btnToggleThumbnail = new QToolButton(toolbarWidget);
    m_btnToggleThumbnail->setText("○");  // 默认关闭状态显示空心圆
    m_btnToggleThumbnail->setToolTip(tr("开启缩略图显示"));
    m_btnToggleThumbnail->setCheckable(true);
    m_btnToggleThumbnail->setChecked(m_showThumbnails);
    connect(m_btnToggleThumbnail, &QToolButton::clicked, this, &MainWindow::toggleThumbnailMode);
    toolbarLayout->addWidget(m_btnToggleThumbnail);
    
    // 创建地址栏堆叠容器
    m_addressStack = new QStackedWidget(toolbarWidget);
    
    // 面包屑导航
    m_breadcrumbWidget = new QWidget();
    m_breadcrumbLayout = new QHBoxLayout(m_breadcrumbWidget);
    m_breadcrumbLayout->setContentsMargins(0, 0, 0, 0);
    m_breadcrumbLayout->setSpacing(0);
    m_addressStack->addWidget(m_breadcrumbWidget);
    
    // 完整路径文本框
    m_addressBar = new QLineEdit();
    m_addressBar->setPlaceholderText(tr("输入路径..."));
    m_addressBar->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_addressBar, &QLineEdit::returnPressed, this, &MainWindow::onAddressBarTextChanged);
    connect(m_addressBar, &QLineEdit::customContextMenuRequested, this, &MainWindow::showAddressBarContextMenu);
    m_addressStack->addWidget(m_addressBar);
    
    // 默认显示面包屑导航
    m_addressStack->setCurrentWidget(m_breadcrumbWidget);
    m_showFullPath = false;
    
    toolbarLayout->addWidget(m_addressStack, 1);
    
    m_breadcrumbArea->setWidget(toolbarWidget);
    
    updateNavigationButtons();
}

void MainWindow::navigateToPath(const QString &path) {
    if (!QDir(path).exists()) return;
    
    // 停止任何正在播放的媒体
    if (m_mediaViewer) {
        m_mediaViewer->stop();
    }
    
    // 更新历史记录
    if (m_historyIndex >= 0 && m_historyIndex < m_history.size() - 1) {
        m_history = m_history.mid(0, m_historyIndex + 1);
    }
    if (m_history.isEmpty() || m_history.last() != path) {
        m_history.append(path);
        m_historyIndex = m_history.size() - 1;
    }
    
    m_currentPath = path;
    
    // 先禁用排序，然后更新模型根路径，最后重新启用排序
    m_tableView->setSortingEnabled(false);
    m_treeView->setSortingEnabled(false);
    
    // 关键修复：更新模型的根路径
    m_model->setRootPath(path);
    QModelIndex currentIndex = m_model->index(path);
    m_tableView->setRootIndex(currentIndex);
    m_listView->setRootIndex(currentIndex);
    m_treeView->setRootIndex(currentIndex);
    
    // 重新应用排序设置以确保排序功能正常工作
    applySortingSettings();
    
    updateNavigationButtons();
    updateBreadcrumb();
    
    // 更新状态栏
    int fileCount = countFilesInDirectory(path);
    statusBar()->showMessage(tr("当前目录: %1  |  %2 项").arg(path).arg(fileCount));
}

void MainWindow::goBack() {
    if (m_historyIndex > 0) {
        m_historyIndex--;
        m_currentPath = m_history[m_historyIndex];
        
        // 先禁用排序
        m_tableView->setSortingEnabled(false);
        m_treeView->setSortingEnabled(false);
        
        // 关键修复：更新模型的根路径
        m_model->setRootPath(m_currentPath);
        QModelIndex currentIndex = m_model->index(m_currentPath);
        m_tableView->setRootIndex(currentIndex);
        m_listView->setRootIndex(currentIndex);
        m_treeView->setRootIndex(currentIndex);
        
        // 重新应用排序设置
        applySortingSettings();
        
        updateNavigationButtons();
        updateBreadcrumb();
        int fileCount = countFilesInDirectory(m_currentPath);
        statusBar()->showMessage(tr("当前目录: %1  |  %2 项").arg(m_currentPath).arg(fileCount));
    }
}

void MainWindow::goForward() {
    if (m_historyIndex < m_history.size() - 1) {
        m_historyIndex++;
        m_currentPath = m_history[m_historyIndex];
        
        // 先禁用排序
        m_tableView->setSortingEnabled(false);
        m_treeView->setSortingEnabled(false);
        
        // 关键修复：更新模型的根路径
        m_model->setRootPath(m_currentPath);
        QModelIndex currentIndex = m_model->index(m_currentPath);
        m_tableView->setRootIndex(currentIndex);
        m_listView->setRootIndex(currentIndex);
        m_treeView->setRootIndex(currentIndex);
        
        // 重新应用排序设置
        applySortingSettings();
        
        updateNavigationButtons();
        updateBreadcrumb();
        int fileCount = countFilesInDirectory(m_currentPath);
        statusBar()->showMessage(tr("当前目录: %1  |  %2 项").arg(m_currentPath).arg(fileCount));
    }
}

void MainWindow::goUp() {
    QDir dir(m_currentPath);
    if (dir.cdUp()) {
        navigateToPath(dir.absolutePath());
    }
}

void MainWindow::updateNavigationButtons() {
    m_btnBack->setEnabled(m_historyIndex > 0);
    m_btnForward->setEnabled(m_historyIndex < m_history.size() - 1);
    m_btnUp->setEnabled(m_currentPath != QDir::rootPath());
}

void MainWindow::updateBreadcrumb() {
    // 更新地址栏文本（如果当前是完整路径模式）
    if (m_showFullPath && m_addressBar) {
        m_addressBar->setText(m_currentPath);
    }
    
    // 清空旧的面包屑
    QLayoutItem *item;
    while ((item = m_breadcrumbLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    
    // 构建路径部分
    QStringList parts;
    QString path = m_currentPath;
    
#ifdef Q_OS_WIN
    parts = path.split("/", Qt::SkipEmptyParts);
#else
    if (path == "/") {
        parts << "/";
    } else {
        parts = path.split("/", Qt::SkipEmptyParts);
        parts.prepend("/");
    }
#endif
    
    // 路径压缩逻辑：当路径层级过多时进行智能压缩
    const int maxVisibleParts = 5; // 最多显示5个路径段
    bool needCompression = parts.size() > maxVisibleParts;
    
    QStringList displayParts;
    QStringList fullPaths; // 对应的完整路径
    
    if (needCompression) {
        // 压缩策略：显示根目录 + "..." + 最后3级目录
        displayParts.append(parts[0]); // 根目录
        fullPaths.append(parts[0]);
        
        if (parts.size() > 4) {
            displayParts.append("..."); // 省略标记
            fullPaths.append(""); // 空路径表示这是省略标记
        }
        
        // 添加最后3级目录
        int startIndex = qMax(1, parts.size() - 3);
        for (int i = startIndex; i < parts.size(); ++i) {
            displayParts.append(parts[i]);
            fullPaths.append(""); // 稍后计算完整路径
        }
    } else {
        displayParts = parts;
        fullPaths.resize(parts.size());
    }
    
    // 构建面包屑导航
    QString accumulatedPath;
    int originalIndex = 0; // 在原始parts中的索引
    
    for (int i = 0; i < displayParts.size(); ++i) {
        if (i > 0) {
            auto *separator = new QLabel(" > ", m_breadcrumbWidget);
            separator->setStyleSheet("QLabel { color: #888; }");
            m_breadcrumbLayout->addWidget(separator);
        }
        
        const QString part = displayParts[i];
        
        if (part == "...") {
            // 省略标记 - 创建一个特殊的按钮
            auto *ellipsisBtn = new QPushButton("...", m_breadcrumbWidget);
            ellipsisBtn->setFlat(true);
            ellipsisBtn->setStyleSheet("QPushButton { color: #666; font-weight: bold; }");
            ellipsisBtn->setToolTip(tr("点击显示完整路径\n完整路径: %1").arg(m_currentPath));
            
            // 点击省略号切换到完整路径模式
            connect(ellipsisBtn, &QPushButton::clicked, this, [this]() {
                toggleAddressBarMode();
            });
            
            m_breadcrumbLayout->addWidget(ellipsisBtn);
            
            // 跳过中间的路径段
            if (needCompression && parts.size() > 4) {
                originalIndex = parts.size() - 3;
                // 重新计算累积路径
                accumulatedPath = "";
                for (int j = 0; j < originalIndex; ++j) {
                    if (parts[j] == "/") {
                        accumulatedPath = "/";
                    } else {
                        if (accumulatedPath == "/") {
                            accumulatedPath += parts[j];
                        } else {
                            accumulatedPath += "/" + parts[j];
                        }
                    }
                }
            }
            continue;
        }
        
        // 计算当前路径段的完整路径
        if (part == "/") {
            accumulatedPath = "/";
        } else {
            if (accumulatedPath == "/") {
                accumulatedPath += part;
            } else {
                accumulatedPath += "/" + part;
            }
        }
        
        // 创建路径按钮
        auto *btn = new QPushButton(part, m_breadcrumbWidget);
        btn->setFlat(true);
        btn->setCursor(Qt::PointingHandCursor);
        
        // 设置样式
        if (i == displayParts.size() - 1) {
            // 当前目录 - 高亮显示
            btn->setStyleSheet("QPushButton { font-weight: bold; color: #2196F3; }");
        } else {
            btn->setStyleSheet("QPushButton:hover { background-color: #E3F2FD; }");
        }
        
        // 设置工具提示
        btn->setToolTip(tr("导航到: %1").arg(accumulatedPath));
        
        const QString targetPath = accumulatedPath;
        connect(btn, &QPushButton::clicked, this, [this, targetPath]() {
            navigateToPath(targetPath);
        });
        
        m_breadcrumbLayout->addWidget(btn);
        originalIndex++;
    }
    
    m_breadcrumbLayout->addStretch();
}

void MainWindow::onItemClicked(const QModelIndex &index) {
    if (!index.isValid()) return;
    const QFileInfo info = m_model->fileInfo(index);
    const QString path = info.absoluteFilePath();
    
    if (info.isDir()) {
        showFileDetails(path);
        return;
    }

    if (isImageFile(path)) {
        showImage(path);
        statusBar()->showMessage(tr("预览图片: %1").arg(path));
        return;
    }
    
    if (isAudioFile(path) || isVideoFile(path)) {
        showMedia(path);
        statusBar()->showMessage(tr("播放媒体: %1").arg(path));
        return;
    }
    
#ifdef HAVE_QT_PDF_CORE
    if (isPdfFile(path)) {
        showPdf(path);
        statusBar()->showMessage(tr("预览PDF: %1").arg(path));
        return;
    }
#endif
    // 办公文件预览 - 改为显示文件详情
    if (isOfficeFile(path)) {
        showFileDetails(path);
        QFileInfo fileInfo(path);
        QString fileType = fileInfo.suffix().toUpper();
        statusBar()->showMessage(tr("办公文档 (%1): %2").arg(fileType, path));
        return;
    }
    if (isTextLikeFile(path)) {
        showText(path);
        statusBar()->showMessage(tr("预览文本: %1").arg(path));
        return;
    }
    
    // 不支持预览的文件，显示文件详情
    showFileDetails(path);
    statusBar()->showMessage(tr("暂不支持此类型预览，显示文件详情: %1").arg(path));
}

void MainWindow::showImage(const QString &path) {
    // 停止任何正在播放的媒体
    if (m_mediaViewer) {
        m_mediaViewer->stop();
    }
    
    const bool ok = m_imageViewer->loadImage(path);
    if (ok) {
        m_stack->setCurrentWidget(m_imageViewer);
    } else {
        showInfo(tr("无法打开图片: %1").arg(path));
    }
}

void MainWindow::showText(const QString &path) {
    // 停止任何正在播放的媒体
    if (m_mediaViewer) {
        m_mediaViewer->stop();
    }
    
    if (m_textViewer->loadText(path)) {
        m_stack->setCurrentWidget(m_textViewer);
    } else {
        showInfo(tr("无法打开文本: %1").arg(path));
    }
}

void MainWindow::showMedia(const QString &path) {
    if (m_mediaViewer->loadMedia(path)) {
        m_stack->setCurrentWidget(m_mediaViewer);
    } else {
        showInfo(tr("无法打开媒体文件: %1").arg(path));
    }
}

#ifdef HAVE_QT_PDF_CORE
void MainWindow::showPdf(const QString &path) {
    // 停止任何正在播放的媒体
    if (m_mediaViewer) {
        m_mediaViewer->stop();
    }
    
    if (m_pdfCoreViewer->loadPdf(path)) {
        m_stack->setCurrentWidget(m_pdfCoreViewer);
    } else {
        showInfo(tr("无法打开 PDF: %1").arg(path));
    }
}
#endif

void MainWindow::showInfo(const QString &message) {
    // 停止任何正在播放的媒体
    if (m_mediaViewer) {
        m_mediaViewer->stop();
    }
    
    m_infoLabel->setText(message);
    m_stack->setCurrentWidget(m_infoLabel);
}

void MainWindow::onShortcutClicked(const QString &path) {
    if (QDir(path).exists()) {
        navigateToPath(path);
    }
}

QString MainWindow::formatFileSize(qint64 size) {
    if (size < 1024) return QString::number(size) + " B";
    if (size < 1024 * 1024) return QString::number(size / 1024.0, 'f', 2) + " KB";
    if (size < 1024 * 1024 * 1024) return QString::number(size / (1024.0 * 1024.0), 'f', 2) + " MB";
    return QString::number(size / (1024.0 * 1024.0 * 1024.0), 'f', 2) + " GB";
}

int MainWindow::countFilesInDirectory(const QString &path) {
    QDir dir(path);
    return dir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot).count();
}

void MainWindow::showFileDetails(const QString &path) {
    // 停止任何正在播放的媒体
    if (m_mediaViewer) {
        m_mediaViewer->stop();
    }
    
    QFileInfo info(path);
    
    // 清空之前的详情（除了图标）
    QLayoutItem *item;
    while (m_detailsPanel->layout()->count() > 1) {
        item = m_detailsPanel->layout()->takeAt(1);
        if (item) {
            delete item->widget();
            delete item;
        }
    }
    
    auto *layout = qobject_cast<QVBoxLayout*>(m_detailsPanel->layout());
    if (!layout) return;
    
    // 更新大图标
    // 检测是否为深色主题
    QPalette palette = this->palette();
    bool isDarkTheme = palette.color(QPalette::Window).lightness() < 128;
    
    if (info.isDir()) {
        // 文件夹图标 - 使用系统图标
        QIcon folderIcon = m_model->fileIcon(m_model->index(path));
        if (!folderIcon.isNull()) {
            m_detailIcon->setPixmap(folderIcon.pixmap(120, 120));
        } else {
            folderIcon = style()->standardIcon(QStyle::SP_DirIcon);
            m_detailIcon->setPixmap(folderIcon.pixmap(120, 120));
        }
        // 根据主题设置背景
        if (isDarkTheme) {
            m_detailIcon->setStyleSheet("QLabel { background-color: rgba(255, 255, 255, 0.05); border-radius: 8px; padding: 20px; }");
        } else {
            m_detailIcon->setStyleSheet("QLabel { background-color: #E3F2FD; border-radius: 8px; padding: 20px; }");
        }
    } else if (isImageFile(path)) {
        // 图片文件：优先显示缩略图
        QPixmap pixmap(path);
        if (!pixmap.isNull()) {
            m_detailIcon->setPixmap(pixmap.scaled(150, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            if (isDarkTheme) {
                m_detailIcon->setStyleSheet("QLabel { background-color: rgba(255, 255, 255, 0.05); border: 1px solid rgba(255, 255, 255, 0.1); border-radius: 8px; padding: 10px; }");
            } else {
                m_detailIcon->setStyleSheet("QLabel { background-color: #FAFAFA; border: 2px solid #E0E0E0; border-radius: 8px; padding: 10px; }");
            }
        } else {
            // 图片加载失败，使用系统图标
            QIcon fileIcon = m_model->fileIcon(m_model->index(path));
            m_detailIcon->setPixmap(fileIcon.pixmap(120, 120));
            if (isDarkTheme) {
                m_detailIcon->setStyleSheet("QLabel { background-color: rgba(255, 255, 255, 0.05); border-radius: 8px; padding: 20px; }");
            } else {
                m_detailIcon->setStyleSheet("QLabel { background-color: #FFF3E0; border-radius: 8px; padding: 20px; }");
            }
        }
    } else {
        // 其他文件类型：使用系统提供的文件图标
        QIcon fileIcon = m_model->fileIcon(m_model->index(path));
        
        if (!fileIcon.isNull()) {
            // 使用系统图标
            m_detailIcon->setPixmap(fileIcon.pixmap(120, 120));
        } else {
            // 降级方案：使用我们自定义的图标
            QString iconPath;
            QString ext = info.suffix().toLower();
            
            // 压缩文件
            if (QStringList{"zip", "rar", "7z", "tar", "gz", "bz2", "xz"}.contains(ext)) {
                iconPath = ":/icons/icons/file-archive.svg";
            }
            // 代码文件
            else if (QStringList{"cpp", "c", "h", "hpp", "py", "js", "ts", "java", "go", "rs", "sh", "bat"}.contains(ext)) {
                iconPath = ":/icons/icons/file-code.svg";
            }
            // 音频文件
            else if (isAudioFile(path)) {
                iconPath = ":/icons/icons/music.svg";
            }
            // 视频文件
            else if (isVideoFile(path)) {
                iconPath = ":/icons/icons/video.svg";
            }
            // 文档文件
            else if (QStringList{"pdf", "doc", "docx", "txt", "md", "odt", "xls", "xlsx", "ppt", "pptx", "ods", "odp", "rtf"}.contains(ext)) {
                iconPath = ":/icons/icons/documents.svg";
            }
            // 未知类型
            else {
                iconPath = ":/icons/icons/file-unknown.svg";
            }
            
            QIcon customIcon(iconPath);
            if (!customIcon.isNull()) {
                m_detailIcon->setPixmap(customIcon.pixmap(120, 120));
            } else {
                fileIcon = style()->standardIcon(QStyle::SP_FileIcon);
                m_detailIcon->setPixmap(fileIcon.pixmap(120, 120));
            }
        }
        
        // 根据文件类型和主题设置背景色
        QString bgColor;
        QString ext = info.suffix().toLower();
        
        if (isDarkTheme) {
            // 深色主题：使用半透明白色
            bgColor = "rgba(255, 255, 255, 0.05)";
        } else {
            // 浅色主题：使用彩色背景
            if (QStringList{"zip", "rar", "7z", "tar", "gz", "bz2", "xz", "deb", "rpm"}.contains(ext)) {
            bgColor = "#FFE5CC";  // 橙色 - 压缩/安装包
        } else if (QStringList{"cpp", "c", "h", "hpp", "py", "js", "ts", "java", "go", "rs", "sh", "bat"}.contains(ext)) {
            bgColor = "#D6EAF8";  // 蓝色 - 代码
        } else if (isAudioFile(path)) {
            bgColor = "#E8DAEF";  // 紫色 - 音频
        } else if (isVideoFile(path)) {
            bgColor = "#FADBD8";  // 红色 - 视频
        } else if (QStringList{"pdf", "doc", "docx", "txt", "md", "odt", "xls", "xlsx", "ppt", "pptx", "ods", "odp", "rtf"}.contains(ext)) {
                bgColor = "#FCF3CF";  // 黄色 - 文档
            } else if (QStringList{"exe", "msi", "app", "dmg"}.contains(ext)) {
                bgColor = "#D5F4E6";  // 绿色 - 可执行文件
            } else {
                bgColor = "#F2F3F4";  // 灰色 - 未知
            }
        }
        
        m_detailIcon->setStyleSheet(QString("QLabel { background-color: %1; border-radius: 8px; padding: 20px; }").arg(bgColor));
    }
    
    // 添加分隔线 - 根据主题调整颜色
    auto *separator = new QFrame(m_detailsPanel);
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    if (isDarkTheme) {
        separator->setStyleSheet("QFrame { color: rgba(255, 255, 255, 0.1); margin: 10px 0; }");
    } else {
        separator->setStyleSheet("QFrame { color: #E0E0E0; margin: 10px 0; }");
    }
    layout->addWidget(separator);
    
    // 文件信息 - 根据主题调整颜色
    auto addInfo = [layout, this, isDarkTheme](const QString &label, const QString &value, const QString &lightColor = "#555") {
        auto *container = new QWidget(m_detailsPanel);
        auto *hbox = new QHBoxLayout(container);
        hbox->setContentsMargins(10, 8, 10, 8);
        hbox->setSpacing(15);
        
        auto *labelWidget = new QLabel(label, container);
        QString labelColor = isDarkTheme ? "rgba(255, 255, 255, 0.6)" : "#888";
        labelWidget->setStyleSheet(QString("QLabel { color: %1; font-size: 13px; }").arg(labelColor));
        labelWidget->setMinimumWidth(70);
        labelWidget->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        
        auto *valueWidget = new QLabel(value, container);
        QString valueColor = isDarkTheme ? "rgba(255, 255, 255, 0.9)" : lightColor;
        valueWidget->setStyleSheet(QString("QLabel { color: %1; font-size: 13px; font-weight: 500; }").arg(valueColor));
        valueWidget->setWordWrap(true);
        valueWidget->setTextInteractionFlags(Qt::TextSelectableByMouse);
        
        hbox->addWidget(labelWidget);
        hbox->addWidget(valueWidget, 1);
        
        // 添加背景色交替效果
        static int rowCount = 0;
        if (rowCount % 2 == 0) {
            QString bgColor = isDarkTheme ? "rgba(255, 255, 255, 0.03)" : "#F9F9F9";
            container->setStyleSheet(QString("QWidget { background-color: %1; border-radius: 4px; }").arg(bgColor));
        }
        rowCount++;
        
        layout->addWidget(container);
    };
    
    // 文件名（加粗显示）
    addInfo(tr("名称"), info.fileName(), "#2C3E50");
    
    // 类型
    QString typeStr;
    if (info.isDir()) {
        typeStr = tr("目录");
    } else if (info.suffix().isEmpty()) {
        typeStr = tr("文件");
    } else {
        typeStr = info.suffix().toUpper() + " " + tr("文件");
    }
    addInfo(tr("类型"), typeStr, "#3498DB");
    
    // 大小
    if (info.isDir()) {
        int itemCount = countFilesInDirectory(path);
        addInfo(tr("大小"), QString::number(itemCount) + " " + tr("项"), "#16A085");
    } else {
        addInfo(tr("大小"), formatFileSize(info.size()), "#16A085");
    }
    
    // 修改时间（更重要，放在前面）
    addInfo(tr("修改时间"), info.lastModified().toString("yyyy-MM-dd HH:mm:ss"), "#E67E22");
    
    // 访问时间
    addInfo(tr("访问时间"), info.lastRead().toString("yyyy-MM-dd HH:mm:ss"), "#95A5A6");
    
    // 路径（使用小字体）
    auto *pathContainer = new QWidget(m_detailsPanel);
    auto *pathLayout = new QVBoxLayout(pathContainer);
    pathLayout->setContentsMargins(10, 8, 10, 8);
    
    auto *pathLabel = new QLabel(tr("完整路径"), pathContainer);
    QString pathLabelColor = isDarkTheme ? "rgba(255, 255, 255, 0.6)" : "#888";
    pathLabel->setStyleSheet(QString("QLabel { color: %1; font-size: 12px; }").arg(pathLabelColor));
    
    auto *pathValue = new QLabel(info.absoluteFilePath(), pathContainer);
    QString pathBgColor = isDarkTheme ? "rgba(255, 255, 255, 0.05)" : "#F0F0F0";
    QString pathTextColor = isDarkTheme ? "rgba(255, 255, 255, 0.7)" : "#7F8C8D";
    pathValue->setStyleSheet(QString("QLabel { color: %1; font-size: 11px; background-color: %2; padding: 5px; border-radius: 3px; }").arg(pathTextColor, pathBgColor));
    pathValue->setWordWrap(true);
    pathValue->setTextInteractionFlags(Qt::TextSelectableByMouse);
    
    pathLayout->addWidget(pathLabel);
    pathLayout->addWidget(pathValue);
    layout->addWidget(pathContainer);
    
    layout->addStretch();
    
    m_stack->setCurrentWidget(m_detailsPanel);
}

void MainWindow::showContextMenu(const QPoint &pos) {
    QMenu contextMenu(tr("视图选项"), this);
    
    // 图标大小子菜单
    QMenu *iconSizeMenu = contextMenu.addMenu(tr("图标大小"));
    
    QAction *smallIcon = iconSizeMenu->addAction(tr("小 (16x16)"));
    connect(smallIcon, &QAction::triggered, this, [this]() { setIconSize(16); });
    smallIcon->setCheckable(true);
    
    QAction *mediumIcon = iconSizeMenu->addAction(tr("中 (24x24)"));
    connect(mediumIcon, &QAction::triggered, this, [this]() { setIconSize(24); });
    mediumIcon->setCheckable(true);
    
    QAction *largeIcon = iconSizeMenu->addAction(tr("大 (32x32)"));
    connect(largeIcon, &QAction::triggered, this, [this]() { setIconSize(32); });
    largeIcon->setCheckable(true);
    
    QAction *extraLargeIcon = iconSizeMenu->addAction(tr("特大 (48x48)"));
    connect(extraLargeIcon, &QAction::triggered, this, [this]() { setIconSize(48); });
    extraLargeIcon->setCheckable(true);
    
    // 根据当前视图设置默认选中的图标大小
    QWidget *currentView = m_fileViewStack->currentWidget();
    int currentSize = 24; // 默认大小
    if (currentView == m_listView) {
        // 图标视图默认选择特大 (48x48)
        currentSize = 48;
        extraLargeIcon->setChecked(true);
    } else if (currentView == m_tableView) {
        // 列表视图默认选择大 (32x32)
        currentSize = 32;
        largeIcon->setChecked(true);
    } else if (currentView == m_treeView) {
        // 树形视图默认选择中 (24x24)
        currentSize = 24;
        mediumIcon->setChecked(true);
    }
    
    // 设置其他选项的选中状态
    smallIcon->setChecked(currentSize == 16);
    if (currentSize != 48) extraLargeIcon->setChecked(false);
    if (currentSize != 32) largeIcon->setChecked(false);
    if (currentSize != 24) mediumIcon->setChecked(false);
    
    contextMenu.addSeparator();
    
    // 排列方式菜单
    QMenu *viewModeMenu = contextMenu.addMenu(tr("排列方式"));
    
    QAction *iconViewAction = viewModeMenu->addAction(tr("图标视图"));
    iconViewAction->setCheckable(true);
    iconViewAction->setChecked(m_fileViewStack->currentWidget() == m_listView);
    connect(iconViewAction, &QAction::triggered, this, [this]() { setViewMode("icon"); });
    
    QAction *tableViewAction = viewModeMenu->addAction(tr("列表视图"));
    tableViewAction->setCheckable(true);
    tableViewAction->setChecked(m_fileViewStack->currentWidget() == m_tableView);
    connect(tableViewAction, &QAction::triggered, this, [this]() { setViewMode("table"); });
    
    QAction *treeViewAction = viewModeMenu->addAction(tr("树形视图"));
    treeViewAction->setCheckable(true);
    treeViewAction->setChecked(m_fileViewStack->currentWidget() == m_treeView);
    connect(treeViewAction, &QAction::triggered, this, [this]() { setViewMode("tree"); });
    
    contextMenu.addSeparator();
    
    // 显示/隐藏列（一级菜单）
    QMenu *columnsMenu = contextMenu.addMenu(tr("显示队列"));
    
    QAction *showName = columnsMenu->addAction(tr("名称"));
    showName->setCheckable(true);
    showName->setChecked(!m_tableView->isColumnHidden(0));
    showName->setEnabled(false);  // 名称列不能隐藏
    
    QAction *showSize = columnsMenu->addAction(tr("大小"));
    showSize->setCheckable(true);
    showSize->setChecked(!m_tableView->isColumnHidden(1));
    connect(showSize, &QAction::toggled, this, [this](bool checked) { toggleColumn(1, checked); });
    
    QAction *showType = columnsMenu->addAction(tr("类型"));
    showType->setCheckable(true);
    showType->setChecked(!m_tableView->isColumnHidden(2));
    connect(showType, &QAction::toggled, this, [this](bool checked) { toggleColumn(2, checked); });
    
    QAction *showDate = columnsMenu->addAction(tr("修改日期"));
    showDate->setCheckable(true);
    showDate->setChecked(!m_tableView->isColumnHidden(3));
    connect(showDate, &QAction::toggled, this, [this](bool checked) { toggleColumn(3, checked); });
    
    contextMenu.addSeparator();
    
    // 行高调整 - 通过空格实现两端对齐，与四字菜单项保持一致
    QMenu *rowHeightMenu = contextMenu.addMenu(tr("行　　高"));  // 在"行"和"高"之间添加全角空格
    
    QAction *compactRow = rowHeightMenu->addAction(tr("紧凑"));
    connect(compactRow, &QAction::triggered, this, [this]() {
        m_tableView->verticalHeader()->setDefaultSectionSize(20);
    });
    
    QAction *normalRow = rowHeightMenu->addAction(tr("正常"));
    connect(normalRow, &QAction::triggered, this, [this]() {
        m_tableView->verticalHeader()->setDefaultSectionSize(30);
    });
    normalRow->setCheckable(true);
    normalRow->setChecked(true);
    
    QAction *comfortableRow = rowHeightMenu->addAction(tr("舒适"));
    connect(comfortableRow, &QAction::triggered, this, [this]() {
        m_tableView->verticalHeader()->setDefaultSectionSize(40);
    });
    
    QAction *spaciousRow = rowHeightMenu->addAction(tr("宽松"));
    connect(spaciousRow, &QAction::triggered, this, [this]() {
        m_tableView->verticalHeader()->setDefaultSectionSize(50);
    });
    
    // 关于 - 通过空格实现两端对齐，与四字菜单项保持一致
    QAction *aboutAction = contextMenu.addAction(tr("关　　于"));  // 在"关"和"于"之间添加全角空格
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAboutDialog);
    
    // 检测当前主题（深色/浅色）
    QPalette palette = this->palette();
    bool isDarkTheme = palette.color(QPalette::Window).lightness() < 128;
    
    // 根据主题设置菜单样式
    QString menuStyleSheet;
    if (isDarkTheme) {
        // 深色主题样式
        menuStyleSheet = 
            "QMenu {"
            "    background-color: #2b2b2b;"
            "    border: 1px solid #555555;"
            "    border-radius: 4px;"
            "    color: #ffffff;"
            "}"
            "QMenu::item {"
            "    padding: 8px 30px 8px 30px;"
            "    border: 1px solid transparent;"
            "    font-size: 14px;"
            "    color: #ffffff;"
            "}"
            "QMenu::item:selected {"
            "    background-color: #404040;"
            "    border-color: #606060;"
            "    color: #ffffff;"
            "}"
            "QMenu::separator {"
            "    height: 1px;"
            "    background-color: #555555;"
            "    margin: 4px 0px;"
            "}"
            "QMenu::right-arrow {"
            "    image: none;"
            "    border: none;"
            "    width: 0px;"
            "}"
            "QMenu::right-arrow:after {"
            "    content: '▶';"
            "    color: #cccccc;"
            "    font-size: 12px;"
            "}";
    } else {
        // 浅色主题样式
        menuStyleSheet = 
            "QMenu {"
            "    background-color: white;"
            "    border: 1px solid #CCCCCC;"
            "    border-radius: 4px;"
            "    color: #000000;"
            "}"
            "QMenu::item {"
            "    padding: 8px 30px 8px 30px;"
            "    border: 1px solid transparent;"
            "    font-size: 14px;"
            "    color: #000000;"
            "}"
            "QMenu::item:selected {"
            "    background-color: #E3F2FD;"
            "    border-color: #BBDEFB;"
            "    color: #000000;"
            "}"
            "QMenu::separator {"
            "    height: 1px;"
            "    background-color: #E0E0E0;"
            "    margin: 4px 0px;"
            "}"
            "QMenu::right-arrow {"
            "    image: none;"
            "    border: none;"
            "    width: 0px;"
            "}"
            "QMenu::right-arrow:after {"
            "    content: '▶';"
            "    color: #666666;"
            "    font-size: 12px;"
            "}";
    }
    
    contextMenu.setStyleSheet(menuStyleSheet);
    
    // 显示菜单 - 获取当前视图的视口
    if (currentView) {
        if (auto *tableView = qobject_cast<QTableView*>(currentView)) {
            contextMenu.exec(tableView->viewport()->mapToGlobal(pos));
        } else if (auto *listView = qobject_cast<QListView*>(currentView)) {
            contextMenu.exec(listView->viewport()->mapToGlobal(pos));
        } else if (auto *treeView = qobject_cast<QTreeView*>(currentView)) {
            contextMenu.exec(treeView->viewport()->mapToGlobal(pos));
        }
    }
}

void MainWindow::setIconSize(int size) {
    m_tableView->setIconSize(QSize(size, size));
    m_listView->setIconSize(QSize(size * 1.5, size * 1.5));  // 图标视图使用更大图标
    m_treeView->setIconSize(QSize(size - 4, size - 4));  // 树视图使用较小图标
    
    // 根据图标大小自动调整行高
    int rowHeight = size + 10;
    m_tableView->verticalHeader()->setDefaultSectionSize(rowHeight);
    
    // 调整图标视图的网格大小
    int gridSize = size * 3;
    m_listView->setGridSize(QSize(gridSize, gridSize + 20));
    
    statusBar()->showMessage(tr("图标大小已设置为 %1x%1").arg(size), 2000);
}

void MainWindow::toggleColumn(int column, bool visible) {
    m_tableView->setColumnHidden(column, !visible);
    m_treeView->setColumnHidden(column, !visible);
    
    QString columnName;
    switch (column) {
        case 1: columnName = tr("大小"); break;
        case 2: columnName = tr("类型"); break;
        case 3: columnName = tr("修改日期"); break;
        default: columnName = tr("列"); break;
    }
    
    statusBar()->showMessage(tr("%1列已%2").arg(columnName).arg(visible ? tr("显示") : tr("隐藏")), 2000);
}

void MainWindow::setViewMode(const QString &mode) {
    if (mode == "table") {
        m_fileViewStack->setCurrentWidget(m_tableView);
        setIconSize(32); // 列表视图默认使用大图标 (32x32)
        statusBar()->showMessage(tr("已切换到列表视图"), 2000);
    } else if (mode == "icon") {
        m_fileViewStack->setCurrentWidget(m_listView);
        setIconSize(48); // 图标视图默认使用特大图标 (48x48)
        statusBar()->showMessage(tr("已切换到图标视图"), 2000);
    } else if (mode == "tree") {
        m_fileViewStack->setCurrentWidget(m_treeView);
        setIconSize(24); // 树形视图默认使用中图标 (24x24)
        statusBar()->showMessage(tr("已切换到树形视图"), 2000);
    }
}

void MainWindow::refreshCurrentPath() {
    if (m_currentPath.isEmpty()) return;
    
    // 刷新文件系统模型
    QModelIndex currentIndex = m_model->index(m_currentPath);
    m_model->setRootPath("");  // 重置
    m_model->setRootPath(m_currentPath);
    m_tableView->setRootIndex(currentIndex);
    m_listView->setRootIndex(currentIndex);
    m_treeView->setRootIndex(currentIndex);
    
    // 重新应用排序设置
    applySortingSettings();
    
    statusBar()->showMessage(tr("已刷新: %1").arg(m_currentPath), 2000);
}

void MainWindow::toggleAddressBarMode() {
    m_showFullPath = !m_showFullPath;
    
    if (m_showFullPath) {
        // 切换到完整路径模式
        m_addressBar->setText(m_currentPath);
        m_addressStack->setCurrentWidget(m_addressBar);
        m_btnToggleAddressBar->setText("[ ]");
        m_btnToggleAddressBar->setToolTip(tr("切换到面包屑导航"));
        m_addressBar->selectAll();
        m_addressBar->setFocus();
    } else {
        // 切换到面包屑导航模式
        m_addressStack->setCurrentWidget(m_breadcrumbWidget);
        m_btnToggleAddressBar->setText("|||");
        m_btnToggleAddressBar->setToolTip(tr("切换到完整路径"));
        updateBreadcrumb();
    }
}

void MainWindow::onAddressBarTextChanged() {
    QString path = m_addressBar->text().trimmed();
    if (!path.isEmpty() && QDir(path).exists()) {
        navigateToPath(path);
        // 切换回面包屑模式
        m_showFullPath = false;
        toggleAddressBarMode();
    } else {
        // 路径无效，显示错误提示
        statusBar()->showMessage(tr("路径无效: %1").arg(path), 3000);
        m_addressBar->setText(m_currentPath);
    }
}

void MainWindow::toggleThumbnailMode() {
    m_showThumbnails = !m_showThumbnails;
    
    // 更新按钮状态和图标
    m_btnToggleThumbnail->setChecked(m_showThumbnails);
    if (m_showThumbnails) {
        m_btnToggleThumbnail->setText("◎");
        m_btnToggleThumbnail->setToolTip(tr("关闭缩略图显示"));
        statusBar()->showMessage(tr("缩略图显示已开启"), 2000);
    } else {
        m_btnToggleThumbnail->setText("○");
        m_btnToggleThumbnail->setToolTip(tr("开启缩略图显示"));
        statusBar()->showMessage(tr("缩略图显示已关闭"), 2000);
    }
    
    // 清空缩略图缓存
    if (m_iconProvider) {
        // 重新设置图标提供器以刷新显示
        m_model->setIconProvider(nullptr);
        m_model->setIconProvider(m_iconProvider);
    }
    
    // 刷新当前视图
    QModelIndex currentIndex = m_model->index(m_currentPath);
    m_tableView->setRootIndex(QModelIndex());
    m_listView->setRootIndex(QModelIndex());
    m_treeView->setRootIndex(QModelIndex());
    
    m_tableView->setRootIndex(currentIndex);
    m_listView->setRootIndex(currentIndex);
    m_treeView->setRootIndex(currentIndex);
}

void MainWindow::applySortingSettings() {
    // 使用 QTimer 延迟应用排序设置，确保模型完全加载后再设置
    QTimer::singleShot(0, this, [this]() {
        // 重新启用排序功能并应用默认排序
        m_tableView->setSortingEnabled(true);
        m_tableView->sortByColumn(0, Qt::AscendingOrder);
        
        m_treeView->setSortingEnabled(true);
        m_treeView->sortByColumn(0, Qt::AscendingOrder);
        
        // 确保排序指示器显示
        m_tableView->horizontalHeader()->setSortIndicatorShown(true);
        m_tableView->horizontalHeader()->setSectionsClickable(true);
        
        m_treeView->header()->setSortIndicatorShown(true);
        m_treeView->header()->setSectionsClickable(true);
        
        // 调试信息
        qDebug() << "Applied sorting settings (delayed) - TableView sorting enabled:" << m_tableView->isSortingEnabled();
        qDebug() << "Applied sorting settings (delayed) - TreeView sorting enabled:" << m_treeView->isSortingEnabled();
    });
}

void MainWindow::showAboutDialog() {
    QDialog *aboutDialog = new QDialog(this);
    aboutDialog->setWindowTitle(tr("关于文件管理器预览"));
    aboutDialog->setFixedSize(480, 540);
    aboutDialog->setStyleSheet("QDialog { background-color: #F5F5F5; }");
    
    auto *mainLayout = new QVBoxLayout(aboutDialog);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // 中间内容区域
    QWidget *contentWidget = new QWidget(aboutDialog);
    contentWidget->setStyleSheet("QWidget { background-color: #F5F5F5; }");
    auto *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(40, 50, 40, 20);
    contentLayout->setSpacing(20);
    contentLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    
    // Logo 区域
    QLabel *logoLabel = new QLabel(contentWidget);
    QPixmap logo(":/icons/icons/app-logo.svg");
    if (!logo.isNull()) {
        logoLabel->setPixmap(logo.scaled(140, 140, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        logoLabel->setFixedSize(140, 140);
        logoLabel->setStyleSheet(
            "QLabel { "
            "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #667eea, stop:1 #764ba2);"
            "  border-radius: 20px; "
            "}"
        );
    }
    logoLabel->setAlignment(Qt::AlignCenter);
    contentLayout->addWidget(logoLabel, 0, Qt::AlignHCenter);
    
    // 应用名称
    QLabel *appName = new QLabel(tr("文件管理器预览"), contentWidget);
    appName->setStyleSheet("QLabel { color: #333; font-size: 24px; font-weight: bold; }");
    appName->setAlignment(Qt::AlignCenter);
    contentLayout->addWidget(appName);
    
    // 版本号
    QLabel *version = new QLabel(tr("版本: 1.0.1"), contentWidget);
    version->setStyleSheet("QLabel { color: #888; font-size: 15px; }");
    version->setAlignment(Qt::AlignCenter);
    contentLayout->addWidget(version);
    
    // 技术信息
    QLabel *techInfo = new QLabel(tr("基于 Qt6 与 C++17 构建"), contentWidget);
    techInfo->setStyleSheet("QLabel { color: #888; font-size: 14px; }");
    techInfo->setAlignment(Qt::AlignCenter);
    contentLayout->addWidget(techInfo);
    
    // 版权信息
    QLabel *copyright = new QLabel(tr("版权所有 © 2025 克亮"), contentWidget);
    copyright->setStyleSheet("QLabel { color: #888; font-size: 14px; }");
    copyright->setAlignment(Qt::AlignCenter);
    contentLayout->addWidget(copyright);
    
    // 简介
    QLabel *description = new QLabel(
        tr("文件管理工具，支持多种格式预览。"),
        contentWidget
    );
    description->setWordWrap(true);
    description->setStyleSheet("QLabel { color: #666; font-size: 14px; }");
    description->setAlignment(Qt::AlignCenter);
    contentLayout->addWidget(description);
    
    mainLayout->addWidget(contentWidget, 1);
    
    // 底部按钮区域
    QWidget *buttonWidget = new QWidget(aboutDialog);
    buttonWidget->setStyleSheet("QWidget { background-color: #F5F5F5; }");
    auto *buttonLayout = new QHBoxLayout(buttonWidget);
    buttonLayout->setContentsMargins(40, 10, 40, 25);
    
    QPushButton *closeButton = new QPushButton(tr("确定"), buttonWidget);
    closeButton->setFixedSize(160, 44);
    closeButton->setCursor(Qt::PointingHandCursor);
    closeButton->setStyleSheet(
        "QPushButton { "
        "  background-color: white; "
        "  color: #667eea; "
        "  border: 2px solid #667eea; "
        "  border-radius: 8px; "
        "  font-size: 15px; "
        "  font-weight: bold; "
        "} "
        "QPushButton:hover { "
        "  background-color: #667eea; "
        "  color: white; "
        "} "
        "QPushButton:pressed { "
        "  background-color: #5568d3; "
        "  border-color: #5568d3; "
        "}"
    );
    connect(closeButton, &QPushButton::clicked, aboutDialog, &QDialog::accept);
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    buttonLayout->addStretch();
    
    mainLayout->addWidget(buttonWidget);
    
    aboutDialog->exec();
    aboutDialog->deleteLater();
}

void MainWindow::showAddressBarContextMenu(const QPoint &pos) {
    QMenu contextMenu(tr("编辑"), this);
    
    // 检测当前主题（深色/浅色）
    QPalette palette = this->palette();
    bool isDarkTheme = palette.color(QPalette::Window).lightness() < 128;
    
    // 获取地址栏的标准动作
    QAction *undoAction = m_addressBar->createStandardContextMenu()->actions().value(0);
    QAction *redoAction = m_addressBar->createStandardContextMenu()->actions().value(1);
    QAction *cutAction = m_addressBar->createStandardContextMenu()->actions().value(2);
    QAction *copyAction = m_addressBar->createStandardContextMenu()->actions().value(3);
    QAction *pasteAction = m_addressBar->createStandardContextMenu()->actions().value(4);
    QAction *deleteAction = m_addressBar->createStandardContextMenu()->actions().value(5);
    QAction *selectAllAction = m_addressBar->createStandardContextMenu()->actions().value(6);
    
    // 创建中文菜单项
    QAction *chineseUndo = contextMenu.addAction(tr("撤销"));
    chineseUndo->setEnabled(m_addressBar->isUndoAvailable());
    connect(chineseUndo, &QAction::triggered, m_addressBar, &QLineEdit::undo);
    
    QAction *chineseRedo = contextMenu.addAction(tr("重做"));
    chineseRedo->setEnabled(m_addressBar->isRedoAvailable());
    connect(chineseRedo, &QAction::triggered, m_addressBar, &QLineEdit::redo);
    
    contextMenu.addSeparator();
    
    QAction *chineseCut = contextMenu.addAction(tr("剪切"));
    chineseCut->setEnabled(m_addressBar->hasSelectedText());
    connect(chineseCut, &QAction::triggered, m_addressBar, &QLineEdit::cut);
    
    QAction *chineseCopy = contextMenu.addAction(tr("复制"));
    chineseCopy->setEnabled(m_addressBar->hasSelectedText());
    connect(chineseCopy, &QAction::triggered, m_addressBar, &QLineEdit::copy);
    
    QAction *chinesePaste = contextMenu.addAction(tr("粘贴"));
    chinesePaste->setEnabled(!QApplication::clipboard()->text().isEmpty());
    connect(chinesePaste, &QAction::triggered, m_addressBar, &QLineEdit::paste);
    
    contextMenu.addSeparator();
    
    QAction *chineseDelete = contextMenu.addAction(tr("删除"));
    chineseDelete->setEnabled(m_addressBar->hasSelectedText());
    connect(chineseDelete, &QAction::triggered, m_addressBar, &QLineEdit::del);
    
    contextMenu.addSeparator();
    
    QAction *chineseSelectAll = contextMenu.addAction(tr("全选"));
    chineseSelectAll->setEnabled(!m_addressBar->text().isEmpty());
    connect(chineseSelectAll, &QAction::triggered, m_addressBar, &QLineEdit::selectAll);
    
    // 根据主题设置菜单样式
    QString menuStyleSheet;
    if (isDarkTheme) {
        // 深色主题样式
        menuStyleSheet = 
            "QMenu {"
            "    background-color: #2b2b2b;"
            "    border: 1px solid #555555;"
            "    border-radius: 4px;"
            "    color: #ffffff;"
            "}"
            "QMenu::item {"
            "    padding: 8px 30px 8px 30px;"
            "    border: 1px solid transparent;"
            "    font-size: 14px;"
            "    color: #ffffff;"
            "}"
            "QMenu::item:selected {"
            "    background-color: #404040;"
            "    border-color: #606060;"
            "    color: #ffffff;"
            "}"
            "QMenu::separator {"
            "    height: 1px;"
            "    background-color: #555555;"
            "    margin: 4px 0px;"
            "}";
    } else {
        // 浅色主题样式
        menuStyleSheet = 
            "QMenu {"
            "    background-color: white;"
            "    border: 1px solid #CCCCCC;"
            "    border-radius: 4px;"
            "    color: #000000;"
            "}"
            "QMenu::item {"
            "    padding: 8px 30px 8px 30px;"
            "    border: 1px solid transparent;"
            "    font-size: 14px;"
            "    color: #000000;"
            "}"
            "QMenu::item:selected {"
            "    background-color: #E3F2FD;"
            "    border-color: #BBDEFB;"
            "    color: #000000;"
            "}"
            "QMenu::separator {"
            "    height: 1px;"
            "    background-color: #E0E0E0;"
            "    margin: 4px 0px;"
            "}";
    }
    
    contextMenu.setStyleSheet(menuStyleSheet);
    
    // 显示菜单
    contextMenu.exec(m_addressBar->mapToGlobal(pos));
}