#include "combatpage.h"
#include "AttackCard.h"
#include "CurseCard.h"
#include "SkillCard.h"
#include "PowerCard.h"
#include "StatusCard.h"

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

CombatPage::CombatPage(QWidget *parent, bool isLeader)
    : QWidget(parent)
    , m_isLeader(isLeader)
    , m_battleManager(nullptr)
    , m_localPlayer(nullptr)
{
    setWindowFlags(Qt::Window);
    setWindowTitle("Combat");

    setupCombat();
    buildUI();
    updateAllUI();

    connect(&NetworkManager::instance(), &NetworkManager::game_action_received, this,
            &CombatPage::handleNetworkMessage);
}

void CombatPage::setupCombat()
{
    m_battleManager = new BattleManager();

    // بازیکن خودم رو می‌سازم (هر کلاینت این کارو برای خودش می‌کنه)
    m_localPlayer = new Player("Player", 80, 80, 3, 99, m_battleManager);
    m_battleManager->addPlayer(m_localPlayer);

    // فقط Leader دشمن رو spawn می‌کنه
    if (m_isLeader) {
        m_battleManager->spawnEnemy(new JawWorm());
        m_battleManager->startCombat();
    }
}

void CombatPage::buildUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // --- بالا: وضعیت هم‌تیمی ---
    QHBoxLayout *teammateRow = new QHBoxLayout();
    m_teammateNameLabel = new QLabel("Teammate");
    m_teammateHpBar = new QProgressBar();
    m_teammateHpBar->setFixedWidth(200);
    teammateRow->addWidget(m_teammateNameLabel);
    teammateRow->addWidget(m_teammateHpBar);
    teammateRow->addStretch();
    mainLayout->addLayout(teammateRow);

    // --- وسط: دشمن ---
    QVBoxLayout *enemyBox = new QVBoxLayout();
    m_enemyNameLabel = new QLabel("Enemy");
    m_enemyNameLabel->setAlignment(Qt::AlignCenter);
    m_enemyHpBar = new QProgressBar();
    m_enemyHpBar->setFixedWidth(300);
    enemyBox->addWidget(m_enemyNameLabel, 0, Qt::AlignCenter);
    enemyBox->addWidget(m_enemyHpBar, 0, Qt::AlignCenter);
    mainLayout->addLayout(enemyBox);

    mainLayout->addStretch();

    // --- پایین: خودم ---
    QHBoxLayout *bottomRow = new QHBoxLayout();

    QVBoxLayout *myStatusBox = new QVBoxLayout();
    m_myHpBar = new QProgressBar();
    m_myHpBar->setFixedWidth(150);
    m_myEnergyLabel = new QLabel("Energy: 0/0");
    myStatusBox->addWidget(m_myHpBar);
    myStatusBox->addWidget(m_myEnergyLabel);
    bottomRow->addLayout(myStatusBox);

    m_myCardsContainer = new QWidget();
    QHBoxLayout *cardsLayout = new QHBoxLayout(m_myCardsContainer);
    bottomRow->addWidget(m_myCardsContainer, 1);

    m_endTurnBtn = new QPushButton("End Turn");
    bottomRow->addWidget(m_endTurnBtn);

    mainLayout->addLayout(bottomRow);

    setLayout(mainLayout);
}

void CombatPage::updateMyHandUI()
{
    QLayout *layout = m_myCardsContainer->layout();
    QLayoutItem *child;
    while ((child = layout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            child->widget()->hide();
            delete child->widget();
        }
        delete child;
    }

    const std::vector<Card*>& hand = m_localPlayer->getHand();
    for (Card *card : hand) {
        if (!card) continue;

        QPushButton *cardBtn = new QPushButton(QString::fromStdString(card->getName()));
        cardBtn->setFixedSize(100, 140);
        cardBtn->setEnabled(card->isPlayable());

        connect(cardBtn, &QPushButton::clicked, this, [this, card]() {
            const auto& enemies = m_battleManager->getEnemies();
            if (enemies.empty()) return;

            m_battleManager->playCardAction(m_localPlayer, card, enemies[0]);
            m_battleManager->cleanupDeadEnemies();

            updateAllUI();
        });

        layout->addWidget(cardBtn);
    }
}

void CombatPage::updateMyHandUI()
{
    QLayout *layout = m_myCardsContainer->layout();
    QLayoutItem *child;
    while ((child = layout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            child->widget()->hide();
            delete child->widget();
        }
        delete child;
    }

    const std::vector<Card*>& hand = m_localPlayer->getHand();
    for (Card *card : hand) {
        if (!card) continue;
        QPushButton *cardBtn = new QPushButton(QString::fromStdString(card->getName()));
        cardBtn->setFixedSize(100, 140);
        layout->addWidget(cardBtn);
    }
}

void CombatPage::handleNetworkMessage(const QJsonObject &obj)
{
    QString type = obj["type"].toString();
    // قدم‌های بعدی: پردازش player_action, enemy_state, end_turn
}

CombatPage::~CombatPage()
{
    delete m_battleManager;
}