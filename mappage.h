#ifndef MAPPAGE_H
#define MAPPAGE_H

#include <QDialog>
#include "mapview.h"
#include "gamemap.h"

class MapPage : public QWidget
{
    Q_OBJECT
public:
    explicit MapPage(QWidget *parent = nullptr);
    ~MapPage();

private:
    MapView *m_mapView;
    GameMap *m_gameMap;
};

#endif // MAPPAGE_H