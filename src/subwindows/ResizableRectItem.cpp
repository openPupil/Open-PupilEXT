
#include "ResizableRectItem.h"
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QGraphicsScene>
#include <QCursor>

#include <QtMath>
#include <iostream>

#include <QApplication>

static ResizeDirections resizeDirections;

// Horizontal and vertical distance from the cursor position at the time of
// mouse-click to the nearest respective side of the rectangle. Whether
// it's left or right, and top or bottom, depends on which side we'll be
// resizing. We use that to calculate the rectangle from the mouse position
// during the mouse move events.
static qreal horizontalDistance;
static qreal verticalDistance;


ResizableRectItem::ResizableRectItem(QRectF rect, QGraphicsItem *parent) : QGraphicsRectItem(parent) {

    setRect(rect);
    setAcceptHoverEvents(true);
}

ResizableRectItem::ResizableRectItem(QRectF rect, QSizeF aspectRatio, QGraphicsItem *parent) : QGraphicsRectItem(parent) {

    setAspectRatioLock(aspectRatio);

    setRect(rect);
    setAcceptHoverEvents(true);
}

void ResizableRectItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QRectF innerRect = getInnerRect();

    // Get the resize-directions.
    const QPointF &pos = event->pos();
    if (pos.x() < innerRect.left()) {
        resizeDirections.horizontal = resizeDirections.Left;
        horizontalDistance = rect().left() - pos.x();
        setCursor(QCursor(Qt::SizeHorCursor));
    } else if (pos.x() > innerRect.right()) {
        resizeDirections.horizontal = resizeDirections.Right;
        horizontalDistance = rect().right() - pos.x();
        setCursor(QCursor(Qt::SizeHorCursor));
    } else {
        resizeDirections.horizontal = resizeDirections.HorzNone;
    }

    if (pos.y() < innerRect.top()) {
        resizeDirections.vertical = resizeDirections.Top;
        verticalDistance = rect().top() - pos.y();
        setCursor(QCursor(Qt::SizeVerCursor));
    } else if (pos.y() > innerRect.bottom()) {
        resizeDirections.vertical = resizeDirections.Bottom;
        verticalDistance = rect().bottom() - pos.y();
        setCursor(QCursor(Qt::SizeVerCursor));
    } else {
        resizeDirections.vertical = resizeDirections.VertNone;
    }

    // GB begin
    //const QPointF &pos = event->pos();
    //QPointF pos = event->pos();
    Qt::KeyboardModifiers key = QApplication::queryKeyboardModifiers();
    if( ARLock.width()>0 && 
        ARLock.height()>0 && 
        key != Qt::AltModifier
        ) {

        if(resizeDirections.horizontal == resizeDirections.HorzNone && resizeDirections.vertical != resizeDirections.VertNone) {
            if(pos.x()>rect().center().x())
                resizeDirections.horizontal = resizeDirections.Right;
            else
                resizeDirections.horizontal = resizeDirections.Left;
        }

        if(resizeDirections.vertical == resizeDirections.VertNone && resizeDirections.horizontal != resizeDirections.HorzNone) {
            if(pos.y()>rect().center().y())
                resizeDirections.vertical = resizeDirections.Bottom;
            else
                resizeDirections.vertical = resizeDirections.Top;
        }
    }
    // GB end

    if(resizeDirections.horizontal != resizeDirections.HorzNone && resizeDirections.vertical != resizeDirections.VertNone) {
        if((pos.x() > rect().center().x() && pos.y() > rect().center().y()) || (pos.x() < rect().center().x() && pos.y() < rect().center().y()))
            setCursor(QCursor(Qt::SizeFDiagCursor));
        else
            setCursor(QCursor(Qt::SizeBDiagCursor));
    }


    // If not a resize event, pass it to base class so the move-event can be
    // implemented.
    if (!resizeDirections.any()) {
        setCursor(QCursor(Qt::SizeAllCursor));
        QGraphicsRectItem::mousePressEvent(event);
    }
}

void ResizableRectItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    // If not a resize event, pass it to base class so move event can be implemented.
    if (!resizeDirections.any()) {
        setCursor(QCursor(Qt::SizeAllCursor));

        QGraphicsRectItem::mouseMoveEvent(event);
        emit onChange();
    } else {
        resizeRect(event);
    }
}

void ResizableRectItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    QRectF innerRect = getInnerRect();

    const QPointF &pos = event->pos();

    bool horSet=false, vertSet=false;

    if(pos.x() < innerRect.left() && pos.x() > rect().left()) {
        setCursor(QCursor(Qt::SizeHorCursor));
        horSet = true;
    } else if(pos.x() > innerRect.right() && pos.x() < rect().right()) {
        setCursor(QCursor(Qt::SizeHorCursor));
        horSet = true;
    }

    //std::cout << pos.y() << " - " << innerRect.top() << " - " << rect().top() << std::endl;
    //std::cout << pos.y() << " - " << innerRect.bottom() << " - " << rect().bottom() << std::endl;
    if(pos.y() < innerRect.top() && pos.y() > rect().top()) {
        setCursor(QCursor(Qt::SizeVerCursor));
        vertSet = true;
    } else if(pos.y() > innerRect.bottom() && pos.y() < rect().bottom()) {
        setCursor(QCursor(Qt::SizeVerCursor));
        vertSet = true;
    }

    if(horSet && vertSet) {
        if((pos.x() > rect().center().x() && pos.y() > rect().center().y()) || (pos.x() < rect().center().x() && pos.y() < rect().center().y()))
            setCursor(QCursor(Qt::SizeFDiagCursor));
        else
            setCursor(QCursor(Qt::SizeBDiagCursor));
    }

    if(!horSet && !vertSet) {
        setCursor(QCursor(Qt::ArrowCursor));
    }

    QGraphicsRectItem::hoverMoveEvent(event);
}

void ResizableRectItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    setCursor(QCursor(Qt::ArrowCursor));
    // If not a resize event, pass it to base class so move event can be implemented.
    if (!resizeDirections.any()) {
        QGraphicsRectItem::mouseReleaseEvent(event);
    } else {
        resizeRect(event);
    }
}

void ResizableRectItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QGraphicsRectItem::paint(painter, option, widget);

    // We draw the inner-rect after main rect.
    // Drawing order matters if alpha-transparency is used.
    const QPen &oldPen = painter->pen();
    const QBrush &oldBrush = painter->brush();
    painter->setPen(Qt::DashLine);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(getInnerRect());
    painter->setPen(oldPen);
    painter->setBrush(oldBrush);
}

QRectF ResizableRectItem::getInnerRect() const
{
    qreal a = 16;
    return rect().adjusted(a, a, -a, -a);
}

QRectF ResizableRectItem::getRect() const
{
    return rect();
}

