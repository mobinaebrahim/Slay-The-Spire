#include <QApplication>
#include <QFontDatabase>
#include <QInputDialog>

#include "mainmenuwindow.h"
#include "usermanager.h"
#include "scoremanager.h"
#include "friendmanager.h"
#include "savemanager.h"
#include "gamemap.h"
#include "networkmanager.h"
#include "cursormanager.h"


//---SMTP library---
#include "smtp/smtpclient.h"
#include "smtp/mimemessage.h"
#include "smtp/emailaddress.h"
#include "smtp/mimetext.h"


//---add for test send email---
/*bool send_verification_email(const QString &to_email, const QString &code)
{
    SmtpClient smtp_client("smtp.gmail.com", 465, SmtpClient::SslConnection);

    MimeMessage mail_message;
    mail_message.setSender(EmailAddress("stackoverflowteam.dimo@gmail.com", "Slay the Spire"));
    mail_message.addRecipient(EmailAddress(to_email));
    mail_message.setSubject("Your Verification Code");

    MimeText mail_text;
    mail_text.setText("Your verification code is: " + code);
    mail_message.addPart(&mail_text);

    smtp_client.connectToHost();
    if (!smtp_client.waitForReadyConnected()) {
        qDebug() << "STEP FAILED: connectToHost";
        return false;
    }
    qDebug() << "OK: connected";

    smtp_client.login("stackoverflowteam.dimo@gmail.com", "pqsqslspchwfbinl");
    if (!smtp_client.waitForAuthenticated()) {
        qDebug() << "STEP FAILED: login";
        return false;
    }
    qDebug() << "OK: authenticated";

    smtp_client.sendMail(mail_message);
    if (!smtp_client.waitForMailSent()) {
        qDebug() << "STEP FAILED: sendMail";
        return false;
    }
    qDebug() << "OK: mail sent";

    smtp_client.quit();
    return true;
}*/

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //---test SMTP---
    //bool result = send_verification_email("mobinaebrahim1385@gmail.com", "123456");
    //qDebug() << "Email sent result:" << result;

    //---acces to database---
    user_manager::instance();
    ScoreManager::instance();
    FriendManager::instance();
    SaveManager::instance();

    //---font---

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

    //---cursor---
    CursorManager::applySavedCursor();


    MainMenuWindow w;

    //---server test ---

    /*NetworkManager::instance().connect_to_server("127.0.0.1", 5000);

    QObject::connect(&NetworkManager::instance(), &NetworkManager::connected_to_server, [](){
        qDebug() << "TEST: Connected! Creating room...";
        NetworkManager::instance().create_room();
    });

    QObject::connect(&NetworkManager::instance(), &NetworkManager::room_created, [](const QString &code){
        qDebug() << "TEST: Room created with code:" << code;
    });

    QObject::connect(&NetworkManager::instance(), &NetworkManager::room_joined, [](const QString &code){
        qDebug() << "TEST: Joined room:" << code;
    });*/

    /*NetworkManager::instance().connect_to_server("127.0.0.1", 5000);

    QObject::connect(&NetworkManager::instance(), &NetworkManager::connected_to_server, [](){
        bool createNew = QMessageBox::question(nullptr, "Test Mode",
                                               "Create a new room? (No = Join existing room)",
                                               QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes;

        if (createNew) {
            NetworkManager::instance().create_room();
        } else {
            bool ok;
            QString code = QInputDialog::getText(nullptr, "Join Room", "Enter room code:", QLineEdit::Normal, "", &ok);
            if (ok && !code.isEmpty()) {
                NetworkManager::instance().join_room(code);
            }
        }
    });

    QObject::connect(&NetworkManager::instance(), &NetworkManager::room_created, [](const QString &code){
        qDebug() << "TEST: Room created with code:" << code;
    });

    QObject::connect(&NetworkManager::instance(), &NetworkManager::room_joined, [](const QString &code){
        qDebug() << "TEST: Joined room:" << code;
    });*/


    w.showFullScreen();
    return a.exec();
}
