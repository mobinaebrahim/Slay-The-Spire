#include "scoremanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QSqlDatabase>

static bool ensureDatabaseOpen() {
    QSqlDatabase db;
    if (QSqlDatabase::contains("qt_sql_default_connection")) {
        db = QSqlDatabase::database("qt_sql_default_connection");
    } else {
        db = QSqlDatabase::addDatabase("QSQLITE");
    }
    if (db.isOpen()) return true;
    db.setDatabaseName("users.db");
    if (!db.open()) {
        qDebug() << "Error opening database:" << db.lastError().text();
        return false;
    }
    return true;
}

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
    if (!ensureDatabaseOpen()) return false;

    QSqlQuery query;
    bool success = query.exec(
        "CREATE TABLE IF NOT EXISTS scoreboard ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
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

bool ScoreManager::add_score(const QString &username, int score, int floorReached, int playDuration, bool isVictory)
{
    if (username.isEmpty() || score < 0 || floorReached < 0 || playDuration < 0) {
        qDebug() << "Invalid score parameters";
        return false;
    }

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

QList<Score_entry> ScoreManager::get_scores(const QString &sortBy)
{
    QList<Score_entry> results;

    QString orderBy;
    if (sortBy == "floor") {
        orderBy = "highest_floor DESC";
    } else if (sortBy == "wins") {
        orderBy = "total_wins DESC";
    } else if (sortBy == "time") {
        orderBy = "total_duration DESC";
    } else {
        orderBy = "total_score DESC";
    }

    QString queryStr =
        "SELECT username, "
        "SUM(score) AS total_score, "
        "MAX(floor_reached) AS highest_floor, "
        "SUM(CASE WHEN is_victory = 1 THEN 1 ELSE 0 END) AS total_wins, "
        "SUM(play_duration) AS total_duration "
        "FROM scoreboard "
        "GROUP BY username "
        "ORDER BY " + orderBy;

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
    if (username.isEmpty()) return results;

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

Score_entry ScoreManager::get_character_stats(const QString &username)
{
    Score_entry entry;
    entry.username = username;
    entry.total_score = 0;
    entry.highest_floor = 0;
    entry.total_wins = 0;
    entry.total_duration = 0;

    if (username.isEmpty()) return entry;

    QSqlQuery query;
    query.prepare(
        "SELECT "
        "SUM(score) AS total_score, "
        "MAX(floor_reached) AS highest_floor, "
        "SUM(CASE WHEN is_victory = 1 THEN 1 ELSE 0 END) AS total_wins, "
        "SUM(play_duration) AS total_duration "
        "FROM scoreboard WHERE username = ?"
        );
    query.addBindValue(username);

    if (!query.exec()) {
        qDebug() << "Error fetching character stats:" << query.lastError().text();
        return entry;
    }

    if (query.next()) {
        entry.total_score = query.value("total_score").toInt();
        entry.highest_floor = query.value("highest_floor").toInt();
        entry.total_wins = query.value("total_wins").toInt();
        entry.total_duration = query.value("total_duration").toInt();
    }

    return entry;
}