void ResizableRectItem::resizeRect(QGraphicsSceneMouseEvent *event)
{
    // I don't use QRectF because its members can't be manipulated
    // directly.
    qreal left = rect().left();
    qreal top = rect().top();
    qreal right = left + rect().width();
    qreal bottom = top + rect().height();

    // GB begin   
    //const QPointF &pos = event->pos();
    QPointF pos = event->pos();
    Qt::KeyboardModifiers key = QApplication::queryKeyboardModifiers();
    if(ARLock.width()>0 && ARLock.height()>0 && key != Qt::AltModifier) {

        float originalDeltaX;
        float originalDeltaY;
        if(resizeDirections.horizontal == resizeDirections.Right && resizeDirections.vertical == resizeDirections.Bottom) {
            originalDeltaX = pos.x() - rect().topLeft().x();
            originalDeltaY = pos.y() - rect().topLeft().y();
        } else if(resizeDirections.horizontal == resizeDirections.Left && resizeDirections.vertical == resizeDirections.Bottom) {
            originalDeltaX = rect().topRight().x() - pos.x();
            originalDeltaY = pos.y() - rect().topRight().y();
        } else if(resizeDirections.horizontal == resizeDirections.Right && resizeDirections.vertical == resizeDirections.Top) {
            originalDeltaX = pos.x() - rect().bottomLeft().x();
            originalDeltaY = rect().bottomLeft().y() - pos.y();
        } else if(resizeDirections.horizontal == resizeDirections.Left && resizeDirections.vertical == resizeDirections.Top) {
            originalDeltaX = rect().bottomRight().x() - pos.x();
            originalDeltaY = rect().bottomRight().y() - pos.y();
        }

        float modifiedDeltaX;
        float modifiedDeltaY;
        if(originalDeltaX > originalDeltaY) {
            modifiedDeltaX = originalDeltaX;
            modifiedDeltaY = ARLock.height()/ARLock.width() * originalDeltaX;
        }
        if(originalDeltaX <= originalDeltaY) {
            modifiedDeltaX = ARLock.width()/ARLock.height() * originalDeltaY;
            modifiedDeltaY = originalDeltaY;
        }

        if(resizeDirections.horizontal == resizeDirections.Right && resizeDirections.vertical == resizeDirections.Bottom) {
            pos = QPointF(rect().topLeft().x() + modifiedDeltaX, rect().topLeft().y() + modifiedDeltaY);
        } else if(resizeDirections.horizontal == resizeDirections.Left && resizeDirections.vertical == resizeDirections.Bottom) {
            pos = QPointF(rect().topRight().x() - modifiedDeltaX, rect().topRight().y() + modifiedDeltaY);
        } else if(resizeDirections.horizontal == resizeDirections.Right && resizeDirections.vertical == resizeDirections.Top) {
            pos = QPointF(rect().bottomLeft().x() + modifiedDeltaX, rect().bottomLeft().y() - modifiedDeltaY);
        } else if(resizeDirections.horizontal == resizeDirections.Left && resizeDirections.vertical == resizeDirections.Top) {
            pos = QPointF(rect().bottomRight().x() - modifiedDeltaX, rect().bottomRight().y() - modifiedDeltaY);
        }

        keepingAspectRatio = true;
    } else if(key==Qt::AltModifier) {
        keepingAspectRatio = false;
    }

    if( resizeDirections.horizontal == resizeDirections.Right && (pos.x() - rect().left()) < minSize.width() ) {
        //std::cout << "Too small to the left" << std::endl;
        return;
    }
    if( resizeDirections.horizontal == resizeDirections.Left && (rect().right() - pos.x()) < minSize.width() ) {
        //std::cout << "Too small to the right" << std::endl;
        return;
    }

    if( resizeDirections.vertical == resizeDirections.Bottom && (pos.y() - rect().top()) < minSize.height() ) {
        //std::cout << "Too small to the top" << std::endl;
        return;
    }
    if( resizeDirections.vertical == resizeDirections.Top && (rect().bottom() - pos.y()) < minSize.height() ) {
        //std::cout << "Too small to the bottom" << std::endl;
        return;
    }
    // GB end

    if (resizeDirections.horizontal == resizeDirections.Left) {
        left = pos.x() + horizontalDistance;
    } //else if (resizeDirections.horizontal == resizeDirections.Right) {
    if (resizeDirections.horizontal == resizeDirections.Right) {
        right = pos.x() + horizontalDistance;
    }

    if (resizeDirections.vertical == resizeDirections.Top) {
        top = pos.y() + verticalDistance;
    } //else if (resizeDirections.vertical == resizeDirections.Bottom) {
    if (resizeDirections.vertical == resizeDirections.Bottom) {
        bottom = pos.y() + verticalDistance;
    }

    QRectF newRect(left, top, right-left, bottom-top);
    //settings->validateRect(&newRect, resizeDirections);

    if (newRect != rect()) {
        // The documentation states this function should be called prior to any changes
        // in the geometry:
        // Prepares the item for a geometry change. Call this function before
        // changing the bounding rect of an item to keep QGraphicsScene's index
        // up to date.
        prepareGeometryChange();

        // For top and left resizing, we move the item's position in the
        // parent. This is because we want any child items it has to move along
        // with it, preserving their distance relative to the top-left corner
        // of the rectangle, because this is the most-expected behavior from a
        // user's point of view.
        // mapToParent() is needed for rotated rectangles.
        setPos(mapToParent(newRect.topLeft() - rect().topLeft()));
        newRect.translate(rect().topLeft() - newRect.topLeft());

        setRect(newRect);
        emit onChange();
    }
}


void ResizableRectItem::setMinSize(QSizeF size) {

    //if(size.x() >= 1)

    minSize = size;
}

/**
 * Sets position to 0 so new rect is in image space.
*/
void ResizableRectItem::setNormalizedRect(const QRectF rect){
    setPos(0.0f,0.0f);
    setRect(rect);
}
