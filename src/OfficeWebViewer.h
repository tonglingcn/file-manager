#ifndef OFFICEWEBVIEWER_H
#define OFFICEWEBVIEWER_H

#include <QWidget>

class QWebEngineView;
class QVBoxLayout;
class QLabel;
class QPushButton;

class OfficeWebViewer : public QWidget {
    Q_OBJECT

public:
    explicit OfficeWebViewer(QWidget *parent = nullptr);
    ~OfficeWebViewer();

    // 加载办公文档（自动转换为 HTML）
    bool loadDocument(const QString &filePath);

private:
    void setupUI();
    QString convertDocxToHtml(const QString &filePath);
    QString convertXlsxToHtml(const QString &filePath);
    QString generateErrorHtml(const QString &message);

    QWebEngineView *m_webView;
    QVBoxLayout *m_layout;
    QLabel *m_statusLabel;
    QString m_currentFile;
};

#endif // OFFICEWEBVIEWER_H
