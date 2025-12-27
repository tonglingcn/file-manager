#pragma once

#ifdef HAVE_QT_PDF_CORE
#include <QWidget>
#include <QtPdf/QPdfDocument>
#include <QLabel>
#include <QToolBar>
#include <QAction>

class PdfSimpleViewer : public QWidget {
    Q_OBJECT
public:
    explicit PdfSimpleViewer(QWidget *parent = nullptr);
    bool loadPdf(const QString &path);

private slots:
    void zoomIn();
    void zoomOut();
    void nextPage();
    void prevPage();

protected:
    void resizeEvent(QResizeEvent*) override;
    void wheelEvent(QWheelEvent*) override;

private:
    void renderCurrent();

    QPdfDocument *m_doc {nullptr};
    QLabel *m_canvas {nullptr};
    QToolBar *m_toolbar {nullptr};

    int m_page {0};
    qreal m_zoom {1.0};
    bool m_fitToWindow {true};
    bool m_useA4Aspect {true};
    QAction *m_actFit {nullptr};
    QAction *m_actA4 {nullptr};
};
#endif // HAVE_QT_PDF_CORE
