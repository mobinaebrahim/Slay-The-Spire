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

    // Prints the map to the console for testing without a GUI
    void printToConsole() const;

    MapNode* nodeAt(int floor, int indexInFloor) const;
    int floorCount() const;
    int roomCountAt(int floor) const;

private:
    void pickCampfireFloors();

    // Step 1: Build the skeleton (room counts + connections)
    void buildSkeleton();
    void connectFloors(int floorA, int floorB);
    void validateFloor(int floorB);

    // Step 2: Assign room types
    void assignRoomTypes();
    RoomType randomRoomType(int floorIndex) const;

    void clearMap();

    QVector<QVector<MapNode*>> m_floors;

    int m_totalFloors;
    int m_minRooms;
    int m_maxRooms;

    // Indices of the locked floors (computed from m_totalFloors)
    int m_enemyFloor;
    int m_treasureFloor;
    int m_bossFloor;

    // The two floors that become fully campfire (spread across the map, not adjacent)
    int m_campfireFloor1;
    int m_campfireFloor2;
};

#endif // GAMEMAP_H