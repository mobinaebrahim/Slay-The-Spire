#include "gamemap.h"

#include <QRandomGenerator>
#include <QDebug>
#include <cmath>
#include <algorithm>

GameMap::GameMap(int totalFloors, int minRoomsPerFloor, int maxRoomsPerFloor)
    : m_totalFloors(totalFloors)
    , m_minRooms(minRoomsPerFloor)
    , m_maxRooms(maxRoomsPerFloor)
{
    m_enemyFloor = 0;
    m_treasureFloor = m_totalFloors / 2;
    m_bossFloor = m_totalFloors - 1;

    // Campfire floors are picked randomly each time generate() runs (see pickCampfireFloors)
    m_campfireFloor1 = -1;
    m_campfireFloor2 = -1;
}

GameMap::~GameMap()
{
    clearMap();
}

void GameMap::clearMap()
{
    for (auto &floor : m_floors) {
        qDeleteAll(floor);
        floor.clear();
    }
    m_floors.clear();
}

void GameMap::generate()
{
    clearMap();
    pickCampfireFloors();
    buildSkeleton();
    assignRoomTypes();
}

void GameMap::pickCampfireFloors()
{
    // First campfire floor: somewhere between the enemy floor and the treasure floor
    int lowStart = m_enemyFloor + 1;
    int lowEnd = m_treasureFloor - 1;
    if (lowEnd < lowStart) lowEnd = lowStart; // safety for very small maps
    m_campfireFloor1 = QRandomGenerator::global()->bounded(lowStart, lowEnd + 1);

    // Second campfire floor: somewhere between the treasure floor and the boss floor
    int highStart = m_treasureFloor + 1;
    int highEnd = m_bossFloor - 1;
    if (highEnd < highStart) highEnd = highStart; // safety for very small maps
    m_campfireFloor2 = QRandomGenerator::global()->bounded(highStart, highEnd + 1);
}

// ============================================================
// Step 1: Build the skeleton
// ============================================================

void GameMap::buildSkeleton()
{
    m_floors.resize(m_totalFloors);

    for (int f = 0; f < m_totalFloors; ++f) {
        int count;
        if (f == m_bossFloor) {
            count = 1;
        } else {
            count = QRandomGenerator::global()->bounded(m_minRooms, m_maxRooms + 1);
        }

        for (int i = 0; i < count; ++i) {
            m_floors[f].append(new MapNode(f, i));
        }
    }

    for (int f = 0; f < m_totalFloors - 1; ++f) {
        connectFloors(f, f + 1);
        validateFloor(f + 1);
    }
}

void GameMap::connectFloors(int floorA, int floorB)
{
    const QVector<MapNode*> &roomsA = m_floors[floorA];
    const QVector<MapNode*> &roomsB = m_floors[floorB];

    int nA = roomsA.size();
    int nB = roomsB.size();

    for (int i = 0; i < nA; ++i) {
        double pct = (nA == 1) ? 0.0 : static_cast<double>(i) / (nA - 1);
        double target = pct * (nB - 1);

        int lower = static_cast<int>(std::floor(target));
        int upper = static_cast<int>(std::ceil(target));

        lower = qBound(0, lower, nB - 1);
        upper = qBound(0, upper, nB - 1);

        roomsA[i]->addChild(roomsB[lower]);
        if (upper != lower) {
            roomsA[i]->addChild(roomsB[upper]);
        }
    }
}

void GameMap::validateFloor(int floorB)
{
    const QVector<MapNode*> &roomsA = m_floors[floorB - 1];
    const QVector<MapNode*> &roomsB = m_floors[floorB];

    int nA = roomsA.size();
    int nB = roomsB.size();

    for (int j = 0; j < nB; ++j) {
        if (!roomsB[j]->parents().isEmpty())
            continue;

        double pctB = (nB == 1) ? 0.0 : static_cast<double>(j) / (nB - 1);
        double targetA = pctB * (nA - 1);

        int nearest = static_cast<int>(std::round(targetA));
        nearest = qBound(0, nearest, nA - 1);

        roomsA[nearest]->addChild(roomsB[j]);
    }
}

// ============================================================
// Step 2: Assign room types
// ============================================================

void GameMap::assignRoomTypes()
{
    // Fixed rules - these floors are filled directly
    for (MapNode *node : m_floors[m_enemyFloor])
        node->setRoomType(RoomType::ENEMY);

    for (MapNode *node : m_floors[m_treasureFloor])
        node->setRoomType(RoomType::TREASURE);

    for (MapNode *node : m_floors[m_bossFloor])
        node->setRoomType(RoomType::BOSS);

    // The two campfire floors - all their rooms. Since the map is layered
    // (every path must pass through every floor), this guarantees without
    // exception that every path has exactly 2 campfires - no need to trace
    // individual paths.
    for (MapNode *node : m_floors[m_campfireFloor1])
        node->setRoomType(RoomType::CAMPFIRE);

    for (MapNode *node : m_floors[m_campfireFloor2])
        node->setRoomType(RoomType::CAMPFIRE);

    // Remaining floors - fully random, no extra campfires
    for (int f = 0; f < m_totalFloors; ++f) {
        bool locked = (f == m_enemyFloor || f == m_treasureFloor || f == m_bossFloor ||
                       f == m_campfireFloor1 || f == m_campfireFloor2);
        if (locked)
            continue;

        for (MapNode *node : m_floors[f]) {
            node->setRoomType(randomRoomType(f));
        }
    }
}

RoomType GameMap::randomRoomType(int floorIndex) const
{
    // ELITE is only allowed from the Treasure floor onward
    bool eliteAllowed = (floorIndex >= m_treasureFloor);

    int roll = QRandomGenerator::global()->bounded(100);

    if (eliteAllowed) {
        // ENEMY 45% | EVENT 25% | ELITE 20% | SHOP 10%
        if (roll < 45) return RoomType::ENEMY;
        if (roll < 70) return RoomType::EVENT;
        if (roll < 90) return RoomType::ELITE;
        return RoomType::SHOP;
    } else {
        // ENEMY 55% | EVENT 30% | SHOP 15%
        if (roll < 55) return RoomType::ENEMY;
        if (roll < 85) return RoomType::EVENT;
        return RoomType::SHOP;
    }
}

// ============================================================
// Accessors and test printing
// ============================================================

MapNode* GameMap::nodeAt(int floor, int indexInFloor) const
{
    if (floor < 0 || floor >= m_floors.size())
        return nullptr;
    if (indexInFloor < 0 || indexInFloor >= m_floors[floor].size())
        return nullptr;
    return m_floors[floor][indexInFloor];
}

int GameMap::floorCount() const
{
    return m_floors.size();
}

int GameMap::roomCountAt(int floor) const
{
    if (floor < 0 || floor >= m_floors.size())
        return 0;
    return m_floors[floor].size();
}

void GameMap::printToConsole() const
{
    for (int f = m_totalFloors - 1; f >= 0; --f) {
        QString line = QString("Floor %1: ").arg(f + 1, 2);

        for (MapNode *node : m_floors[f]) {
            line += QString("[%1]").arg(node->typeLetter());
        }

        qDebug().noquote() << line;
    }
}