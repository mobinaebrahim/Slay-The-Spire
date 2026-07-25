#include "networkmanager.h"
#include "usermanager.h"
#include <QDebug>
#include <QJsonDocument>

NetworkManager& NetworkManager::instance()
{
    static NetworkManager instance;
    return instance;
}

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
{
    socket = new QTcpSocket(this);

    connect(socket, &QTcpSocket::readyRead, this, &NetworkManager::on_ready_read);
    connect(socket, &QTcpSocket::connected, this, &NetworkManager::on_connected);
    connect(socket, &QTcpSocket::disconnected, this, &NetworkManager::on_disconnected);
}

void NetworkManager::connect_to_server(const QString &host, quint16 port)
{
    socket->connectToHost(host, port);
}

void NetworkManager::send_json(const QJsonObject &obj)
{
    if (socket->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "Cannot send: not connected to server!";
        return;
    }
    QJsonDocument doc(obj);
    socket->write(doc.toJson(QJsonDocument::Compact) + "\n");
}

void NetworkManager::create_room()
{
    m_isLeader = true;
    // به‌جای تکیه بر m_username (که ممکنه هیچ‌جا set نشده باشه)،
    // مستقیم از user_manager (همون سیستم لاگین) یوزرنیم واقعی رو می‌گیریم.
    m_username = user_manager::instance().get_current_username();

    QJsonObject msg;
    msg["type"] = "create_room";
    msg["username"] = m_username;
    send_json(msg);
}

void NetworkManager::join_room(const QString &room_code)
{
    m_isLeader = false;
    m_username = user_manager::instance().get_current_username();

    QJsonObject msg;
    msg["type"] = "join_room";
    msg["room_code"] = room_code;
    msg["username"] = m_username;
    send_json(msg);
}

void NetworkManager::send_game_action(const QJsonObject &action)
{
    send_json(action);
}

bool NetworkManager::is_connected() const
{
    return socket->state() == QAbstractSocket::ConnectedState;
}

void NetworkManager::on_ready_read()
{
    while (socket->canReadLine()) {
        QByteArray line = socket->readLine().trimmed();
        if (line.isEmpty()) continue;

        QJsonDocument doc = QJsonDocument::fromJson(line);
        if (!doc.isObject()) continue;

        QJsonObject obj = doc.object();
        QString type = obj["type"].toString();

        if (type == "room_created") {
            emit room_created(obj["room_code"].toString());
        }
        else if (type == "room_joined") {
            emit room_joined(obj["room_code"].toString());
        }
        else if (type == "error") {
            emit room_error(obj["message"].toString());
        }
        else {
            emit game_action_received(obj);
        }
    }
}

void NetworkManager::on_connected()
{
    qDebug() << "Connected to server!";
    emit connected_to_server();
}

void NetworkManager::on_disconnected()
{
    qDebug() << "Disconnected from server!";
    emit disconnected_from_server();
}