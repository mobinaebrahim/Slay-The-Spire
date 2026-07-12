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

    // playerInfoGroup = new QGroupBox("Player", this);
    // QVBoxLayout* playerLayout = new QVBoxLayout(playerInfoGroup);

    // playerNameLabel = new QLabel("Name: Dina");
    // playerHpLabel = new QLabel("HP: 80/80");
    // playerBlockLabel = new QLabel("Block: 0");
    // playerEnergyLabel = new QLabel("Energy: 3/3");
    // playerStatusLabel = new QLabel("Status: None");

    // QFont font = playerNameLabel->font();
    // font.setPointSize(11);
    // playerNameLabel->setFont(font);
    // playerHpLabel->setFont(font);
    // playerBlockLabel->setFont(font);
    // playerEnergyLabel->setFont(font);
    // playerStatusLabel->setFont(font);

    // playerLayout->addWidget(playerNameLabel);
    // playerLayout->addWidget(playerHpLabel);
    // playerLayout->addWidget(playerBlockLabel);
    // playerLayout->addWidget(playerEnergyLabel);
    // playerLayout->addWidget(playerStatusLabel);

    // enemyInfoGroup = new QGroupBox("Enemies", this);
    // enemyListLayout = new QVBoxLayout(enemyInfoGroup);

    // QHBoxLayout* infoLayout = new QHBoxLayout;
    // infoLayout->addWidget(playerInfoGroup);
    // infoLayout->addWidget(enemyInfoGroup);
    // infoLayout->addStretch();

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

    mainLayout->addWidget(ui->EndTurnButton);
    //mainLayout->addLayout(infoLayout);

    mainLayout->addStretch(1);
    mainLayout->addStretch(3);

    mainLayout->addWidget(ui->CardsContainer);

    delete this->centralWidget()->layout();
    this->centralWidget()->setLayout(mainLayout);

    QHBoxLayout* cardLayout = new QHBoxLayout(ui->CardsContainer);
    ui->CardsContainer->setLayout(cardLayout);
    ui->EndTurnButton->setFixedHeight(40);

    backgroundLabel = new QLabel(this);
    backgroundLabel->setPixmap(QPixmap(":/images/scene.png").scaled(this->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    backgroundLabel->setGeometry(0, 0, this->width(), this->height());
    backgroundLabel->lower();

    std::srand(std::time(nullptr));
    battleManager = new BattleManager();
    playerObject = new Player("Dina", 80, 80, 3, battleManager);
    battleManager->spawnEnemy(new GremlinKnob());
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


void MainWindow::updateCharacterUI() {
    if (!playerObject) return;

    // playerNameLabel->setText(QString("Name: %1").arg(QString::fromStdString(playerObject->getName())));
    // playerHpLabel->setText(QString("HP: %1/%2").arg(playerObject->getHp()).arg(playerObject->getMaxHp()));
    // playerBlockLabel->setText(QString("Block: %1").arg(playerObject->getBlock()));
    // playerEnergyLabel->setText(QString("Energy: %1/%2").arg(playerObject->getEnergy()).arg(playerObject->getMaxEnergy()));

    // QString statusText;
    // for (auto* effect : playerObject->getEffects()) {
    //     statusText += QString::fromStdString(effect->getName()) + ":" + QString::number(effect->getAmount()) + " ";
    // }
    // if (statusText.isEmpty())
    //statusText = "None";
    // playerStatusLabel->setText("Status: " + statusText);

    const auto& enemies = battleManager->getEnemies();
    if (!enemies.empty()) {
        Enemy* enemy = enemies[0];
        QString enemyName = QString::fromStdString(enemy->getName());
        QPixmap enemyPixmap = getEnemyPixmap(enemyName);
        enemySpriteLabel->setPixmap(enemyPixmap.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        enemySpriteLabel->show();
    } else
        enemySpriteLabel->hide();

    // QLayoutItem* child;
    // while ((child = enemyListLayout->takeAt(0)) != nullptr) {
    //     if (child->widget()) {
    //         child->widget()->hide();
    //         delete child->widget();
    //     }
    //     delete child;
    // }

    // if (enemies.empty()) {
    //     QLabel* emptyLabel = new QLabel("No enemies", enemyInfoGroup);
    //     enemyListLayout->addWidget(emptyLabel);
    // } else {
    //     for (Enemy* enemy : enemies) {
    //         QGroupBox* enemyBox = new QGroupBox(enemyInfoGroup);
    //         QHBoxLayout* enemyBoxLayout = new QHBoxLayout(enemyBox);

    //         QVBoxLayout* infoLayout = new QVBoxLayout();
    //         infoLayout->addWidget(new QLabel(QString("Name: %1").arg(QString::fromStdString(enemy->getName())), enemyBox));
    //         infoLayout->addWidget(new QLabel(QString("HP: %1/%2").arg(enemy->getHp()).arg(enemy->getMaxHp()), enemyBox));
    //         infoLayout->addWidget(new QLabel(QString("Block: %1").arg(enemy->getBlock()), enemyBox));
    //         infoLayout->addWidget(new QLabel(QString("Intent: %1").arg(QString::fromStdString(enemy->getIntentString())), enemyBox));

    //         enemyBoxLayout->addLayout(infoLayout);
    //         enemyBoxLayout->addStretch();

    //         enemyListLayout->addWidget(enemyBox);
    //     }
    // }
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