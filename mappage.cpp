#include "mappage.h"
#include "usermanager.h"

MapPage::MapPage(QWidget *parent, bool isLeader, bool isMultiplayer, int existingSaveId, const QJsonObject &savedMapData)
    : QWidget(parent)
    , m_isLeader(isLeader)
    , m_isMultiplayer(isMultiplayer)
    , m_combatOpen(false)
    , m_saveId(existingSaveId)
{
    setWindowFlags(Qt::Window);
    setWindowTitle("Map");

    m_gameMap = new GameMap();
    m_mapView = new MapView(this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_mapView);
    setLayout(layout);

    if (m_isLeader) {
        if (!m_isMultiplayer && !savedMapData.isEmpty()) {
            // ادامه‌ی یه بازی تک‌نفره‌ی ذخیره‌شده: نقشه رو از روی
            // JSON بازسازی می‌کنیم (نه generate تازه)، پس پیشرفت
            // (اتاق‌های visited/available و موقعیت فعلی) حفظ می‌شه.
            m_gameMap->fromJson(savedMapData);
        } else {
            m_gameMap->generate();
            m_gameMap->startRun();

            if (!m_isMultiplayer) {
                // بازی تک‌نفره‌ی کاملاً جدید: همین الان یه ردیف سیو بساز
                // تا از همین اول یه id معتبر برای آپدیت‌های بعدی داشته باشیم.
                QString currentUser = user_manager::instance().get_current_username();
                m_saveId = SaveManager::instance().create_save(
                    currentUser, "Autosave", "IronClad", 0, 0, m_gameMap->toJson());
            }
        }

        m_mapView->buildScene(m_gameMap);

        QTimer::singleShot(0, this, [this](){
            m_mapView->buildScene(m_gameMap);
        });

        if (m_isMultiplayer) {
            QTimer::singleShot(300, this, [this](){
                sendMapData();
            });
        }
    }

    if (!m_isMultiplayer) {
        // بازی تک‌نفره: هیچ listener شبکه‌ای لازم نیست.
    } else {
        connect(&NetworkManager::instance(), &NetworkManager::game_action_received, this,
                [this](const QJsonObject &obj){
                    QString type = obj["type"].toString();
                    qDebug() << "MapPage received:" << type << "isLeader=" << m_isLeader << "combatOpen=" << m_combatOpen;

                    if (type == "map_data" && !m_isLeader) {
                        handleIncomingMapData(obj["map"].toObject());
                    }
                    else if (type == "room_selected" && !m_isLeader) {
                        handleIncomingRoomSelected(obj["floor"].toInt(), obj["index"].toInt());
                    }
                    else if (type == "combat_started") {
                        qDebug() << "combat_started received! combatOpen=" << m_combatOpen;
                        if (!m_combatOpen) {
                            qDebug() << "Opening CombatPage for" << (m_isLeader ? "Leader" : "Teammate");
                            onCombatStarted();
                        } else {
                            qDebug() << "Combat already open, ignoring.";
                        }
                    }
                });
    }

    connect(m_mapView, &MapView::roomClicked, this, [this](MapNode *node){
        if (!m_isLeader) return;

        QTimer::singleShot(0, this, [this, node](){
            bool ok = m_gameMap->selectRoom(node);
            if (ok) {
                AudioManager::instance().playEffect(":/assets/music/map_room.mp3");
                m_mapView->buildScene(m_gameMap);
                handleRoomEntered(node);
                persistProgress();

                if (m_isMultiplayer) {
                    QJsonObject msg;
                    msg["type"] = "room_selected";
                    msg["floor"] = node->floor();
                    msg["index"] = node->index();
                    NetworkManager::instance().send_game_action(msg);
                }
            }
        });
    });

    connect(m_mapView, &MapView::returnClicked, this, [this](){
        this->close();
    });
}

void MapPage::sendMapData()
{
    QJsonObject msg;
    msg["type"] = "map_data";
    msg["map"] = m_gameMap->toJson();
    NetworkManager::instance().send_game_action(msg);
}

void MapPage::persistProgress()
{
    // فقط بازی تک‌نفره سیو می‌شه؛ Multiplayer فعلاً مفهوم Save/Continue نداره.
    if (m_isMultiplayer || m_saveId < 0) return;

    int floor = m_gameMap->currentNode() ? m_gameMap->currentNode()->floor() : 0;
    SaveManager::instance().update_save(m_saveId, 0, floor, m_gameMap->toJson());
}

