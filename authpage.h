#ifndef AUTHPAGE_H
#define AUTHPAGE_H

#include <QDialog>
#include <QPainter>
#include <QPixmap>
#include <QGraphicsDropShadowEffect>
#include"usermanager.h"
#include"mainmenuwindow.h"
#include <QRegularExpression>

namespace Ui {
class AuthPage;
}

class AuthPage : public QDialog
{
    Q_OBJECT

public:
    explicit AuthPage(QWidget *parent = nullptr);
    ~AuthPage();
    void show_login_page();
    void show_register_page();

private:
    Ui::AuthPage *ui;

    void handle_register_sub();
    void handle_verify_code();
    void handle_login_sub();
    void handle_forgot_code_sub();
    void handle_send_reset_code();
    void handle_new_password_sub();

protected:
    void paintEvent(QPaintEvent *event) override;

};

#endif // AUTHPAGE_H
