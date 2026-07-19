#include "friendmanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

FriendManager& FriendManager::instance()
{
    static FriendManager instance;
    return instance;
}

FriendManager::FriendManager()
{
    createTable();
}

FriendManager::~FriendManager()
{
}

bool FriendManager::createTable()
{
    QSqlQuery query;
    bool success = query.exec(
        "CREATE TABLE IF NOT EXISTS friendships ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "user_a TEXT NOT NULL, "
        "user_b TEXT NOT NULL, "
        "status TEXT NOT NULL)"
        );

    if (!success) {
        qDebug() << "Error creating friendships table:" << query.lastError().text();
        return false;
    }

    return true;
}

bool FriendManager::sendFriendRequest(const QString &fromUsername, const QString &toUsername)
{
    if (areFriends(fromUsername, toUsername)) {
        qDebug() << "Already friends or request pending!";
        return false;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO friendships (user_a, user_b, status) VALUES (:fromUser, :toUser, 'pending')");
    query.bindValue(":fromUser", fromUsername);
    query.bindValue(":toUser", toUsername);

    if (!query.exec()) {
        qDebug() << "Error sending friend request:" << query.lastError().text();
        return false;
    }

    return true;
}

bool FriendManager::acceptFriendRequest(const QString &username, const QString &requesterUsername)
{
    QSqlQuery query;
    query.prepare("UPDATE friendships SET status = 'accepted' "
                  "WHERE user_a = :requester AND user_b = :username AND status = 'pending'");
    query.bindValue(":requester", requesterUsername);
    query.bindValue(":username", username);

    if (!query.exec()) {
        qDebug() << "Error accepting friend request:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

bool FriendManager::rejectFriendRequest(const QString &username, const QString &requesterUsername)
{
    QSqlQuery query;
    query.prepare("DELETE FROM friendships "
                  "WHERE user_a = :requester AND user_b = :username AND status = 'pending'");
    query.bindValue(":requester", requesterUsername);
    query.bindValue(":username", username);

    if (!query.exec()) {
        qDebug() << "Error rejecting friend request:" << query.lastError().text();
        return false;
    }

    return true;
}

bool FriendManager::removeFriend(const QString &username, const QString &friendUsername)
{
    QSqlQuery query;
    query.prepare("DELETE FROM friendships "
                  "WHERE status = 'accepted' AND "
                  "((user_a = :user1 AND user_b = :user2) OR (user_a = :user2 AND user_b = :user1))");
    query.bindValue(":user1", username);
    query.bindValue(":user2", friendUsername);

    if (!query.exec()) {
        qDebug() << "Error removing friend:" << query.lastError().text();
        return false;
    }

    return true;
}

bool FriendManager::areFriends(const QString &username1, const QString &username2)
{
    QSqlQuery query;
    query.prepare("SELECT id FROM friendships "
                  "WHERE (user_a = :user1 AND user_b = :user2) OR (user_a = :user2 AND user_b = :user1)");
    query.bindValue(":user1", username1);
    query.bindValue(":user2", username2);
    query.exec();

    return query.next();
}

QStringList FriendManager::getFriendsList(const QString &username)
{
    QStringList friends;

    QSqlQuery query;
    query.prepare("SELECT user_a, user_b FROM friendships "
                  "WHERE (user_a = :username OR user_b = :username) AND status = 'accepted'");
    query.bindValue(":username", username);
    query.exec();

    while (query.next()) {
        QString userA = query.value("user_a").toString();
        QString userB = query.value("user_b").toString();
        friends.append(userA == username ? userB : userA);
    }

    return friends;
}

QStringList FriendManager::getPendingRequests(const QString &username)
{
    QStringList requesters;

    QSqlQuery query;
    query.prepare("SELECT user_a FROM friendships "
                  "WHERE user_b = :username AND status = 'pending'");
    query.bindValue(":username", username);
    query.exec();

    while (query.next()) {
        requesters.append(query.value("user_a").toString());
    }

    return requesters;
}

bool FriendManager::createGameInvitesTable()
{
    QSqlQuery query;
    bool success = query.exec(
        "CREATE TABLE IF NOT EXISTS game_invites ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "from_user TEXT NOT NULL, "
        "to_user TEXT NOT NULL, "
        "room_code TEXT NOT NULL, "
        "status TEXT NOT NULL)"
        );

    if (!success) {
        qDebug() << "Error creating game_invites table:" << query.lastError().text();
        return false;
    }
    return true;
}

bool FriendManager::sendGameInvite(const QString &fromUsername, const QString &toUsername, const QString &roomCode)
{
    QSqlQuery query;
    query.prepare("INSERT INTO game_invites (from_user, to_user, room_code, status) "
                  "VALUES (:fromUser, :toUser, :roomCode, 'pending')");
    query.bindValue(":fromUser", fromUsername);
    query.bindValue(":toUser", toUsername);
    query.bindValue(":roomCode", roomCode);

    if (!query.exec()) {
        qDebug() << "Error sending game invite:" << query.lastError().text();
        return false;
    }
    return true;
}
