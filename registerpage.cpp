#include "registerpage.h"
#include "ui_registerpage.h"
#include "usermanager.h"
#include "loginpage.h"
#include <QMessageBox>

Dialogregisterpage::Dialogregisterpage(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialogregisterpage)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);


    connect(ui->submit_button,&QPushButton::clicked,this, &Dialogregisterpage::handle_register_sub);
    connect(ui->back_button,&QPushButton::clicked,this,&QDialog::close);


}

Dialogregisterpage::~Dialogregisterpage()
{
    delete ui;
}

void Dialogregisterpage::handle_register_sub(){
    QString username = ui->lineEdit_username->text();
    QString password = ui->lineEdit_password->text();
    QString email = ui->lineEdit_email->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Error", "Username and password cannot be empty!");
        return;
    }

    bool success = user_manager::instance().register_user(username, password, email);

    if (success) {
        user_manager::instance().set_current_user(username,password);
        QMessageBox::information(this, "Success", "Registration successful!");

        MainWindow *mainWin = qobject_cast<MainWindow*>(this->parentWidget());
        if (mainWin) {
            mainWin->go_to_menu();
        }

        this->close();
    }
    else {
        QMessageBox::warning(this, "Error", "Username already exists! Click Login to sign in.");
    }
}