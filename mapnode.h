#ifndef MAPNODE_H
#define MAPNODE_H

#include <QVector>
#include <QChar>

enum class RoomType {
    UNSET,
    ENEMY,
    ELITE,
    EVENT,
    TREASURE,
    CAMPFIRE,
    BOSS,
    SHOP
};

class MapNode
{
public:
    MapNode(int floor, int indexInFloor);

    int floor() const;
    int index() const;

    RoomType roomType() const;
    void setRoomType(RoomType type);

    // Connects this node to a room on the next floor (bidirectional:
    // registers itself as parent on the child too)
    void addChild(MapNode *child);

    const QVector<MapNode*>& children() const;
    const QVector<MapNode*>& parents() const;

    bool visited() const;
    void setVisited(bool v);

    bool available() const;
    void setAvailable(bool v);

    // Used by the campfire-coverage check
    int minCampfire() const;
    void setMinCampfire(int v);

    // A single letter representing the room type, used only for console test printing
    QChar typeLetter() const;

private:
    int m_floor;
    int m_index;
    RoomType m_roomType = RoomType::UNSET;

    QVector<MapNode*> m_children; // rooms on the next floor this node connects to
    QVector<MapNode*> m_parents;  // rooms on the previous floor that connect to this node

    bool m_visited = false;
    bool m_available = false;

    int m_minCampfire = 0;
};

#endif // MAPNODE_H