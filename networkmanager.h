#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QTcpSocket>

class NetworkManager : public QObject
{
    Q_OBJECT

public:
    static NetworkManager& instance();

    void connect_to_server(const QString &host, quint16 port);
    void send_json(const QString &message);
    bool is_connected() const;

signals:
    void message_received(const QString &message);
    void connected_to_server();
    void disconnected_from_server();

private slots:
    void on_ready_read();
    void on_connected();
    void on_disconnected();
    void create_room();


private:
    explicit NetworkManager(QObject *parent = nullptr);
    NetworkManager(const NetworkManager&) = delete;
    NetworkManager& operator=(const NetworkManager&) = delete;

    QTcpSocket *socket;
};

#endif // NETWORKMANAGER_H