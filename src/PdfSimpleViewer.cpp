#include "PdfSimpleViewer.h"
#ifdef HAVE_QT_PDF_CORE
#include <QVBoxLayout>
#include <QImage>
#include <QPixmap>
#include <QSize>
#include <QResizeEvent>
#include <QWheelEvent>
#include <QtPdf/QPdfDocumentRenderOptions>

PdfSimpleViewer::PdfSimpleViewer(QWidget *parent) : QWidget(parent) {
    m_doc = new QPdfDocument(this);

    m_toolbar = new QToolBar(this);
    auto *actZoomIn = m_toolbar->addAction("+");
    auto *actZoomOut = m_toolbar->addAction("-");
    auto *actPrev = m_toolbar->addAction("<");
    auto *actNext = m_toolbar->addAction(">");
    m_actFit = m_toolbar->addAction("适应");
    m_actA4 = m_toolbar->addAction("A4");
    m_actFit->setCheckable(true);
    m_actA4->setCheckable(true);
    m_actFit->setChecked(true);
    m_actA4->setChecked(true);

    connect(actZoomIn, &QAction::triggered, this, &PdfSimpleViewer::zoomIn);
    connect(actZoomOut, &QAction::triggered, this, &PdfSimpleViewer::zoomOut);
    connect(actPrev, &QAction::triggered, this, &PdfSimpleViewer::prevPage);
    connect(actNext, &QAction::triggered, this, &PdfSimpleViewer::nextPage);
    connect(m_actFit, &QAction::toggled, this, [this](bool on){ m_fitToWindow = on; renderCurrent(); });
    connect(m_actA4, &QAction::toggled, this, [this](bool on){ m_useA4Aspect = on; if (m_fitToWindow) renderCurrent(); });

    m_canvas = new QLabel(this);
    m_canvas->setAlignment(Qt::AlignCenter);
    m_canvas->setMinimumSize(100,100);

    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0,0,0,0);
    lay->addWidget(m_toolbar);
    lay->addWidget(m_canvas, 1);
}

bool PdfSimpleViewer::loadPdf(const QString &path) {
    m_doc->load(path);
    if (m_doc->status() != QPdfDocument::Status::Ready) {
        m_canvas->setText(tr("无法加载 PDF"));
        return false;
    }
    m_page = 0;
    m_zoom = 1.0;
    m_fitToWindow = true;
    renderCurrent();
    return true;
}

void PdfSimpleViewer::renderCurrent() {
    if (!m_doc || m_doc->status() != QPdfDocument::Status::Ready) return;
    const QSizeF ptSize = m_doc->pagePointSize(m_page);
    if (!ptSize.isValid()) return;

    QSize pixelSize;
    if (m_fitToWindow) {
        const QSize area = m_canvas->size();
        if (area.width() < 10 || area.height() < 10) return;
        const qreal targetRatio = m_useA4Aspect ? (210.0/297.0) : (ptSize.width() / ptSize.height());
        int w = area.width();
        int h = static_cast<int>(w / targetRatio);
        if (h > area.height()) {
            h = area.height();
            w = static_cast<int>(h * targetRatio);
        }
        pixelSize = QSize(w, h);
    } else {
        // 72 points per inch; render at 96 DPI scaled by zoom
        const qreal dpi = 96.0 * m_zoom;
        const qreal scale = dpi / 72.0;
        pixelSize = QSize(qRound(ptSize.width() * scale), qRound(ptSize.height() * scale));
    }

    QPdfDocumentRenderOptions opts;
    QImage img = m_doc->render(m_page, pixelSize, opts);
    if (img.isNull()) {
        m_canvas->setText(tr("渲染失败"));
        return;
    }
    m_canvas->setPixmap(QPixmap::fromImage(img));
}

void PdfSimpleViewer::zoomIn() {
    m_fitToWindow = false;
    m_zoom *= 1.2;
    if (m_zoom > 8.0) m_zoom = 8.0;
    renderCurrent();
}

void PdfSimpleViewer::zoomOut() {
    m_fitToWindow = false;
    m_zoom /= 1.2;
    if (m_zoom < 0.2) m_zoom = 0.2;
    renderCurrent();
}

void PdfSimpleViewer::nextPage() {
    if (!m_doc) return;
    if (m_page + 1 < m_doc->pageCount()) {
        ++m_page;
        renderCurrent();
    }
}

void PdfSimpleViewer::prevPage() {
    if (!m_doc) return;
    if (m_page > 0) {
        --m_page;
        renderCurrent();
    }
}

void PdfSimpleViewer::resizeEvent(QResizeEvent *e) {
    QWidget::resizeEvent(e);
    if (m_fitToWindow) {
        renderCurrent();
    }
}

void PdfSimpleViewer::wheelEvent(QWheelEvent *e) {
    if (e->angleDelta().y() > 0) {
        zoomIn();
    } else if (e->angleDelta().y() < 0) {
        zoomOut();
    }
    e->accept();
}
#endif // HAVE_QT_PDF_CORE
