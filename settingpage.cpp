#include "settingpage.h"
#include "ui_settingpage.h"
#include <QCryptographicHash>

SettingPage::SettingPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingPage)
{
    ui->setupUi(this);

    // Window bedoone border va frame
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

    ui->stackedWidget->setCurrentIndex(-1);

    // --- Main menu tabs ---
    connect(ui->btnTabAudio, &QPushButton::clicked, this, [this](){
        AudioManager::instance().playEffect(":/assets/music/menu_button.mp3");
        ui->stackedWidget->setCurrentWidget(ui->audio);
    });

    connect(ui->btnTabPassword, &QPushButton::clicked, this, [this](){
        AudioManager::instance().playEffect(":/assets/music/menu_button.mp3");
        ui->stackedWidget->setCurrentWidget(ui->changepass);
    });

    connect(ui->btnTabShortkey, &QPushButton::clicked, this, [this](){
        AudioManager::instance().playEffect(":/assets/music/menu_button.mp3");
        ui->stackedWidget->setCurrentWidget(ui->shortkey);
    });

    connect(ui->btnReturn, &QPushButton::clicked, this, [this](){
        AudioManager::instance().playEffect(":/assets/music/menu_button.mp3");
        this->close();
    });

    // --- Back buttons (hameye page ha) ---
    auto goBack = [this]() {
        AudioManager::instance().playEffect(":/assets/music/menu_button.mp3");
        ui->stackedWidget->setCurrentIndex(-1);
    };
    connect(ui->btnback2, &QPushButton::clicked, this, goBack);
    connect(ui->btnbackAudio, &QPushButton::clicked, this, goBack);
    connect(ui->btnbackShortkey, &QPushButton::clicked, this, goBack);

    // --- Confirm ---
    connect(ui->btnConfirm, &QPushButton::clicked, this, [this](){
        AudioManager::instance().playEffect(":/assets/music/confrim.mp3");
        handleChangePassword();
    });

    // --- Sliders ---
    connect(ui->sliderMusic, &QSlider::valueChanged, this, [](int value){
        AudioManager::instance().setMusicVolume(value);
    });
    connect(ui->sliderEffects, &QSlider::valueChanged, this, [](int value){
        AudioManager::instance().setEffectsVolume(value);
    });
    ui->sliderMusic->setValue(AudioManager::instance().musicVolume());
    ui->sliderEffects->setValue(AudioManager::instance().effectsVolume());

    // --- Cursor buttons ---
    connect(ui->btnCusor1, &QPushButton::clicked, this, [this](){
        AudioManager::instance().playEffect(":/assets/music/menu_button.mp3");
        CursorManager::applyCursor(0);
    });
    connect(ui->btnCusor2, &QPushButton::clicked, this, [this](){
        AudioManager::instance().playEffect(":/assets/music/menu_button.mp3");
        CursorManager::applyCursor(1);
    });
    connect(ui->btnCusor3, &QPushButton::clicked, this, [this](){
        AudioManager::instance().playEffect(":/assets/music/menu_button.mp3");
        CursorManager::applyCursor(2);
    });

    // Vaghti stack avaz mishe, button haye main menu hide/show beshan
    connect(ui->stackedWidget, &QStackedWidget::currentChanged,
            this, &SettingPage::updateButtonVisibility);

    updateButtonVisibility(-1);
}

void SettingPage::updateButtonVisibility(int index)
{
    bool isMainPage = (index == -1);

    ui->btnTabAudio->setVisible(isMainPage);
    ui->btnTabPassword->setVisible(isMainPage);
    ui->btnTabShortkey->setVisible(isMainPage);
    ui->btnReturn->setVisible(isMainPage);
}

void SettingPage::handleChangePassword()
{
    ui->label_passwordChangeError->clear();

    QString currentInput = ui->lineEdit_currentPassword->text();
    QString hashedInput = QString(QCryptographicHash::hash(currentInput.toUtf8(), QCryptographicHash::Sha256).toHex());
    QString realCurrentPassword = user_manager::instance().get_current_password();
    QString newPassword = ui->lineEdit_newPassword->text();
    QString confirmNewPassword = ui->lineEdit_confirmNewPassword->text();
    QString username = user_manager::instance().get_current_username();

    if (hashedInput != realCurrentPassword) {
        ui->label_passwordChangeError->setText("Current password is incorrect!");
        AudioManager::instance().playEffect(":/assets/music/error.mp3");
        return;
    }

    if (newPassword.length() < 8) {
        ui->label_passwordChangeError->setText("New password must be at least 8 characters!");
        AudioManager::instance().playEffect(":/assets/music/error.mp3");
        return;
    }

    if (newPassword != confirmNewPassword) {
        ui->label_passwordChangeError->setText("Passwords don't match!");
        AudioManager::instance().playEffect(":/assets/music/error.mp3");
        return;
    }

    bool success = user_manager::instance().reset_password(username, newPassword);

    if (success) {
        user_manager::instance().set_current_user(username, newPassword);
        ui->label_passwordChangeError->setText("Password changed successfully!");
        AudioManager::instance().playEffect(":/assets/music/success.mp3");

        ui->lineEdit_currentPassword->clear();
        ui->lineEdit_newPassword->clear();
        ui->lineEdit_confirmNewPassword->clear();

        QTimer::singleShot(1000, this, [this]() {
            ui->stackedWidget->setCurrentIndex(-1);
        });
    } else {
        ui->label_passwordChangeError->setText("Something went wrong. Try again!");
        AudioManager::instance().playEffect(":/assets/music/error.mp3");
    }
}

void SettingPage::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QPixmap background(":/assets/authpage/background.png");
    painter.drawPixmap(this->rect(), background);
    QWidget::paintEvent(event);
}

SettingPage::~SettingPage()
{
    delete ui;
}