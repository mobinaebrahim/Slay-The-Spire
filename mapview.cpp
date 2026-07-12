#include "mapview.h"

MapView::MapView(QWidget *parent)
    : QGraphicsView(parent)
{
    m_scene = new QGraphicsScene(this);
    setScene(m_scene);
}

void MapView::buildScene(GameMap *map)
{
    m_scene->clear();

    const int verticalSpacing = 90;
    const int horizontalSpacing = 80;
    const int nodeRadius = 20;

    for (int f = 0; f < map->floorCount(); ++f) {
        int count = map->roomCountAt(f);

        for (int i = 0; i < count; ++i) {
            MapNode *node = map->nodeAt(f, i);

            double x = (i - (count - 1) / 2.0) * horizontalSpacing;
            double y = -f * verticalSpacing;

            m_scene->addEllipse(x - nodeRadius, y - nodeRadius,
                                nodeRadius * 2, nodeRadius * 2);
        }
    }
}