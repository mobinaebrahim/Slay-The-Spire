#include "gameserver.h"


GameServer::GameServer(QObject *parent)
    : QObject(parent)
{
    server = new QTcpServer(this);
    connect(server, &QTcpServer::newConnection, this, &GameServer::on_new_connection);
}

bool GameServer::start_listening(quint16 port)
{
    if (!server->listen(QHostAddress::Any, port)) {
        qDebug() << "Server failed to start:" << server->errorString();
        return false;
    }
    qDebug() << "Server is listening on port" << port;
    return true;
}

QString GameServer::generate_room_code()
{
    int code = QRandomGenerator::global()->bounded(100000, 999999);
    return QString::number(code);
}

void GameServer::on_new_connection()
{
    QTcpSocket *newClient = server->nextPendingConnection();
    qDebug() << "New client connected:" << newClient->peerAddress().toString();

    connect(newClient, &QTcpSocket::readyRead, this, &GameServer::on_client_data_ready);
    connect(newClient, &QTcpSocket::disconnected, this, &GameServer::on_client_disconnected);
}

void GameServer::on_client_data_ready()
{
    QTcpSocket *senderSocket = qobject_cast<QTcpSocket*>(sender());
    if (!senderSocket) return;

    while (senderSocket->canReadLine()) {
        QByteArray line = senderSocket->readLine().trimmed();
        if (line.isEmpty()) continue;

        QJsonDocument doc = QJsonDocument::fromJson(line);
        if (!doc.isObject()) {
            qDebug() << "Received invalid JSON:" << line;
            continue;
        }

        handle_message(senderSocket, doc.object());
    }
}

void GameServer::handle_message(QTcpSocket *senderSocket, const QJsonObject &message)
{
    QString type = message["type"].toString();

    if (type == "create_room") {
        handle_create_room(senderSocket, message);
    }
    else if (type == "join_room") {
        handle_join_room(senderSocket, message);
    }
    else {
        QString roomCode = client_room.value(senderSocket);
        if (roomCode.isEmpty()) {
            qDebug() << "Client not in a room, ignoring message.";
            return;
        }

        QJsonDocument forwardDoc(message);
        for (QTcpSocket *client : rooms[roomCode]) {
            if (client != senderSocket) {
                client->write(forwardDoc.toJson(QJsonDocument::Compact) + "\n");
            }
        }
    }
}

void GameServer::on_client_disconnected()
{
    QTcpSocket *disconnectedSocket = qobject_cast<QTcpSocket*>(sender());
    if (!disconnectedSocket) return;

    QString roomCode = client_room.value(disconnectedSocket);
    if (!roomCode.isEmpty()) {
        rooms[roomCode].removeAll(disconnectedSocket);

        if (roomGames.contains(roomCode)) {
            RoomGame &game = roomGames[roomCode];
            game.socketToPlayer.remove(disconnectedSocket);
            game.playerAlive.remove(disconnectedSocket);
            game.endedTurn.remove(disconnectedSocket);

            if (!rooms[roomCode].isEmpty()) {
                QTcpSocket *newLeader = rooms[roomCode][0];
                QJsonObject leaderMsg;
                leaderMsg["type"] = "leader_changed";
                leaderMsg["you_are_leader"] = true;
                send_to_client(newLeader, leaderMsg);

                for (int i = 1; i < rooms[roomCode].size(); ++i) {
                    QJsonObject notLeaderMsg;
                    notLeaderMsg["type"] = "leader_changed";
                    notLeaderMsg["you_are_leader"] = false;
                    send_to_client(rooms[roomCode][i], notLeaderMsg);
                }
            }

            if (rooms[roomCode].isEmpty()) {
                delete game.battleManager;
                roomGames.remove(roomCode);
                rooms.remove(roomCode);
            }
        }
        client_room.remove(disconnectedSocket);
    }

    socketUsernames.remove(disconnectedSocket);

    qDebug() << "Client disconnected.";
    disconnectedSocket->deleteLater();
}

// ---Broadcast helpers---

void GameServer::send_to_client(QTcpSocket *client, const QJsonObject &message)
{
    QJsonDocument doc(message);
    client->write(doc.toJson(QJsonDocument::Compact) + "\n");
}

void GameServer::broadcast_to_room(const QString &roomCode, const QJsonObject &message)
{
    QJsonDocument doc(message);
    for (QTcpSocket *client : rooms[roomCode]) {
        client->write(doc.toJson(QJsonDocument::Compact) + "\n");
    }
}

QJsonObject GameServer::buildCombatOver(bool victory)
{
    QJsonObject state;
    state["type"] = "combat_over";
    state["victory"] = victory;
    return state;
}

//--- State building---

