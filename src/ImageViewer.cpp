#include "ImageViewer.h"

#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QWheelEvent>
#include <QResizeEvent>
#include <QImageReader>
#include <QPixmap>

ImageViewer::ImageViewer(QWidget *parent) : QGraphicsView(parent) {
    m_scene = new QGraphicsScene(this);
    setScene(m_scene);

    m_pixItem = new QGraphicsPixmapItem();
    m_scene->addItem(m_pixItem);

    setBackgroundBrush(QColor(240, 240, 240));
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    setDragMode(QGraphicsView::ScrollHandDrag);
}

bool ImageViewer::loadImage(const QString &path) {
    QImageReader reader(path);
    reader.setAutoTransform(true);
    const QImage img = reader.read();
    if (img.isNull()) {
        m_hasImage = false;
        m_pixItem->setPixmap(QPixmap());
        return false;
    }
    m_pixItem->setPixmap(QPixmap::fromImage(img));
    m_hasImage = true;
    m_userZoomed = false;
    resetTransform();
    fitToWindow();
    return true;
}

void ImageViewer::fitToWindow() {
    if (!m_hasImage || m_pixItem->pixmap().isNull()) return;
    // Fit while keeping aspect ratio and fully filling the view without cropping
    QRectF bounds = m_pixItem->boundingRect();
    if (bounds.isEmpty()) return;
    // add small margins to avoid scrollbars due to rounding
    fitInView(bounds, Qt::KeepAspectRatio);
}

void ImageViewer::wheelEvent(QWheelEvent *event) {
    if (!m_hasImage) { QGraphicsView::wheelEvent(event); return; }
    const QPoint delta = event->angleDelta();
    if (delta.y() == 0) { event->accept(); return; }

    m_userZoomed = true;
    const double factor = (delta.y() > 0) ? 1.25 : 0.8;

    // Prevent over-zooming
    const double currentScaleX = transform().m11();
    const double currentScaleY = transform().m22();
    const double newScaleX = currentScaleX * factor;
    const double newScaleY = currentScaleY * factor;
    const double minScale = 0.05;
    const double maxScale = 50.0;
    if (newScaleX < minScale || newScaleY < minScale || newScaleX > maxScale || newScaleY > maxScale) {
        event->accept();
        return;
    }

    scale(factor, factor);
    event->accept();
}

void ImageViewer::resizeEvent(QResizeEvent *event) {
    QGraphicsView::resizeEvent(event);
    if (m_hasImage && !m_userZoomed) {
        resetTransform();
        fitToWindow();
    }
}
