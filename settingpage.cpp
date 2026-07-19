#include "settingpage.h"
#include "ui_settingpage.h"

SettingPage::SettingPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingPage)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Window);

    // Start with no tab selected - nothing from the stack is shown yet
    ui->stackedWidget->setCurrentIndex(-1);

    // --- Tab buttons: pick which page to show ---
    connect(ui->btnTabAudio, &QPushButton::clicked, this, [this](){
        AudioManager::instance().playEffect(":/assets/music/menu_button.mp3");
        ui->stackedWidget->setCurrentWidget(ui->audio);
    });

    connect(ui->btnTabPassword, &QPushButton::clicked, this, [this](){
        AudioManager::instance().playEffect(":/assets/music/menu_button.mp3");
        ui->stackedWidget->setCurrentWidget(ui->changepass);
    });

    // --- Audio page: volume sliders ---
    connect(ui->sliderMusic, &QSlider::valueChanged, this, [](int value){
        AudioManager::instance().setMusicVolume(value);
    });

    connect(ui->sliderEffects, &QSlider::valueChanged, this, [](int value){
        AudioManager::instance().setEffectsVolume(value);
    });

    // Show sliders at whatever the saved volume already is
    ui->sliderMusic->setValue(AudioManager::instance().musicVolume());
    ui->sliderEffects->setValue(AudioManager::instance().effectsVolume());

    // --- Audio page: cursor shape buttons ---
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

    // --- Audio page: back/confirm - both just return to "no tab selected" ---
    // (audio settings already apply live via the sliders above, so there's
    // nothing extra to "confirm" here)
    connect(ui->btnback2_2, &QPushButton::clicked, this, [this](){
        AudioManager::instance().playEffect(":/assets/music/menu_button.mp3");
        ui->stackedWidget->setCurrentIndex(-1);
    });

    connect(ui->btnConfirm_2, &QPushButton::clicked, this, [this](){
        AudioManager::instance().playEffect(":/assets/music/confrim.mp3");
        ui->stackedWidget->setCurrentIndex(-1);
    });

    // --- Change password page ---
    connect(ui->btnConfirm, &QPushButton::clicked, this, [this](){
        AudioManager::instance().playEffect(":/assets/music/confrim.mp3");
        handleChangePassword();
    });

    connect(ui->btnback2, &QPushButton::clicked, this, [this](){
        AudioManager::instance().playEffect(":/assets/music/menu_button.mp3");
        ui->stackedWidget->setCurrentIndex(-1);
    });

    connect(ui->btnReturn, &QPushButton::clicked, this, [this](){
        AudioManager::instance().playEffect(":/assets/music/menu_button.mp3");
        this->close();
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
