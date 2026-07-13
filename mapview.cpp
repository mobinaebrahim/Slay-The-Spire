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

    const int verticalSpacing = 280;
    const int horizontalSpacing = 250;
    const int nodeRadius = 90;

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

    // Add a background image sized to cover the whole map, placed behind everything
    qreal minX = 0, maxX = 0, minY = 0, maxY = 0;
    bool first = true;
    for (auto it = m_positions.constBegin(); it != m_positions.constEnd(); ++it) {
        QPointF p = it.value();
        if (first) {
            minX = maxX = p.x();
            minY = maxY = p.y();
            first = false;
        } else {
            minX = qMin(minX, p.x());
            maxX = qMax(maxX, p.x());
            minY = qMin(minY, p.y());
            maxY = qMax(maxY, p.y());
        }
    }

    qreal padding = 150;
    qreal contentWidth = (maxX - minX) + padding * 2;
    qreal contentHeight = (maxY - minY) + padding * 2;

    // Make sure the background is at least as big as the visible viewport,
    // so there's no gray gap on the sides
    qreal finalWidth = qMax(contentWidth, (qreal)this->viewport()->width());
    qreal finalHeight = qMax(contentHeight, (qreal)this->viewport()->height());

    qreal centerX = (minX + maxX) / 2.0;

    QRectF bgRect(centerX - finalWidth / 2.0, minY - padding,
                  finalWidth, finalHeight);

    QPixmap bgPixmap(":/assets/map/background.png");
    if (!bgPixmap.isNull()) {
        QPixmap scaledBg = bgPixmap.scaled(bgRect.size().toSize(),
                                           Qt::IgnoreAspectRatio,
                                           Qt::SmoothTransformation);
        QGraphicsPixmapItem *bgItem = m_scene->addPixmap(scaledBg);
        bgItem->setPos(bgRect.topLeft());
        bgItem->setZValue(-10); // make sure it stays behind the lines and circles
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

            // Boss gets a bigger circle than everything else
            qreal radius = (node->roomType() == RoomType::BOSS) ? nodeRadius * 1.6 : nodeRadius;

            MapNodeItem *item = new MapNodeItem(node,
                                                pos.x() - radius, pos.y() - radius,
                                                radius * 2, radius * 2);

            item->setPen(QPen(Qt::NoPen));
            item->setBrush(Qt::NoBrush); // no colored ring anymore - the circle stays invisible

            connect(item, &MapNodeItem::clicked, this, &MapView::roomClicked);

            // Add the room-type icon on top of the circle

            //***
            item->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
            QString iconPath = iconPathForRoomType(node->roomType());
            QPixmap pix(iconPath);
            if (!pix.isNull()) {
                qreal iconSize = radius * 2.0; // matches the circle size (bigger for boss)

                QGraphicsPixmapItem *iconItem = new QGraphicsPixmapItem(item); // child of the circle
                QPixmap scaled = pix.scaled(iconSize, iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                iconItem->setPixmap(scaled);

                QRectF ellipseRect = item->rect();
                iconItem->setPos(ellipseRect.center().x() - scaled.width() / 2.0,
                                 ellipseRect.center().y() - scaled.height() / 2.0);

                // Apply a visual effect on the icon itself based on room state
                if (node->available()) {
                    // Bright glowing highlight for rooms the player can click right now
                    QGraphicsDropShadowEffect *glow = new QGraphicsDropShadowEffect;
                    glow->setColor(QColor(255, 215, 0)); // gold/yellow glow
                    glow->setOffset(0, 0);
                    glow->setBlurRadius(40);
                    iconItem->setGraphicsEffect(glow);
                } else if (node->visited()) {
                    // Darken rooms the player has already passed through
                    QGraphicsColorizeEffect *dark = new QGraphicsColorizeEffect;
                    dark->setColor(Qt::black);
                    dark->setStrength(0.6);
                    iconItem->setGraphicsEffect(dark);
                }
                // locked rooms (neither available nor visited) get no effect - shown as normal icon
            }

            m_scene->addItem(item);
        }
    }
}