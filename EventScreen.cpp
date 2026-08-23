#include "EventScreen.h"
#include <QTimer>
#include <QResizeEvent>
#include <QDebug>
#include <cstdlib>
#include <ctime>

static const double CARD_ASPECT_W = 1456.0;
static const double CARD_ASPECT_H = 729.0;

EventScreen::EventScreen(Player* player, QWidget* parent)
    : QWidget(parent), player(player), currentType(EventType::OminousForge) {
    std::srand(std::time(nullptr));
    setStyleSheet("background-color: black;");
    setupUI();
}

QString EventScreen::imagePathForEvent(EventType type) const {
    switch (type) {
    case EventType::OminousForge: return ":/assets/events/OminousForge.png";
    case EventType::GoldenIdol:   return ":/assets/events/GoldenIdol.png";
    case EventType::Augmenter:    return ":/assets/events/Augmenter.png";
    case EventType::FaceTrader:   return ":/assets/events/FaceTrader.png";
    case EventType::TheColosseum: return ":/assets/events/TheColosseum.png";
    case EventType::GoldenShrine: return ":/assets/events/GoldenShrine.png";
    case EventType::Lab:          return ":/assets/events/Lab.png";
    case EventType::ShiningLight: return ":/assets/events/ShiningLight.png";
    case EventType::TheSerpent:   return ":/assets/events/TheSsssssserpant.png";
    }
    return "";
}

void EventScreen::setupUI() {
    setMinimumSize(1280, 720);

    cardContainer = new QWidget(this);

    frameLabel = new QLabel(cardContainer);
    frameLabel->setScaledContents(true);
    QPixmap framePix(":/assets/events/event_frame.png");
    if (!framePix.isNull())
        frameLabel->setPixmap(framePix);
    else
        frameLabel->setStyleSheet("background-color: #1a1a1a; border: 2px solid #555;");

    eventImageLabel = new QLabel(cardContainer);
    eventImageLabel->setAlignment(Qt::AlignCenter);
    eventImageLabel->setStyleSheet("background: transparent;");
    eventImageLabel->setScaledContents(true);

    titleLabel = new QLabel(cardContainer);
    titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    titleLabel->setWordWrap(true);
    titleLabel->setStyleSheet(
        "color: #f5c518; font-size: 30px; font-weight: 900; background: transparent;");

    descLabel = new QLabel(cardContainer);
    descLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet(
        "color: #dddddd; font-size: 16px; background: transparent; line-height: 1.4;");

    optionsContainer = new QWidget(cardContainer);
    optionsContainer->setStyleSheet("background: transparent;");

    frameLabel->lower();
    eventImageLabel->raise();
    titleLabel->raise();
    descLabel->raise();
    optionsContainer->raise();
}

void EventScreen::layoutCard() {
    int availW = width();
    int availH = height();

    double scale = std::min(availW / CARD_ASPECT_W, availH / CARD_ASPECT_H);
    int cardW = static_cast<int>(CARD_ASPECT_W * scale);
    int cardH = static_cast<int>(CARD_ASPECT_H * scale);
    int cardX = (availW - cardW) / 2;
    int cardY = (availH - cardH) / 2;

    cardContainer->setGeometry(cardX, cardY, cardW, cardH);
    frameLabel->setGeometry(0, 0, cardW, cardH);

    int imgX = static_cast<int>(0.0755 * cardW);
    int imgY = static_cast<int>(0.2537 * cardH);
    int imgW = static_cast<int>(0.3229 * cardW);
    int imgH = static_cast<int>(0.6379 * cardH);
    eventImageLabel->setGeometry(imgX, imgY, imgW, imgH);

    QString imgPath = imagePathForEvent(currentType);
    QPixmap pix(imgPath);
    if (!pix.isNull()) {
        eventImageLabel->setPixmap(pix);
        eventImageLabel->setStyleSheet("background: transparent;");
    } else {
        qDebug() << "Event image NOT FOUND:" << imgPath;
        eventImageLabel->setStyleSheet("background-color: #0d1a1a; border: 2px solid #6ea8a8;");
        eventImageLabel->setText("No Image");
    }

    int textX = static_cast<int>(0.4121 * cardW);
    int textY = static_cast<int>(0.15 * cardH);
    int textW = static_cast<int>(0.53 * cardW);

    titleLabel->setGeometry(textX, textY, textW, static_cast<int>(0.12 * cardH));
    descLabel->setGeometry(textX, textY + static_cast<int>(0.14 * cardH), textW, static_cast<int>(0.38 * cardH));

    int titleFontPx = std::max(16, static_cast<int>(30 * scale));
    int descFontPx = std::max(11, static_cast<int>(16 * scale));
    titleLabel->setStyleSheet(QString("color: #f5c518; font-size: %1px; font-weight: 900; background: transparent;").arg(titleFontPx));
    descLabel->setStyleSheet(QString("color: #dddddd; font-size: %1px; background: transparent;").arg(descFontPx));

    int optX = static_cast<int>(0.4258 * cardW);
    int optW = static_cast<int>(0.467 * cardW);
    int barH = static_cast<int>(0.082 * cardH);
    int barSpacing = static_cast<int>(0.007 * cardH);

    int n = (int)optionButtons.size();
    int totalH = n * barH + (n - 1) * barSpacing;
    int startY = cardH - static_cast<int>(0.08 * cardH) - totalH;

    optionsContainer->setGeometry(0, 0, cardW, cardH);

    for (int i = 0; i < n; ++i) {
        int y = startY + i * (barH + barSpacing);
        optionButtons[i]->setGeometry(optX, y, optW, barH);

        int btnFontPx = std::max(10, static_cast<int>(15 * scale));
        QPixmap barPix(":/assets/events/option_bar.png");
        if (!barPix.isNull()) {
            optionButtons[i]->setStyleSheet(QString(
                                                "QPushButton { border-image: url(assets/events/option_bar.png) 0 0 0 0 stretch stretch; "
                                                "color: white; font-weight: bold; font-size: %1px; text-align: left; padding-left: 20px; border: none; }"
                                                "QPushButton:hover { background-color: rgba(255,255,255,30); }"
                                                ).arg(btnFontPx));
        } else {
            optionButtons[i]->setStyleSheet(QString(
                                                "QPushButton { background-color: #1f5f5f; color: white; font-weight: bold; "
                                                "font-size: %1px; text-align: left; padding-left: 20px; border-radius: 6px; "
                                                "border: 2px solid #3fa3a3; }"
                                                "QPushButton:hover { background-color: #2a7a7a; }"
                                                "QPushButton:disabled { background-color: #222; color: #666; border-color: #444; }"
                                                ).arg(btnFontPx));
        }
    }
}

