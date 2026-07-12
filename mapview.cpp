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
    m_positions.clear();

    const int verticalSpacing = 90;
    const int horizontalSpacing = 80;
    const int nodeRadius = 20;

    // Step 1: calculate and store each room's position (nothing drawn yet)
    for (int f = 0; f < map->floorCount(); ++f) {
        int count = map->roomCountAt(f);

        for (int i = 0; i < count; ++i) {
            MapNode *node = map->nodeAt(f, i);

            double x = (i - (count - 1) / 2.0) * horizontalSpacing;
            double y = -f * verticalSpacing;

            m_positions[node] = QPointF(x, y);
        }
    }

    // Step 2: draw connection lines (must be drawn before the circles so
    // the circles end up on top of them, not underneath)
    for (int f = 0; f < map->floorCount(); ++f) {
        for (int i = 0; i < map->roomCountAt(f); ++i) {
            MapNode *node = map->nodeAt(f, i);
            QPointF from = m_positions[node];

            for (MapNode *child : node->children()) {
                QPointF to = m_positions[child];
                m_scene->addLine(from.x(), from.y(), to.x(), to.y());
            }
        }
    }

    // Step 3: now draw the circles (on top of the lines)
    for (int f = 0; f < map->floorCount(); ++f) {
        for (int i = 0; i < map->roomCountAt(f); ++i) {
            MapNode *node = map->nodeAt(f, i);
            QPointF pos = m_positions[node];

            m_scene->addEllipse(pos.x() - nodeRadius, pos.y() - nodeRadius,
                                nodeRadius * 2, nodeRadius * 2);
        }
    }
}