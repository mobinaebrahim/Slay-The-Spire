#ifndef GAMESERVER_H
#define GAMESERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>

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
    QList<QTcpSocket*> clients;
};

#endif // GAMESERVER_H