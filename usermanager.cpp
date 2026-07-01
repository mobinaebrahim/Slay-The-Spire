#include "usermanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QCryptographicHash>

//console output just help me to debug it easier :)
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
        "email TEXT)"
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

bool user_manager::insert_user(const QString &username, const QString &hashedPassword, const QString &email){
    QSqlQuery query;
    query.prepare("INSERT INTO users (username, password, email) VALUES (:username, :password, :email)");
    query.bindValue(":username", username);
    query.bindValue(":password", hashedPassword);
    query.bindValue(":email", email);

    if (!query.exec()) {
        qDebug() << "Error registering user:" << query.lastError().text();
        return false;
    }
    return true;
}

bool user_manager::register_user(const QString &username, const QString &password, const QString &email){
    if(usernmae_exist(username)){
        qDebug() << "Username exists!";
        return false;
    }

    QString hashedPassword = QString(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());

    if(insert_user(username,hashedPassword,email))
        return true;
    return false;
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

