#include <QApplication>
#include <QFontDatabase>
#include <QInputDialog>
#include <QResource>

#include "mainmenuwindow.h"
#include "usermanager.h"
#include "scoremanager.h"
#include "friendmanager.h"
#include "savemanager.h"
#include "gamemap.h"
#include "networkmanager.h"
#include "cursormanager.h"
#include "mappage.h"

#include "smtp/smtpclient.h"
#include "smtp/mimemessage.h"
#include "smtp/emailaddress.h"
#include "smtp/mimetext.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QResource::registerResource(QCoreApplication::applicationDirPath() + "/resources.rcc");

    user_manager::instance();
    ScoreManager::instance();
    FriendManager::instance();
    SaveManager::instance();

    QFontDatabase::addApplicationFont(":/assets/font/Cinzel-Regular.ttf");
    QFontDatabase::addApplicationFont(":/assets/font/Cinzel-Medium.ttf");
    QFontDatabase::addApplicationFont(":/assets/font/Cinzel-SemiBold.ttf");
    QFontDatabase::addApplicationFont(":/assets/font/Cinzel-Bold.ttf");
    QFontDatabase::addApplicationFont(":/assets/font/Cinzel-ExtraBold.ttf");
    QFontDatabase::addApplicationFont(":/assets/font/Cinzel-Black.ttf");
    QFontDatabase::addApplicationFont(":/assets/font/Cinzel-VariableFont_wght.ttf");
    QFontDatabase::addApplicationFont(":/assets/font/Marcellus-Regular.ttf");
    QFontDatabase::addApplicationFont(":/assets/font/IMFellEnglish-Regular.ttf");
    QFontDatabase::addApplicationFont(":/assets/font/IMFellEnglish-Italic.ttf");
    QFontDatabase::addApplicationFont(":/assets/font/RINGM___.TTF");

    CursorManager::applySavedCursor();

    MainMenuWindow w;

    NetworkManager::instance().connect_to_server("127.0.0.1", 5000);


    w.showFullScreen();
    return a.exec();
}