#include "authpage.h"
#include "ui_authpage.h"
#include "audiomanager.h"

AuthPage::AuthPage(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AuthPage)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    //ui->lineEdit_forgot_code->hide();
    //ui->label_8->hide();

    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(12);
    shadow->setOffset(2, 2);
    shadow->setColor(QColor(0, 0, 0, 220));

    ui->label_23->setGraphicsEffect(shadow);

    QGraphicsDropShadowEffect *glow = new QGraphicsDropShadowEffect(this);
    glow->setBlurRadius(2);
    glow->setOffset(2,2);
    glow->setColor(QColor(0,0,0,200));
    ui->btnForgotPassword->setGraphicsEffect(glow);

    /*QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(2);
    shadow->setOffset(1,1);
    shadow->setColor(QColor(0,0,0,255));*/

    auto effect = new QGraphicsDropShadowEffect(this);
    effect->setBlurRadius(6);
    effect->setOffset(1,1);
    effect->setColor(QColor(0,0,0,255));

    ui->label_7->setGraphicsEffect(effect);


    connect(ui->btnConfirm, &QPushButton::clicked, this, [this](){
        AudioManager::instance().playEffect(":/assets/music/confrim.mp3");

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
        AudioManager::instance().playEffect(":/assets/music/menu_button.mp3");

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
        AudioManager::instance().playEffect(":/assets/music/menu_button.mp3");

        ui->label_loginError->clear();
        ui->stackedWidget->setCurrentWidget(ui->page_forgot_code);
    });

    connect(ui->btnSendResetCode,&QPushButton::clicked,this,[this](){
        //ui->label_8->show();
        //ui->lineEdit_forgot_code->show();
        //ui->label_7->hide();
        //->lineEdit_forgot_username->hide();
        //ui->btnSendResetCode->hide();
        //ui->label_18->hide();
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

void AuthPage::showError(QLabel *label, const QString &text)
{
    label->setText(text);
    AudioManager::instance().playEffect(":/assets/music/error.mp3");
}

void AuthPage::handle_register_sub()
{
    QString username = ui->lineEdit_reg_username->text();
    QString password = ui->lineEdit_reg_password->text();
    QString confirmPassword = ui->lineEdit_confirmPassword->text();
    QString email = ui->lineEdit_email->text();

    if (username.isEmpty() || password.isEmpty()) {
        showError(ui->label_registerError, "Username and password cannot be empty!");
        return;
    }

    if (password.length() < 8) {
        showError(ui->label_registerError, "Password must be at least 8 characters!");
        return;
    }

    if (password != confirmPassword) {
        showError(ui->label_registerError, "Confirm password doesn't match!");
        return;
    }

    QRegularExpression emailRegex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    if (!emailRegex.match(email).hasMatch()) {
        showError(ui->label_registerError, "Incorrect email format!");
        return;
    }

    bool success = user_manager::instance().register_user(username, password, email);

    if (success) {
        ui->label_registerError->clear();
        ui->stackedWidget->setCurrentWidget(ui->page_verify);
    } else {
        showError(ui->label_registerError, "Username already exists!");
    }
}

void AuthPage::handle_login_sub()
{
    QString username = ui->lineEdit_lgn_username->text();
    QString password = ui->lineEdit_lgn_password->text();

    if (username.isEmpty() || password.isEmpty()) {
        showError(ui->label_loginError, "Username and password cannot be empty!");
        return;
    }

    if (!user_manager::instance().username_exist(username)) {  // FIX: was usernmae_exist
        showError(ui->label_loginError, "Username not exist!");
        return;
    }

    bool success = user_manager::instance().login_user(username, password);

    if (!success) {
        showError(ui->label_loginError, "Incorrect password!");
        return;
    }

    ui->label_loginError->clear();
    user_manager::instance().set_current_user(username, password);

    MainMenuWindow *mainWin = qobject_cast<MainMenuWindow*>(this->parentWidget());
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
        showError(ui->label_verifyError, "Please enter the verification code!");
        return;
    }

    bool success = user_manager::instance().verify_email(username, code);

    if (success) {
        ui->label_verifyError->clear();
        QString password = ui->lineEdit_reg_password->text();
        user_manager::instance().set_current_user(username, password);

        MainMenuWindow *mainWin = qobject_cast<MainMenuWindow*>(this->parentWidget());
        if (mainWin) {
            mainWin->go_to_menu();
        }
        this->close();
    } else {
        showError(ui->label_verifyError, "Verify code not match!");
    }
}

void AuthPage::handle_send_reset_code()
{
    QString username = ui->lineEdit_forgot_username->text();

    if (username.isEmpty()) {
        showError(ui->label_forgotError, "Please enter your username!");
        return;
    }

    bool success = user_manager::instance().send_password_reset_code(username);

    if (success) {
        ui->label_forgotError->setText("Code sent! Check your email.");

        //ui->lineEdit_forgot_username->hide();
        //ui->btnSendResetCode->hide();

        ui->lineEdit_forgot_code->show();
    } else {
        showError(ui->label_forgotError, "Username not exist!");
    }
}

void AuthPage::handle_forgot_code_sub()
{
    QString username = ui->lineEdit_forgot_username->text();
    QString code = ui->lineEdit_forgot_code->text();

    if (code.isEmpty()) {
        showError(ui->label_forgotError, "Please enter the verification code!");
        return;
    }

    bool success = user_manager::instance().verify_reset_code(username, code);

    if (success) {
        ui->label_forgotError->clear();
        ui->stackedWidget->setCurrentWidget(ui->page_new_password);
    } else {
        showError(ui->label_forgotError, "Verify code not match!");
    }
}

void AuthPage::handle_new_password_sub()
{
    QString newPassword = ui->lineEdit_newPassword->text();
    QString confirmNewPassword = ui->lineEdit_confirmNewPassword->text();
    QString username = ui->lineEdit_forgot_username->text();

    if (newPassword.isEmpty()) {
        showError(ui->label_newPasswordError, "Password cannot be empty!");
        return;
    }

    if (newPassword.length() < 8) {
        showError(ui->label_newPasswordError, "Password must be at least 8 characters!");
        return;
    }

    if (newPassword != confirmNewPassword) {
        showError(ui->label_newPasswordError, "Passwords don't match!");
        return;
    }

    bool success = user_manager::instance().reset_password(username, newPassword);

    if (success) {
        ui->label_newPasswordError->clear();
        ui->stackedWidget->setCurrentWidget(ui->page_login);
    } else {
        showError(ui->label_newPasswordError, "Something went wrong. Try again!");
    }
}

void AuthPage::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QPixmap background(":/assets/authpage/background.png");
    painter.drawPixmap(this->rect(), background);

    QWidget::paintEvent(event);
}