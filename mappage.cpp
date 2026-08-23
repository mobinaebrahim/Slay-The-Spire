#include "mappage.h"
#include "usermanager.h"
#include "mainwindow.h"
#include "Potion.h"
#include "Relics.h"
//#include "TreasureScreen.h"
// Needed to turn m_deckNames (plain strings, saved/synced form of a deck)
// back into real Card* objects when building a temporary Player for
// Shop/Event/Campfire.
extern Card* createCardByName(const std::string& name);
// Already used elsewhere in the codebase (declared the same way in
// EventScreen.h) — reused here for relic persistence.
extern Relic* createRelicByName(const std::string& name);

// FIX: mainwindow.cpp confirmed a real global createPotionByName(name)
// already exists (declared transitively via Potion.h, included through
// player.h) — no local guess-based factory needed after all. Declared
// explicitly here too, matching the extern convention already used for
// the other two factories above.
extern Potion* createPotionByName(const std::string& name);

MapPage::MapPage(QWidget *parent, bool isLeader, bool isMultiplayer, int existingSaveId, const QJsonObject &savedMapData)
    : QWidget(parent)
    , m_isLeader(isLeader)
    , m_isMultiplayer(isMultiplayer)
    , m_combatOpen(false)
    , m_saveId(existingSaveId)
{
    setWindowFlags(Qt::Window);
    setWindowTitle("Map");

    m_runTimer.start();

    m_gameMap = new GameMap();
    m_mapView = new MapView(this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_mapView);
    setLayout(layout);

    // --- Top HUD (HP / Gold / Deck) ---
    buildHud();

    // --- Parse save data ---
    QJsonObject mapJson;
    QJsonObject playerJson;

    if (!savedMapData.isEmpty()) {
        if (savedMapData.contains("map")) {
            // new format: { "map": {...}, "player": {...} }
            mapJson = savedMapData["map"].toObject();
            playerJson = savedMapData["player"].toObject();
        } else {
            // old format (backward compat): just the map JSON
            mapJson = savedMapData;
        }
    }

    if (m_isLeader) {
        if (!m_isMultiplayer && !savedMapData.isEmpty()) {
            // continuing a single-player save
            m_gameMap->fromJson(mapJson);

            m_playerHp    = playerJson["hp"].toInt(80);
            m_playerMaxHp = playerJson["max_hp"].toInt(80);
            m_playerGold  = playerJson["gold"].toInt(99);

            QJsonArray deckArr = playerJson["deck"].toArray();
            for (const QJsonValue &v : deckArr)
                m_deckNames.push_back(v.toString().toStdString());

            // NEW: same treatment as deck, so bought potions/relics survive
            // a full app restart via "Continue" instead of just in-session.
            QJsonArray potionArr = playerJson["potions"].toArray();
            for (const QJsonValue &v : potionArr)
                m_potionNames.push_back(v.toString().toStdString());

            QJsonArray relicArr = playerJson["relics"].toArray();
            for (const QJsonValue &v : relicArr)
                m_relicNames.push_back(v.toString().toStdString());

            // FIX: if the app was closed mid-combat, resume that fight
            // fresh (full pre-combat HP) instead of silently treating the
            // room as skipped.
            m_pendingCombatType = playerJson["pendingCombatType"].toString();
        } else {
            m_gameMap->generate();
            m_gameMap->startRun();

            if (!m_isMultiplayer) {
                QString currentUser = user_manager::instance().get_current_username();

                QJsonObject fullSave;
                fullSave["map"] = m_gameMap->toJson();

                QJsonObject playerObj;
                playerObj["hp"] = m_playerHp;
                playerObj["max_hp"] = m_playerMaxHp;
                playerObj["gold"] = m_playerGold;
                fullSave["player"] = playerObj;

                m_saveId = SaveManager::instance().create_save(
                    currentUser, "Autosave", "IronClad", 0, 0, fullSave);
            }
        }

        m_mapView->buildScene(m_gameMap);

        QTimer::singleShot(0, this, [this](){
            m_mapView->buildScene(m_gameMap);
        });

        // FIX: resume the interrupted fight (if any) once the map is ready.
        if (!m_isMultiplayer && !m_pendingCombatType.isEmpty() && m_gameMap->currentNode()) {
            CombatType resumeType = combatTypeFromString(m_pendingCombatType);
            MapNode *resumeNode = m_gameMap->currentNode();
            QTimer::singleShot(0, this, [this, resumeNode, resumeType](){
                openSinglePlayerCombat(resumeNode, resumeType, true);
            });
        }

        if (m_isMultiplayer) {
            QTimer::singleShot(300, this, [this](){
                sendMapData();
            });
        }
    }

    // NEW: disable map clicking for non-leader in multiplayer
    if (m_isMultiplayer && !m_isLeader) {
        m_mapView->setClickEnabled(false);
    }

    updateHud();

    if (!m_isMultiplayer) {
    } else {
        connect(&NetworkManager::instance(), &NetworkManager::game_action_received, this,
                [this](const QJsonObject &obj){
                    QString type = obj["type"].toString();
                    qDebug() << "MapPage received:" << type << "isLeader=" << m_isLeader << "combatOpen=" << m_combatOpen;

                    // FIX: changed from "map" to "data" to match server key
                    if (type == "map_data" && !m_isLeader) {
                        handleIncomingMapData(obj["data"].toObject());
                    }
                    else if (type == "room_selected" && !m_isLeader) {
                        handleIncomingRoomSelected(obj["floor"].toInt(), obj["index"].toInt());
                    }
                    // NEW: leader finished a Shop/Event/Campfire screen —
                    // dismiss the "waiting for leader" overlay. This relies
                    // on GameServer's generic fallback (any message type it
                    // doesn't specifically handle gets relayed to the rest
                    // of the room), so no server changes were needed.
                    else if (type == "room_event_finished" && !m_isLeader) {
                        hideWaitingOverlay();
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
                    // NEW: update HUD from combat results if server sends player data
                    else if (type == "combat_over") {
                        QJsonArray players = obj["players"].toArray();
                        for (const QJsonValue &v : players) {
                            QJsonObject pObj = v.toObject();
                            QString username = pObj["username"].toString();
                            // Update local HUD if this is our data
                            // (In a real client you'd match by username or ID)
                            if (!username.isEmpty()) {
                                m_playerHp = pObj["hp"].toInt(m_playerHp);
                                m_playerMaxHp = pObj["max_hp"].toInt(m_playerMaxHp);
                                m_playerGold = pObj["gold"].toInt(m_playerGold);
                                updateHud();
                            }
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
    // FIX: changed key from "map" to "data" to match server handler
    msg["data"] = m_gameMap->toJson();
    NetworkManager::instance().send_game_action(msg);
}

void MapPage::buildHud()
{
    m_topHud = new QWidget(this);
    m_topHud->setFixedHeight(48);
    m_topHud->setStyleSheet(
        "background-color: rgba(30, 20, 15, 220); "
        "border-bottom: 2px solid rgba(255, 215, 130, 60);"
        );

    QHBoxLayout* hudLayout = new QHBoxLayout(m_topHud);
    hudLayout->setContentsMargins(12, 4, 12, 4);
    hudLayout->setSpacing(8);

    QLabel* hpIcon = new QLabel(m_topHud);
    hpIcon->setPixmap(QPixmap(":/images/icons/hp.png").scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    hpIcon->setFixedSize(32, 32);

    m_hpBar = new QProgressBar(m_topHud);
    m_hpBar->setRange(0, m_playerMaxHp);
    m_hpBar->setValue(m_playerHp);
    m_hpBar->setTextVisible(true);
    m_hpBar->setFixedSize(140, 20);
    m_hpBar->setStyleSheet(
        "QProgressBar { border: 2px solid #3a1f1f; border-radius: 6px; "
        "background: #2b1414; color: white; font-weight: bold; text-align: center; }"
        "QProgressBar::chunk { background-color: qlineargradient(x1:0,y1:0,x2:1,y2:0, "
        "stop:0 #8e0e0e, stop:1 #d94040); border-radius: 4px; }"
        );

    m_hpLabel = new QLabel(m_topHud);
    m_hpLabel->setStyleSheet("color: white; font-weight: bold; font-size: 13px; background: transparent;");

    hudLayout->addWidget(hpIcon);
    hudLayout->addWidget(m_hpBar);
    hudLayout->addWidget(m_hpLabel);
    hudLayout->addStretch();

    QLabel* goldIcon = new QLabel(m_topHud);
    goldIcon->setPixmap(QPixmap(":/images/icons/gold.png").scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    goldIcon->setFixedSize(32, 32);

    m_goldLabel = new QLabel(m_topHud);
    m_goldLabel->setStyleSheet("color: white; font-weight: bold; font-size: 14px; background: transparent;");

    hudLayout->addWidget(goldIcon);
    hudLayout->addWidget(m_goldLabel);
    hudLayout->addSpacing(12);

    QLabel* deckIcon = new QLabel(m_topHud);
    deckIcon->setPixmap(QPixmap(":/images/icons/deck.png").scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    deckIcon->setFixedSize(32, 32);

    m_deckLabel = new QLabel(m_topHud);
    m_deckLabel->setStyleSheet("color: white; font-weight: bold; font-size: 14px; background: transparent;");

    hudLayout->addWidget(deckIcon);
    hudLayout->addWidget(m_deckLabel);

    QVBoxLayout* mainLayout = qobject_cast<QVBoxLayout*>(this->layout());
    if (mainLayout) {
        mainLayout->insertWidget(0, m_topHud);
    }
}

void MapPage::updateHud()
{
    m_hpBar->setMaximum(m_playerMaxHp);
    m_hpBar->setValue(m_playerHp);
    m_hpBar->setFormat(QString("%1 / %2").arg(m_playerHp).arg(m_playerMaxHp));
    m_hpLabel->setText(QString("%1/%2").arg(m_playerHp).arg(m_playerMaxHp));

    m_goldLabel->setText(QString::number(m_playerGold));

    int deckCount = m_deckNames.empty() ? 15 : static_cast<int>(m_deckNames.size());
    m_deckLabel->setText(QString::number(deckCount));
}

void MapPage::persistProgress()
{
    if (m_isMultiplayer || m_saveId < 0) return;

    int floor = m_gameMap->currentNode() ? m_gameMap->currentNode()->floor() : 0;

    QJsonObject fullSave;
    fullSave["map"] = m_gameMap->toJson();

    QJsonObject playerObj;
    playerObj["hp"] = m_playerHp;
    playerObj["max_hp"] = m_playerMaxHp;
    playerObj["gold"] = m_playerGold;

    QJsonArray deckArr;
    for (const auto &name : m_deckNames)
        deckArr.append(QString::fromStdString(name));
    playerObj["deck"] = deckArr;

    // NEW: same treatment as deck — without this, anything bought/found
    // survives only until the app closes, then silently vanishes on reload.
    QJsonArray potionArr;
    for (const auto &name : m_potionNames)
        potionArr.append(QString::fromStdString(name));
    playerObj["potions"] = potionArr;

    QJsonArray relicArr;
    for (const auto &name : m_relicNames)
        relicArr.append(QString::fromStdString(name));
    playerObj["relics"] = relicArr;

    // FIX: remember whether a fight was in progress when this was written,
    // so a reload can resume it instead of silently skipping the room.
    playerObj["pendingCombatType"] = m_pendingCombatType;

    fullSave["player"] = playerObj;

    // FIX: used to always save score=0 here. Now uses the real current score.
    SaveManager::instance().update_save(m_saveId, currentScore(floor), floor, fullSave);
}

// FIX: called right before a single-player combat opens, so that if the app
// closes mid-fight, the save on disk records which fight needs resuming.
void MapPage::markCombatInProgress(CombatType type)
{
    m_pendingCombatType = combatTypeToString(type);
    persistProgress();
}

// FIX: called once a fight is fully resolved (won), so the save no longer
// thinks a combat is still pending.
void MapPage::clearCombatInProgress()
{
    m_pendingCombatType.clear();
}

QString MapPage::combatTypeToString(CombatType type)
{
    switch (type) {
    case CombatType::Elite: return "elite";
    case CombatType::Boss:  return "boss";
    default:                return "normal";
    }
}

CombatType MapPage::combatTypeFromString(const QString &s)
{
    if (s == "elite") return CombatType::Elite;
    if (s == "boss")  return CombatType::Boss;
    return CombatType::Normal;
}

// Score formula based on the actual run stats the design doc asks for:
// damage dealt + elites killed + floor reached. Weights are a reasonable
// default — tune them if you want kills/floor to matter more than damage.
int MapPage::currentScore(int floor) const
{
    return m_totalDamageDealt + m_elitesKilled * 100 + floor * 50;
}

// FIX: this was the missing piece — ScoreManager::add_score() was defined
// but never called anywhere in the game, so the scoreboard/leaderboard
// stayed empty. This writes one row per finished run (win or loss).
void MapPage::saveRunScore(bool victory)
{
    QString currentUser = user_manager::instance().get_current_username();
    if (currentUser.isEmpty()) {
        qDebug() << "saveRunScore: no current user, skipping";
        return;
    }

    int floor = m_gameMap->currentNode() ? m_gameMap->currentNode()->floor() : 0;
    int score = currentScore(floor);
    int durationSecs = m_runTimer.isValid() ? static_cast<int>(m_runTimer.elapsed() / 1000) : 0;

    bool ok = ScoreManager::instance().add_score(currentUser, score, floor, durationSecs, victory);
    if (!ok) {
        qDebug() << "saveRunScore: failed to save score for" << currentUser;
    }
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

    MPCombatWindow *combat = new MPCombatWindow(nullptr, m_isLeader);

    combat->setAttribute(Qt::WA_DeleteOnClose);

    this->hide();

    connect(combat, &MPCombatWindow::combatFinished, this, [this, combat](bool victory, int damageDealt) {
        qDebug() << "Combat finished, victory=" << victory;
        combat->close();
        m_combatOpen = false;
        this->show();

        m_totalDamageDealt += damageDealt;

        // FIX: this block used to be empty, so multiplayer wins were never
        // saved to the scoreboard. Only the leader saves, to avoid duplicate
        // rows for the same match (teammate would report the same result).
        if (victory && m_isLeader) {
            saveRunScore(true);
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
    if (m_waitingOverlay && m_waitingOverlay->isVisible())
        m_waitingOverlay->setGeometry(0, 0, width(), height());
}

void MapPage::showWaitingOverlay(const QString &text)
{
    if (!m_waitingOverlay) {
        m_waitingOverlay = new QWidget(this);
        m_waitingOverlay->setStyleSheet("background-color: rgba(0,0,0,180);");

        auto *lbl = new QLabel(m_waitingOverlay);
        lbl->setObjectName("waitingLabel");
        lbl->setAlignment(Qt::AlignCenter);
        lbl->setStyleSheet("color: white; font-size: 20px; font-weight: bold; background: transparent;");

        auto *lay = new QVBoxLayout(m_waitingOverlay);
        lay->setAlignment(Qt::AlignCenter);
        lay->addWidget(lbl);
    }

    if (QLabel *lbl = m_waitingOverlay->findChild<QLabel*>("waitingLabel"))
        lbl->setText(text);

    m_waitingOverlay->setGeometry(0, 0, width(), height());
    m_waitingOverlay->show();
    m_waitingOverlay->raise();
}

void MapPage::hideWaitingOverlay()
{
    if (m_waitingOverlay)
        m_waitingOverlay->hide();
}

Player* MapPage::buildPlayerFromProgress()
{
    // NOTE: BattleManager* is nullptr here on purpose — Shop/Event/Campfire
    // never call anything that needs it (no card play, no combat).
    Player *p = new Player("Hero", m_playerHp, m_playerMaxHp, 3, m_playerGold, nullptr);

    if (m_deckNames.empty()) {
        // Same default starter deck implied by the "empty = use default
        // deck on first fight" comment on m_deckNames in mappage.h.
        for (int i = 0; i < 5; ++i) {
            if (Card *c = createCardByName("Strike")) p->addCardToDrawPile(c);
        }
        for (int i = 0; i < 4; ++i) {
            if (Card *c = createCardByName("Defend")) p->addCardToDrawPile(c);
        }
    } else {
        for (const std::string &name : m_deckNames) {
            if (Card *c = createCardByName(name)) p->addCardToDrawPile(c);
        }
    }

    // NEW: restore potions/relics from last sync.
    for (const std::string &name : m_potionNames) {
        if (Potion *pot = createPotionByName(name)) p->addPotion(pot);
    }
    for (const std::string &name : m_relicNames) {
        if (Relic *r = createRelicByName(name)) p->addRelic(r);
    }

    return p;
}

void MapPage::syncProgressFromPlayer(Player* p)
{
    if (!p) return;

    m_playerHp    = p->getHp();
    m_playerMaxHp = p->getMaxHp();
    m_playerGold  = p->getGold();

    m_deckNames.clear();
    for (Card *c : p->getFullDeck()) {
        if (c) m_deckNames.push_back(c->getName());
    }

    // NEW: mirror deck sync for potions/relics.
    m_potionNames.clear();
    for (Potion *pot : p->getPotions()) {
        if (pot) m_potionNames.push_back(pot->getName());
    }
    m_relicNames.clear();
    for (Relic *r : p->getRelics()) {
        if (r) m_relicNames.push_back(r->getName());
    }

    updateHud();
    persistProgress();
}

void MapPage::openSinglePlayerCombat(MapNode *node, CombatType type, bool isResume)
{
    Q_UNUSED(node); // later you can pick a different enemy set based on room/floor

    if (!isResume) {
        // entering a new room -> +5 HP (capped at max). Skipped on resume so
        // reopening the same interrupted fight doesn't grant a free heal.
        m_playerHp = qMin(m_playerHp + 5, m_playerMaxHp);
        updateHud();
    }

    // FIX: record that this fight is now in progress, so if the app closes
    // before it resolves, reopening the save will resume this same fight.
    markCombatInProgress(type);

    // FIX: MainWindow's real constructor (confirmed from mainwindow.cpp)
    // takes potionNames as its own parameter, separate from deckNames.
    MainWindow *combatWindow = new MainWindow(nullptr, m_playerHp, m_playerMaxHp,
                                              m_playerGold, m_deckNames, m_potionNames, type);
    combatWindow->setAttribute(Qt::WA_DeleteOnClose);
    this->hide();

    // FIX: this used to expect combatFinished(bool, int, int, int, vector<string>,
    // int damageDealt, bool wasElite) — 7 params. mainwindow.cpp's actual emit
    // calls only ever pass 6: (victory, hp, maxHp, gold, deck, potions). The old
    // lambda's extra 2 params don't exist on the real signal at all, so this
    // wouldn't have compiled. damageDealt/wasElite aren't tracked anywhere in
    // mainwindow.cpp either (no member, no getter) — there's currently no way to
    // recover them from here, so m_totalDamageDealt/m_elitesKilled no longer get
    // incremented from single-player fights. Score for single-player runs is
    // effectively floor-based until MainWindow exposes that data again (a getter,
    // or the params restored on the signal).
    connect(combatWindow, &MainWindow::combatFinished, this,
            [this, type](bool victory, int finalHp, int maxHp,
                         int finalGold, const std::vector<std::string>& finalDeck,
                         const std::vector<std::string>& finalPotions) {
                if (victory) {
                    // FIX: fight resolved — no longer pending.
                    clearCombatInProgress();
                    m_playerHp     = finalHp;
                    m_playerMaxHp  = maxHp;
                    m_playerGold   = finalGold;
                    m_deckNames    = finalDeck;
                    m_potionNames  = finalPotions;
                    updateHud();
                    persistProgress(); // auto-save after every victory

                    // FIX: beating the boss finishes the run — save the score now.
                    if (type == CombatType::Boss) {
                        saveRunScore(true);
                    }
                } else {
                    m_playerHp = 0; // dead
                }
            });

    connect(combatWindow, &QObject::destroyed, this, [this]() {
        if (m_playerHp <= 0) {
            // FIX: dying ends the run — save the score (as a loss) before leaving.
            saveRunScore(false);

            // FIX: the save still has "pendingCombatType" set from this fight.
            // If we left it on disk, hitting Continue later would resume and
            // re-fight (and could win) the exact battle that just killed the
            // player, undoing their death. Delete the save instead, same as
            // the explicit Abandon button does.
            if (!m_isMultiplayer && m_saveId >= 0) {
                SaveManager::instance().delete_save(m_saveId);
            }

            emit runAbandoned();
            this->close(); // defeat -> back to main menu
        } else {
            this->show();
            m_mapView->buildScene(m_gameMap);
        }
    });

    combatWindow->showFullScreen();
}

void MapPage::openCombat(MapNode *node)
{
    qDebug() << "openCombat called, isLeader=" << m_isLeader << "isMultiplayer=" << m_isMultiplayer;

    if (!m_isMultiplayer) {
        openSinglePlayerCombat(node, CombatType::Normal);
        return;
    }

    if (m_isLeader) {
        QJsonObject msg;
        msg["type"] = "start_combat";
        msg["enemy_name"] = "JawWorm";
        NetworkManager::instance().send_game_action(msg);
    }
}

void MapPage::openElite(MapNode *node)
{
    if (!m_isMultiplayer) {
        openSinglePlayerCombat(node, CombatType::Elite);
        return;
    }
    QMessageBox::information(this, "Elite", "Entered an elite room.");
}

void MapPage::openEvent(MapNode *node)
{
    Q_UNUSED(node);

    if (m_isMultiplayer && !m_isLeader) {
        showWaitingOverlay("Waiting for the leader to finish the event...");
        return;
    }

    Player *tempPlayer = buildPlayerFromProgress();
    EventScreen *eventScreen = new EventScreen(tempPlayer, nullptr);
    eventScreen->setAttribute(Qt::WA_DeleteOnClose);

    // TODO: no per-room event data exists yet (MapNode doesn't carry an
    // EventType), so we just pick one at random each time. Wire this up to
    // real per-node data once the map generator assigns one.
    static const EventType allEventTypes[] = {
        EventType::OminousForge, EventType::GoldenIdol, EventType::Augmenter,
        EventType::FaceTrader, EventType::TheColosseum, EventType::GoldenShrine,
        EventType::Lab, EventType::ShiningLight, EventType::TheSerpent
    };
    eventScreen->setEvent(allEventTypes[rand() % 9]);

    this->hide();

    connect(eventScreen, &EventScreen::finished, this, [this, eventScreen, tempPlayer]() {
        syncProgressFromPlayer(tempPlayer);
        delete tempPlayer;

        eventScreen->close();
        this->show();
        m_mapView->buildScene(m_gameMap);

        if (m_isMultiplayer) {
            QJsonObject msg;
            msg["type"] = "room_event_finished";
            NetworkManager::instance().send_game_action(msg);
        }
    });

    // TODO: EventScreen also emits requestCombat(Enemy*), requestCardTransform(int),
    // and requestCardSelection(function<void(Card*)>) for options like "The Colosseum"
    // (fight), "Lab" (transform cards), "The Serpent"/"Shining Light" (pick a card).
    // None of those are wired up yet — they need, respectively: a route into
    // single-/multiplayer combat starting from a pre-built Enemy*, a "random card of
    // rarity X" factory helper, and a card-picker UI we don't have. Any option in
    // EventScreen that relies on these will currently do nothing when clicked.
    // requestCardUpgrade is the one exception — wired below since CampfireScreen's
    // smith logic gives us the exact pattern for it already.
    connect(eventScreen, &EventScreen::requestCardUpgrade, this, [tempPlayer]() {
        const auto &deck = tempPlayer->getFullDeck();
        if (!deck.empty()) {
            deck[rand() % deck.size()]->upgrade();
        }
    });

    eventScreen->showFullScreen();
}

void MapPage::openShop(MapNode *node)
{
    Q_UNUSED(node);

    // FIX: was previously a stub QMessageBox for everyone, single-player
    // included. Per the coop spec, only the leader gets a real screen in
    // multiplayer; the teammate(s) wait until they're done.
    if (m_isMultiplayer && !m_isLeader) {
        showWaitingOverlay("Waiting for the leader to finish shopping...");
        return;
    }

    Player *tempPlayer = buildPlayerFromProgress();
    ShopScreen *shop = new ShopScreen(tempPlayer, nullptr);
    shop->setAttribute(Qt::WA_DeleteOnClose);
    this->hide();

    connect(shop, &ShopScreen::finished, this, [this, shop, tempPlayer]() {
        syncProgressFromPlayer(tempPlayer);
        delete tempPlayer;

        shop->close();
        this->show();
        m_mapView->buildScene(m_gameMap);

        if (m_isMultiplayer) {
            QJsonObject msg;
            msg["type"] = "room_event_finished";
            NetworkManager::instance().send_game_action(msg);
        }
    });

    shop->showFullScreen();
}

void MapPage::openCampfire(MapNode *node)
{
    Q_UNUSED(node);

    if (m_isMultiplayer && !m_isLeader) {
        showWaitingOverlay("Waiting for the leader at the campfire...");
        return;
    }

    Player *tempPlayer = buildPlayerFromProgress();
    CampfireScreen *campfire = new CampfireScreen(tempPlayer, nullptr);
    campfire->setAttribute(Qt::WA_DeleteOnClose);
    this->hide();

    connect(campfire, &CampfireScreen::finished, this, [this, campfire, tempPlayer]() {
        syncProgressFromPlayer(tempPlayer);
        delete tempPlayer;

        campfire->close();
        this->show();
        m_mapView->buildScene(m_gameMap);

        if (m_isMultiplayer) {
            QJsonObject msg;
            msg["type"] = "room_event_finished";
            NetworkManager::instance().send_game_action(msg);
        }
    });

    campfire->showFullScreen();
}

void MapPage::openTreasure(MapNode *node)
{
    /*//Player tempPlayer = new Player(m_username, m_playerHp, m_playerMaxHp, 3, m_playerGold, nullptr);
    //TreasureScreen* treasure = new TreasureScreen(tempPlayer, nullptr);
    //treasure->setAttribute(Qt::WA_DeleteOnClose);
    treasure->setWindowFlags(Qt::Window);

    this->hide();

    connect(treasure, &TreasureScreen::finished, this, [this, treasure, tempPlayer]() {
        //m_playerGold = tempPlayer->getGold();

        updateHud();
        persistProgress();

        //delete tempPlayer;
        treasure->close();
        this->show();
    });

    treasure->showFullScreen();*/
}

void MapPage::openBossFight(MapNode *node)
{
    if (!m_isMultiplayer) {
        openSinglePlayerCombat(node, CombatType::Boss);
        return;
    }
    QMessageBox::information(this, "Boss", "You reached the boss!");
}

MapPage::~MapPage()
{
    delete m_gameMap;
}