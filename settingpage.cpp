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

}

SettingPage::~SettingPage()
{
    delete ui;
}
