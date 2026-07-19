#include "gameserver.h"
#include <QDebug>

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
    clients.append(newClient);

    qDebug() << "New client connected. Total clients:" << clients.size();

    connect(newClient, &QTcpSocket::readyRead, this, &GameServer::on_client_data_ready);
    connect(newClient, &QTcpSocket::disconnected, this, &GameServer::on_client_disconnected);
}

void GameServer::on_client_data_ready()
{
    QTcpSocket *senderSocket = qobject_cast<QTcpSocket*>(sender());
    if (!senderSocket) return;

    QByteArray data = senderSocket->readAll();
    qDebug() << "Received data:" << data;

    for (QTcpSocket *client : clients) {
        if (client != senderSocket) {
            client->write(data);
        }
    }
}

void GameServer::on_client_disconnected()
{
    QTcpSocket *disconnectedSocket = qobject_cast<QTcpSocket*>(sender());
    if (!disconnectedSocket) return;

    clients.removeAll(disconnectedSocket);
    qDebug() << "Client disconnected. Total clients:" << clients.size();

    disconnectedSocket->deleteLater();
}

