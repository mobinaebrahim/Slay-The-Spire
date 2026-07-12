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
        "score INTEGER, "
        "floor_reached INTEGER, "
        "play_duration INTEGER, "
        "is_victory INTEGER DEFAULT 0, "
        "date_achieved TEXT)"
        );

    if (!success) {
        qDebug() << "Error creating scoreboard table:" << query.lastError().text();
        return false;
    }

    return true;
}

/*bool ScoreManager::add_score(const QString &username, const QString &character, int score, int floorReached, int playDuration, bool isVictory)
{
    QSqlQuery query;
    query.prepare("INSERT INTO scoreboard (username, character, score, floor_reached, play_duration, is_victory, date_achieved) "
                  "VALUES (:username, :character, :score, :floor, :duration, :victory, datetime('now'))");
    query.bindValue(":username", username);
    query.bindValue(":character", character);
    query.bindValue(":score", score);
    query.bindValue(":floor", floorReached);
    query.bindValue(":duration", playDuration);
    query.bindValue(":victory", isVictory ? 1 : 0);

    if (!query.exec()) {
        qDebug() << "Error adding score:" << query.lastError().text();
        return false;
    }

    return true;
}*/

bool ScoreManager::add_score(const QString &username, int score, int floorReached, int playDuration, bool isVictory)
{
    QSqlQuery query;
    query.prepare("INSERT INTO scoreboard (username, score, floor_reached, play_duration, is_victory, date_achieved) "
                  "VALUES (?, ?, ?, ?, ?, datetime('now'))");
    query.addBindValue(username);
    query.addBindValue(score);
    query.addBindValue(floorReached);
    query.addBindValue(playDuration);
    query.addBindValue(isVictory ? 1 : 0);

    if (!query.exec()) {
        qDebug() << "Error adding score:" << query.lastError().text();
        return false;
    }
    return true;
}

/*QList<Score_entry> ScoreManager::get_scores(const QString &characterFilter, const QString &sortBy)
{
    QList<Score_entry> results;

    bool hasFilter = (characterFilter != "All");

    QString queryStr =
        "SELECT username, character, "
        "SUM(score) AS total_score, "
        "MAX(floor_reached) AS highest_floor, "
        "SUM(CASE WHEN is_victory = 1 THEN 1 ELSE 0 END) AS total_wins, "
        "SUM(play_duration) AS total_duration "
        "FROM scoreboard ";

    if (hasFilter) {
        queryStr += "WHERE character = ? ";
    }

    queryStr += "GROUP BY username, character ";

    if (sortBy == "floor") {
        queryStr += "ORDER BY highest_floor DESC";
    } else if (sortBy == "wins") {
        queryStr += "ORDER BY total_wins DESC";
    } else if (sortBy == "time") {
        queryStr += "ORDER BY total_duration DESC";
    } else {
        queryStr += "ORDER BY total_score DESC";
    }

    QSqlQuery query;
    qDebug() << "QUERY:" << queryStr;
    query.prepare(queryStr);

    if (hasFilter) {
        query.addBindValue(characterFilter);
    }

    if (!query.exec()) {
        qDebug() << "Error fetching scores:" << query.lastError().text();
        return results;
    }

    while (query.next()) {
        Score_entry entry;
        entry.username = query.value("username").toString();
        entry.character = query.value("character").toString();
        entry.total_score = query.value("total_score").toInt();
        entry.highest_floor = query.value("highest_floor").toInt();
        entry.total_wins = query.value("total_wins").toInt();
        entry.total_duration = query.value("total_duration").toInt();
        results.append(entry);
    }

    return results;
}*/

QList<Score_entry> ScoreManager::get_scores(const QString &sortBy)
{
    QList<Score_entry> results;

    QString queryStr =
        "SELECT username, "
        "SUM(score) AS total_score, "
        "MAX(floor_reached) AS highest_floor, "
        "SUM(CASE WHEN is_victory = 1 THEN 1 ELSE 0 END) AS total_wins, "
        "SUM(play_duration) AS total_duration "
        "FROM scoreboard "
        "GROUP BY username ";

    if (sortBy == "floor") {
        queryStr += "ORDER BY highest_floor DESC";
    } else if (sortBy == "wins") {
        queryStr += "ORDER BY total_wins DESC";
    } else if (sortBy == "time") {
        queryStr += "ORDER BY total_duration DESC";
    } else {
        queryStr += "ORDER BY total_score DESC";
    }

    QSqlQuery query;
    query.prepare(queryStr);

    if (!query.exec()) {
        qDebug() << "Error fetching scores:" << query.lastError().text();
        return results;
    }

    while (query.next()) {
        Score_entry entry;
        entry.username = query.value("username").toString();
        entry.total_score = query.value("total_score").toInt();
        entry.highest_floor = query.value("highest_floor").toInt();
        entry.total_wins = query.value("total_wins").toInt();
        entry.total_duration = query.value("total_duration").toInt();
        results.append(entry);
    }

    return results;
}

QList<Run_entry> ScoreManager::get_run_history(const QString &username)
{
    QList<Run_entry> results;

    QSqlQuery query;
    query.prepare("SELECT username, score, floor_reached, play_duration, is_victory, date_achieved "
                  "FROM scoreboard WHERE username = ? ORDER BY date_achieved DESC");
    query.addBindValue(username);

    if (!query.exec()) {
        qDebug() << "Error fetching run history:" << query.lastError().text();
        return results;
    }

    while (query.next()) {
        Run_entry entry;
        entry.username = query.value("username").toString();
        entry.score = query.value("score").toInt();
        entry.floor_reached = query.value("floor_reached").toInt();
        entry.play_duration = query.value("play_duration").toInt();
        entry.is_victory = query.value("is_victory").toInt() == 1;
        entry.date_achieved = query.value("date_achieved").toString();
        results.append(entry);
    }

    return results;
}