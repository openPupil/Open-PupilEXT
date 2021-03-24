
#include "ResizableRectItem.h"
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QGraphicsScene>
#include <QCursor>

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

    //setCursor(QCursor(Qt::ArrowCursor));
//    SizeVerCursor,
//    SizeHorCursor,
//    SizeBDiagCursor,
//    SizeFDiagCursor,
//    SizeAllCursor,


    // I don't use QRectF because its members can't be manipulated
    // directly.
    qreal left = rect().left();
    qreal top = rect().top();
    qreal right = left + rect().width();
    qreal bottom = top + rect().height();

    if (resizeDirections.horizontal == resizeDirections.Left) {
        left = event->pos().x() + horizontalDistance;
    } else if (resizeDirections.horizontal == resizeDirections.Right) {
        right = event->pos().x() + horizontalDistance;
    }

    if (resizeDirections.vertical == resizeDirections.Top) {
        top = event->pos().y() + verticalDistance;
    } else if (resizeDirections.vertical == resizeDirections.Bottom) {
        bottom = event->pos().y() + verticalDistance;
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