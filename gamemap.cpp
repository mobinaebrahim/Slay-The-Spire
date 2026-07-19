#include "gamemap.h"

#include <QRandomGenerator>
#include <QDebug>
#include <cmath>
#include <climits>
#include <algorithm>

GameMap::GameMap(int totalFloors, int minRoomsPerFloor, int maxRoomsPerFloor)
    : m_totalFloors(totalFloors)
    , m_minRooms(minRoomsPerFloor)
    , m_maxRooms(maxRoomsPerFloor)
{
    m_enemyFloor = 0;
    m_treasureFloor = m_totalFloors / 2;
    m_bossFloor = m_totalFloors - 1;
    m_checkFloor1 = -1;
    m_checkFloor2 = -1;
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
    pickCheckFloors();
    buildSkeleton();
    assignRoomTypes();
    guaranteeCampfireCoverage();
}

void GameMap::pickCheckFloors()
{
    int lowStart = m_enemyFloor + 1;
    int lowEnd = m_treasureFloor - 1;
    if (lowEnd < lowStart) lowEnd = lowStart;
    m_checkFloor1 = QRandomGenerator::global()->bounded(lowStart, lowEnd + 1);

    int highStart = m_treasureFloor + 1;
    int highEnd = m_bossFloor - 1;
    if (highEnd < highStart) highEnd = highStart;
    m_checkFloor2 = QRandomGenerator::global()->bounded(highStart, highEnd + 1);
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
// Step 2: Assign room types (includes an ordinary random chance of CAMPFIRE)
// ============================================================

void GameMap::assignRoomTypes()
{
    for (MapNode *node : m_floors[m_enemyFloor])
        node->setRoomType(RoomType::ENEMY);

    for (MapNode *node : m_floors[m_treasureFloor])
        node->setRoomType(RoomType::TREASURE);

    for (MapNode *node : m_floors[m_bossFloor])
        node->setRoomType(RoomType::BOSS);

    for (int f = 0; f < m_totalFloors; ++f) {
        bool locked = (f == m_enemyFloor || f == m_treasureFloor || f == m_bossFloor);
        if (locked)
            continue;

        for (MapNode *node : m_floors[f]) {
            node->setRoomType(randomRoomType(f));
        }
    }
}

RoomType GameMap::randomRoomType(int floorIndex) const
{
    bool eliteAllowed = (floorIndex >= m_treasureFloor);
    int roll = QRandomGenerator::global()->bounded(100);

    if (eliteAllowed) {
        if (roll < 35) return RoomType::ENEMY;
        if (roll < 57) return RoomType::EVENT;
        if (roll < 75) return RoomType::ELITE;
        if (roll < 85) return RoomType::SHOP;
        return RoomType::CAMPFIRE;
    } else {
        if (roll < 42) return RoomType::ENEMY;
        if (roll < 70) return RoomType::EVENT;
        if (roll < 85) return RoomType::SHOP;
        return RoomType::CAMPFIRE;
    }
}

// ============================================================
// Step 3: Fix only the specific rooms missing campfire coverage
// ============================================================

void GameMap::computeMinCampfire()
{
    for (int f = 0; f < m_totalFloors; ++f) {
        for (MapNode *node : m_floors[f]) {
            int best = 0;

            if (f > 0) {
                best = INT_MAX;
                for (MapNode *parent : node->parents()) {
                    best = std::min(best, parent->minCampfire());
                }
                if (best == INT_MAX)
                    best = 0;
            }

            int bonus = (node->roomType() == RoomType::CAMPFIRE) ? 1 : 0;
            node->setMinCampfire(best + bonus);
        }
    }
}

void GameMap::guaranteeCampfireCoverage()
{
    computeMinCampfire();
    for (MapNode *node : m_floors[m_checkFloor1]) {
        if (node->minCampfire() < 1) {
            node->setRoomType(RoomType::CAMPFIRE);
        }
    }

    computeMinCampfire();
    for (MapNode *node : m_floors[m_checkFloor2]) {
        if (node->minCampfire() < 2) {
            node->setRoomType(RoomType::CAMPFIRE);
        }
    }

    computeMinCampfire();
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

void GameMap::startRun()
{
    for (auto &floorVec : m_floors) {
        for (MapNode *n : floorVec) {
            n->setVisited(false);
            n->setAvailable(false);
        }
    }

    m_currentNode = nullptr;

    for (MapNode *n : m_floors[m_enemyFloor]) {
        n->setAvailable(true);
    }
}

bool GameMap::selectRoom(MapNode *target)
{
    if (!target || !target->available())
        return false;

    for (MapNode *n : m_floors[target->floor()]) {
        n->setAvailable(false);
    }

    target->setVisited(true);
    m_currentNode = target;

    for (MapNode *child : target->children()) {
        child->setAvailable(true);
    }

    return true;
}

MapNode* GameMap::currentNode() const
{
    return m_currentNode;
}

bool GameMap::isAtBoss() const
{
    return m_currentNode && m_currentNode->floor() == m_bossFloor;
}

// ============================================================
// Multiplayer sync (JSON)
// ============================================================

static QString roomTypeToString(RoomType type)
{
    switch (type) {
    case RoomType::ENEMY:    return "ENEMY";
    case RoomType::ELITE:    return "ELITE";
    case RoomType::EVENT:    return "EVENT";
    case RoomType::TREASURE: return "TREASURE";
    case RoomType::CAMPFIRE: return "CAMPFIRE";
    case RoomType::BOSS:     return "BOSS";
    case RoomType::SHOP:     return "SHOP";
    default:                 return "UNSET";
    }
}

static RoomType roomTypeFromString(const QString &str)
{
    if (str == "ENEMY")    return RoomType::ENEMY;
    if (str == "ELITE")    return RoomType::ELITE;
    if (str == "EVENT")    return RoomType::EVENT;
    if (str == "TREASURE") return RoomType::TREASURE;
    if (str == "CAMPFIRE") return RoomType::CAMPFIRE;
    if (str == "BOSS")     return RoomType::BOSS;
    if (str == "SHOP")     return RoomType::SHOP;
    return RoomType::UNSET;
}

QJsonObject GameMap::toJson() const
{
    QJsonObject obj;
    obj["totalFloors"] = m_totalFloors;

    QJsonArray floorsArr;
    for (const auto &floorVec : m_floors) {
        QJsonArray floorArr;
        for (MapNode *node : floorVec) {
            QJsonObject n;
            n["index"] = node->index();
            n["roomType"] = roomTypeToString(node->roomType());

            QJsonArray childrenArr;
            for (MapNode *child : node->children()) {
                childrenArr.append(child->index());
            }
            n["children"] = childrenArr;

            floorArr.append(n);
        }
        floorsArr.append(floorArr);
    }
    obj["floors"] = floorsArr;
    return obj;
}

void GameMap::fromJson(const QJsonObject &obj)
{
    clearMap();

    m_totalFloors = obj["totalFloors"].toInt();
    m_enemyFloor = 0;
    m_bossFloor = m_totalFloors - 1;
    m_treasureFloor = m_totalFloors / 2;

    QJsonArray floorsArr = obj["floors"].toArray();
    m_floors.resize(floorsArr.size());

    for (int f = 0; f < floorsArr.size(); ++f) {
        QJsonArray floorArr = floorsArr[f].toArray();
        for (const QJsonValue &v : floorArr) {
            QJsonObject n = v.toObject();
            int idx = n["index"].toInt();
            MapNode *node = new MapNode(f, idx);
            node->setRoomType(roomTypeFromString(n["roomType"].toString()));
            m_floors[f].append(node);
        }
    }

    for (int f = 0; f < floorsArr.size(); ++f) {
        QJsonArray floorArr = floorsArr[f].toArray();
        for (int i = 0; i < floorArr.size(); ++i) {
            QJsonObject n = floorArr[i].toObject();
            QJsonArray childrenArr = n["children"].toArray();
            MapNode *node = m_floors[f][i];

            for (const QJsonValue &cv : childrenArr) {
                int childIdx = cv.toInt();
                if (f + 1 < m_floors.size() && childIdx < m_floors[f + 1].size()) {
                    node->addChild(m_floors[f + 1][childIdx]);
                }
            }
        }
    }
}

/*#include "gamemap.h"

#include <QRandomGenerator>
#include <QDebug>
#include <cmath>
#include <climits>
#include <algorithm>

GameMap::GameMap(int totalFloors, int minRoomsPerFloor, int maxRoomsPerFloor)
    : m_totalFloors(totalFloors)
    , m_minRooms(minRoomsPerFloor)
    , m_maxRooms(maxRoomsPerFloor)
{
    m_enemyFloor = 0;
    m_treasureFloor = m_totalFloors / 2;
    m_bossFloor = m_totalFloors - 1;
    m_checkFloor1 = -1;
    m_checkFloor2 = -1;
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
    pickCheckFloors();
    buildSkeleton();
    assignRoomTypes();
    guaranteeCampfireCoverage();
}

void GameMap::pickCheckFloors()
{
    // Small buffer of 1 floor before each checkpoint (not too tight, so the
    // checkpoint floor can land anywhere across the map, not just squeezed
    // near the end).
    int lowStart = m_enemyFloor + 1;
    int lowEnd = m_treasureFloor - 1;
    if (lowEnd < lowStart) lowEnd = lowStart;
    m_checkFloor1 = QRandomGenerator::global()->bounded(lowStart, lowEnd + 1);

    int highStart = m_treasureFloor + 1;
    int highEnd = m_bossFloor - 1;
    if (highEnd < highStart) highEnd = highStart;
    m_checkFloor2 = QRandomGenerator::global()->bounded(highStart, highEnd + 1);
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
// Step 2: Assign room types (includes an ordinary random chance of CAMPFIRE)
// ============================================================

void GameMap::assignRoomTypes()
{
    for (MapNode *node : m_floors[m_enemyFloor])
        node->setRoomType(RoomType::ENEMY);

    for (MapNode *node : m_floors[m_treasureFloor])
        node->setRoomType(RoomType::TREASURE);

    for (MapNode *node : m_floors[m_bossFloor])
        node->setRoomType(RoomType::BOSS);

    // Every other floor - ordinary random assignment, including a normal
    // chance of CAMPFIRE like any other room type. No floor is dedicated
    // entirely to campfire at this stage.
    for (int f = 0; f < m_totalFloors; ++f) {
        bool locked = (f == m_enemyFloor || f == m_treasureFloor || f == m_bossFloor);
        if (locked)
            continue;

        for (MapNode *node : m_floors[f]) {
            node->setRoomType(randomRoomType(f));
        }
    }
}

RoomType GameMap::randomRoomType(int floorIndex) const
{
    bool eliteAllowed = (floorIndex >= m_treasureFloor);
    int roll = QRandomGenerator::global()->bounded(100);

    if (eliteAllowed) {
        // ENEMY 35% | EVENT 22% | ELITE 18% | SHOP 10% | CAMPFIRE 15%
        if (roll < 35) return RoomType::ENEMY;
        if (roll < 57) return RoomType::EVENT;
        if (roll < 75) return RoomType::ELITE;
        if (roll < 85) return RoomType::SHOP;
        return RoomType::CAMPFIRE;
    } else {
        // ENEMY 42% | EVENT 28% | SHOP 15% | CAMPFIRE 15%
        if (roll < 42) return RoomType::ENEMY;
        if (roll < 70) return RoomType::EVENT;
        if (roll < 85) return RoomType::SHOP;
        return RoomType::CAMPFIRE;
    }
}

// ============================================================
// Step 3: Fix only the specific rooms missing campfire coverage
// ============================================================

void GameMap::computeMinCampfire()
{
    for (int f = 0; f < m_totalFloors; ++f) {
        for (MapNode *node : m_floors[f]) {
            int best = 0;

            if (f > 0) {
                best = INT_MAX;
                for (MapNode *parent : node->parents()) {
                    best = std::min(best, parent->minCampfire());
                }
                if (best == INT_MAX)
                    best = 0;
            }

            int bonus = (node->roomType() == RoomType::CAMPFIRE) ? 1 : 0;
            node->setMinCampfire(best + bonus);
        }
    }
}

void GameMap::guaranteeCampfireCoverage()
{
    // First pass: based on whatever campfires random chance already placed,
    // see which rooms on checkFloor1 have NOT seen a single campfire yet.
    computeMinCampfire();
    for (MapNode *node : m_floors[m_checkFloor1]) {
        if (node->minCampfire() < 1) {
            node->setRoomType(RoomType::CAMPFIRE);
        }
    }

    // Recompute (checkFloor1 changed some room types), then check the second
    // checkpoint against the "at least 2" requirement.
    computeMinCampfire();
    for (MapNode *node : m_floors[m_checkFloor2]) {
        if (node->minCampfire() < 2) {
            node->setRoomType(RoomType::CAMPFIRE);
        }
    }

    // Recompute once more so floors AFTER checkFloor2 (including the boss)
    // reflect the fix - otherwise their cached values would be stale.
    computeMinCampfire();
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

void GameMap::startRun()
{
    for (auto &floorVec : m_floors) {
        for (MapNode *n : floorVec) {
            n->setVisited(false);
            n->setAvailable(false);
        }
    }

    m_currentNode = nullptr;

    for (MapNode *n : m_floors[m_enemyFloor]) {
        n->setAvailable(true);
    }
}

bool GameMap::selectRoom(MapNode *target)
{
    if (!target || !target->available())
        return false;

    for (MapNode *n : m_floors[target->floor()]) {
        n->setAvailable(false);
    }

    target->setVisited(true);
    m_currentNode = target;

    for (MapNode *child : target->children()) {
        child->setAvailable(true);
    }

    return true;
}

MapNode* GameMap::currentNode() const
{
    return m_currentNode;
}

bool GameMap::isAtBoss() const
{
    return m_currentNode && m_currentNode->floor() == m_bossFloor;
}*/