#ifndef MAPVIEW_H
#define MAPVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include "gamemap.h"

class MapView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit MapView(QWidget *parent = nullptr);

    void buildScene(GameMap *map);
private:
    QGraphicsScene *m_scene;
};
#endif // MAPVIEW_H