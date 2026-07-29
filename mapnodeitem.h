#ifndef MAPNODEITEM_H
#define MAPNODEITEM_H

#include <QGraphicsEllipseItem>
#include <QGraphicsSceneMouseEvent>
#include "mapnode.h"

class MapNodeItem : public QObject, public QGraphicsEllipseItem
{
    Q_OBJECT
public:
    MapNodeItem(MapNode *node, qreal x, qreal y, qreal w, qreal h, bool clickable = true)
        : QGraphicsEllipseItem(x, y, w, h)
        , m_node(node)
        , m_clickable(clickable)
    {
        setAcceptHoverEvents(true);
    }

    MapNode* mapNode() const { return m_node; }

signals:
    void clicked(MapNode *node);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override
    {
        // NEW: only clickable if enabled AND node is available
        if (m_clickable && m_node->available()) {
            emit clicked(m_node);
        }
        QGraphicsEllipseItem::mousePressEvent(event);
    }

private:
    MapNode *m_node;
    bool m_clickable;
};

#endif // MAPNODEITEM_H