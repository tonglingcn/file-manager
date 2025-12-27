#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QGraphicsView>

class QGraphicsScene;
class QGraphicsPixmapItem;

class ImageViewer : public QGraphicsView {
    Q_OBJECT
public:
    explicit ImageViewer(QWidget *parent = nullptr);

    bool loadImage(const QString &path);

protected:
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void fitToWindow();

    QGraphicsScene *m_scene {nullptr};
    QGraphicsPixmapItem *m_pixItem {nullptr};
    bool m_hasImage {false};
    bool m_userZoomed {false};
};

#endif // IMAGEVIEWER_H
