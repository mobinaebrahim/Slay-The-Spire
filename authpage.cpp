#include "authpage.h"
#include "ui_authpage.h"

AuthPage::AuthPage(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AuthPage)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    ui->lineEdit_forgot_code->hide();
    ui->label_8->hide();

    connect(ui->btnConfirm, &QPushButton::clicked, this, [this](){
        QWidget *currentPage = ui->stackedWidget->currentWidget();

        qDebug() << "Current page object name:" << currentPage->objectName();


        if (currentPage == ui->page_register) {
            handle_register_sub();
        }
        else if (currentPage == ui->page_verify) {
            handle_verify_code();
            qDebug() << "Calling handle_verify_code";

        }
        else if (currentPage == ui->page_login) {
            handle_login_sub();
        }
        else if (currentPage == ui->page_forgot_code) {
            handle_forgot_code_sub();
        }
        else if (currentPage == ui->page_new_password) {
            handle_new_password_sub();
        }
    });

    connect(ui->btnBack, &QPushButton::clicked, this, [this](){
        QWidget *currentPage = ui->stackedWidget->currentWidget();

        if (currentPage == ui->page_register) {
            this->close();
        }
        else if (currentPage == ui->page_verify) {
            ui->stackedWidget->setCurrentWidget(ui->page_register);
        }
        else if (currentPage == ui->page_login) {
            this->close();
        }
        else if (currentPage == ui->page_forgot_code) {
            ui->stackedWidget->setCurrentWidget(ui->page_login);
        }
        else if (currentPage == ui->page_new_password) {
            ui->stackedWidget->setCurrentWidget(ui->page_forgot_code);
        }
    });

    connect(ui->btnForgotPassword, &QPushButton::clicked, this, [this](){
        ui->label_loginError->clear();
        ui->stackedWidget->setCurrentWidget(ui->page_forgot_code);
    });

    connect(ui->btnSendResetCode,&QPushButton::clicked,this,[this](){
        ui->label_8->show();
        ui->lineEdit_forgot_code->show();
        ui->label_7->hide();
        ui->lineEdit_forgot_username->hide();
        ui->btnSendResetCode->hide();
        handle_send_reset_code();
    });

}

AuthPage::~AuthPage()
{
    delete ui;
}

void AuthPage::show_login_page()
{
    ui->stackedWidget->setCurrentWidget(ui->page_login);
}

void AuthPage::show_register_page()
{
    ui->stackedWidget->setCurrentWidget(ui->page_register);
}

void AuthPage::handle_register_sub()
{
    QString username = ui->lineEdit_reg_username->text();
    QString password = ui->lineEdit_reg_password->text();
    QString confirmPassword = ui->lineEdit_confirmPassword->text();
    QString email = ui->lineEdit_email->text();

    if (username.isEmpty() || password.isEmpty()) {
        ui->label_registerError->setText("Username and password cannot be empty!");
        return;
    }

    if (password.length() < 8) {
        ui->label_registerError->setText("Password must be at least 8 characters!");
        return;
    }

    if (password != confirmPassword) {
        ui->label_registerError->setText("Confirm password doesn't match!");
        return;
    }

    QRegularExpression emailRegex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    if (!emailRegex.match(email).hasMatch()) {
        ui->label_registerError->setText("Incorrect email format!");
        return;
    }

    bool success = user_manager::instance().register_user(username, password, email);

    if (success) {
        ui->label_registerError->clear();
        ui->stackedWidget->setCurrentWidget(ui->page_verify);
    } else {
        ui->label_registerError->setText("Username already exists!");
    }
}

void AuthPage::handle_login_sub()
{
    QString username = ui->lineEdit_lgn_username->text();
    QString password = ui->lineEdit_lgn_password->text();

    if (username.isEmpty() || password.isEmpty()) {
        ui->label_loginError->setText("Username and password cannot be empty!");
        return;
    }

    if (!user_manager::instance().usernmae_exist(username)) {
        ui->label_loginError->setText("Username not exist!");
        return;
    }

    bool success = user_manager::instance().login_user(username, password);

    if (!success) {
        ui->label_loginError->setText("Incorrect password!");
        return;
    }

    ui->label_loginError->clear();
    user_manager::instance().set_current_user(username, password);

    MainWindow *mainWin = qobject_cast<MainWindow*>(this->parentWidget());
    if (mainWin) {
        mainWin->go_to_menu();
    }
    this->close();
}

void AuthPage::handle_verify_code()
{
    QString username = ui->lineEdit_reg_username->text();
    QString code = ui->lineEdit_verifyCode->text();

    if (code.isEmpty()) {
        ui->label_verifyError->setText("Please enter the verification code!");
        return;
    }

    bool success = user_manager::instance().verify_email(username, code);

    if (success) {
        ui->label_verifyError->clear();
        QString password = ui->lineEdit_reg_password->text();
        user_manager::instance().set_current_user(username, password);

        MainWindow *mainWin = qobject_cast<MainWindow*>(this->parentWidget());
        if (mainWin) {
            mainWin->go_to_menu();
        }
        this->close();
    } else {
        ui->label_verifyError->setText("Verify code not match!");
    }
}

void AuthPage::handle_send_reset_code()
{
    QString username = ui->lineEdit_forgot_username->text();

    if (username.isEmpty()) {
        ui->label_forgotError->setText("Please enter your username!");
        return;
    }

    bool success = user_manager::instance().send_password_reset_code(username);

    if (success) {
        ui->label_forgotError->setText("Code sent! Check your email.");

        ui->lineEdit_forgot_username->hide();
        ui->btnSendResetCode->hide();

        ui->lineEdit_forgot_code->show();
    } else {
        ui->label_forgotError->setText("Username not exist!");
    }
}

void AuthPage::handle_forgot_code_sub()
{
    QString username = ui->lineEdit_forgot_username->text();
    QString code = ui->lineEdit_forgot_code->text();

    if (code.isEmpty()) {
        ui->label_forgotError->setText("Please enter the verification code!");
        return;
    }

    bool success = user_manager::instance().verify_reset_code(username, code);

    if (success) {
        ui->label_forgotError->clear();
        ui->stackedWidget->setCurrentWidget(ui->page_new_password);
    } else {
        ui->label_forgotError->setText("Verify code not match!");
    }
}

void AuthPage::handle_new_password_sub()
{
    QString newPassword = ui->lineEdit_newPassword->text();
    QString confirmNewPassword = ui->lineEdit_confirmNewPassword->text();
    QString username = ui->lineEdit_forgot_username->text();

    if (newPassword.isEmpty()) {
        ui->label_newPasswordError->setText("Password cannot be empty!");
        return;
    }

    if (newPassword.length() < 8) {
        ui->label_newPasswordError->setText("Password must be at least 8 characters!");
        return;
    }

    if (newPassword != confirmNewPassword) {
        ui->label_newPasswordError->setText("Passwords don't match!");
        return;
    }

    bool success = user_manager::instance().reset_password(username, newPassword);

    if (success) {
        ui->label_newPasswordError->clear();
        ui->stackedWidget->setCurrentWidget(ui->page_login);
    } else {
        ui->label_newPasswordError->setText("Something went wrong. Try again!");
    }
}

void AuthPage::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QPixmap background(":/assets/authpage/background.png");
    painter.drawPixmap(this->rect(), background);

    QWidget::paintEvent(event);
}