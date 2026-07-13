#include "mappage.h"

MapPage::MapPage(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::Window);
    resize(800, 800);
    setWindowTitle("Map");

    m_gameMap = new GameMap();
    m_gameMap->generate();
    m_gameMap->startRun();

    m_mapView = new MapView(this);
    m_mapView->buildScene(m_gameMap);
    m_mapView->setGeometry(0, 0, width(), height());

    connect(m_mapView, &MapView::roomClicked, this, [this](MapNode *node){
        bool ok = m_gameMap->selectRoom(node);
        if (ok) {
            m_mapView->buildScene(m_gameMap);

            if (m_gameMap->isAtBoss()) {
            }
        }
    });
}

MapPage::~MapPage()
{
    delete m_gameMap;
}