#ifndef MAPPAGE_H
#define MAPPAGE_H

#include <QWidget>
#include <QTimer>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QJsonObject>
#include <QJsonArray>
#include <vector>
#include <string>

#include "mapview.h"
#include "gamemap.h"
#include "audiomanager.h"
#include "networkmanager.h"
#include "mpcombatwindow.h"
//#include "mainwindow.h"   // needed for CombatType
#include "savemanager.h"

class MapPage : public QWidget
{
    Q_OBJECT
public:
    explicit MapPage(QWidget *parent = nullptr, bool isLeader = true, bool isMultiplayer = true,
                     int existingSaveId = -1, const QJsonObject &savedMapData = QJsonObject());
    ~MapPage();

private slots:
    void onCombatStarted();

private:
    MapView *m_mapView;
    GameMap *m_gameMap;
    bool m_isLeader = true;
    bool m_isMultiplayer = true;
    bool m_combatOpen = false;
    int m_saveId = -1;

    // Progress kept between combats
    int m_playerHp = 80;
    int m_playerMaxHp = 80;
    int m_playerGold = 99;
    std::vector<std::string> m_deckNames; // empty = use default deck on first fight

    // Top HUD (HP / Gold / Deck)
    QWidget *m_topHud = nullptr;
    QProgressBar *m_hpBar = nullptr;
    QLabel *m_hpLabel = nullptr;
    QLabel *m_goldLabel = nullptr;
    QLabel *m_deckLabel = nullptr;

    void buildHud();
    void updateHud();

    void persistProgress();
    void handleRoomEntered(MapNode *node);

    void openSinglePlayerCombat(MapNode *node, CombatType type);

    void openCombat(MapNode *node);
    void openElite(MapNode *node);
    void openEvent(MapNode *node);
    void openShop(MapNode *node);
    void openCampfire(MapNode *node);
    void openTreasure(MapNode *node);
    void openBossFight(MapNode *node);

    void sendMapData();
    void handleIncomingMapData(const QJsonObject &mapJson);
    void handleIncomingRoomSelected(int floor, int index);

protected:
    void resizeEvent(QResizeEvent *event) override;
};

#endif // MAPPAGE_H