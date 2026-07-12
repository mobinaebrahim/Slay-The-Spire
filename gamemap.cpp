#include "gamemap.h"

#include <QRandomGenerator>
#include <QDebug>
#include <cmath>
#include <climits>

GameMap::GameMap(int totalFloors, int minRoomsPerFloor, int maxRoomsPerFloor)
    : m_totalFloors(totalFloors)
    , m_minRooms(minRoomsPerFloor)
    , m_maxRooms(maxRoomsPerFloor)
{
    // طبقه‌ی وسط به‌عنوان Treasure در نظر گرفته میشه (برای ۱۱ طبقه => اندیس ۵ یعنی طبقه ۶)
    m_enemyFloor = 0;
    m_treasureFloor = m_totalFloors / 2;
    m_bossFloor = m_totalFloors - 1;
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
    buildSkeleton();
    assignRoomTypes();
    fixCampfireRule();
}

// ============================================================
// مرحله ۱: ساخت اسکلت
// ============================================================

void GameMap::buildSkeleton()
{
    m_floors.resize(m_totalFloors);

    for (int f = 0; f < m_totalFloors; ++f) {
        int count;
        if (f == m_bossFloor) {
            count = 1; // طبقه‌ی Boss همیشه فقط یک اتاق داره
        } else {
            count = QRandomGenerator::global()->bounded(m_minRooms, m_maxRooms + 1);
        }

        for (int i = 0; i < count; ++i) {
            m_floors[f].append(new MapNode(f, i));
        }
    }

    // اتصال هر جفت طبقه‌ی پشت‌سرهم
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
        // موقعیت درصدی این اتاق در طبقه‌ی خودش
        double pct = (nA == 1) ? 0.0 : static_cast<double>(i) / (nA - 1);

        // همون درصد رو روی طبقه‌ی بعد پروجکت می‌کنیم
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
            continue; // این اتاق قبلاً حداقل یه ورودی داره، مشکلی نداره

        // بی‌راهه! نزدیک‌ترین اتاق طبقه‌ی پایین‌تر رو پیدا کن و اجباری وصلش کن
        double pctB = (nB == 1) ? 0.0 : static_cast<double>(j) / (nB - 1);
        double targetA = pctB * (nA - 1);

        int nearest = static_cast<int>(std::round(targetA));
        nearest = qBound(0, nearest, nA - 1);

        roomsA[nearest]->addChild(roomsB[j]);
    }
}

// ============================================================
// مرحله ۲: تعیین نوع اتاق‌ها
// ============================================================

void GameMap::assignRoomTypes()
{
    // قوانین ثابت - این طبقه‌ها مستقیم پر میشن، بدون رندوم
    for (MapNode *node : m_floors[m_enemyFloor])
        node->setRoomType(RoomType::ENEMY);

    for (MapNode *node : m_floors[m_treasureFloor])
        node->setRoomType(RoomType::TREASURE);

    for (MapNode *node : m_floors[m_bossFloor])
        node->setRoomType(RoomType::BOSS);

    // بقیه‌ی طبقه‌ها با رندوم + اعتبارسنجی (retry loop)
    for (int f = 0; f < m_totalFloors; ++f) {
        if (f == m_enemyFloor || f == m_treasureFloor || f == m_bossFloor)
            continue;

        for (MapNode *node : m_floors[f]) {
            RoomType chosen;
            int attempts = 0;
            const int maxAttempts = 20;

            do {
                chosen = randomRoomType(f);
                attempts++;
            } while (violatesRules(node, chosen) && attempts < maxAttempts);

            node->setRoomType(chosen);
        }
    }
}

RoomType GameMap::randomRoomType(int floorIndex) const
{
    // ELITE فقط از طبقه‌ی Treasure به بعد مجازه (تقلید از قانون بازی اصلی)
    bool eliteAllowed = (floorIndex >= m_treasureFloor);

    int roll = QRandomGenerator::global()->bounded(100); // عدد ۰ تا ۹۹

    if (eliteAllowed) {
        // ENEMY 40% | EVENT 25% | ELITE 15% | SHOP 10% | CAMPFIRE 10%
        if (roll < 40) return RoomType::ENEMY;
        if (roll < 65) return RoomType::EVENT;
        if (roll < 80) return RoomType::ELITE;
        if (roll < 90) return RoomType::SHOP;
        return RoomType::CAMPFIRE;
    } else {
        // بدون ELITE: وزنش بین بقیه پخش میشه
        // ENEMY 50% | EVENT 30% | SHOP 10% | CAMPFIRE 10%
        if (roll < 50) return RoomType::ENEMY;
        if (roll < 80) return RoomType::EVENT;
        if (roll < 90) return RoomType::SHOP;
        return RoomType::CAMPFIRE;
    }
}

