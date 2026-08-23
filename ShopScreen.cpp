#include "ShopScreen.h"
#include <QTimer>
#include <QGridLayout>
#include <cstdlib>
#include <ctime>
#include "card.h"
#include "Potion.h"
#include "Relics.h"

extern Card* createCardByName(const std::string& name);

ShopScreen::ShopScreen(Player* player, QWidget* parent)
    : QWidget(parent), player(player), cardRemovalCost(50) {
    std::srand(std::time(nullptr));
    generateOffers();
    setupUI();
    refreshGoldDisplay();
    updateOfferButtons();
}

ShopScreen::~ShopScreen() {
    for (auto& o : allOffers) {
        if (o.card) delete o.card;
        if (o.potion) delete o.potion;
        if (o.relic) delete o.relic;
    }
}

void ShopScreen::generateOffers() {
    std::vector<std::string> commonNames = {
        "Strike", "Defend", "Bash", "ShrugItOff", "Inflame", "FeelNoPain", "TwinStrike"
    };
    std::vector<std::string> rareNames = {
        "Barricade", "DemonForm", "Impervious", "Bludgeon", "Metallicize"
    };

    // 2 Common cards (100-150 gold)
    for (int i = 0; i < 2; ++i) {
        ShopOffer o{ ShopOffer::CardType, 100 + (rand() % 51), nullptr, nullptr, nullptr };
        o.card = createCardByName(commonNames[rand() % commonNames.size()]);
        allOffers.push_back(o);
    }

    // 1 Rare card (280-300 gold) — Rare cards NEVER go on sale
    ShopOffer rare{ ShopOffer::CardType, 280 + (rand() % 21), nullptr, nullptr, nullptr };
    rare.card = createCardByName(rareNames[rand() % rareNames.size()]);
    allOffers.push_back(rare);

    // Sale: 30% chance one COMMON card is 80 gold
    if ((rand() % 100) < 30) {
        int saleIdx = rand() % 2; // Only common cards (index 0 or 1)
        allOffers[saleIdx].price = 80;
    }

    // 3 Potions (60-80 gold, FairyInABottle always 125)
    std::vector<std::function<Potion* ()>> potFactories = {
        []() { return new BlockPotion(); },
        []() { return new FirePotion(); },
        []() { return new EnergyPotion(); },
        []() { return new SwiftPotion(); },
        []() { return new FairyInABottle(); }
    };
    for (int i = 0; i < 3; ++i) {
        Potion* p = potFactories[rand() % potFactories.size()]();
        int price = 60 + (rand() % 21);
        if (dynamic_cast<FairyInABottle*>(p)) price = 125;
        ShopOffer o{ ShopOffer::PotionType, price, nullptr, p, nullptr };
        allOffers.push_back(o);
    }
}

