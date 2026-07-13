#include "mappage.h"

MapPage::MapPage(QWidget *parent)
    : QDialog(parent)
{
    resize(800, 800);
    setWindowTitle("Map");

    m_gameMap = new GameMap();
    m_gameMap->generate();
    m_gameMap->startRun();

    m_mapView = new MapView(this);
    m_mapView->buildScene(m_gameMap);

    m_mapView->setGeometry(0, 0, width(), height());
}

MapPage::~MapPage()
{
    delete m_gameMap;
}