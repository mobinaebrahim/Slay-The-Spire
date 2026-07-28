#include "CampfireScreen.h"
#include "../Relics.h"
#include "../card.h"
#include <QTimer>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

CampfireScreen::CampfireScreen(Player* player, QWidget* parent)
    : QWidget(parent), player(player) {
    setupUI();
    updateButtonStates();
}

void CampfireScreen::setupUI() {
    setMinimumSize(1280, 720);

    backgroundLabel = new QLabel(this);
    backgroundLabel->setPixmap(QPixmap(":/images/campfire_bg.png").scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    backgroundLabel->setGeometry(0, 0, width(), height());

    menuWidget = new QWidget(this);
    menuWidget->setGeometry(0, 0, width(), height());

    auto* menuLayout = new QVBoxLayout(menuWidget);
    menuLayout->setAlignment(Qt::AlignCenter);
    menuLayout->setSpacing(30);

    auto* title = new QLabel("Campfire", menuWidget);
    title->setStyleSheet("color: #f5c518; font-size: 42px; font-weight: 900; background: transparent;");
    title->setAlignment(Qt::AlignCenter);
    menuLayout->addWidget(title);

    auto* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(60);
    btnLayout->setAlignment(Qt::AlignCenter);

    restBtn = new QPushButton(menuWidget);
    restBtn->setFixedSize(180, 180);
    restBtn->setIcon(QIcon(":/images/campfire/rest_icon.png"));
    restBtn->setIconSize(QSize(140, 140));
    restBtn->setStyleSheet(
        "QPushButton { background-color: rgba(0,0,0,120); border: 2px solid #a8e6cf; border-radius: 20px; }"
        "QPushButton:hover { background-color: rgba(0,0,0,180); border: 3px solid #ffffff; }"
        "QPushButton:disabled { opacity: 0.3; border-color: #555; }");
    connect(restBtn, &QPushButton::clicked, this, &CampfireScreen::onRestClicked);

    smithBtn = new QPushButton(menuWidget);
    smithBtn->setFixedSize(180, 180);
    smithBtn->setIcon(QIcon(":/images/campfire/smith_icon.png"));
    smithBtn->setIconSize(QSize(140, 140));
    smithBtn->setStyleSheet(
        "QPushButton { background-color: rgba(0,0,0,120); border: 2px solid #f5c518; border-radius: 20px; }"
        "QPushButton:hover { background-color: rgba(0,0,0,180); border: 3px solid #ffffff; }"
        "QPushButton:disabled { opacity: 0.3; border-color: #555; }");
    connect(smithBtn, &QPushButton::clicked, this, &CampfireScreen::onSmithClicked);

    liftBtn = new QPushButton(menuWidget);
    liftBtn->setFixedSize(180, 180);
    liftBtn->setIcon(QIcon(":/images/campfire/lift_icon.png"));
    liftBtn->setIconSize(QSize(140, 140));
    liftBtn->setStyleSheet(
        "QPushButton { background-color: rgba(0,0,0,120); border: 2px solid #ff6b6b; border-radius: 20px; }"
        "QPushButton:hover { background-color: rgba(0,0,0,180); border: 3px solid #ffffff; }"
        "QPushButton:disabled { opacity: 0.3; border-color: #555; }");
    connect(liftBtn, &QPushButton::clicked, this, &CampfireScreen::onLiftClicked);

    btnLayout->addWidget(restBtn);
    btnLayout->addWidget(smithBtn);
    btnLayout->addWidget(liftBtn);
    menuLayout->addLayout(btnLayout);

    restWidget = new QWidget(this);
    restWidget->setGeometry(0, 0, width(), height());
    restWidget->hide();

    restBgLabel = new QLabel(restWidget);
    restBgLabel->setPixmap(QPixmap(":/images/campfire/rest_bg.png").scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    restBgLabel->setGeometry(0, 0, width(), height());

    auto* restLayout = new QVBoxLayout(restWidget);
    restLayout->setAlignment(Qt::AlignCenter);
    restLayout->setSpacing(20);

    restInfoLabel = new QLabel(restWidget);
    restInfoLabel->setStyleSheet("color: white; font-size: 24px; font-weight: bold; background: transparent;");
    restInfoLabel->setAlignment(Qt::AlignCenter);
    restLayout->addWidget(restInfoLabel);

    restContinueBtn = new QPushButton("Continue", restWidget);
    restContinueBtn->setFixedSize(160, 50);
    restContinueBtn->setStyleSheet(
        "QPushButton { background-color: #2d4a3e; color: #a8e6cf; font-size: 16px; font-weight: bold; border-radius: 8px; border: 2px solid #a8e6cf; }"
        "QPushButton:hover { background-color: #3a634f; }");
    connect(restContinueBtn, &QPushButton::clicked, this, &CampfireScreen::onRestContinue);
    restLayout->addWidget(restContinueBtn, 0, Qt::AlignCenter);

    smithWidget = new QWidget(this);
    smithWidget->setGeometry(0, 0, width(), height());
    smithWidget->setStyleSheet("background-color: rgba(10,10,15,230);");
    smithWidget->hide();

    auto* smithLayout = new QVBoxLayout(smithWidget);
    smithLayout->setAlignment(Qt::AlignCenter);
    smithLayout->setSpacing(20);

    smithTitle = new QLabel("Choose a card to upgrade", smithWidget);
    smithTitle->setStyleSheet("color: #f5c518; font-size: 28px; font-weight: bold; background: transparent;");
    smithTitle->setAlignment(Qt::AlignCenter);
    smithLayout->addWidget(smithTitle);

    smithCardsContainer = new QWidget(smithWidget);
    auto* cardsGrid = new QGridLayout(smithCardsContainer);
    cardsGrid->setSpacing(20);
    cardsGrid->setAlignment(Qt::AlignCenter);
    smithLayout->addWidget(smithCardsContainer, 0, Qt::AlignCenter);

    smithCancelBtn = new QPushButton("Cancel", smithWidget);
    smithCancelBtn->setFixedSize(140, 45);
    smithCancelBtn->setStyleSheet(
        "QPushButton { background-color: #444; color: white; font-size: 14px; font-weight: bold; border-radius: 6px; }"
        "QPushButton:hover { background-color: #666; }");
    connect(smithCancelBtn, &QPushButton::clicked, this, &CampfireScreen::onCancelSmith);
    smithLayout->addWidget(smithCancelBtn, 0, Qt::AlignCenter);
}

void CampfireScreen::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    backgroundLabel->setGeometry(0, 0, width(), height());
    menuWidget->setGeometry(0, 0, width(), height());
    restWidget->setGeometry(0, 0, width(), height());
    restBgLabel->setGeometry(0, 0, width(), height());
    smithWidget->setGeometry(0, 0, width(), height());
}

void CampfireScreen::updateButtonStates() {
    bool canLift = false;
    for (auto* relic : player->getRelics()) {
        if (Girya* girya = dynamic_cast<Girya*>(relic)) {
            if (girya->getLiftsRemaining() > 0) {
                canLift = true;
                break;
            }
        }
    }
    liftBtn->setEnabled(canLift);
}

void CampfireScreen::showMenu() {
    menuWidget->show();
    menuWidget->raise();
    restWidget->hide();
    smithWidget->hide();
}

void CampfireScreen::showRest() {
    menuWidget->hide();
    restWidget->show();
    restWidget->raise();
    smithWidget->hide();
}

void CampfireScreen::showSmith() {
    menuWidget->hide();
    restWidget->hide();
    smithWidget->show();
    smithWidget->raise();
    buildSmithCards();
}

void CampfireScreen::onRestClicked() {
    int healAmount = static_cast<int>(player->getMaxHp() * 0.2);
    player->increaseHP(healAmount);
    restInfoLabel->setText("You rested and healed " + QString::number(healAmount) + " HP.");
    showRest();
}

void CampfireScreen::onRestContinue() {
    emit finished();
}

void CampfireScreen::onSmithClicked() {
    showSmith();
}

void CampfireScreen::buildSmithCards() {
    QLayout* oldLayout = smithCardsContainer->layout();
    if (oldLayout) {
        QLayoutItem* child;
        while ((child = oldLayout->takeAt(0)) != nullptr) {
            if (child->widget()) {
                child->widget()->hide();
                delete child->widget();
            }
            delete child;
        }
        delete oldLayout;
    }

    auto* grid = new QGridLayout(smithCardsContainer);
    grid->setSpacing(20);
    grid->setAlignment(Qt::AlignCenter);

    const auto& deck = player->getFullDeck();
    int col = 0, row = 0;
    for (int i = 0; i < (int)deck.size(); ++i) {
        Card* card = deck[i];
        if (!card) continue;

        QPushButton* cardBtn = new QPushButton(smithCardsContainer);
        cardBtn->setFixedSize(130, 175);
        QString imgPath = ":/images/cards/" + QString::fromStdString(card->getName()) + ".png";
        cardBtn->setIcon(QIcon(imgPath));
        cardBtn->setIconSize(QSize(130, 175));
        cardBtn->setStyleSheet(
            "QPushButton { border: 2px solid #555; border-radius: 8px; background: transparent; }"
            "QPushButton:hover { border: 3px solid #f5c518; }");
        connect(cardBtn, &QPushButton::clicked, this, [this, i]() { onCardUpgradeClicked(i); });
        grid->addWidget(cardBtn, row, col);

        col++;
        if (col >= 6) {
            col = 0;
            row++;
        }
    }
}

void CampfireScreen::onCardUpgradeClicked(int index) {
    auto deck = player->getFullDeck();
    if (index < 0 || index >= (int)deck.size()) return;

    Card* card = deck[index];
    card->upgrade();

    smithTitle->setText(QString::fromStdString(card->getName()) + " upgraded!");
    smithTitle->setStyleSheet("color: #4caf50; font-size: 28px; font-weight: bold; background: transparent;");

    QTimer::singleShot(800, this, [this]() {
        smithTitle->setText("Choose a card to upgrade");
        smithTitle->setStyleSheet("color: #f5c518; font-size: 28px; font-weight: bold; background: transparent;");
        emit finished();
    });
}

void CampfireScreen::onCancelSmith() {
    showMenu();
}

void CampfireScreen::onLiftClicked() {
    for (auto* relic : player->getRelics()) {
        if (Girya* girya = dynamic_cast<Girya*>(relic)) {
            if (girya->getLiftsRemaining() > 0) {
                girya->onCampsite(player);
                break;
            }
        }
    }
    restBtn->setEnabled(false);
    smithBtn->setEnabled(false);
    liftBtn->setEnabled(false);
    QTimer::singleShot(1000, this, &CampfireScreen::finished);
}
