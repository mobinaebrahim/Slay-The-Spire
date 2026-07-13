#include "mapview.h"
#include "mapnodeitem.h"

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

    const int verticalSpacing = 220;
    const int horizontalSpacing = 190;
    const int nodeRadius = 60;

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

    /*// Step 3: now draw the circles (on top of the lines)
    for (int f = 0; f < map->floorCount(); ++f) {
        for (int i = 0; i < map->roomCountAt(f); ++i) {
            MapNode *node = map->nodeAt(f, i);
            QPointF pos = m_positions[node];

            QBrush brush;
            if (node->available()) {
                brush = QBrush(Qt::yellow);
            } else if (node->visited()) {
                brush = QBrush(Qt::darkGray);
            } else {
                brush = QBrush(Qt::lightGray);
            }

            MapNodeItem *item = new MapNodeItem(node,
                                                pos.x() - nodeRadius, pos.y() - nodeRadius,
                                                nodeRadius * 2, nodeRadius * 2);
            item->setPen(QPen(Qt::black));
            item->setBrush(brush);

            connect(item, &MapNodeItem::clicked, this, &MapView::roomClicked);

            m_scene->addItem(item);
        }
    }*/

    // Step 3: draw the circles on top of the lines, colored by state, and clickable
    for (int f = 0; f < map->floorCount(); ++f) {
        for (int i = 0; i < map->roomCountAt(f); ++i) {
            MapNode *node = map->nodeAt(f, i);
            QPointF pos = m_positions[node];

            QBrush brush;
            if (node->available()) {
                brush = QBrush(Qt::yellow);
            } else if (node->visited()) {
                brush = QBrush(Qt::darkGray);
            } else {
                brush = QBrush(Qt::lightGray);
            }

            MapNodeItem *item = new MapNodeItem(node,
                                                pos.x() - nodeRadius, pos.y() - nodeRadius,
                                                nodeRadius * 2, nodeRadius * 2);
            item->setPen(QPen(Qt::NoPen));
            item->setBrush(brush);

            connect(item, &MapNodeItem::clicked, this, &MapView::roomClicked);

            // Add the room-type icon on top of the circle

            //***
            item->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
            QString iconPath = iconPathForRoomType(node->roomType());
            QPixmap pix(iconPath);
            if (!pix.isNull()) {
                qreal iconSize = nodeRadius * 2.0; // slightly smaller than the circle

                QGraphicsPixmapItem *iconItem = new QGraphicsPixmapItem(item); // child of the circle
                QPixmap scaled = pix.scaled(iconSize, iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                iconItem->setPixmap(scaled);

                QRectF ellipseRect = item->rect();
                iconItem->setPos(ellipseRect.center().x() - scaled.width() / 2.0,
                                 ellipseRect.center().y() - scaled.height() / 2.0);
            }

            m_scene->addItem(item);
        }
    }
}