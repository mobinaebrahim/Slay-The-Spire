#include "loginpage.h"
#include "ui_loginpage.h"
#include <QMessageBox>
#include <QCryptographicHash>



Dialogloginpage::Dialogloginpage(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialogloginpage)
{
    ui->setupUi(this);
    connect(ui->login_button,&QPushButton::clicked,this,&Dialogloginpage::login_check);
    connect(ui->back_button,&QPushButton::clicked,this,&QDialog::close);

}

Dialogloginpage::~Dialogloginpage()
{
    delete ui;
}

void Dialogloginpage::login_check(){
    QString username = ui->lineEdit_username->text();
    QString password = ui->lineEdit_password->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Error", "Username and password cannot be empty!");
        return;
    }

    QString hashedPassword = QString(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());
    bool exist_user = user_manager::instance().usernmae_exist(username);
    if(!exist_user){
        QMessageBox::warning(this, "Error", "Username not exist!");
        return;
    }

    bool success = user_manager::instance().login_user(username,password);
    if (success) {
        QMessageBox::information(this, "Success", "Login successful!");
        this->close();
    }
    else{
        QMessageBox::warning(this, "Error", "Incorrect username or password!");
        return;
    }
}