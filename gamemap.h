#ifndef GAMEMAP_H
#define GAMEMAP_H

#include <QVector>
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

private:
    void buildSkeleton();
    void connectFloors(int floorA, int floorB);
    void validateFloor(int floorB);

    void assignRoomTypes();
    RoomType randomRoomType(int floorIndex) const;
    bool violatesRules(MapNode *node, RoomType candidate) const;

    void fixCampfireRule();
    void computeMinCampfire();

    void clearMap();

    QVector<QVector<MapNode*>> m_floors;

    int m_totalFloors;
    int m_minRooms;
    int m_maxRooms;

    int m_enemyFloor;
    int m_treasureFloor;
    int m_bossFloor;
};

#endif // GAMEMAP_H
