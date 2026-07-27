#ifndef MAPPAGE_H
#define MAPPAGE_H

#include <QDialog>
#include <QTimer>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QJsonObject>

#include "mapview.h"
#include "gamemap.h"
#include "audiomanager.h"
#include "networkmanager.h"
#include "mpcombatwindow.h"
#include "mainwindow.h"
#include "savemanager.h"

class MapPage : public QWidget
{
    Q_OBJECT
public:
    // savedMapData: اگه خالی نباشه، یعنی داریم یه سیوی قبلی رو ادامه می‌دیم
    // (به‌جای generate() یه نقشه‌ی تازه، از روی این JSON بازسازی می‌کنیم).
    // existingSaveId: اگه >= 0 باشه، همون ردیف تو دیتابیس آپدیت می‌شه؛
    // اگه -1 باشه (بازی تک‌نفره‌ی جدید)، یه سیوی تازه ساخته می‌شه.
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

    void persistProgress();

    void handleRoomEntered(MapNode *node);

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