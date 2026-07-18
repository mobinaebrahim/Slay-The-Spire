#include <QCoreApplication>
#include "gameserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    GameServer server;
    if (!server.start_listening(5000)) {
        return -1;
    }

    return a.exec();
}