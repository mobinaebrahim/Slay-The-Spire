#ifndef MAPVIEW_H
#define MAPVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMap>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsColorizeEffect>
#include <QGraphicsProxyWidget>
#include <QPushButton>
#include "gamemap.h"


class MapView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit MapView(QWidget *parent = nullptr);
    void buildScene(GameMap *map);

    // NEW: disable/enable room clicking (for non-leader lock)
    void setClickEnabled(bool enabled) { m_clickEnabled = enabled; }

signals:
    void roomClicked(MapNode *node);
    void returnClicked();

private:
    QGraphicsScene *m_scene;
    QMap<MapNode*, QPointF> m_positions;
    QString iconPathForRoomType(RoomType type) const;
    QGraphicsProxyWidget *m_returnButtonProxy = nullptr;
    void addReturnButton();

    // NEW
    bool m_clickEnabled = true;
};

#endif // MAPVIEW_H