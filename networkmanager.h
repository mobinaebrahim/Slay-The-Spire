#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QTcpSocket>
#include <QJsonObject>

class NetworkManager : public QObject
{
    Q_OBJECT

public:
    static NetworkManager& instance();

    void connect_to_server(const QString &host, quint16 port);
    void create_room();
    void join_room(const QString &room_code);
    void send_game_action(const QJsonObject &action);
    bool is_connected() const;

    void setIsLeader(bool leader) { m_isLeader = leader; }
    bool isLeader() const { return m_isLeader; }

signals:
    void connected_to_server();
    void disconnected_from_server();
    void room_created(const QString &room_code);
    void room_joined(const QString &room_code);
    void room_error(const QString &error_message);
    void game_action_received(const QJsonObject &action);

private slots:
    void on_ready_read();
    void on_connected();
    void on_disconnected();

private:
    explicit NetworkManager(QObject *parent = nullptr);
    NetworkManager(const NetworkManager&) = delete;
    NetworkManager& operator=(const NetworkManager&) = delete;

    void send_json(const QJsonObject &obj);

    QTcpSocket *socket;
    bool m_isLeader = false;
};

#endif // NETWORKMANAGER_H