#include "savemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QJsonDocument>

SaveManager& SaveManager::instance()
{
    static SaveManager instance;
    return instance;
}

SaveManager::SaveManager()
{
    create_table();
}

SaveManager::~SaveManager()
{
}

bool SaveManager::create_table()
{
    QSqlQuery query;
    bool success = query.exec(
        "CREATE TABLE IF NOT EXISTS game_saves ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "username TEXT NOT NULL, "
        "save_name TEXT NOT NULL, "
        "character TEXT, "
        "score INTEGER, "
        "floor INTEGER, "
        "game_data TEXT, "
        "save_date TEXT)"
        );

    if (!success) {
        qDebug() << "Error creating game_saves table:" << query.lastError().text();
        return false;
    }

    return true;
}

bool SaveManager::create_save(const QString &username, const QString &save_name, const QString &character,
                              int score, int floor, const QJsonObject &game_data)
{
    QJsonDocument doc(game_data);
    QString jsonString = doc.toJson(QJsonDocument::Compact);

    QSqlQuery query;
    query.prepare("INSERT INTO game_saves (username, save_name, character, score, floor, game_data, save_date) "
                  "VALUES (:username, :save_name, :character, :score, :floor, :game_data, datetime('now'))");
    query.bindValue(":username", username);
    query.bindValue(":save_name", save_name);
    query.bindValue(":character", character);
    query.bindValue(":score", score);
    query.bindValue(":floor", floor);
    query.bindValue(":game_data", jsonString);

    if (!query.exec()) {
        qDebug() << "Error creating save:" << query.lastError().text();
        return false;
    }

    return true;
}

bool SaveManager::update_save(int save_id, int score, int floor, const QJsonObject &game_data)
{
    QJsonDocument doc(game_data);
    QString jsonString = doc.toJson(QJsonDocument::Compact);

    QSqlQuery query;
    query.prepare("UPDATE game_saves SET score = :score, floor = :floor, game_data = :game_data, "
                  "save_date = datetime('now') WHERE id = :id");
    query.bindValue(":score", score);
    query.bindValue(":floor", floor);
    query.bindValue(":game_data", jsonString);
    query.bindValue(":id", save_id);

    if (!query.exec()) {
        qDebug() << "Error updating save:" << query.lastError().text();
        return false;
    }

    return true;
}

bool SaveManager::delete_save(int save_id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM game_saves WHERE id = :id");
    query.bindValue(":id", save_id);

    if (!query.exec()) {
        qDebug() << "Error deleting save:" << query.lastError().text();
        return false;
    }

    return true;
}

QJsonObject SaveManager::load_save(int save_id)
{
    QSqlQuery query;
    query.prepare("SELECT game_data FROM game_saves WHERE id = :id");
    query.bindValue(":id", save_id);

    if (!query.exec()) {
        qDebug() << "Error loading save:" << query.lastError().text();
        return QJsonObject();
    }

    if (query.next()) {
        QString jsonString = query.value("game_data").toString();
        QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8());
        return doc.object();
    }

    return QJsonObject();
}

QList<save_entry> SaveManager::get_save_list(const QString &username)
{
    QList<save_entry> results;

    QSqlQuery query;
    query.prepare("SELECT id, save_name, character, score, floor, save_date "
                  "FROM game_saves WHERE username = :username ORDER BY save_date DESC");
    query.bindValue(":username", username);

    if (!query.exec()) {
        qDebug() << "Error fetching save list:" << query.lastError().text();
        return results;
    }

    while (query.next()) {
        save_entry entry;
        entry.id = query.value("id").toInt();
        entry.save_name = query.value("save_name").toString();
        entry.character = query.value("character").toString();
        entry.score = query.value("score").toInt();
        entry.floor = query.value("floor").toInt();
        entry.save_date = query.value("save_date").toString();
        results.append(entry);
    }

    return results;
}