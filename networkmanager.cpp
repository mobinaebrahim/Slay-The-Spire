#include "networkmanager.h"
#include <QDebug>

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

void NetworkManager::send_message(const QString &message)
{
    if (socket->state() == QAbstractSocket::ConnectedState) {
        socket->write(message.toUtf8());
    } else {
        qDebug() << "Cannot send message: not connected to server!";
    }
}

bool NetworkManager::is_connected() const
{
    return socket->state() == QAbstractSocket::ConnectedState;
}

void NetworkManager::on_ready_read()
{
    QByteArray data = socket->readAll();
    QString message = QString::fromUtf8(data);
    qDebug() << "Received from server:" << message;
    emit message_received(message);
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