void ShopScreen::setupUI() {
    setMinimumSize(1280, 720);

    backgroundLabel = new QLabel(this);
    backgroundLabel->setPixmap(QPixmap(":/assets/shop_bg.png")
        .scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    backgroundLabel->setGeometry(0, 0, width(), height());

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(40, 30, 40, 30);

    auto* topBar = new QHBoxLayout();
    auto* title = new QLabel("Shop", this);
    title->setStyleSheet("color: #f5c518; font-size: 42px; font-weight: 900; background: transparent;");
    topBar->addWidget(title);
    topBar->addStretch();

    goldLabel = new QLabel(this);
    goldLabel->setStyleSheet("color: #ffd700; font-size: 22px; font-weight: bold; background: transparent;");
    topBar->addWidget(goldLabel);
    mainLayout->addLayout(topBar);

    auto* cardsTitle = new QLabel("Cards", this);
    cardsTitle->setStyleSheet("color: white; font-size: 20px; font-weight: bold; background: transparent;");
    mainLayout->addWidget(cardsTitle, 0, Qt::AlignLeft);

    auto* cardsLayout = new QHBoxLayout();
    cardsLayout->setSpacing(25);
    cardsLayout->setAlignment(Qt::AlignLeft);
    for (int i = 0; i < 3; ++i) {
        if (allOffers[i].card) {
            auto* w = createCardOfferWidget(allOffers[i].card, allOffers[i].price, i);
            cardsLayout->addWidget(w);
        }
    }
    mainLayout->addLayout(cardsLayout);

    auto* potionsTitle = new QLabel("Potions", this);
    potionsTitle->setStyleSheet("color: white; font-size: 20px; font-weight: bold; background: transparent;");
    mainLayout->addWidget(potionsTitle, 0, Qt::AlignLeft);

    auto* potionsLayout = new QHBoxLayout();
    potionsLayout->setSpacing(25);
    potionsLayout->setAlignment(Qt::AlignLeft);
    for (int i = 3; i < 6; ++i) {
        if (allOffers[i].potion) {
            auto* w = createPotionOfferWidget(allOffers[i].potion, allOffers[i].price, i);
            potionsLayout->addWidget(w);
        }
    }
    mainLayout->addLayout(potionsLayout);

    auto* removalLayout = new QHBoxLayout();
    removalBtn = new QPushButton("Remove a Card (" + QString::number(cardRemovalCost) + " Gold)", this);
    removalBtn->setFixedSize(280, 55);
    removalBtn->setStyleSheet(
        "QPushButton { background-color: #8b0000; color: white; font-weight: bold; "
        "border-radius: 10px; font-size: 15px; border: 2px solid #ff4444; }"
        "QPushButton:hover { background-color: #a50000; }"
        "QPushButton:disabled { background-color: #444; color: #888; border-color: #555; }");
    connect(removalBtn, &QPushButton::clicked, this, &ShopScreen::onCardRemoval);
    removalLayout->addWidget(removalBtn);
    mainLayout->addLayout(removalLayout);

    leaveBtn = new QPushButton("Leave Shop", this);
    leaveBtn->setFixedSize(180, 50);
    leaveBtn->setStyleSheet(
        "QPushButton { background-color: #2d4a3e; color: white; font-weight: bold; "
        "border-radius: 10px; font-size: 16px; }"
        "QPushButton:hover { background-color: #3a634f; }");
    connect(leaveBtn, &QPushButton::clicked, this, &ShopScreen::onLeaveShop);
    mainLayout->addWidget(leaveBtn, 0, Qt::AlignCenter);

    removalOverlay = new QWidget(this);
    removalOverlay->setStyleSheet("background-color: rgba(0,0,0,230);");
    removalOverlay->hide();

    auto* ovLayout = new QVBoxLayout(removalOverlay);
    ovLayout->setAlignment(Qt::AlignCenter);

    auto* ovTitle = new QLabel("Choose a card to remove", removalOverlay);
    ovTitle->setStyleSheet("color: #ff6b6b; font-size: 26px; font-weight: bold; background: transparent;");
    ovTitle->setAlignment(Qt::AlignCenter);
    ovLayout->addWidget(ovTitle);

    removalCardsContainer = new QWidget(removalOverlay);
    ovLayout->addWidget(removalCardsContainer);

    auto* cancelBtn = new QPushButton("Cancel", removalOverlay);
    cancelBtn->setFixedSize(140, 45);
    cancelBtn->setStyleSheet(
        "QPushButton { background-color: #444; color: white; font-weight: bold; border-radius: 8px; font-size: 14px; }"
        "QPushButton:hover { background-color: #666; }");
    connect(cancelBtn, &QPushButton::clicked, this, &ShopScreen::onCancelRemoval);
    ovLayout->addWidget(cancelBtn, 0, Qt::AlignCenter);

    buildRemovalOverlay();
}

QWidget* ShopScreen::createCardOfferWidget(Card* card, int price, int index) {
    auto* container = new QWidget(this);
    auto* vbox = new QVBoxLayout(container);
    vbox->setAlignment(Qt::AlignCenter);
    vbox->setSpacing(8);

    QPushButton* cardBtn = new QPushButton(container);
    cardBtn->setFixedSize(130, 175);
    QString imgPath = ":/ assets/cards/" + QString::fromStdString(card->getName()) + ".png";
    cardBtn->setIcon(QIcon(imgPath));
    cardBtn->setIconSize(QSize(130, 175));
    cardBtn->setStyleSheet(
        "QPushButton { border: 2px solid #5ec8ff; border-radius: 8px; background: transparent; }"
        "QPushButton:hover { border: 3px solid #ffffff; }");
    connect(cardBtn, &QPushButton::clicked, this, [this, index]() { onBuyOffer(index); });
    vbox->addWidget(cardBtn);

    QLabel* priceLbl = new QLabel(QString::number(price) + " Gold", container);
    priceLbl->setStyleSheet("color: #ffd700; font-weight: bold; font-size: 14px; background: transparent;");
    priceLbl->setAlignment(Qt::AlignCenter);
    vbox->addWidget(priceLbl);

    allOffers[index].btn = cardBtn;
    allOffers[index].priceLabel = priceLbl;
    return container;
}

QWidget* ShopScreen::createPotionOfferWidget(Potion* potion, int price, int index) {
    auto* container = new QWidget(this);
    auto* vbox = new QVBoxLayout(container);
    vbox->setAlignment(Qt::AlignCenter);
    vbox->setSpacing(8);

    QPushButton* potBtn = new QPushButton(container);
    potBtn->setFixedSize(100, 100);
    QString imgPath = ":/assets/potions/" + QString::fromStdString(potion->getName()) + ".png";
    potBtn->setIcon(QIcon(imgPath));
    potBtn->setIconSize(QSize(90, 90));
    potBtn->setStyleSheet(
        "QPushButton { border: 2px solid #ff88cc; border-radius: 8px; background: transparent; }"
        "QPushButton:hover { border: 3px solid #ffffff; }");
    connect(potBtn, &QPushButton::clicked, this, [this, index]() { onBuyOffer(index); });
    vbox->addWidget(potBtn);

    QLabel* priceLbl = new QLabel(QString::number(price) + " Gold", container);
    priceLbl->setStyleSheet("color: #ffd700; font-weight: bold; font-size: 14px; background: transparent;");
    priceLbl->setAlignment(Qt::AlignCenter);
    vbox->addWidget(priceLbl);

    allOffers[index].btn = potBtn;
    allOffers[index].priceLabel = priceLbl;
    return container;
}

void ShopScreen::refreshGoldDisplay() {
    goldLabel->setText("Gold: " + QString::number(player->getGold()));
}

void ShopScreen::updateOfferButtons() {
    for (size_t i = 0; i < allOffers.size(); ++i) {
        auto& offer = allOffers[i];
        bool sold = false;
        if (offer.type == ShopOffer::CardType && offer.card == nullptr) sold = true;
        else if (offer.type == ShopOffer::PotionType && offer.potion == nullptr) sold = true;
        else if (offer.type == ShopOffer::RelicType && offer.relic == nullptr) sold = true;

        bool canAfford = player->getGold() >= offer.price;

        if (offer.btn) {
            offer.btn->setEnabled(!sold && canAfford);
            if (sold) {
                offer.btn->setStyleSheet("QPushButton { border: 2px solid #444; border-radius: 8px; background: rgba(0,0,0,100); }");
                offer.btn->setIcon(QIcon());
            }
        }
        if (offer.priceLabel) {
            if (sold) {
                offer.priceLabel->setText("SOLD");
                offer.priceLabel->setStyleSheet("color: #888; font-weight: bold; font-size: 14px; background: transparent;");
            }
        }
    }

    removalBtn->setText("Remove a Card (" + QString::number(cardRemovalCost) + " Gold)");
    bool canRemove = player->getGold() >= cardRemovalCost && !player->getFullDeck().empty();
    removalBtn->setEnabled(canRemove);
}

void ShopScreen::buildRemovalOverlay() {
    if (QLayout* oldLayout = removalCardsContainer->layout()) {
        QLayoutItem* item;
        while ((item = oldLayout->takeAt(0)) != nullptr) {
            if (item->widget()) delete item->widget();
            delete item;
        }
        delete oldLayout;
    }

    auto* grid = new QGridLayout(removalCardsContainer);
    grid->setSpacing(15);
    grid->setAlignment(Qt::AlignCenter);

    const auto& deck = player->getFullDeck();
    int col = 0, row = 0;
    for (Card* card : deck) {
        QPushButton* cardBtn = new QPushButton(removalCardsContainer);
        cardBtn->setFixedSize(120, 160);
        QString imgPath = ":/assets/cards/" + QString::fromStdString(card->getName()) + ".png";
        cardBtn->setIcon(QIcon(imgPath));
        cardBtn->setIconSize(QSize(120, 160));
        cardBtn->setStyleSheet(
            "QPushButton { border: 2px solid #aaa; border-radius: 6px; background: transparent; }"
            "QPushButton:hover { border: 3px solid #ff4444; }");
        connect(cardBtn, &QPushButton::clicked, this, [this, card]() { onRemoveCardFromDeck(card); });
        grid->addWidget(cardBtn, row, col);
        if (++col >= 6) { col = 0; ++row; }
    }
}

void ShopScreen::onBuyOffer(int index) {
    if (index < 0 || index >= (int)allOffers.size()) return;
    ShopOffer& offer = allOffers[index];

    if (player->getGold() < offer.price) {
        if (offer.priceLabel) {
            offer.priceLabel->setStyleSheet("color: #ff4444; font-weight: bold; font-size: 14px; background: transparent;");
            QTimer::singleShot(300, this, [this, index]() {
                if (index >= 0 && index < (int)allOffers.size() && allOffers[index].priceLabel) {
                    allOffers[index].priceLabel->setStyleSheet("color: #ffd700; font-weight: bold; font-size: 14px; background: transparent;");
                }
                });
        }
        return;
    }

    player->loseGold(offer.price);

    switch (offer.type) {
    case ShopOffer::CardType:
        if (offer.card) { player->addCardToDrawPile(offer.card); offer.card = nullptr; }
        break;
    case ShopOffer::PotionType:
        if (offer.potion) { player->addPotion(offer.potion); offer.potion = nullptr; }
        break;
    case ShopOffer::RelicType:
        if (offer.relic) { player->addRelic(offer.relic); offer.relic = nullptr; }
        break;
    }

    refreshGoldDisplay();
    updateOfferButtons();
}

void ShopScreen::onCardRemoval() {
    if (player->getGold() < cardRemovalCost) {
        removalBtn->setStyleSheet(
            "QPushButton { background-color: #ff4444; color: white; font-weight: bold; "
            "border-radius: 10px; font-size: 15px; border: 2px solid #ff4444; }");
        QTimer::singleShot(300, this, [this]() {
            removalBtn->setStyleSheet(
                "QPushButton { background-color: #8b0000; color: white; font-weight: bold; "
                "border-radius: 10px; font-size: 15px; border: 2px solid #ff4444; }"
                "QPushButton:hover { background-color: #a50000; }"
                "QPushButton:disabled { background-color: #444; color: #888; border-color: #555; }");
            });
        return;
    }
    buildRemovalOverlay();
    removalOverlay->setGeometry(0, 0, width(), height());
    removalOverlay->show();
    removalOverlay->raise();
}

void ShopScreen::onCancelRemoval() {
    removalOverlay->hide();
}

void ShopScreen::onRemoveCardFromDeck(Card* card) {
    if (!card || player->getGold() < cardRemovalCost) return;

    player->loseGold(cardRemovalCost);
    player->removeCardFromDeck(card);
    cardRemovalCost += 25;

    removalOverlay->hide();
    refreshGoldDisplay();
    updateOfferButtons();
}

void ShopScreen::onLeaveShop() {
    emit finished();
}

void ShopScreen::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    backgroundLabel->setGeometry(0, 0, width(), height());
    backgroundLabel->setPixmap(QPixmap(":/assets/shop_bg.png")
        .scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    if (removalOverlay) {
        removalOverlay->setGeometry(0, 0, width(), height());
    }
}