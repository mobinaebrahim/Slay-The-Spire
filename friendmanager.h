#ifndef FRIENDMANAGER_H
#define FRIENDMANAGER_H
#include <QString>
#include <QStringList>
#include <QList>

struct GameInviteEntry {
    QString from_user;
    QString room_code;
};

class FriendManager
{
public:
    static FriendManager& instance();

    bool sendFriendRequest(const QString &fromUsername, const QString &toUsername);
    bool acceptFriendRequest(const QString &username, const QString &requesterUsername);
    bool rejectFriendRequest(const QString &username, const QString &requesterUsername);
    bool removeFriend(const QString &username, const QString &friendUsername);
    bool areFriends(const QString &username1, const QString &username2);

    QStringList getFriendsList(const QString &username);
    QStringList getPendingRequests(const QString &username);

private:
    FriendManager();
    ~FriendManager();
    FriendManager(const FriendManager&) = delete;
    FriendManager& operator=(const FriendManager&) = delete;
    bool createTable();

public:
    bool sendGameInvite(const QString &fromUsername, const QString &toUsername, const QString &roomCode);
    bool acceptGameInvite(const QString &username, const QString &fromUsername, QString &outRoomCode);
    bool rejectGameInvite(const QString &username, const QString &fromUsername);
    QList<GameInviteEntry> getPendingGameInvites(const QString &username);

    // NEW: non-destructive peek + explicit delete
    bool peekGameInviteRoomCode(const QString &username, const QString &fromUsername, QString &outRoomCode);
    bool deleteGameInvite(const QString &username, const QString &fromUsername);

private:
    bool createGameInvitesTable();
};

#endif // FRIENDMANAGER_H