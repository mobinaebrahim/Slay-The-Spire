#include "mainwindow.h"
#include "loginpage.h"
#include "registerpage.h"
#include "usermanager.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //acces to database
    user_manager::instance();

    MainWindow w;
    w.show();
    return a.exec();
}
