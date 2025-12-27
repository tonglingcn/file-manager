#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QModelIndex>
#include <QFileSystemModel>
#include <QFileIconProvider>
#include <QHash>
#include <QIcon>

class QFileSystemModel;
class QTreeView;
class QListView;
class QStackedWidget;
class QLabel;
class ImageViewer;
class TextPreviewer;
class MediaViewer;
class QListWidget;
class QTableView;
class QToolBar;
class QToolButton;
class QLineEdit;
class QScrollArea;
class QHBoxLayout;
#ifdef HAVE_QT_PDF_CORE
class PdfSimpleViewer;
#endif
#ifdef HAVE_QT_WEBENGINE
class OfficeWebViewer;
#endif

// 自定义图标提供器，支持图片缩略图
class ThumbnailIconProvider : public QFileIconProvider {
public:
    ThumbnailIconProvider(bool *enableThumbnails) : m_enableThumbnails(enableThumbnails) {}
    QIcon icon(const QFileInfo &info) const override;
private:
    mutable QHash<QString, QIcon> m_thumbnailCache;
    bool *m_enableThumbnails;
    static bool isImageFile(const QString &filePath);
};

// 自定义文件系统模型，用于支持中文列标题和类型显示
class CustomFileSystemModel : public QFileSystemModel {
    Q_OBJECT
public:
    explicit CustomFileSystemModel(QObject *parent = nullptr) : QFileSystemModel(parent) {}
    
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            switch (section) {
                case 0: return tr("名称");
                case 1: return tr("大小");
                case 2: return tr("类型");
                case 3: return tr("修改日期");
                default: return QFileSystemModel::headerData(section, orientation, role);
            }
        }
        return QFileSystemModel::headerData(section, orientation, role);
    }
    
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        // 对于非类型和非日期列，使用默认实现以保持排序功能
        if (index.column() != 2 && index.column() != 3) {
            return QFileSystemModel::data(index, role);
        }
        
        // 对于类型列和日期列的非显示角色，返回原始数据以支持排序
        if (role != Qt::DisplayRole) {
            return QFileSystemModel::data(index, role);
        }
        
        // 处理类型列的中文显示
        if (index.column() == 2) {
            QFileInfo info = QFileSystemModel::fileInfo(index);
            if (info.isDir()) {
                return tr("目录");
            } else if (info.suffix().isEmpty()) {
                return tr("文件");
            } else {
                return info.suffix().toUpper() + " " + tr("文件");
            }
        }
        
        // 处理日期列的格式化显示（补零格式）
        if (index.column() == 3) {
            QFileInfo info = QFileSystemModel::fileInfo(index);
            QDateTime dateTime = info.lastModified();
            return dateTime.toString("yyyy/MM/dd HH:mm");
        }
        
        return QFileSystemModel::data(index, role);
    }
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onItemClicked(const QModelIndex &index);
    void onShortcutClicked(const QString &path);
    void goBack();
    void goForward();
    void goUp();
    void updateNavigationButtons();
    void updateBreadcrumb();
    void toggleAddressBarMode();
    void onAddressBarTextChanged();
    void toggleThumbnailMode();
    void showContextMenu(const QPoint &pos);
    void showAddressBarContextMenu(const QPoint &pos);
    void setIconSize(int size);
    void toggleColumn(int column, bool visible);
    void setViewMode(const QString &mode);
    void applySortingSettings();
    void refreshCurrentPath();
    void showAboutDialog();

private:
    void setupUI();
    void setupToolbar();
    void navigateToPath(const QString &path);
    void showImage(const QString &path);
    void showText(const QString &path);
    void showMedia(const QString &path);
#ifdef HAVE_QT_PDF_CORE
    void showPdf(const QString &path);
#endif
    void showInfo(const QString &message);
    void showFileDetails(const QString &path);
    QString formatFileSize(qint64 size);
    int countFilesInDirectory(const QString &path);

    CustomFileSystemModel *m_model {nullptr};
    ThumbnailIconProvider *m_iconProvider {nullptr};
    QListWidget *m_shortcuts {nullptr};
    QStackedWidget *m_fileViewStack {nullptr};
    QTableView *m_tableView {nullptr};
    QListView *m_listView {nullptr};
    QTreeView *m_treeView {nullptr};
    QStackedWidget *m_stack {nullptr};
    ImageViewer *m_imageViewer {nullptr};
    TextPreviewer *m_textViewer {nullptr};
    MediaViewer *m_mediaViewer {nullptr};
#ifdef HAVE_QT_PDF_CORE
    PdfSimpleViewer *m_pdfCoreViewer {nullptr};
#endif
#ifdef HAVE_QT_WEBENGINE
    OfficeWebViewer *m_officeWebViewer {nullptr};
#endif
    QLabel *m_infoLabel {nullptr};
    QWidget *m_detailsPanel {nullptr};
    QLabel *m_detailIcon {nullptr};
    
    // 导航相关
    QToolButton *m_btnBack {nullptr};
    QToolButton *m_btnForward {nullptr};
    QToolButton *m_btnUp {nullptr};
    QScrollArea *m_breadcrumbArea {nullptr};
    QWidget *m_breadcrumbWidget {nullptr};
    QHBoxLayout *m_breadcrumbLayout {nullptr};
    
    // 地址栏切换功能
    QToolButton *m_btnToggleAddressBar {nullptr};
    QLineEdit *m_addressBar {nullptr};
    QStackedWidget *m_addressStack {nullptr};
    bool m_showFullPath {false};
    
    // 缩略图开关功能
    QToolButton *m_btnToggleThumbnail {nullptr};
    bool m_showThumbnails {false};
    
    QStringList m_history;
    int m_historyIndex {-1};
    QString m_currentPath;
};

#endif // MAINWINDOW_H
