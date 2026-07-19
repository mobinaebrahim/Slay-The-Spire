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

SettingPage::~SettingPage()
{
    delete ui;
}
