#ifndef USERMANAGER_H
#define USERMANAGER_H
#include <QSqlDatabase>
#include <QString>

#include "smtp/smtpclient.h"
#include "smtp/mimemessage.h"
#include "smtp/emailaddress.h"
#include "smtp/mimetext.h"
#include <QRandomGenerator>

class user_manager
{
public:
    static user_manager& instance();
    ~user_manager();

    bool open_database();
    bool register_user(const QString &username , const QString &password , const QString &email);
    bool login_user(const QString &username , const QString &password);
    bool username_exist(const QString &username);  // FIX: typo was usernmae_exist
    void set_current_user(const QString &username , const QString &password);
    QString get_current_username() const;
    QString get_current_password() const;

    bool verify_email(const QString &username, const QString &code);
    bool is_account_verified(const QString &username);

    bool send_password_reset_code(const QString &username);
    bool verify_reset_code(const QString &username, const QString &code);
    bool reset_password(const QString &username, const QString &new_password);

private:
    user_manager();
    user_manager(const user_manager&) = delete;
    user_manager& operator=(const user_manager&) = delete;
    QSqlDatabase data_base;
    QString current_username;
    QString current_password;  // FIX: now stores SHA256 hash, not plaintext
    bool insert_user(const QString &username, const QString &hashedPassword, const QString &email, const QString &verificationCode);
    bool verify_password(const QString &username, const QString &hashedPassword);

    QString generate_verification_code();
    bool send_verification_email(const QString &to_email, const QString &code);

    // Validation helpers
    bool validateUsername(const QString &username);
    bool validatePassword(const QString &password);
    bool validateEmail(const QString &email);
};
#endif // USERMANAGER_H