void MapPage::handleIncomingRoomSelected(int floor, int index)
{
    MapNode *node = m_gameMap->nodeAt(floor, index);
    if (!node) return;

    bool ok = m_gameMap->selectRoom(node);
    if (ok) {
        AudioManager::instance().playEffect(":/assets/music/map_room.mp3");
        m_mapView->buildScene(m_gameMap);
        handleRoomEntered(node);
    }
}

void MapPage::handleRoomEntered(MapNode *node)
{
    switch (node->roomType()) {
    case RoomType::ENEMY:
        openCombat(node);
        break;
    case RoomType::ELITE:
        openElite(node);
        break;
    case RoomType::EVENT:
        openEvent(node);
        break;
    case RoomType::SHOP:
        openShop(node);
        break;
    case RoomType::CAMPFIRE:
        openCampfire(node);
        break;
    case RoomType::TREASURE:
        openTreasure(node);
        break;
    case RoomType::BOSS:
        openBossFight(node);
        break;
    default:
        break;
    }
}

void MapPage::onCombatStarted()
{
    if (m_combatOpen) return;
    m_combatOpen = true;

    qDebug() << "Creating CombatPage for" << (m_isLeader ? "Leader" : "Teammate");
    MPCombatWindow *combat = new MPCombatWindow(this, m_isLeader);    combat->setAttribute(Qt::WA_DeleteOnClose);

    this->hide();

    connect(combat, &MPCombatWindow::combatFinished, this, [this, combat](bool victory) {
        qDebug() << "Combat finished, victory=" << victory;
        combat->close();
        m_combatOpen = false;
        this->show();

        if (victory && m_isLeader) {
            // TODO: برو طبقه بعدی
        }
    });

    combat->showFullScreen();
}

QString MapView::iconPathForRoomType(RoomType type) const
{
    switch (type) {
    case RoomType::ENEMY:    return ":/assets/map/enemy.png";
    case RoomType::ELITE:    return ":/assets/map/elite.png";
    case RoomType::EVENT:    return ":/assets/map/event.png";
    case RoomType::SHOP:     return ":/assets/map/shop.png";
    case RoomType::CAMPFIRE: return ":/assets/map/campfire.png";
    case RoomType::TREASURE: return ":/assets/map/treasure.png";
    case RoomType::BOSS:     return ":/assets/map/boss.png";
    default: return QString();
    }
}

void MapPage::handleIncomingMapData(const QJsonObject &mapJson)
{
    m_gameMap->fromJson(mapJson);
    m_gameMap->startRun();
    m_mapView->buildScene(m_gameMap);

    QTimer::singleShot(0, this, [this](){
        m_mapView->buildScene(m_gameMap);
    });
}

void MapPage::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
}

void MapPage::openCombat(MapNode *node)
{
    qDebug() << "openCombat called, isLeader=" << m_isLeader << "isMultiplayer=" << m_isMultiplayer;

    if (!m_isMultiplayer) {
        // بازی تک‌نفره: صفحه‌ی نبرد محلی دوستمون (MainWindow) رو مستقیم
        // باز می‌کنیم — هیچ پیامی به سرور فرستاده نمی‌شه.
        MainWindow *combatWindow = new MainWindow(nullptr);
        combatWindow->setAttribute(Qt::WA_DeleteOnClose);

        this->hide();

        connect(combatWindow, &QObject::destroyed, this, [this](){
            this->show();
        });

        combatWindow->showFullScreen();
        return;
    }

    if (m_isLeader) {
        // Leader sends start_combat to server
        QJsonObject msg;
        msg["type"] = "start_combat";
        msg["enemy_name"] = "JawWorm"; // TODO: based on node
        NetworkManager::instance().send_game_action(msg);
        // MPCombatWindow will be opened when combat_started is received
    }
    // Teammate waits for combat_started from server
}

void MapPage::openElite(MapNode *node)
{
    QMessageBox::information(this, "Elite", "Entered an elite room.");
}

void MapPage::openEvent(MapNode *node)
{
    QMessageBox::information(this, "Event", "Entered an event room.");
}

void MapPage::openShop(MapNode *node)
{
    QMessageBox::information(this, "Shop", "Entered a shop room.");
}

void MapPage::openCampfire(MapNode *node)
{
    QMessageBox::information(this, "Campfire", "Entered a campfire room.");
}

void MapPage::openTreasure(MapNode *node)
{
    QMessageBox::information(this, "Treasure", "Entered a treasure room.");
}

void MapPage::openBossFight(MapNode *node)
{
    QMessageBox::information(this, "Boss", "You reached the boss!");
}

MapPage::~MapPage()
{
    delete m_gameMap;
}
