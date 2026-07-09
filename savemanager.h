#ifndef SAVEMANAGER_H
#define SAVEMANAGER_H
#include <QSqlDatabase>
#include <QString>
#include <QList>
#include <QJsonObject>

struct save_entry {
    int id;
    QString save_name;
    QString character;
    int score;
    int floor;
    QString save_date;
};

class SaveManager
{
public:
    static SaveManager& instance();

    bool create_save(const QString &username, const QString &save_name, const QString &character,
                     int score, int floor, const QJsonObject &game_data);
    bool update_save(int save_id, int score, int floor, const QJsonObject &game_data);
    bool delete_save(int save_id);
    QJsonObject load_save(int save_id);
    QList<save_entry> get_save_list(const QString &username);

private:
    SaveManager();
    ~SaveManager();
    SaveManager(const SaveManager&) = delete;
    SaveManager& operator=(const SaveManager&) = delete;
    bool create_table();
};

#endif // SAVEMANAGER_H