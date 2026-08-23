#include "TreasureScreen.h"
#include <cstdlib>
#include <ctime>
TreasureScreen::TreasureScreen(Player* player, QWidget* parent)
    : QWidget(parent), player(player) {
    std::srand(std::time(nullptr));
    setupUI();
    grantReward();
}
void TreasureScreen::setupUI() {
    setMinimumSize(1280, 720);
    setStyleSheet("background-color: black;");
    // ---- تک عکس تمام‌صفحه ----
    backgroundLabel = new QLabel(this);
    backgroundLabel->setPixmap(QPixmap(":/assets/treasure.png")
                                   .scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    backgroundLabel->setGeometry(0, 0, width(), height());
    backgroundLabel->lower();

    // ---- متن جایزه ----
    rewardLabel = new QLabel(this);
    rewardLabel->setAlignment(Qt::AlignCenter);
    rewardLabel->setStyleSheet(
        "color: #f5c518; font-size: 28px; font-weight: 900; background: transparent;");
    rewardLabel->raise();

    // ---- دکمه‌ی ادامه ----
    continueBtn = new QPushButton("Continue", this);
    continueBtn->setFixedSize(180, 55);
    continueBtn->setStyleSheet(
        "QPushButton { background-color: #2d4a3e; color: #a8e6cf; font-size: 18px; "
        "font-weight: bold; border-radius: 10px; border: 2px solid #a8e6cf; }"
        "QPushButton:hover { background-color: #3a634f; }");
    connect(continueBtn, &QPushButton::clicked, this, &TreasureScreen::onContinueClicked);
    continueBtn->raise();

    // ---- صدای باز شدن صندوق ----
    soundOutput = new QAudioOutput(this);
    soundPlayer = new QMediaPlayer(this);
    soundPlayer->setAudioOutput(soundOutput);
    soundPlayer->setSource(QUrl("qrc:/assets/audio/treasure_open.mp3"));
    soundOutput->setVolume(0.8);
    soundPlayer->play();
}
void TreasureScreen::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    backgroundLabel->setGeometry(0, 0, width(), height());
    backgroundLabel->setPixmap(QPixmap(":/assets/treasure/treasure.png")
                                   .scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));

    int labelW = 600, labelH = 50;
    rewardLabel->setGeometry((width() - labelW) / 2, height() - 180, labelW, labelH);

    int btnW = 180, btnH = 55;
    continueBtn->setGeometry((width() - btnW) / 2, height() - 100, btnW, btnH);
}
void TreasureScreen::grantReward() {
    int goldAmount = 30 + (rand() % 51); // 30 تا 80 طلا
    player->increaseGold(goldAmount);
    rewardLabel->setText("You found " + QString::number(goldAmount) + " Gold!");
}
void TreasureScreen::onContinueClicked() {
    soundPlayer->stop();
    emit finished();
}