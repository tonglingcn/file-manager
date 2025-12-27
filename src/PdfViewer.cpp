#include "PdfViewer.h"
#ifdef HAVE_QT_PDF
#include <QWheelEvent>
#include <QVBoxLayout>

PdfViewer::PdfViewer(QWidget *parent) : QWidget(parent) {
    m_doc = new QPdfDocument(this);
    m_view = new QPdfView(this);
    m_view->setPageMode(QPdfView::SinglePage);
    m_view->setZoomMode(QPdfView::FitInView);

    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0,0,0,0);
    lay->addWidget(m_view);
}

bool PdfViewer::loadPdf(const QString &path) {
    const QPdfDocument::Status st = m_doc->load(path);
    if (st != QPdfDocument::Status::Ready) {
        m_hasDoc = false;
        m_view->setDocument(nullptr);
        return false;
    }
    m_view->setDocument(m_doc);
    m_view->setPage(0);
    m_hasDoc = true;
    m_userZoomed = false;
    fitToWindow();
    return true;
}

void PdfViewer::fitToWindow() {
    if (!m_hasDoc) return;
    m_view->setZoomMode(QPdfView::FitInView);
}

void PdfViewer::wheelEvent(QWheelEvent *event) {
    if (!m_hasDoc) { QWidget::wheelEvent(event); return; }
    const QPoint d = event->angleDelta();
    if (d.y() == 0) { event->accept(); return; }

    m_userZoomed = true;
    m_view->setZoomMode(QPdfView::Custom);

    const qreal current = m_view->zoomFactor();
    const qreal factor = (d.y() > 0) ? 1.2 : 1/1.2;
    qreal next = current * factor;
    const qreal minZ = 0.1;
    const qreal maxZ = 8.0;
    if (next < minZ) next = minZ; else if (next > maxZ) next = maxZ;
    m_view->setZoomFactor(next);
    event->accept();
}

void PdfViewer::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    if (m_hasDoc && !m_userZoomed) {
        fitToWindow();
    }
}
#endif // HAVE_QT_PDF