QJsonObject GameServer::buildStateUpdate(RoomGame &game, const QString &roomCode)
{
    QJsonObject state;
    state["type"] = "state_update";
    state["is_player_turn"] = game.isPlayerTurn;

    QJsonArray playersArray;
    for (QTcpSocket *socket : rooms[roomCode]) {
        Player* p = game.socketToPlayer.value(socket, nullptr);
        if (!p) continue;

        QJsonObject pObj;
        pObj["hp"] = p->getHp();
        pObj["max_hp"] = p->getMaxHp();
        pObj["energy"] = p->getEnergy();
        pObj["max_energy"] = p->getMaxEnergy();
        pObj["is_alive"] = game.playerAlive.value(socket, false);
        pObj["username"] = QString::fromStdString(p->getName());

        QJsonArray handArray;
        for (Card* c : p->getHand()) {
            if (c) handArray.append(QString::fromStdString(c->getName()));
        }
        pObj["hand"] = handArray;

        pObj["hand_size"] = (int)p->getHand().size();
        pObj["draw_pile_size"] = p->getDrawPileSize();
        pObj["discard_pile_size"] = p->getDiscardPileSize();

        playersArray.append(pObj);
    }
    state["players"] = playersArray;

    QJsonArray enemiesArray;
    for (Enemy* e : game.battleManager->getEnemies()) {
        QJsonObject eObj;
        eObj["name"] = QString::fromStdString(e->getName());
        eObj["hp"] = e->getHp();
        eObj["max_hp"] = e->getMaxHp();
        eObj["intent"] = QString::fromStdString(e->getIntentString());
        enemiesArray.append(eObj);
    }
    state["enemies"] = enemiesArray;

    return state;
}

void GameServer::broadcast_state_update(const QString &roomCode)
{
    if (!roomGames.contains(roomCode)) return;
    RoomGame &game = roomGames[roomCode];
    if (!game.battleManager) return;

    QList<QTcpSocket*> roomClients = rooms[roomCode];
    QJsonObject baseState = buildStateUpdate(game, roomCode);
    QJsonArray basePlayers = baseState["players"].toArray();

    for (int i = 0; i < roomClients.size(); ++i) {
        QTcpSocket *socket = roomClients[i];
        QJsonObject state = baseState;
        QJsonArray playersArray = basePlayers;

        if (i < playersArray.size()) {
            QJsonObject playerObj = playersArray[i].toObject();
            playerObj["is_leader"] = (i == 0);
            playerObj["is_you"] = true;
            playersArray[i] = playerObj;
        }

        for (int j = 0; j < playersArray.size(); ++j) {
            if (j != i) {
                QJsonObject playerObj = playersArray[j].toObject();
                playerObj["is_you"] = false;
                playersArray[j] = playerObj;
            }
        }

        state["players"] = playersArray;
        send_to_client(socket, state);
    }
}

// ---Combat: initialize player deck---

void GameServer::initialize_player_deck(Player* player)
{
    for (int i = 0; i < 5; ++i) {
        Card* strike = createCardByName("Strike");
        if (strike) player->addCardToDrawPile(strike);
    }
    for (int i = 0; i < 4; ++i) {
        Card* defend = createCardByName("Defend");
        if (defend) player->addCardToDrawPile(defend);
    }
}

// ---Combat: spawn enemy---

void GameServer::spawn_enemy_for_room(RoomGame &game, const QString &enemyName)
{
    Enemy* enemy = nullptr;

    if (enemyName == "JawWorm") enemy = new JawWorm();
    else if (enemyName == "Cultist") enemy = new Cultist();
    else if (enemyName == "Louse") enemy = new Louse();
    else if (enemyName == "SmallSlime") enemy = new SmallSlime();
    else if (enemyName == "MediumSlime") enemy = new MediumSlime();
    else if (enemyName == "LargeSlime") enemy = new LargeSlime();
    else if (enemyName == "BlueSlaver") enemy = new BlueSlaver();
    else if (enemyName == "RedSlaver") enemy = new RedSlaver();
    else if (enemyName == "Looter") enemy = new Thief("Looter");
    else if (enemyName == "Mugger") enemy = new Thief("Mugger");
    else if (enemyName == "SphericGuardian") enemy = new SphericGuardian();
    else if (enemyName == "GremlinKnob") enemy = new GremlinKnob();
    else if (enemyName == "ThreeSentries") enemy = new ThreeSentries("ThreeSentries", 38, 42);
    else if (enemyName == "BookOfStabbing") enemy = new BookOfStabbing();
    else if (enemyName == "Taskmaster") enemy = new Taskmaster();
    else if (enemyName == "KingSlime") enemy = new KingSlime(game.battleManager);
    else if (enemyName == "Hexaghost") enemy = new Hexaghost();
    else if (enemyName == "TheChamp") enemy = new TheChamp();
    else enemy = new JawWorm();

    if (game.isMultiplayer && enemy) {
        int newMaxHp = enemy->getMaxHp() * 2;
        int newHp = enemy->getHp() * 2;
        enemy->setMaxHp(newMaxHp);
        enemy->setHp(newHp);
    }

    if (enemy) {
        game.battleManager->spawnEnemy(enemy);
    }
}

