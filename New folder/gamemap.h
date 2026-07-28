
#ifndef GAMEMAP_H
#define GAMEMAP_H

#include <QVector>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QJsonObject>
#include <QJsonArray>
#include "mapnode.h"

class GameMap
{
public:
    explicit GameMap(int totalFloors = 11, int minRoomsPerFloor = 2, int maxRoomsPerFloor = 4);
    ~GameMap();

    void generate();
    void printToConsole() const;

    MapNode* nodeAt(int floor, int indexInFloor) const;
    int floorCount() const;
    int roomCountAt(int floor) const;

    // Path navigation
    void startRun();
    bool selectRoom(MapNode *target);
    MapNode* currentNode() const;
    bool isAtBoss() const;

    void buildScene(GameMap *map);

    // Multiplayer sync
    QJsonObject toJson() const;
    void fromJson(const QJsonObject &obj);

private:
    void pickCheckFloors();
    QGraphicsScene *m_scene;


    // Step 1: Build the skeleton (room counts + connections)
    void buildSkeleton();
    void connectFloors(int floorA, int floorB);
    void validateFloor(int floorB);

    // Step 2: Assign room types (includes an ordinary random chance of CAMPFIRE)
    void assignRoomTypes();
    RoomType randomRoomType(int floorIndex) const;

    // Step 3: Fix only the specific rooms that are missing a campfire on their path so far
    void guaranteeCampfireCoverage();
    void computeMinCampfire();

    void clearMap();

    QVector<QVector<MapNode*>> m_floors;

    int m_totalFloors;
    int m_minRooms;
    int m_maxRooms;

    int m_enemyFloor;
    int m_treasureFloor;
    int m_bossFloor;

    // Two checkpoint floors used only to verify campfire coverage - NOT forced
    // to be entirely campfire. Only individual deficient rooms get fixed here.
    int m_checkFloor1;
    int m_checkFloor2;

    MapNode *m_currentNode = nullptr;
};

#endif // GAMEMAP_H
