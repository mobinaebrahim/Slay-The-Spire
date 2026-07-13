#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>
#include <QPushButton>
#include<QDebug>
#include<Qfile>
#include <QLabel>
#include <QLayout>
#include <QLayoutItem>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "../card.h"
#include "../AttackCard.h"
#include "../CurseCard.h"
#include "../SkillCard.h"
#include "../PowerCard.h"
#include "../StatusCard.h"
#include "../Player.h"
#include "../BattleManager.h"
#include "../BossStruggles.h"
#include "../character.h"
#include "../NormalEnemies.h"
#include "../EliteEnemies.h"
#include "../enemy.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setMinimumSize(1280, 720);

    playerEnergyOrb = new QLabel(this);
    playerEnergyOrb->setFixedSize(70, 70);
    playerEnergyOrb->setPixmap(QPixmap(":/images/icons/energy.png").scaled(70, 70, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    energyOrbCountLabel = new QLabel(playerEnergyOrb);
    energyOrbCountLabel->setGeometry(0, 0, 70, 70);
    energyOrbCountLabel->setAlignment(Qt::AlignCenter);
    energyOrbCountLabel->setStyleSheet("background: transparent; color: white; font-weight: 900; font-size: 20px;");

    topHudBar = new QWidget(this);
    topHudBar->setStyleSheet(
        "background-color: rgba(300, 300, 300, 250); "
        "border-bottom: 2px solid rgba(255, 215, 130, 60);"
        );

    playerHeartIcon = new QLabel(this);
    playerHeartIcon->setFixedSize(40, 40);
    playerHeartIcon->setPixmap(QPixmap(":/images/icons/hp.png").scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    playerHeartIcon->move(14, 6);

    playerHpTopLabel = new QLabel(this);
    playerHpTopLabel->setGeometry(58, 24, 70, 24);
    playerHpTopLabel->setStyleSheet("color: white; font-weight: bold; font-size: 14px; background: transparent;");

    goldIconLabel = new QLabel(this);
    goldIconLabel->setFixedSize(46, 46);
    goldIconLabel->setPixmap(QPixmap(":/images/icons/gold.png").scaled(46, 46, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    goldIconLabel->move(150, 3);

    goldCountLabel = new QLabel(goldIconLabel);
    goldCountLabel->setGeometry(14, 30, 32, 16);
    goldCountLabel->setAlignment(Qt::AlignCenter);
    goldCountLabel->setStyleSheet("color: white; font-weight: bold; font-size: 14px; background: transparent;");

    drawPileIconLabel = new QLabel(this);
    drawPileIconLabel->setFixedSize(84, 84);
    drawPileIconLabel->setPixmap(QPixmap(":/images/icons/draw_pile.png").scaled(84, 84, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    drawPileCountLabel = new QLabel(drawPileIconLabel);
    drawPileCountLabel->setGeometry(12, 28, 28, 16);
    drawPileCountLabel->setAlignment(Qt::AlignCenter);
    drawPileCountLabel->setStyleSheet(
        "background-color: rgba(0,0,0,170); color: white; font-weight: bold; "
        "font-size: 11px; border-radius: 4px;");

    discardPileIconLabel = new QLabel(this);
    discardPileIconLabel->setFixedSize(84, 84);
    discardPileIconLabel->setPixmap(QPixmap(":/images/icons/discard_pile.png").scaled(84, 84, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    discardPileCountLabel = new QLabel(discardPileIconLabel);
    discardPileCountLabel->setGeometry(12, 28, 28, 16);
    discardPileCountLabel->setAlignment(Qt::AlignCenter);
    discardPileCountLabel->setStyleSheet(
        "background-color: rgba(0,0,0,170); color: white; font-weight: bold; "
        "font-size: 11px; border-radius: 4px;");

    ui->EndTurnButton->setText("");
    ui->EndTurnButton->setIcon(QIcon(":/images/icons/end_turn.png"));
    ui->EndTurnButton->setIconSize(QSize(320, 100));
    ui->EndTurnButton->setFixedSize(230, 82);
    ui->EndTurnButton->setContentsMargins(0, 0, 0, 0);
    ui->EndTurnButton->setStyleSheet(
        "QPushButton { border: none; background: transparent; } "
        "QPushButton:hover { background: rgba(200,200,200,25); border-radius: 5px; } "
        "QPushButton:pressed { background: rgba(200,200,200,50); border-radius: 5px; } "
        "QPushButton:disabled { opacity: 0.3; }");

    ui->EndTurnButton->setParent(this);

    playerHpBar = new QProgressBar(this);
    playerHpBar->setTextVisible(true);
    playerHpBar->setRange(0, 80);
    playerHpBar->setStyleSheet(
        "QProgressBar { border: 2px solid #3a1f1f; border-radius: 6px; "
        "background: #2b1414; color: white; font-weight: bold; text-align: center; }"
        "QProgressBar::chunk { background-color: qlineargradient(x1:0,y1:0,x2:1,y2:0, "
        "stop:0 #8e0e0e, stop:1 #d94040); border-radius: 4px; }"
        );

    playerBlockBadge = new QLabel(this);
    playerBlockBadge->setAlignment(Qt::AlignCenter);
    playerBlockBadge->setFixedSize(30, 30);
    playerBlockBadge->setStyleSheet(
        "background-color: #2b3a55; color: #9fd8ff; border: 2px solid #5c85b0; "
        "border-radius: 15px; font-weight: bold;"
        );
    playerBlockBadge->hide();

    enemyHpBar = new QProgressBar(this);
    enemyHpBar->setTextVisible(true);
    enemyHpBar->setStyleSheet(playerHpBar->styleSheet());

    enemyBlockBadge = new QLabel(this);
    enemyBlockBadge->setAlignment(Qt::AlignCenter);
    enemyBlockBadge->setFixedSize(30, 30);
    enemyBlockBadge->setStyleSheet(playerBlockBadge->styleSheet());
    enemyBlockBadge->hide();

    enemyIntentLabel = new QLabel(this);
    enemyIntentLabel->setAlignment(Qt::AlignCenter);
    enemyIntentLabel->setFixedHeight(28);
    enemyIntentLabel->setStyleSheet(
        "background-color: rgba(20,20,20,190); color: white; border-radius: 8px; "
        "padding: 3px; font-weight: bold;"
        );

    enemyNameLabel = new QLabel(this);
    enemyNameLabel->setAlignment(Qt::AlignCenter);
    enemyNameLabel->setFixedHeight(20);
    enemyNameLabel->setStyleSheet(
        "color: white; font-weight: bold; font-size: 13px; "
        "background-color: rgba(0,0,0,140); border-radius: 4px; padding: 2px;");

    playerSpriteLabel = new QLabel(this);
    playerSpriteLabel->setPixmap(QPixmap(":/images/characters/IronClad.png").scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    playerSpriteLabel->setAlignment(Qt::AlignCenter);
    playerSpriteLabel->setScaledContents(true);

    enemySpriteLabel = new QLabel(this);
    enemySpriteLabel->setAlignment(Qt::AlignCenter);
    enemySpriteLabel->setScaledContents(true);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    mainLayout->addStretch(1);
    mainLayout->addStretch(3);

    QHBoxLayout* bottomBarLayout = new QHBoxLayout();
    bottomBarLayout->addWidget(playerEnergyOrb, 0, Qt::AlignBottom);
    bottomBarLayout->addWidget(ui->CardsContainer, 1);
    bottomBarLayout->addWidget(drawPileIconLabel, 0, Qt::AlignBottom);
    bottomBarLayout->addWidget(discardPileIconLabel, 0, Qt::AlignBottom);
    bottomBarLayout->addSpacing(12);
    mainLayout->addLayout(bottomBarLayout);

    delete this->centralWidget()->layout();
    this->centralWidget()->setLayout(mainLayout);

    QHBoxLayout* cardLayout = new QHBoxLayout(ui->CardsContainer);
    ui->CardsContainer->setLayout(cardLayout);

    backgroundLabel = new QLabel(this);
    backgroundLabel->setPixmap(QPixmap(":/images/scene.png").scaled(this->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    backgroundLabel->setGeometry(0, 0, this->width(), this->height());
    backgroundLabel->lower();
    topHudBar->raise();
    playerHeartIcon->raise();
    playerHpTopLabel->raise();
    goldIconLabel->raise();
    goldCountLabel->raise();
    ui->EndTurnButton->raise();

    std::srand(std::time(nullptr));
    battleManager = new BattleManager();
    playerObject = new Player("Dina", 80, 80, 3, 99, battleManager);
    battleManager->spawnEnemy(new JawWorm());
    initializePlayerDeck(15);
    playerObject->drawCards(5);

    updateHandUI();
    updateCharacterUI();

    animationTimer = new QTimer(this);
    connect(animationTimer, &QTimer::timeout, this, &MainWindow::updateAnimations);
    animationTimer->start(50);
}
MainWindow::~MainWindow()
{
    delete ui;
    delete playerObject;
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);

    if (backgroundLabel) {
        backgroundLabel->resize(this->size());
        QPixmap original(":/images/scene.png");
        backgroundLabel->setPixmap(original.scaled(this->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    }

    int spriteSize = 200;
    int gap = 550;

    currentStartX = (this->width() - (spriteSize * 2 + gap)) / 2;
    basePlayerY = (this->height() / 2) - (spriteSize / 2) - 80;
    baseEnemyY = basePlayerY;

    playerSpriteLabel->setGeometry(currentStartX, basePlayerY, spriteSize, spriteSize);
    enemySpriteLabel->setGeometry(currentStartX + spriteSize + gap, baseEnemyY, spriteSize, spriteSize);

    int barWidth = 180, barHeight = 22;

    topHudBar->setGeometry(0, 0, this->width(), 48);

    playerHpBar->setGeometry(
        currentStartX + spriteSize / 2 - barWidth / 2,
        basePlayerY + spriteSize + 8,
        barWidth, barHeight);

    playerBlockBadge->setGeometry(currentStartX + spriteSize - 30, basePlayerY - 8, 30, 30);

    int enemyX2 = currentStartX + spriteSize + gap;

    enemyHpBar->setGeometry(
        enemyX2 + spriteSize / 2 - barWidth / 2,
        baseEnemyY - 32,
        barWidth, barHeight);

    enemyBlockBadge->setGeometry( enemyX2 + spriteSize - 30, baseEnemyY - 8, 30, 30);

    enemyIntentLabel->setGeometry(enemyX2 - 20, baseEnemyY - 65, spriteSize + 40, 28);

    enemyNameLabel->setGeometry(enemyX2 - 20, baseEnemyY - 90, spriteSize + 40, 20);

    int endTurnW = 230, endTurnH = 82;
    ui->EndTurnButton->setGeometry(
        this->width() - endTurnW + 30,
        this->height() - endTurnH - 100,
        endTurnW, endTurnH);
}

void MainWindow::on_EndTurnButton_clicked()
{
    const auto& hand = playerObject->getHand();
    for (Card* card : hand) {
        if (card && card->getName() == "Burn")
            playerObject->decreaseHp(2);
    }
    playerObject->endTurnCleanUp();
    battleManager->cleanupDeadEnemies();
    playerObject->increaseEnergy(3);
    playerObject->drawCards(5);
    updateHandUI();
    updateCharacterUI();
}

void MainWindow::updateHandUI() {
    QLayout* layout = ui->CardsContainer->layout();
    if (!layout) return;

    QLayoutItem *child;
    while ((child = layout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            child->widget()->hide();
            delete child->widget();
        }
        delete child;
    }

    const std::vector<Card*>& playerHand = playerObject->getHand();

    for (Card* card : playerHand) {
        if (!card)
            continue;

        QPushButton* cardBtn = new QPushButton();
        cardBtn->setFixedSize(140, 180);

        QString cardName = QString::fromStdString(card->getName());
        cardBtn->setIcon(QIcon(":/images/cards/" + cardName + ".png"));
        cardBtn->setIconSize(cardBtn->size());
        cardBtn->setEnabled(card->isPlayable());

        connect(cardBtn, &QPushButton::clicked, [=]() {
            const std::vector<Enemy*>& allEnemies = battleManager->getEnemies();
            if (!allEnemies.empty()) {
                playerObject->playCard(card, allEnemies[0]);
                updateHandUI();
                updateCharacterUI();
            }
        });
        layout->addWidget(cardBtn);
    }
}
Card* createCardByName(const std::string& name) {
    if (name == "Bash")            return new BashCard();
    if (name == "Blood for Blood") return new Blood_for_BloodCard();
    if (name == "Clash")           return new ClashCard();
    if (name == "Feed")            return new FeedCard();
    if (name == "Immolate")        return new ImmolateCard();
    if (name == "PerfectedStrike") return new PerfectedStrike();
    if (name == "Reaper")          return new ReaperCard();
    if (name == "Strike")          return new AttackCard(CardType::Attack, "Strike", "Deal 6 damage", 1, 6, 9);
    if (name == "Bludgeon")        return new AttackCard(CardType::Attack, "Bludgeon", "Deal 32 damage", 3, 32, 42);
    if (name == "TwinStrike")      return new TwinStrikeCard();
    if (name == "Whirlwind")       return new WhirlwindCard();
    if (name == "Barricade")       return new BarricadeCard();
    if (name == "Bloodletting")    return new BloodlettingCard();
    if (name == "Brutality")       return new BrutalityCard();
    if (name == "Burn")            return new BurnCard();
    if (name == "CurseOfBell")     return new CurseOfBellCard();
    if (name == "DualWield")       return new DualWieldCard();
    if (name == "Daze")            return new DazeCard();
    if (name == "Defend")          return new DefendCard();
    if (name == "DemonForm")       return new DemonFormCard();
    if (name == "Disarm")          return new DisarmCard();
    if (name == "Entrench")        return new EntrenchCard();
    if (name == "Exhume")          return new ExhumeCard();
    if (name == "FeelNoPain")      return new FeelNoPainCard();
    if (name == "Impervious")      return new ImperviousCard();
    if (name == "Inflame")         return new InflameCard();
    if (name == "JAX")             return new JAXCard();
    if (name == "LimitBreak")      return new LimitBreakCard();
    if (name == "Metallicize")     return new MetallicizeCard();
    if (name == "Offering")        return new OfferingCard();
    if (name == "Regret")          return new RegretCard();
    if (name == "ShrugItOff")      return new ShrugItOffCard();
    if (name == "Slime")           return new SlimeCard();
    if (name == "Wound")           return new WoundCard();
    return nullptr;
}

void MainWindow::initializePlayerDeck(int totalCards) {
    std::vector<std::string> allCardNames = {
    "Bash", "Blood for Blood", "Clash", "Feed", "Immolate", "PerfectedStrike", "Reaper",
    "Strike", "Bludgeon", "TwinStrike", "Whirlwind", "Barricade", "Bloodletting", "Brutality",
    "DualWield", "Defend", "DemonForm", "Disarm", "Entrench",
    "Exhume", "FeelNoPain", "Impervious", "Inflame", "LimitBreak", "Metallicize",
    "Offering", "ShrugItOff", "Daze", "Slime", "Wound", "Burn", "JAX", "CurseOfBell"};

    for (int i = 0; i < totalCards; ++i) {
        int randomIndex = std::rand() % allCardNames.size();
        std::string randomName = allCardNames[randomIndex];

        Card* newCard = createCardByName(randomName);
        if (newCard)
            playerObject->addCardToDrawPile(newCard);
    }
}


static QString intentToShortText(Enemy* enemy) {
    switch (enemy->getIntentType()) {
    case IntentType::Attack:   return "⚔ " + QString::number(enemy->getIntentValue());
    case IntentType::Defend:   return "🛡 Block";
    case IntentType::Buff:     return "⬆ Buff";
    case IntentType::Debuff:   return "⬇ Debuff";
    case IntentType::Combined: return "⚔🛡 " + QString::number(enemy->getIntentValue());
    case IntentType::Special:  return "❓ Special";
    }
    return "?";
}

void MainWindow::updateCharacterUI() {
    if (!playerObject) return;

    playerHpBar->setMaximum(playerObject->getMaxHp());
    playerHpBar->setValue(playerObject->getHp());
    playerHpBar->setFormat(QString("%1 / %2").arg(playerObject->getHp()).arg(playerObject->getMaxHp()));

    int pBlock = playerObject->getBlock();
    playerBlockBadge->setVisible(pBlock > 0);
    if (pBlock > 0)
        playerBlockBadge->setText(QString::number(pBlock));

    energyOrbCountLabel->setText(QString("%1/%2").arg(playerObject->getEnergy()).arg(playerObject->getMaxEnergy()));

    goldCountLabel->setText(QString::number(playerObject->getGold()));
    playerHpTopLabel->setText(QString("%1/%2").arg(playerObject->getHp()).arg(playerObject->getMaxHp()));
    drawPileCountLabel->setText(QString::number(playerObject->getDrawPileSize()));
    discardPileCountLabel->setText(QString::number(playerObject->getDiscardPileSize()));

    const auto& enemies = battleManager->getEnemies();
    bool hasEnemy = !enemies.empty();

    enemySpriteLabel->setVisible(hasEnemy);
    enemyHpBar->setVisible(hasEnemy);
    enemyIntentLabel->setVisible(hasEnemy);
    enemyNameLabel->setVisible(hasEnemy);
    enemyBlockBadge->setVisible(false);

    if (hasEnemy) {
        Enemy* enemy = enemies[0];
        QString enemyName = QString::fromStdString(enemy->getName());

        QPixmap enemyPixmap = getEnemyPixmap(enemyName);
        enemySpriteLabel->setPixmap(enemyPixmap.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        enemyNameLabel->setText(enemyName);

        enemyHpBar->setMaximum(enemy->getMaxHp());
        enemyHpBar->setValue(enemy->getHp());
        enemyHpBar->setFormat(QString("%1 / %2").arg(enemy->getHp()).arg(enemy->getMaxHp()));

        int eBlock = enemy->getBlock();
        enemyBlockBadge->setVisible(eBlock > 0);
        if (eBlock > 0)
            enemyBlockBadge->setText(QString::number(eBlock));

        enemyIntentLabel->setText(intentToShortText(enemy));
    }
}

QPixmap MainWindow::getEnemyPixmap(const QString& enemyName) {
    if (enemyName == "BlueSlaver") return QPixmap(":/images/enemies/BlueSlaver.png");
    if (enemyName == "BookOfStabbing") return QPixmap(":/images/enemies/BookOfStabbing.png");
    if (enemyName == "Cultist") return QPixmap(":/images/enemies/Cultist.png");
    if (enemyName == "GremlinKnob") return QPixmap(":/images/enemies/GremlinKnob.png");
    if (enemyName == "Hexaghost") return QPixmap(":/images/enemies/Hexaghost.png");
    if (enemyName == "JawWorm") return QPixmap(":/images/enemies/JawWorm.png");
    if (enemyName == "KingSlime") return QPixmap(":/images/enemies/KingSlime.png");
    if (enemyName == "LargeSlime") return QPixmap(":/images/enemies/LargeSlime.png");
    if (enemyName == "Looter") return QPixmap(":/images/enemies/Looter.png");
    if (enemyName == "Louse") return QPixmap(":/images/enemies/Louse.png");
    if (enemyName == "MediumSlime") return QPixmap(":/images/enemies/MediumSlime.png");
    if (enemyName == "Mugger") return QPixmap(":/images/enemies/Mugger.png");
    if (enemyName == "RedSlaver") return QPixmap(":/images/enemies/RedSlaver.png");
    if (enemyName == "SmallSlime") return QPixmap(":/images/enemies/SmallSlime.png");
    if (enemyName == "SphericGuardian") return QPixmap(":/images/enemies/SphericGuardian.png");
    if (enemyName == "Taskmaster") return QPixmap(":/images/enemies/Taskmaster.png");
    if (enemyName == "TheChamp") return QPixmap(":/images/enemies/TheChamp.png");
    if (enemyName == "ThreeSentries") return QPixmap(":/images/enemies/ThreeSentries.png");

    return QPixmap(":/images/enemy_default.png");
}

void MainWindow::updateAnimations() {
    angle += 0.2f;
    int floatOffset = static_cast<int>(std::sin(angle) * 6);

    playerSpriteLabel->setGeometry(currentStartX, basePlayerY + floatOffset, 200, 200);
    enemySpriteLabel->setGeometry(currentStartX + 200 + 550, baseEnemyY - floatOffset, 200, 200);
}