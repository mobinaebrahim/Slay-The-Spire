#ifndef GAMESERVER_H
#define GAMESERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>
#include <QList>
#include <QSet>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRandomGenerator>

#include "BattleManager.h"
#include "player.h"
#include "enemy.h"
#include "NormalEnemies.h"
#include "EliteEnemies.h"
#include "BossStruggles.h"
#include "CardFactory.h"

struct PlayerRunState {
    int hp = 80;
    int maxHp = 80;
    int gold = 99;
    QStringList deckCardNames;
    bool wasAlive = true;
};

struct RoomGame {
    BattleManager* battleManager = nullptr;
    QMap<QTcpSocket*, Player*> socketToPlayer;
    QMap<QTcpSocket*, bool> playerAlive;
    QSet<QTcpSocket*> endedTurn;
    bool combatActive = false;
    bool isPlayerTurn = true;
    int currentPlayerIndex = 0;
    QString currentEnemyName;
    bool isMultiplayer = false;

    // Persistent run data
    QMap<QTcpSocket*, PlayerRunState> playerRunStates;
    QJsonObject mapData;
};

class GameServer : public QObject
{
    Q_OBJECT

public:
    explicit GameServer(QObject *parent = nullptr);
    bool start_listening(quint16 port);

private slots:
    void on_new_connection();
    void on_client_data_ready();
    void on_client_disconnected();

private:
    QTcpServer *server;
    QMap<QString, QList<QTcpSocket*>> rooms;
    QMap<QTcpSocket*, QString> client_room;
    QMap<QString, RoomGame> roomGames;
    QMap<QTcpSocket*, QString> socketUsernames;

    QString generate_room_code();
    void handle_message(QTcpSocket *sender, const QJsonObject &message);

    // Room management
    void handle_create_room(QTcpSocket *sender, const QJsonObject &message);
    void handle_join_room(QTcpSocket *sender, const QJsonObject &message);

    // Combat handlers
    void handle_start_combat(QTcpSocket *sender, const QJsonObject &message);
    void handle_play_card(QTcpSocket *sender, const QJsonObject &message);
    void handle_end_turn(QTcpSocket *sender, const QJsonObject &message);

    // Map / Room sync
    void handle_map_data(QTcpSocket *sender, const QJsonObject &message);
    void handle_room_selected(QTcpSocket *sender, const QJsonObject &message);

    // Game logic
    void initialize_player_deck(Player* player);
    void spawn_enemy_for_room(RoomGame &game, const QString &enemyName);
    void process_enemy_turn(const QString &roomCode);
    void check_combat_over(const QString &roomCode);
    void transfer_leader_if_needed(const QString &roomCode, QTcpSocket *deadSocket);

    // Broadcast
    void send_to_client(QTcpSocket *client, const QJsonObject &message);
    void broadcast_to_room(const QString &roomCode, const QJsonObject &message);
    void broadcast_state_update(const QString &roomCode);
    QJsonObject buildStateUpdate(RoomGame &game, const QString &roomCode);
    QJsonObject buildCombatOver(bool victory);
};

#endif // GAMESERVER_H