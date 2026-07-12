#ifndef SCOREMANAGER_H
#define SCOREMANAGER_H
#include <QSqlDatabase>
#include <QString>
#include <QList>

struct Score_entry {
    QString username;
    int total_score;
    int highest_floor;
    int total_wins;
    int total_duration;
};

class ScoreManager
{
public:
    static ScoreManager& instance();
    bool add_score(const QString &username, int score, int floorReached, int playDuration, bool isVictory);
    QList<Score_entry> get_scores( const QString &sortBy = "score");


private:
    ScoreManager();
    ~ScoreManager();
    ScoreManager(const ScoreManager&) = delete;
    ScoreManager& operator=(const ScoreManager&) = delete;
    bool create_table();
};

#endif // SCOREMANAGER_H