bool GameMap::violatesRules(MapNode *node, RoomType candidate) const
{
    if (candidate == RoomType::CAMPFIRE) {
        // نمی‌خوایم دو تا Campfire پشت‌سرهم (مستقیم وصل) داشته باشیم
        for (MapNode *parent : node->parents()) {
            if (parent->roomType() == RoomType::CAMPFIRE)
                return true;
        }
    }
    return false;
}

// ============================================================
// مرحله ۳: تعمیر دقیق قانون «حداقل ۲ Campfire در هر مسیر»
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
                    best = 0; // احتیاط، نباید پیش بیاد چون validateFloor این حالت رو حذف کرده
            }

            int bonus = (node->roomType() == RoomType::CAMPFIRE) ? 1 : 0;
            node->setMinCampfire(best + bonus);
        }
    }
}

void GameMap::fixCampfireRule()
{
    const int maxIterations = 50;

    for (int iter = 0; iter < maxIterations; ++iter) {
        computeMinCampfire();

        MapNode *bossNode = m_floors[m_bossFloor][0];
        if (bossNode->minCampfire() >= 2)
            return; // همه‌ی مسیرها حداقل ۲ تا Campfire دارن، تمومه

        // دنبال ضعیف‌ترین مسیر می‌گردیم: از Boss به عقب، هر بار والدی رو انتخاب
        // می‌کنیم که کمترین minCampfire رو داره (یعنی بدشانس‌ترین مسیر)
        QVector<MapNode*> path;
        MapNode *current = bossNode;
        path.append(current);

        while (!current->parents().isEmpty()) {
            MapNode *worstParent = nullptr;
            int worstValue = INT_MAX;

            for (MapNode *parent : current->parents()) {
                if (parent->minCampfire() < worstValue) {
                    worstValue = parent->minCampfire();
                    worstParent = parent;
                }
            }

            current = worstParent;
            path.append(current);
        }

        // به‌جای اینکه همیشه اولین اتاق قفل‌نشده (نزدیک‌ترین به Boss) رو انتخاب کنیم،
        // همه‌ی گزینه‌های معتبر این مسیر رو جمع می‌کنیم و رندوم یکی رو انتخاب می‌کنیم.
        // این کار باعث میشه Campfireها به‌جای خوشه‌بستن دور Boss، تو کل مسیر پخش بشن.
        QVector<MapNode*> candidates;
        for (MapNode *node : path) {
            bool locked = (node->floor() == m_enemyFloor ||
                           node->floor() == m_treasureFloor ||
                           node->floor() == m_bossFloor);
            if (locked)
                continue;
            if (node->roomType() != RoomType::CAMPFIRE) {
                candidates.append(node);
            }
        }

        MapNode *toFix = nullptr;
        if (!candidates.isEmpty()) {
            int pick = QRandomGenerator::global()->bounded(candidates.size());
            toFix = candidates[pick];
        }

        if (!toFix) {
            // نباید پیش بیاد مگه اینکه نقشه خیلی کوچیک باشه؛ برای جلوگیری از
            // حلقه‌ی بی‌نهایت اینجا متوقف می‌کنیم
            qWarning() << "fixCampfireRule: no fixable node found on the weakest path";
            return;
        }

        toFix->setRoomType(RoomType::CAMPFIRE);
    }

    qWarning() << "fixCampfireRule: reached max iterations without satisfying the rule";
}

// ============================================================
// دسترسی‌ها و چاپ تست
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
    // از بالاترین طبقه (Boss) به سمت پایین چاپ می‌کنیم تا شکل نقشه رو یادآوری کنه
    for (int f = m_totalFloors - 1; f >= 0; --f) {
        QString line = QString("Floor %1: ").arg(f + 1, 2);

        for (MapNode *node : m_floors[f]) {
            line += QString("[%1]").arg(node->typeLetter());
        }

        qDebug().noquote() << line;
    }
}