// ---Combat: check game over---

void GameServer::check_combat_over(const QString &roomCode)
{
    if (!roomGames.contains(roomCode)) return;
    RoomGame &game = roomGames[roomCode];
    if (!game.combatActive || !game.battleManager) return;

    if (game.battleManager->getEnemies().empty()) {
        game.combatActive = false;
        QJsonObject msg = buildCombatOver(true);
        broadcast_to_room(roomCode, msg);
        qDebug() << "Combat over - VICTORY in room:" << roomCode;
        return;
    }

    bool anyAlive = false;
    for (QTcpSocket *socket : rooms[roomCode]) {
        if (game.playerAlive.value(socket, false)) {
            anyAlive = true;
            break;
        }
    }

    if (!anyAlive) {
        game.combatActive = false;
        QJsonObject msg = buildCombatOver(false);
        broadcast_to_room(roomCode, msg);
        qDebug() << "Combat over - DEFEAT in room:" << roomCode;
    }
}

// ---Combat: leader transfer---

void GameServer::transfer_leader_if_needed(const QString &roomCode, QTcpSocket *deadSocket)
{
    if (!rooms.contains(roomCode) || rooms[roomCode].isEmpty()) return;

    QTcpSocket *currentLeader = nullptr;
    for (QTcpSocket *socket : rooms[roomCode]) {
        if (socket != deadSocket) {
            currentLeader = socket;
            break;
        }
    }

    if (!currentLeader) return;

    for (QTcpSocket *socket : rooms[roomCode]) {
        QJsonObject msg;
        msg["type"] = "leader_changed";
        msg["you_are_leader"] = (socket == currentLeader);
        send_to_client(socket, msg);
    }

    qDebug() << "Leader transferred in room:" << roomCode
             << "new leader:" << currentLeader->peerAddress().toString();
}

// ---Combat: enemy turn---

void GameServer::process_enemy_turn(const QString &roomCode)
{
    if (!roomGames.contains(roomCode)) return;
    RoomGame &game = roomGames[roomCode];
    if (!game.combatActive || !game.battleManager) return;

    game.isPlayerTurn = false;
    game.endedTurn.clear();

    game.battleManager->enemyTurn();
    game.battleManager->cleanupDeadEnemies();

    for (QTcpSocket *socket : rooms[roomCode]) {
        Player* p = game.socketToPlayer.value(socket, nullptr);
        if (p && p->getHp() <= 0 && game.playerAlive.value(socket, false)) {
            game.playerAlive[socket] = false;

            if (rooms[roomCode].indexOf(socket) == 0) {
                transfer_leader_if_needed(roomCode, socket);
            }
        }
    }

    broadcast_state_update(roomCode);
    check_combat_over(roomCode);

    if (game.combatActive) {
        game.isPlayerTurn = true;
        game.currentPlayerIndex = 0;

        for (QTcpSocket *socket : rooms[roomCode]) {
            Player* p = game.socketToPlayer.value(socket, nullptr);
            if (p && game.playerAlive.value(socket, false)) {
                p->resetEnergy();
                p->applyTurnStartEffects();
                p->drawCards(5);
            }
        }

        broadcast_state_update(roomCode);
    }
}

// ---Room management---

void GameServer::handle_create_room(QTcpSocket *senderSocket, const QJsonObject &message)
{
    QString roomCode = generate_room_code();
    rooms[roomCode].append(senderSocket);
    client_room[senderSocket] = roomCode;

    QString username = message["username"].toString("Player");
    socketUsernames[senderSocket] = username;

    QJsonObject response;
    response["type"] = "room_created";
    response["room_code"] = roomCode;
    send_to_client(senderSocket, response);

    qDebug() << "Room created:" << roomCode << "by" << username;
}

void GameServer::handle_join_room(QTcpSocket *senderSocket, const QJsonObject &message)
{
    QString roomCode = message["room_code"].toString();

    if (!rooms.contains(roomCode) || rooms[roomCode].size() >= 2) {
        QJsonObject response;
        response["type"] = "error";
        response["message"] = "Room not found or full";
        send_to_client(senderSocket, response);
        return;
    }

    rooms[roomCode].append(senderSocket);
    client_room[senderSocket] = roomCode;

    QString username = message["username"].toString("Player");
    socketUsernames[senderSocket] = username;

    QJsonObject response;
    response["type"] = "room_joined";
    response["room_code"] = roomCode;
    broadcast_to_room(roomCode, response);

    qDebug() << "Client joined room:" << roomCode << "- Room size:" << rooms[roomCode].size();
}
