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
    qDebug() << "New client connected.";

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
        QString roomCode = generate_room_code();
        rooms[roomCode].append(senderSocket);
        client_room[senderSocket] = roomCode;

        QJsonObject response;
        response["type"] = "room_created";
        response["room_code"] = roomCode;
        QJsonDocument responseDoc(response);
        senderSocket->write(responseDoc.toJson(QJsonDocument::Compact) + "\n");

        qDebug() << "Room created:" << roomCode;
    }
    else if (type == "join_room") {
        QString roomCode = message["room_code"].toString();

        if (!rooms.contains(roomCode) || rooms[roomCode].size() >= 2) {
            QJsonObject response;
            response["type"] = "error";
            response["message"] = "Room not found or full";
            QJsonDocument responseDoc(response);
            senderSocket->write(responseDoc.toJson(QJsonDocument::Compact) + "\n");
            return;
        }

        rooms[roomCode].append(senderSocket);
        client_room[senderSocket] = roomCode;

        QJsonObject response;
        response["type"] = "room_joined";
        response["room_code"] = roomCode;
        QJsonDocument responseDoc(response);

        for (QTcpSocket *client : rooms[roomCode]) {
            client->write(responseDoc.toJson(QJsonDocument::Compact) + "\n");
        }

        qDebug() << "Client joined room:" << roomCode << "- Room size:" << rooms[roomCode].size();
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
        if (rooms[roomCode].isEmpty()) {
            rooms.remove(roomCode);
        }
        client_room.remove(disconnectedSocket);
    }

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