#ifndef MAPNODEITEM_H
#define MAPNODEITEM_H

#include <QGraphicsEllipseItem>
#include <QGraphicsSceneMouseEvent>
#include "mapnode.h"

class MapNodeItem : public QObject, public QGraphicsEllipseItem
{
    Q_OBJECT
public:
    MapNodeItem(MapNode *node, qreal x, qreal y, qreal w, qreal h)
        : QGraphicsEllipseItem(x, y, w, h)
        , m_node(node)
    {
        setAcceptHoverEvents(true);
    }

    MapNode* mapNode() const { return m_node; }

signals:
    void clicked(MapNode *node);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override
    {
        if (m_node->available()) {
            emit clicked(m_node);
        }
        QGraphicsEllipseItem::mousePressEvent(event);
    }

private:
    MapNode *m_node;
};

#endif // MAPNODEITEM_H