void EventScreen::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    layoutCard();
}

void EventScreen::setEvent(EventType type) {
    currentType = type;
    clearOptions();

    switch (type) {
    case EventType::OminousForge:
        titleLabel->setText("Ominous Forge");
        descLabel->setText("You discover a glowing forge. The heat suggests it could be used to upgrade a card.");
        setupOminousForge();
        break;
    case EventType::GoldenIdol:
        titleLabel->setText("Golden Idol");
        descLabel->setText("An ornate statue of a forgotten deity stands before you. Its eyes seem to follow your every move.");
        setupGoldenIdol();
        break;
    case EventType::Augmenter:
        titleLabel->setText("Augmenter");
        descLabel->setText("A strange creature covered in mechanical parts offers to 'improve' your body.");
        setupAugmenter();
        break;
    case EventType::FaceTrader:
        titleLabel->setText("Face Trader");
        descLabel->setText("A mysterious figure with a bag of faces grins at you. 'Care to trade?' he asks.");
        setupFaceTrader();
        break;
    case EventType::TheColosseum:
        titleLabel->setText("The Colosseum");
        descLabel->setText("A booming voice challenges you to prove your strength in the arena against an elite foe.");
        setupTheColosseum();
        break;
    case EventType::GoldenShrine:
        titleLabel->setText("Golden Shrine");
        descLabel->setText("A shrine glittering with gold stands before you. The deity seems to favor the greedy...");
        setupGoldenShrine();
        break;
    case EventType::Lab:
        titleLabel->setText("Lab");
        descLabel->setText("An abandoned laboratory. Broken glass and bubbling beakers litter the room. Some potions remain intact.");
        setupLab();
        break;
    case EventType::ShiningLight:
        titleLabel->setText("Shining Light");
        descLabel->setText("A blinding light envelops the area. You feel the power within your cards reacting to the radiance.");
        setupShiningLight();
        break;
    case EventType::TheSerpent:
        titleLabel->setText("TheSsssssserpant");
        descLabel->setText("A giant serpent coils around a pile of gold. 'Ssssso much gold... for a ssssmall favor,' it hisses.");
        setupTheSerpent();
        break;
    }

    layoutCard();
}

void EventScreen::clearOptions() {
    for (auto* btn : optionButtons) {
        btn->hide();
        btn->deleteLater();
    }
    optionButtons.clear();
    currentOptions.clear();
}

void EventScreen::addOption(const QString& text, const QString& tooltip, std::function<void()> action) {
    QPushButton* btn = new QPushButton(text, optionsContainer);
    btn->setToolTip(tooltip);
    btn->setCursor(Qt::PointingHandCursor);

    int idx = (int)currentOptions.size();
    connect(btn, &QPushButton::clicked, this, [this, idx]() { onOptionClicked(idx); });

    btn->show();
    optionButtons.push_back(btn);
    currentOptions.push_back({ text, tooltip, action });
}

void EventScreen::onOptionClicked(int index) {
    if (index < 0 || index >= (int)currentOptions.size()) return;
    currentOptions[index].action();
}

void EventScreen::onLeave() {
    emit finished();
}

