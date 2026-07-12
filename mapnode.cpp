#include "mapnode.h"

MapNode::MapNode(int floor, int indexInFloor)
    : m_floor(floor)
    , m_index(indexInFloor)
{
}

int MapNode::floor() const
{
    return m_floor;
}

int MapNode::index() const
{
    return m_index;
}

RoomType MapNode::roomType() const
{
    return m_roomType;
}

void MapNode::setRoomType(RoomType type)
{
    m_roomType = type;
}

void MapNode::addChild(MapNode *child)
{
    if (!child)
        return;

    // Prevent duplicate connections (e.g. if validation tries to add the same link twice)
    if (m_children.contains(child))
        return;

    m_children.append(child);
    child->m_parents.append(this);
}

const QVector<MapNode*>& MapNode::children() const
{
    return m_children;
}

const QVector<MapNode*>& MapNode::parents() const
{
    return m_parents;
}

bool MapNode::visited() const
{
    return m_visited;
}

void MapNode::setVisited(bool v)
{
    m_visited = v;
}

bool MapNode::available() const
{
    return m_available;
}

void MapNode::setAvailable(bool v)
{
    m_available = v;
}

QChar MapNode::typeLetter() const
{
    switch (m_roomType) {
    case RoomType::ENEMY:    return QChar('E');
    case RoomType::ELITE:    return QChar('L');
    case RoomType::EVENT:    return QChar('V');
    case RoomType::TREASURE: return QChar('T');
    case RoomType::CAMPFIRE: return QChar('C');
    case RoomType::BOSS:     return QChar('B');
    case RoomType::SHOP:     return QChar('S');
    default:                 return QChar('?');
    }
}