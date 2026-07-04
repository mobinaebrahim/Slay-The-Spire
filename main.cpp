#include "mainwindow.h"
#include "loginpage.h"
#include "registerpage.h"
#include "usermanager.h"
#include <QApplication>
#include "scoremanager.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //acces to database
    user_manager::instance();
    ScoreManager::instance();

    MainWindow w;
    w.showFullScreen();
    return a.exec();
}
