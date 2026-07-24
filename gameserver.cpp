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

// ============================================================
// Broadcast helpers
// ============================================================

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