#ifndef GAMESERVER_H
#define GAMESERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>
#include <QList>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>


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

    QString generate_room_code();
    void handle_message(QTcpSocket *sender, const QJsonObject &message);
};

#endif // GAMESERVER_H