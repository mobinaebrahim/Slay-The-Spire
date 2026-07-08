#include "usermanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QCryptographicHash>


//console output just help me to debug it easier :)
user_manager& user_manager::instance()
{
    static user_manager instance;
    return instance;
}

user_manager::user_manager() {
    open_database();
}

user_manager:: ~user_manager(){
    if(data_base.isOpen()){
        data_base.close();
    }
}

bool user_manager :: open_database(){
    data_base = QSqlDatabase::addDatabase("QSQLITE");
    data_base.setDatabaseName("users.db");

    if(!data_base.open()){
        qDebug() << "Error: could not open database:" << data_base.lastError().text();
        return false;
    }

    QSqlQuery query;
    bool success = query.exec(
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "username TEXT UNIQUE NOT NULL, "
        "password TEXT NOT NULL, "
        "email TEXT, "
        "is_verified INTEGER DEFAULT 0, "
        "verification_code TEXT)"
        );

    if (!success) {
        qDebug() << "Error creating table:" << query.lastError().text();
        return false;
    }

    return true;

}

bool user_manager::usernmae_exist(const QString &username){
    QSqlQuery query;
    query.prepare("SELECT id FROM users WHERE username = :username");
    query.bindValue(":username", username);
    query.exec();
    return query.next();
}

bool user_manager::insert_user(const QString &username, const QString &hashedPassword, const QString &email, const QString &verificationCode){
    QSqlQuery query;
    query.prepare("INSERT INTO users (username, password, email, is_verified, verification_code) "
                  "VALUES (:username, :password, :email, 0, :code)");
    query.bindValue(":username", username);
    query.bindValue(":password", hashedPassword);
    query.bindValue(":email", email);
    query.bindValue(":code", verificationCode);

    if (!query.exec()) {
        qDebug() << "Error registering user:" << query.lastError().text();
        return false;
    }
    return true;
}

bool user_manager::register_user(const QString &username, const QString &password, const QString &email){
    if(usernmae_exist(username)){
        if(is_account_verified(username)) {
            qDebug() << "Username exists and is verified!";
            return false;
        }
        QSqlQuery deleteQuery;
        deleteQuery.prepare("DELETE FROM users WHERE username = :username");
        deleteQuery.bindValue(":username", username);
        deleteQuery.exec();
    }

    QString hashedPassword = QString(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());
    QString verificationCode = generate_verification_code();

    if(!insert_user(username, hashedPassword, email, verificationCode))
        return false;

    if(!send_verification_email(email, verificationCode)) {
        qDebug() << "Failed to send verification email!";
        return false;
    }

    return true;
}

bool user_manager::verify_password(const QString &username, const QString &hashedPassword){
    QSqlQuery query;
    query.prepare("SELECT id FROM users WHERE username = :username AND password = :password");
    query.bindValue(":username", username);
    query.bindValue(":password", hashedPassword);
    query.exec();

    return query.next();
}

bool user_manager::login_user(const QString &username, const QString &password){
    if (!usernmae_exist(username)) {
        qDebug() << "Username does not exist!";
        return false;
    }

    QString hashedPassword = QString(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());

    return verify_password(username, hashedPassword);
}

void user_manager::set_current_user(const QString &username,const QString &password){
    current_username = username;
    current_password = password;
}

QString user_manager::get_current_username() const{
    return current_username;
}

QString user_manager::get_current_password() const{
    return current_password;
}

QString user_manager::generate_verification_code()
{
    int code = QRandomGenerator::global()->bounded(100000, 999999);
    return QString::number(code);
}

bool user_manager::send_verification_email(const QString &to_email, const QString &code)
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
}

bool user_manager::verify_email(const QString &username, const QString &code)
{
    QSqlQuery query;
    query.prepare("SELECT id FROM users WHERE username = :username AND verification_code = :code");
    query.bindValue(":username", username);
    query.bindValue(":code", code);
    query.exec();

    if (!query.next()) {
        qDebug() << "Incorrect verification code!";
        return false;
    }

    QSqlQuery updateQuery;
    updateQuery.prepare("UPDATE users SET is_verified = 1 WHERE username = :username");
    updateQuery.bindValue(":username", username);

    if (!updateQuery.exec()) {
        qDebug() << "Error verifying account:" << updateQuery.lastError().text();
        return false;
    }

    return true;
}

bool user_manager::is_account_verified(const QString &username)
{
    QSqlQuery query;
    query.prepare("SELECT is_verified FROM users WHERE username = :username");
    query.bindValue(":username", username);
    query.exec();

    if (query.next()) {
        return query.value("is_verified").toInt() == 1;
    }

    return false;
}

bool user_manager::send_password_reset_code(const QString &username)
{
    if (!usernmae_exist(username)) {
        return false;
    }

    QString code = generate_verification_code();

    QSqlQuery query;
    query.prepare("UPDATE users SET verification_code = :code WHERE username = :username");
    query.bindValue(":code", code);
    query.bindValue(":username", username);

    if (!query.exec()) {
        qDebug() << "Error updating reset code:" << query.lastError().text();
        return false;
    }

    QSqlQuery emailQuery;
    emailQuery.prepare("SELECT email FROM users WHERE username = :username");
    emailQuery.bindValue(":username", username);
    emailQuery.exec();

    if (!emailQuery.next()) {
        return false;
    }

    QString email = emailQuery.value("email").toString();
    return send_verification_email(email, code);
}

bool user_manager::verify_reset_code(const QString &username, const QString &code)
{
    QSqlQuery query;
    query.prepare("SELECT id FROM users WHERE username = :username AND verification_code = :code");
    query.bindValue(":username", username);
    query.bindValue(":code", code);
    query.exec();

    return query.next();
}

bool user_manager::reset_password(const QString &username, const QString &new_password)
{
    QString hashedPassword = QString(QCryptographicHash::hash(new_password.toUtf8(), QCryptographicHash::Sha256).toHex());

    QSqlQuery query;
    query.prepare("UPDATE users SET password = :password WHERE username = :username");
    query.bindValue(":password", hashedPassword);
    query.bindValue(":username", username);

    if (!query.exec()) {
        qDebug() << "Error resetting password:" << query.lastError().text();
        return false;
    }

    return true;
}

