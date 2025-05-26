#ifndef LOADINGITEM_H
#define LOADINGITEM_H

#include <QGraphicsItem>
#include <QPainter>

class LoadingItem : public QGraphicsItem
{
public:
    LoadingItem();

    QRectF boundingRect() const override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;

};

#endif // LOADINGITEM_H
