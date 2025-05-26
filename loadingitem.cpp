#include "loadingitem.h"

LoadingItem::LoadingItem()
{

}

QRectF LoadingItem::boundingRect() const {

    return QRectF(-25, -25, 50, 50);

}

void LoadingItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *){

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(QPen(Qt::gray, 4));

    QRectF rect(-20, -20, 40, 40);
    painter->drawArc(rect, 0 * 16, 120 * 16);

}
