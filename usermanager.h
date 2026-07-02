#ifndef USERMANAGER_H
#define USERMANAGER_H
#include <QSqlDatabase>
#include <QString>
class user_manager
{
public:
    static user_manager& instance();
    ~user_manager();

    bool open_database();
    bool register_user(const QString &username , const QString &password , const QString &email);
    bool login_user(const QString &username , const QString &password);
    bool usernmae_exist(const QString &username);

private:
    user_manager();
    user_manager(const user_manager&) = delete;
    user_manager& operator=(const user_manager&) = delete;
    QSqlDatabase data_base;
    bool insert_user(const QString &username, const QString &hashedPassword, const QString &email );
    bool verify_password(const QString &username, const QString &hashedPassword);
};
#endif // USERMANAGER_H
