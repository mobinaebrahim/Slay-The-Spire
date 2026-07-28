#include "friendmanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QSqlDatabase>

static bool ensureDatabaseOpen() {
    QSqlDatabase db;
    if (QSqlDatabase::contains("qt_sql_default_connection")) {
        db = QSqlDatabase::database("qt_sql_default_connection");
    } else {
        db = QSqlDatabase::addDatabase("QSQLITE");
    }
    if (db.isOpen()) return true;
    db.setDatabaseName("users.db");
    if (!db.open()) {
        qDebug() << "Error opening database:" << db.lastError().text();
        return false;
    }
    return true;
}

FriendManager& FriendManager::instance()
{
    static FriendManager instance;
    return instance;
}

FriendManager::FriendManager()
{
    createTable();
    createGameInvitesTable();
}

FriendManager::~FriendManager()
{
}

bool FriendManager::createTable()
{
    if (!ensureDatabaseOpen()) return false;

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
    if (fromUsername.isEmpty() || toUsername.isEmpty()) {
        qDebug() << "Invalid usernames for friend request";
        return false;
    }

    if (fromUsername == toUsername) {
        qDebug() << "Cannot send friend request to yourself!";
        return false;
    }

    QSqlQuery checkReverse;
    checkReverse.prepare("SELECT id FROM friendships WHERE user_a = :toUser AND user_b = :fromUser AND status = 'pending'");
    checkReverse.bindValue(":toUser", toUsername);
    checkReverse.bindValue(":fromUser", fromUsername);
    if (!checkReverse.exec()) {
        qDebug() << "Error checking reverse request:" << checkReverse.lastError().text();
        return false;
    }
    if (checkReverse.next()) {
        return acceptFriendRequest(fromUsername, toUsername);
    }

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

    return query.numRowsAffected() > 0;
}

bool FriendManager::removeFriend(const QString &username, const QString &friendUsername)
{
    if (username.isEmpty() || friendUsername.isEmpty()) return false;

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

    return query.numRowsAffected() > 0;
}

bool FriendManager::areFriends(const QString &username1, const QString &username2)
{
    if (username1.isEmpty() || username2.isEmpty()) return false;

    QSqlQuery query;
    query.prepare("SELECT id FROM friendships "
                  "WHERE (user_a = :user1 AND user_b = :user2) OR (user_a = :user2 AND user_b = :user1)");
    query.bindValue(":user1", username1);
    query.bindValue(":user2", username2);

    if (!query.exec()) {
        qDebug() << "Error checking friendship:" << query.lastError().text();
        return false;
    }

    return query.next();
}

QStringList FriendManager::getFriendsList(const QString &username)
{
    QStringList friends;
    if (username.isEmpty()) return friends;

    QSqlQuery query;
    query.prepare("SELECT user_a, user_b FROM friendships "
                  "WHERE (user_a = :username OR user_b = :username) AND status = 'accepted'");
    query.bindValue(":username", username);

    if (!query.exec()) {
        qDebug() << "Error fetching friends list:" << query.lastError().text();
        return friends;
    }

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
    if (username.isEmpty()) return requesters;

    QSqlQuery query;
    query.prepare("SELECT user_a FROM friendships "
                  "WHERE user_b = :username AND status = 'pending'");
    query.bindValue(":username", username);

    if (!query.exec()) {
        qDebug() << "Error fetching pending requests:" << query.lastError().text();
        return requesters;
    }

    while (query.next()) {
        requesters.append(query.value("user_a").toString());
    }

    return requesters;
}

bool FriendManager::createGameInvitesTable()
{
    if (!ensureDatabaseOpen()) return false;

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
    if (fromUsername.isEmpty() || toUsername.isEmpty() || roomCode.isEmpty()) {
        qDebug() << "Invalid game invite parameters";
        return false;
    }

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

bool FriendManager::rejectGameInvite(const QString &username, const QString &fromUsername)
{
    QSqlQuery query;
    query.prepare("DELETE FROM game_invites WHERE from_user = :fromUser AND to_user = :username");
    query.bindValue(":fromUser", fromUsername);
    query.bindValue(":username", username);

    if (!query.exec()) {
        qDebug() << "Error rejecting game invite:" << query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}

QList<GameInviteEntry> FriendManager::getPendingGameInvites(const QString &username)
{
    QList<GameInviteEntry> invites;
    if (username.isEmpty()) return invites;

    QSqlQuery query;
    query.prepare("SELECT from_user, room_code FROM game_invites WHERE to_user = :username AND status = 'pending'");
    query.bindValue(":username", username);

    if (!query.exec()) {
        qDebug() << "Error fetching pending game invites:" << query.lastError().text();
        return invites;
    }

    while (query.next()) {
        GameInviteEntry entry;
        entry.from_user = query.value("from_user").toString();
        entry.room_code = query.value("room_code").toString();
        invites.append(entry);
    }
    return invites;
}

bool FriendManager::acceptGameInvite(const QString &username, const QString &fromUsername, QString &outRoomCode)
{
    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();

    QSqlQuery query;
    query.prepare("SELECT room_code FROM game_invites "
                  "WHERE from_user = :fromUser AND to_user = :username AND status = 'pending'");
    query.bindValue(":fromUser", fromUsername);
    query.bindValue(":username", username);

    if (!query.exec()) {
        qDebug() << "Error fetching game invite:" << query.lastError().text();
        db.rollback();
        return false;
    }

    if (!query.next()) {
        db.rollback();
        return false;
    }

    outRoomCode = query.value("room_code").toString();

    QSqlQuery deleteQuery;
    deleteQuery.prepare("DELETE FROM game_invites WHERE from_user = :fromUser AND to_user = :username");
    deleteQuery.bindValue(":fromUser", fromUsername);
    deleteQuery.bindValue(":username", username);

    if (!deleteQuery.exec()) {
        qDebug() << "Error deleting accepted game invite:" << deleteQuery.lastError().text();
        db.rollback();
        return false;
    }

    db.commit();
    return true;
}

// ... (کد قبلی تا قبل acceptGameInvite همونه) ...

bool FriendManager::peekGameInviteRoomCode(const QString &username, const QString &fromUsername, QString &outRoomCode)
{
    QSqlQuery query;
    query.prepare("SELECT room_code FROM game_invites "
                  "WHERE from_user = :fromUser AND to_user = :username AND status = 'pending'");
    query.bindValue(":fromUser", fromUsername);
    query.bindValue(":username", username);

    if (!query.exec()) {
        qDebug() << "Error peeking game invite:" << query.lastError().text();
        return false;
    }

    if (!query.next()) {
        return false;
    }

    outRoomCode = query.value("room_code").toString();
    return true;
}

bool FriendManager::deleteGameInvite(const QString &username, const QString &fromUsername)
{
    QSqlQuery query;
    query.prepare("DELETE FROM game_invites WHERE from_user = :fromUser AND to_user = :username");
    query.bindValue(":fromUser", fromUsername);
    query.bindValue(":username", username);

    if (!query.exec()) {
        qDebug() << "Error deleting game invite:" << query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}