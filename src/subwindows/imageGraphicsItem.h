#pragma once

/**
    @author Moritz Lode
*/

#include <QGraphicsItem>

/**
    Custom QGraphicsItem that handles the rendering of camera images on a QGraphicsView

    This should be faster than rendering it as a QLabel, we paint the QImage directly to the QGraphicsView canvas

    // TODO could be speed up using opengl painting methods?
*/
class ImageGraphicsItem : public QGraphicsItem {

public:

    explicit ImageGraphicsItem() : image() {

    }

    explicit ImageGraphicsItem(const QImage &img) : image(img) {

    }

    ~ImageGraphicsItem() = default;

    QRectF boundingRect() const override {
        return image.rect();
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override {
        painter->drawImage(image.rect(), image);
    }

private:

    QImage image;

};
