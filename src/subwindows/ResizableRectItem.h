
#ifndef PUPILEXT_RESIZABLERECTITEM_H
#define PUPILEXT_RESIZABLERECTITEM_H

/**
    Source https://github.com/sashoalm/ResizableRectItem
    @author Alexander Almaleh, Moritz Lode (Updated 2020), Gábor Bényei (Updated 2023)
*/

#include <QtWidgets/qgraphicsitem.h>
#include <QtGui/qpen.h>

struct ResizeDirections
{
    enum { HorzNone, Left, Right } horizontal;
    enum { VertNone, Top, Bottom } vertical;
    bool any() { return horizontal || vertical; }
};

/**
    Custom widget that is rendered semi-translucent above a camera image and is resizable by the user

    Used to define a ROI selection by the user
*/
class ResizableRectItem : public QObject, public QGraphicsRectItem {
Q_OBJECT

public:

    explicit ResizableRectItem(QRectF rect, QGraphicsItem *parent=0);
    explicit ResizableRectItem(QRectF rect, QSizeF aspectRatio, QGraphicsItem *parent=0);

    QRectF getRect() const;

    // GB begin
    void setMinSize(QSizeF size);
    void setAspectRatioLock(QSizeF aspectRatio) {
        ARLock = aspectRatio;
    }
    // GB end

private:

    // GB begin
    QSizeF minSize = QSizeF(0,0);
    QSizeF ARLock = QSizeF(0,0);
    bool keepingAspectRatio = true;
    // GB end

    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent  *event) override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    QRectF getInnerRect() const;

    void resizeRect(QGraphicsSceneMouseEvent *event);

signals:

    void onChange();

};


#endif //PUPILEXT_RESIZABLERECTITEM_H
