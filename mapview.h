#ifndef MAPVIEW_H
#define MAPVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMap>
#include "gamemap.h"

class MapView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit MapView(QWidget *parent = nullptr);
    void buildScene(GameMap *map);

signals:
    void roomClicked(MapNode *node);

private:
    QGraphicsScene *m_scene;
    QMap<MapNode*, QPointF> m_positions;
};

#endif // MAPVIEW_H