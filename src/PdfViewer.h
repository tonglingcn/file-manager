#pragma once

#ifdef HAVE_QT_PDF
#include <QWidget>
#include <QtPdf/QPdfDocument>
#include <QtPdfWidgets/QPdfView>

class PdfViewer : public QWidget {
    Q_OBJECT
public:
    explicit PdfViewer(QWidget *parent = nullptr);
    bool loadPdf(const QString &path);

protected:
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void fitToWindow();

    QPdfDocument *m_doc {nullptr};
    QPdfView *m_view {nullptr};
    bool m_hasDoc {false};
    bool m_userZoomed {false};
};
#endif // HAVE_QT_PDF
