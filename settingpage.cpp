#include "settingpage.h"
#include "ui_settingpage.h"

SettingPage::SettingPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingPage)
{
    ui->setupUi(this);
    ui->sliderMusic->setValue(AudioManager::instance().musicVolume());
    ui->sliderEffects->setValue(AudioManager::instance().effectsVolume());

    connect(ui->sliderMusic, &QSlider::valueChanged, this, [](int value){
        AudioManager::instance().setMusicVolume(value);
    });

    connect(ui->sliderEffects, &QSlider::valueChanged, this, [](int value){
        AudioManager::instance().setEffectsVolume(value);
    });

    connect(ui->btnBack, &QPushButton::clicked, this, [this](){
        this->close();
    });

    connect(ui->btnCusor1, &QPushButton::clicked, this, [](){
        CursorManager::applyCursor(0);
    });
    connect(ui->btnCusor2, &QPushButton::clicked, this, [](){
        CursorManager::applyCursor(1);
    });
    connect(ui->btnCusor3, &QPushButton::clicked, this, [](){
        CursorManager::applyCursor(2);
    });

}

void SettingPage::handleChangePassword()
{
    QString currentInput = ui->lineEdit_currentPassword->text();
    QString newPassword = ui->lineEdit_currentPassword->text();
    QString confirmNewPassword = ui->lineEdit_confirmNewPassword->text();

    QString realCurrentPassword = user_manager::instance().get_current_password();
    QString username = user_manager::instance().get_current_username();

    if (currentInput != realCurrentPassword) {
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
    } else {
        ui->label_passwordChangeError->setText("Something went wrong. Try again!");
        AudioManager::instance().playEffect(":/assets/music/error.mp3");
    }
}

SettingPage::~SettingPage()
{
    delete ui;
}
