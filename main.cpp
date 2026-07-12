#include "mainwindow.h"
#include "usermanager.h"
#include <QApplication>
#include "scoremanager.h"
#include "friendmanager.h"
#include "savemanager.h"


//---SMTP library---
#include "smtp/smtpclient.h"
#include "smtp/mimemessage.h"
#include "smtp/emailaddress.h"
#include "smtp/mimetext.h"


///connect (jcowjfk= ijkppwo

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


    MainWindow w;
    w.showFullScreen();
    return a.exec();
}
