#include "settingpage.h"
#include "ui_settingpage.h"
#include <QCryptographicHash>

SettingPage::SettingPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingPage)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_TranslucentBackground);

    // Window bedoone border va frame
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);



    connect(ui->btnReturn, &QPushButton::clicked, this, [this](){
        AudioManager::instance().playEffect(":/assets/music/menu_button.mp3");
        this->close();
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

}

void SettingPage::updateButtonVisibility(int index)
{
    bool isMainPage = (index == -1);


    ui->btnReturn->setVisible(isMainPage);
}



void SettingPage::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    QPixmap background(":/assets/authpage/background.png");

    painter.drawPixmap(rect(), background);

    QWidget::paintEvent(event);
}

SettingPage::~SettingPage()
{
    delete ui;
}