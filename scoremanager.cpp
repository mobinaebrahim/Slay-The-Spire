#include "scoremanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

ScoreManager& ScoreManager::instance()
{
    static ScoreManager instance;
    return instance;
}

ScoreManager::ScoreManager()
{
    create_table();
}

ScoreManager::~ScoreManager()
{
}

bool ScoreManager::create_table()
{
    QSqlQuery query;
    bool success = query.exec(
        "CREATE TABLE IF NOT EXISTS scoreboard ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "user_id INTEGER, "
        "username TEXT, "
        "character TEXT, "
        "score INTEGER, "
        "floor_reached INTEGER, "
        "play_duration INTEGER, "
        "date_achieved TEXT)"
        );

    if (!success) {
        qDebug() << "Error creating scoreboard table:" << query.lastError().text();
        return false;
    }

    return true;
}

bool ScoreManager::add_score(const QString &username, const QString &character, int score, int floorReached,int playDuration)
{
    QSqlQuery query;
    query.prepare("INSERT INTO scoreboard (username, character, score, floor_reached, date_achieved) "
                  "VALUES (:username, :character, :score, :floor, datetime('now'))");
    query.bindValue(":username", username);
    query.bindValue(":character", character);
    query.bindValue(":score", score);
    query.bindValue(":duration", playDuration);
    query.bindValue(":floor", floorReached);

    if (!query.exec()) {
        qDebug() << "Error adding score:" << query.lastError().text();
        return false;
    }

    return true;
}

/*QList<Score_entry> ScoreManager::get_scores(const QString &characterFilter, const QString &sortBy)
{
    QList<Score_entry> results;

    QString queryStr = "SELECT username, character, score, floor_reached, date_achieved FROM scoreboard";

    if (characterFilter != "All") {
        queryStr += " WHERE character = :character";
    }

    if (sortBy == "date") {
        queryStr += " ORDER BY date_achieved DESC";
    } else {
        queryStr += " ORDER BY score DESC";
    }

    QSqlQuery query;
    query.prepare(queryStr);

    if (characterFilter != "All") {
        query.bindValue(":character", characterFilter);
    }

    if (!query.exec()) {
        qDebug() << "Error fetching scores:" << query.lastError().text();
        return results;
    }

    while (query.next()) {
        Score_entry entry;
        entry.username = query.value("username").toString();
        entry.character = query.value("character").toString();
        entry.score = query.value("score").toInt();
        entry.floor_reached = query.value("floor_reached").toInt();
        entry.date_achieved = query.value("date_achieved").toString();
        results.append(entry);
    }

    return results;
}*/

QList<Score_entry> ScoreManager::get_scores(const QString &characterFilter, const QString &sortBy)
{
    QList<Score_entry> results;

    QString queryStr = "SELECT username, character, score, floor_reached, date_achieved "
                       "FROM scoreboard s1 "
                       "WHERE score = (SELECT MAX(score) FROM scoreboard s2 "
                       "WHERE s2.username = s1.username AND s2.character = s1.character)";

    if (characterFilter != "All") {
        queryStr += " AND s1.character = :character";
    }

    if (sortBy == "time") {
        queryStr += " ORDER BY play_duration ASC";
    } else {
        queryStr += " ORDER BY score DESC";
    }

    QSqlQuery query;
    query.prepare(queryStr);

    if (characterFilter != "All") {
        query.bindValue(":character", characterFilter);
    }

    if (!query.exec()) {
        qDebug() << "Error fetching scores:" << query.lastError().text();
        return results;
    }

    while (query.next()) {
        Score_entry entry;
        entry.username = query.value("username").toString();
        entry.character = query.value("character").toString();
        entry.score = query.value("score").toInt();
        entry.floor_reached = query.value("floor_reached").toInt();
        entry.play_duration = query.value("play_duration").toInt();
        entry.date_achieved = query.value("date_achieved").toString();
        results.append(entry);
    }

    return results;
}