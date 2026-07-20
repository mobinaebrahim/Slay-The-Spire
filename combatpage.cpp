#include "combatpage.h"

CombatPage::CombatPage(QWidget *parent, bool isLeader)
    : QWidget(parent)
    , m_isLeader(isLeader)
{
    setWindowFlags(Qt::Window);
    setWindowTitle("Combat");

    buildUI();
    setupCombat();

    connect(&NetworkManager::instance(), &NetworkManager::game_action_received, this,
            &CombatPage::handleNetworkMessage);
}

void CombatPage::setupCombat()
{
    // دیگه هیچ BattleManager/Player محلی نمی‌سازیم.
    // فقط اگه Leader هستیم، به سرور می‌گیم combat رو شروع کنه.
    if (m_isLeader) {
        QJsonObject msg;
        msg["type"] = "start_combat";
        msg["enemy_name"] = "JawWorm"; // فعلاً ثابت، بعداً بر اساس نوع اتاق عوض میشه
        NetworkManager::instance().send_game_action(msg);
    }
    // اگه Leader نیستیم، فقط منتظر پیام "combat_started" / "state_update" می‌مونیم
}

void CombatPage::buildUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QHBoxLayout *teammateRow = new QHBoxLayout();
    m_teammateNameLabel = new QLabel("Teammate");
    m_teammateHpBar = new QProgressBar();
    m_teammateHpBar->setFixedWidth(200);
    teammateRow->addWidget(m_teammateNameLabel);
    teammateRow->addWidget(m_teammateHpBar);
    teammateRow->addStretch();
    mainLayout->addLayout(teammateRow);

    QVBoxLayout *enemyBox = new QVBoxLayout();
    m_enemyNameLabel = new QLabel("Enemy");
    m_enemyNameLabel->setAlignment(Qt::AlignCenter);
    m_enemyHpBar = new QProgressBar();
    m_enemyHpBar->setFixedWidth(300);
    enemyBox->addWidget(m_enemyNameLabel, 0, Qt::AlignCenter);
    enemyBox->addWidget(m_enemyHpBar, 0, Qt::AlignCenter);
    mainLayout->addLayout(enemyBox);

    mainLayout->addStretch();

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

void CombatPage::sendPlayCard(const QString &cardName)
{
    QJsonObject msg;
    msg["type"] = "play_card";
    msg["card_name"] = cardName;
    NetworkManager::instance().send_game_action(msg);
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

    for (const QString &cardName : m_myHand) {
        QPushButton *cardBtn = new QPushButton(cardName);
        cardBtn->setFixedSize(100, 140);

        connect(cardBtn, &QPushButton::clicked, this, [this, cardName]() {
            sendPlayCard(cardName);
            // منتظر state_update از سرور می‌مونیم، خودمون UI رو دستی آپدیت نمی‌کنیم
        });

        layout->addWidget(cardBtn);
    }
}

void CombatPage::updateAllUI()
{
    m_myHpBar->setMaximum(m_myMaxHp);
    m_myHpBar->setValue(m_myHp);
    m_myEnergyLabel->setText(QString("Energy: %1/%2").arg(m_myEnergy).arg(m_myMaxEnergy));

    m_teammateHpBar->setMaximum(m_teammateMaxHp);
    m_teammateHpBar->setValue(m_teammateHp);

    m_enemyNameLabel->setText(m_enemyName);
    m_enemyHpBar->setMaximum(m_enemyMaxHp);
    m_enemyHpBar->setValue(m_enemyHp);

    updateMyHandUI();
}

void CombatPage::handleNetworkMessage(const QJsonObject &obj)
{
    QString type = obj["type"].toString();

    if (type == "combat_started") {
        // فعلاً کاری لازم نیست، منتظر state_update اول می‌مونیم
        return;
    }

    if (type == "state_update") {
        QJsonArray players = obj["players"].toArray();

        int myIndex = m_isLeader ? 0 : 1;
        int teammateIndex = m_isLeader ? 1 : 0;

        if (myIndex < players.size()) {
            QJsonObject me = players[myIndex].toObject();
            m_myHp = me["hp"].toInt();
            m_myMaxHp = me["max_hp"].toInt();
            m_myEnergy = me["energy"].toInt();
            m_myMaxEnergy = me["max_energy"].toInt();

            m_myHand.clear();
            for (const QJsonValue &v : me["hand"].toArray())
                m_myHand.append(v.toString());
        }

        if (teammateIndex < players.size()) {
            QJsonObject teammate = players[teammateIndex].toObject();
            m_teammateHp = teammate["hp"].toInt();
            m_teammateMaxHp = teammate["max_hp"].toInt();
        }

        QJsonArray enemies = obj["enemies"].toArray();
        if (!enemies.isEmpty()) {
            QJsonObject enemy = enemies[0].toObject();
            m_enemyName = enemy["name"].toString();
            m_enemyHp = enemy["hp"].toInt();
            m_enemyMaxHp = enemy["max_hp"].toInt();
        }

        updateAllUI();
        return;
    }
}

CombatPage::~CombatPage()
{
}