#include "mappage.h"

MapPage::MapPage(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::Window);
    setWindowTitle("Map");

    m_gameMap = new GameMap();
    m_gameMap->generate();
    m_gameMap->startRun();

    m_mapView = new MapView(this);
    m_mapView->buildScene(m_gameMap);

    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_mapView);
    setLayout(layout);

    QTimer::singleShot(0, this, [this](){
        m_mapView->buildScene(m_gameMap);
    });

    connect(m_mapView, &MapView::roomClicked, this, [this](MapNode *node){
        QTimer::singleShot(0, this, [this, node](){
            bool ok = m_gameMap->selectRoom(node);
            if (ok) {
                m_mapView->buildScene(m_gameMap);
                handleRoomEntered(node);
            }
        });
    });

    connect(m_mapView, &MapView::returnClicked, this, [this](){
        this->close();
    });
}

void MapPage::handleRoomEntered(MapNode *node)
{
    switch (node->roomType()) {
    case RoomType::ENEMY:    openCombat(node);   break;
    case RoomType::ELITE:    openElite(node);    break;
    case RoomType::EVENT:    openEvent(node);    break;
    case RoomType::SHOP:     openShop(node);     break;
    case RoomType::CAMPFIRE: openCampfire(node); break;
    case RoomType::TREASURE: openTreasure(node); break;
    case RoomType::BOSS:     openBossFight(node);break;
    default: break;
    }
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

void MapPage::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
}

void MapPage::openCombat(MapNode *node)
{
    QMessageBox::information(this, "Combat", "Entered a normal enemy room.");
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