#ifndef MAPPAGE_H
#define MAPPAGE_H

#include <QDialog>
#include <QTimer>
#include <QMessageBox>
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

    void handleRoomEntered(MapNode *node);


    void openCombat(MapNode *node);
    void openElite(MapNode *node);
    void openEvent(MapNode *node);
    void openShop(MapNode *node);
    void openCampfire(MapNode *node);
    void openTreasure(MapNode *node);
    void openBossFight(MapNode *node);
};

#endif // MAPPAGE_H