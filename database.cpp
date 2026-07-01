#include "database.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QCryptographicHash>

//console output just help me to debug it easier :)
database::database() {
    open_database();
}

database:: ~database(){
    if(data_base.isOpen()){
        data_base.close();
    }
}

bool database :: open_database(){
    data_base = QSqlDatabase::addDatabase("QSQLITE");
    data_base.setDatabaseName("users.db");

    if(!data_base.isOpen()){
        qDebug() << "Error: could not open database:" << db.lastError().text();
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

bool database ::register_user(const QString &username, const QString &password, const QString &email){

}