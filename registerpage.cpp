#include "registerpage.h"
#include "ui_registerpage.h"
#include "usermanager.h"
#include "loginpage.h"
#include "mainwindow.h"
#include <QMessageBox>
#include <QRegularExpression>
#include <QDebug>

Dialogregisterpage::Dialogregisterpage(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialogregisterpage)
{
    ui->setupUi(this);

    // مخفی کردن ویجت Verify Code از اول
    ui->widget->hide();

    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);

    connect(ui->submit_button, &QPushButton::clicked, this, &Dialogregisterpage::handle_register_sub);
    connect(ui->back_button, &QPushButton::clicked, this, &QDialog::close);
    connect(ui->verifycode_button, &QPushButton::clicked, this, &Dialogregisterpage::handle_verify_code);
}

Dialogregisterpage::~Dialogregisterpage()
{
    delete ui;
}

void Dialogregisterpage::handle_register_sub()
{
    QString username = ui->lineEdit_username->text();
    QString password = ui->lineEdit_password->text();
    QString email = ui->lineEdit_email->text();
    QString confirmPassword = ui->lineEdit_confirmPassword->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Error", "Username and password cannot be empty!");
        return;
    }

    if (password.length() < 8) {
        QMessageBox::warning(this, "Error", "Password must be at least 8 characters!");
        return;
    }

    if (password != confirmPassword) {
        QMessageBox::warning(this, "Error", "Confirm password doesn't match!");
        return;
    }

    QRegularExpression emailRegex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    if (!emailRegex.match(email).hasMatch()) {
        QMessageBox::warning(this, "Error", "Incorrect email format!");
        return;
    }

    bool success = user_manager::instance().register_user(username, password, email);
    qDebug() << "Register success:" << success;

    if (success) {
        user_manager::instance().set_current_user(username, password);

        // مخفی کردن فیلدهای ثبت‌نام
        ui->lineEdit_username->setEnabled(false);
        ui->lineEdit_password->hide();
        ui->lineEdit_confirmPassword->hide();
        ui->lineEdit_email->hide();
        ui->submit_button->hide();
        ui->label_password->hide();
        ui->label_confirmPassword->hide();
        ui->label_email->hide();

        // نشون دادن ویجت Verify Code
        ui->widget->show();
        ui->widget->raise();

        qDebug() << "Widget shown, isVisible:" << ui->widget->isVisible();
    } else {
        QMessageBox::warning(this, "Error", "Username already exists! Click Login to sign in.");
    }
}

void Dialogregisterpage::handle_verify_code()
{
    QString username = ui->lineEdit_username->text();
    QString code = ui->lineEdit_verifycode->text();

    bool success = user_manager::instance().verify_email(username, code);

    if (success) {
        QMessageBox::information(this, "Success", "Account verified successfully!");

        MainWindow *mainWin = qobject_cast<MainWindow*>(this->parentWidget());
        if (mainWin) {
            mainWin->go_to_menu();
        }
        this->close();
    } else {
        QMessageBox::warning(this, "Error", "Incorrect verification code!");
    }
}


/*#include "registerpage.h"
#include "ui_registerpage.h"
#include "usermanager.h"
#include "loginpage.h"
#include <QMessageBox>
#include <QRegularExpression>

Dialogregisterpage::Dialogregisterpage(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialogregisterpage)
{
    ui->setupUi(this);
    ui->widget->hide();
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);


    connect(ui->submit_button,&QPushButton::clicked,this, &Dialogregisterpage::handle_register_sub);
    connect(ui->back_button,&QPushButton::clicked,this,&QDialog::close);

    connect(ui->verifycode_button, &QPushButton::clicked, this, &Dialogregisterpage::handle_verify_code);


}

Dialogregisterpage::~Dialogregisterpage()
{
    delete ui;
}

void Dialogregisterpage::handle_register_sub(){
    //qDebug() << "Function called!";

    QString username = ui->lineEdit_username->text();
    QString password = ui->lineEdit_password->text();
    QString email = ui->lineEdit_email->text();
    QString confirmPassword = ui->lineEdit_confirmPassword->text();

    //qDebug() << "Password:" << password;
    //qDebug() << "Confirm Password:" << confirmPassword;
    //qDebug() << "Are they equal?" << (password == confirmPassword);

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Error", "Username and password cannot be empty!");
        return;
    }

    if (password.length() < 8) {
        QMessageBox::warning(this, "Error", "Password must be at least 8 characters!");
        return;
    }

    if (password != confirmPassword) {
        QMessageBox::warning(this, "Error", "Confirm password doesn't match!");
        return;
    }

    QRegularExpression emailRegex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    if (!emailRegex.match(email).hasMatch()) {
        QMessageBox::warning(this, "Error", "Incorrect email format!");
        return;
    }

    bool success = user_manager::instance().register_user(username, password, email);
    qDebug() << "Register success:" << success;

    if (success) {
        qDebug() << "Inside success block, about to show widget";
        user_manager::instance().set_current_user(username, password);
        ui->widget->show();
        qDebug() << "Widget show() called";
    }



    else {
        QMessageBox::warning(this, "Error", "Username already exists! Click Login to sign in.");
    }


}

void Dialogregisterpage::handle_verify_code()
{
    QString username = ui->lineEdit_username->text();
    QString code = ui->lineEdit_verifycode->text();

    bool success = user_manager::instance().verify_email(username, code);

    if (success) {
        QMessageBox::information(this, "Success", "Account verified successfully!");

        MainWindow *mainWin = qobject_cast<MainWindow*>(this->parentWidget());
        if (mainWin) {
            mainWin->go_to_menu();
        }
        this->close();
    } else {
        QMessageBox::warning(this, "Error", "Incorrect verification code!");
    }
}*/