/* ============================================================ */
void EventScreen::setupOminousForge() {
    addOption("[Forge] Upgrade a card",
              "Upgrade one card in your deck.",
              [this]() {
                  emit requestCardUpgrade();
                  emit finished();
              });
    addOption("[Rummage] Obtain Warped Tongs + Curse",
              "Gain Warped Tongs relic, but receive a Necronomicurse.",
              [this]() {
                  player->addCardToDrawPile(createCardByName("Necronomicurse"));
                  emit finished();
              });
    addOption("[Leave] Nothing happens",
              "Leave the forge alone.",
              [this]() { onLeave(); });
}

void EventScreen::setupGoldenIdol() {
    addOption("[Steal] Obtain Golden Idol - 5 Max HP",
              "Gain the Golden Idol relic, but lose 5 Max HP.",
              [this]() {
                  player->loseMaxHP(5);
                  emit finished();
              });
    addOption("[Leave] Nothing happens",
              "Leave the idol untouched.",
              [this]() { onLeave(); });
}

void EventScreen::setupAugmenter() {
    addOption("[Test J.A.X] Obtain J.A.X card",
              "Add the J.A.X card to your deck.",
              [this]() {
                  player->addCardToDrawPile(createCardByName("JAX"));
                  emit finished();
              });
    addOption("[Become Test Subject] Transform 2 cards",
              "Choose 2 cards from your deck to transform into random cards.",
              [this]() {
                  emit requestCardTransform(2);
                  emit finished();
              });
    addOption("[Ingest Mutagens] Obtain Mutagenic Strength",
              "Gain the Mutagenic Strength relic.",
              [this]() {
                  emit finished();
              });
}

void EventScreen::setupFaceTrader() {
    addOption("[Touch] Lose 5-10 Max HP + 75 Gold",
              "Sacrifice some health for gold.",
              [this]() {
                  int dmg = 5 + (rand() % 6);
                  player->takeDamage(dmg);
                  player->increaseGold(75);
                  emit finished();
              });
    addOption("[Trade] Obtain a random Face Relic",
              "Gain Face of Cleric, Gremlin Visage, or Cultist Headpiece.",
              [this]() {
                  std::vector<std::string> faces = {
                      "FaceOfCleric", "GremlinVisage", "CultistHeadpiece"
                  };
                  std::string chosen = faces[rand() % faces.size()];
                  player->addRelic(createRelicByName(chosen));
                  emit finished();
              });
    addOption("[Leave] Nothing happens",
              "Decline the offer.",
              [this]() { onLeave(); });
}

void EventScreen::setupTheColosseum() {
    addOption("[Fight] Fight an Elite for rewards",
              "Engage in combat with an elite enemy for normal elite rewards.",
              [this]() {
                  Enemy* elite = createEliteEnemy("GremlinKnob");
                  emit requestCombat(elite);
              });
    addOption("[Leave] Decline the challenge",
              "Leave the arena.",
              [this]() { onLeave(); });
}

void EventScreen::setupGoldenShrine() {
    addOption("[Pray] Gain 100 Gold",
              "Pray to the shrine and receive gold.",
              [this]() {
                  player->increaseGold(100);
                  emit finished();
              });
    addOption("[Leave] Nothing happens",
              "Leave the shrine alone.",
              [this]() { onLeave(); });
}

void EventScreen::setupLab() {
    addOption("[Search] Obtain 3 random Potions",
              "Search the abandoned lab for usable potions.",
              [this]() {
                  std::vector<std::function<Potion* ()>> factories = {
                      []() { return new BlockPotion(); },
                      []() { return new FirePotion(); },
                      []() { return new EnergyPotion(); },
                      []() { return new SwiftPotion(); },
                      []() { return new FairyInABottle(); }
                  };
                  for (int i = 0; i < 3; ++i) {
                      player->addPotion(factories[rand() % factories.size()]());
                  }
                  emit finished();
              });
    addOption("[Leave] Nothing happens",
              "Leave the lab.",
              [this]() { onLeave(); });
}

void EventScreen::setupShiningLight() {
    addOption("[Enter] Take 20% Max HP damage + Upgrade 2 random cards",
              "Endure the light's burn to upgrade 2 random cards in your deck.",
              [this]() {
                  int dmg = player->getMaxHp() * 0.20;
                  player->takeDamage(dmg);

                  auto deck = player->getFullDeck();
                  if (!deck.empty()) {
                      int idx1 = rand() % deck.size();
                      deck[idx1]->upgrade();

                      if (deck.size() >= 2) {
                          int idx2 = rand() % deck.size();
                          if (idx2 == idx1) idx2 = (idx2 + 1) % deck.size();
                          deck[idx2]->upgrade();
                      }
                  }
                  emit finished();
              });
    addOption("[Leave] Nothing happens",
              "Avoid the light.",
              [this]() { onLeave(); });
}

void EventScreen::setupTheSerpent() {
    addOption("[Agree] Gain 175 Gold + Doubt Curse",
              "Accept the serpent's gold but receive a Doubt curse.",
              [this]() {
                  player->increaseGold(175);
                  player->addCardToDrawPile(createCardByName("Doubt"));
                  emit finished();
              });
    addOption("[Leave] Nothing happens",
              "Decline the serpent's offer.",
              [this]() { onLeave(); });
}