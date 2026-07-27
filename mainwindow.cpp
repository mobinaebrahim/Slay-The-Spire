#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>
#include <QPushButton>
#include <QDebug>
#include <QFile>
#include <QLabel>
#include <QLayout>
#include <QLayoutItem>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QEasingCurve>
#include <QTimer>
#include <QMouseEvent>

#include "card.h"
#include "AttackCard.h"
#include "CurseCard.h"
#include "SkillCard.h"
#include "PowerCard.h"
#include "StatusCard.h"
#include "Player.h"
#include "BattleManager.h"
#include "BossStruggles.h"
#include "character.h"
#include "NormalEnemies.h"
#include "EliteEnemies.h"
#include "enemy.h"
#include "cardfactory.h"
#include "usermanager.h"

MainWindow::MainWindow(QWidget *parent, int initialHp, int maxHp,
                       int initialGold, const std::vector<std::string>& deckNames,
                       CombatType combatType)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    std::srand(std::time(nullptr));
    battleManager = new BattleManager();

    QString playerName = user_manager::instance().get_current_username();
    if (playerName.isEmpty())
        playerName = "player";

    playerObject = new Player(playerName.toStdString(), maxHp, initialHp, 3, initialGold, battleManager);
    battleManager->setPlayer(playerObject);

    switch (combatType) {
    case CombatType::Normal:
        spawnNormalEncounter();
        break;
    case CombatType::Elite:
        spawnEliteEncounter();
        break;
    case CombatType::Boss:
        spawnBossEncounter();
        break;
    }

    if (!deckNames.empty()) {
        for (const auto& name : deckNames) {
            Card* c = createCardByName(name);
            if (c) playerObject->addCardToDrawPile(c);
        }
    } else {
        initializePlayerDeck(15);
    }

    setupShortcuts();
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

void MainWindow::mouseMoveEvent(QMouseEvent* event) {
    if (isDraggingCard && draggedCardIndex >= 0) {
        QPoint mousePos = event->pos();

        const std::vector<Card*>& hand = playerObject->getHand();
        bool isAttack = (draggedCardIndex < (int)hand.size() && hand[draggedCardIndex]
                         && hand[draggedCardIndex]->getType() == CardType::Attack);

        if (isAttack) {
            QLayout* layout = ui->CardsContainer->layout();
            QPushButton* originBtn = (draggedCardIndex < layout->count())
                                         ? qobject_cast<QPushButton*>(layout->itemAt(draggedCardIndex)->widget()): nullptr;

            if (originBtn) {
                QPoint cardTop = originBtn->mapTo(this, QPoint(originBtn->width() / 2, 0));
                dragHoverTarget = nullptr;
                for (auto& slot : enemySlots) {
                    QRect r = slot.sprite->geometry();
                    QPoint topLeft = slot.sprite->mapTo(this, QPoint(0,0));
                    QRect windowRect(topLeft, r.size());
                    if (windowRect.adjusted(-40, -40, 40, 40).contains(mousePos)) {
                        dragHoverTarget = slot.enemy;
                        break;
                    }
                }
                bool overEnemy = (dragHoverTarget != nullptr);
                updateDragArrow(cardTop, mousePos, overEnemy);
            }
        } else if (hoverCardLabel && hoverCardLabel->isVisible()) {
            int w = hoverCardLabel->width();
            int h = hoverCardLabel->height();
            hoverCardLabel->move(mousePos.x() - w/2, mousePos.y() - h/2 - 40);
            hoverCardLabel->raise();
        }
    }
    QMainWindow::mouseMoveEvent(event);
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);

    if (backgroundLabel) {
        backgroundLabel->resize(this->size());
        QPixmap original(":/assets/scene.png");
        backgroundLabel->setPixmap(original.scaled(this->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    }

    int playerW = 200, playerH = 300;
    currentStartX = 100;
    basePlayerY = (this->height() / 2) - (playerH / 2) - 90;
    playerSpriteLabel->setGeometry(currentStartX + 30, basePlayerY, playerW, playerH);

    int barWidth = 180, barHeight = 22;
    topHudBar->setGeometry(0, 0, this->width(), 48);
    playerHpBar->setGeometry(currentStartX + 30 + playerW / 2 - barWidth / 2 + 20, basePlayerY + 250, barWidth, barHeight);
    playerBlockBadge->setGeometry(currentStartX + 20 + playerW - 30, basePlayerY - 8, 30, 30);
    playerHitOverlay->setGeometry(currentStartX + 30, basePlayerY, playerW, playerH);

    int playerSpriteCenterY = basePlayerY + playerH / 2;
    int spriteCenterOffsetInWrapper = 20 + 4 + 26 + 4 + 75;

    int enemyAreaH = 320;
    int enemyContainerTop = playerSpriteCenterY - spriteCenterOffsetInWrapper;
    int enemyAreaX = currentStartX + playerW + 150;
    int enemyAreaW = this->width() - enemyAreaX - 60;
    enemyAreaContainer->setGeometry(enemyAreaX, enemyContainerTop, enemyAreaW, enemyAreaH);
    baseEnemyY = enemyContainerTop;

    int endTurnW = 230, endTurnH = 82;
    ui->EndTurnButton->setGeometry(this->width() - endTurnW + 30, this->height() - endTurnH - 100, endTurnW, endTurnH);

    if (exhaustPileOverlay->isVisible())
        exhaustPileOverlay->setGeometry(0, 0, this->width(), this->height());

    deckIconLabel->move(this->width() - 116, 3);
    mapLabel->move(this->width() - 172, 3);
    settingLabel->move(this->width() - 60, 3);

    if (settingsOverlayImage && settingsOverlayImage->isVisible()) {
        settingsOverlayImage->setGeometry(0, 0, this->width(), this->height());
        settingsOverlayImage->setPixmap(QPixmap(":/assets/settings_overlay.png").scaled(
            this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    playerStatusRow->setGeometry(currentStartX + 40, basePlayerY + 280 , playerW, 30);
}

void MainWindow::on_EndTurnButton_clicked()
{
    if (isGameOver)
        return;

    const auto& hand = playerObject->getHand();
    for (Card* card : hand) {
        if (card && card->getName() == "Burn") {
            int dmg = card->getIsUpgraded() ? 4 : 2;
            playerObject->decreaseHp(dmg);
        }
    }

    checkGameOver();
    if (isGameOver) {
        updateCharacterUI();
        return;
    }

    ui->EndTurnButton->setEnabled(false);
    isAttackAnimating = true;

    battleManager->beginEnemyTurnPhase();

    enemyTurnQueue = battleManager->getEnemies();
    enemyTurnQueueIndex = 0;

    processNextEnemyInQueue();
}

void MainWindow::updateHandUI() {
    hideHoverCard();

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

    for (int i = 0; i < (int)playerHand.size(); ++i) {
        Card* card = playerHand[i];
        if (!card)
            continue;

        QPushButton* btn = new QPushButton();
        btn->setFixedSize(140, 180);

        QString cardName = QString::fromStdString(card->getName());
        QString cardImagePath = ":/assets/cards/" + cardName + ".png";

        btn->setIcon(QIcon(cardImagePath));
        btn->setIconSize(btn->size());
        btn->setEnabled(card->isPlayable());
        btn->setProperty("cardImagePath", cardImagePath);
        btn->setProperty("cardName", cardName);
        btn->setProperty("cardIsAttack", card->getType() == CardType::Attack);
        btn->setProperty("cardIndex", i);
        btn->installEventFilter(this);

        int cardIndex = i;
        connect(btn, &QPushButton::clicked, [=]() {
            playCardAtIndex(cardIndex);
        });
        layout->addWidget(btn);

        QGraphicsOpacityEffect* fadeEffect = new QGraphicsOpacityEffect(btn);
        btn->setGraphicsEffect(fadeEffect);
        fadeEffect->setOpacity(0.0);

        QPropertyAnimation* fadeIn = new QPropertyAnimation(fadeEffect, "opacity", this);
        fadeIn->setDuration(800);
        fadeIn->setStartValue(0.0);
        fadeIn->setEndValue(1.0);
        fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void MainWindow::initializePlayerDeck(int totalCards) {
    for (int i = 0; i < 3; ++i) {
        Card* strikeCard = createCardByName("Strike");
        if (strikeCard)
            playerObject->addCardToDrawPile(strikeCard);
    }
    for (int i = 0; i < 2; ++i) {
        Card* defendCard = createCardByName("Defend");
        if (defendCard)
            playerObject->addCardToDrawPile(defendCard);
    }

    std::vector<std::string> allCardNames = {
                                              "Bash", "Blood for Blood", "Clash", "Feed", "Immolate", "PerfectedStrike", "Reaper",
                                              "Strike", "Bludgeon", "TwinStrike", "Whirlwind", "Barricade", "Bloodletting", "Brutality",
                                              "DualWield", "Defend", "DemonForm", "Disarm", "Entrench",
                                              "Exhume", "FeelNoPain", "Impervious", "Inflame", "LimitBreak", "Metallicize",
                                              "Offering", "ShrugItOff", "Daze", "Slime", "Wound", "Burn", "JAX", "CurseOfBell"};

    int remainingCards = totalCards - 5;

    for (int i = 0; i < remainingCards; ++i) {
        int randomIndex = std::rand() % allCardNames.size();
        std::string randomName = allCardNames[randomIndex];

        Card* newCard = createCardByName(randomName);
        if (newCard)
            playerObject->addCardToDrawPile(newCard);
    }
}

static QString intentToShortText(Enemy* enemy) {

    Hexaghost* hexa = dynamic_cast<Hexaghost*>(enemy);
    if (hexa && hexa->isDividerTurn())
        return "🔥 Divider";

    switch (enemy->getIntentType()) {
    case IntentType::Attack:   return "⚔ " + QString::number(enemy->getIntentValue());
    case IntentType::Defend:   return "🛡 Block";
    case IntentType::Buff:     return "⬆ Buff";
    case IntentType::Debuff:   return "⬇ Debuff";
    case IntentType::Combined: return "⚔🛡 " + QString::number(enemy->getIntentValue());
    case IntentType::Special:  return "❓ Special";
    case IntentType::AttackAddCard: return "⚔🎴 Attack & Add card";
    case IntentType::AttackDebuff: return "⚔⬇ Attack & Debuff";
    }
    return "?";
}

void MainWindow::updateCharacterUI() {
    if (!playerObject) return;

    playerHpBar->setMaximum(playerObject->getMaxHp());
    playerHpBar->setValue(playerObject->getHp());
    playerHpBar->setFormat(QString("%1 / %2").arg(playerObject->getHp()).arg(playerObject->getMaxHp()));
    updateStatusEffectRow(playerStatusRow, playerObject);

    int pBlock = playerObject->getBlock();
    playerBlockBadge->setVisible(pBlock > 0);
    if (pBlock > 0)
        playerBlockBadge->setText(QString::number(pBlock));

    energyOrbCountLabel->setText(QString("%1/%2").arg(playerObject->getEnergy()).arg(playerObject->getMaxEnergy()));
    goldCountLabel->setText(QString::number(playerObject->getGold()));
    playerHpTopLabel->setText(QString("%1/%2").arg(playerObject->getHp()).arg(playerObject->getMaxHp()));
    drawPileCountLabel->setText(QString::number(playerObject->getDrawPileSize()));
    discardPileCountLabel->setText(QString::number(playerObject->getDiscardPileSize()));
    deckCountLabel->setText(QString::number((int)playerObject->getFullDeck().size()));

    int exhaustCount = playerObject->getExhaustPileSize();
    exhaustPileBadge->setVisible(exhaustCount > 0);
    if (exhaustCount > 0) {
        exhaustPileBadge->setText(QString::number(exhaustCount));
        exhaustPileBadge->raise();
    }

    const auto& enemies = battleManager->getEnemies();
    bool needsRebuild = (enemies.size() != enemySlots.size());
    if (!needsRebuild) {
        for (size_t i = 0; i < enemies.size(); ++i) {
            if (enemySlots[i].enemy != enemies[i]) { needsRebuild = true; break; }
        }
    }
    if (needsRebuild)
        rebuildEnemyUI();

    for (auto& slot : enemySlots) {
        Enemy* enemy = slot.enemy;
        QString enemyName = QString::fromStdString(enemy->getName());

        slot.sprite->setPixmap(getEnemyPixmap(enemy).scaled(220, 195, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        slot.nameLabel->setText(enemyName);

        slot.hpBar->setMaximum(enemy->getMaxHp());
        slot.hpBar->setValue(enemy->getHp());
        slot.hpBar->setFormat(QString("%1 / %2").arg(enemy->getHp()).arg(enemy->getMaxHp()));

        updateStatusEffectRow(slot.statusRow, enemy);

        int eBlock = enemy->getBlock();
        slot.blockBadge->setVisible(eBlock > 0);
        if (eBlock > 0)
            slot.blockBadge->setText(QString::number(eBlock));

        slot.intentLabel->setText(intentToShortText(enemy));
    }
}

QPixmap MainWindow::getEnemyPixmap(Enemy* enemy) {
    QString enemyName = QString::fromStdString(enemy->getName());
    if (enemyName == "Louse") {
        Louse* louse = dynamic_cast<Louse*>(enemy);
        if (louse) {
            return QPixmap(louse->getColorChance() == 1
                               ? ":/assets/enemies/Green_Louse.png"
                               : ":/assets/enemies/Red_Louse.png");
        }
    }
    if (enemyName == "BlueSlaver") return QPixmap(":/assets/enemies/BlueSlaver.png");
    if (enemyName == "BookOfStabbing") return QPixmap(":/assets/enemies/BookOfStabbing.png");
    if (enemyName == "Cultist") return QPixmap(":/assets/enemies/Cultist.png");
    if (enemyName == "GremlinKnob") return QPixmap(":/assets/enemies/GremlinKnob.png");
    if (enemyName == "Hexaghost") return QPixmap(":/assets/enemies/Hexaghost.png");
    if (enemyName == "JawWorm") return QPixmap(":/assets/enemies/JawWorm.png");
    if (enemyName == "KingSlime") return QPixmap(":/assets/enemies/KingSlime.png");
    if (enemyName == "LargeSlime") return QPixmap(":/assets/enemies/LargeSlime.png");
    if (enemyName == "Looter") return QPixmap(":/assets/enemies/Looter.png");
    if (enemyName == "MediumSlime") return QPixmap(":/assets/enemies/MediumSlime.png");
    if (enemyName == "Mugger") return QPixmap(":/assets/enemies/Mugger.png");
    if (enemyName == "RedSlaver") return QPixmap(":/assets/enemies/RedSlaver.png");
    if (enemyName == "SmallSlime") return QPixmap(":/assets/enemies/SmallSlime.png");
    if (enemyName == "SphericGuardian") return QPixmap(":/assets/enemies/SphericGuardian.png");
    if (enemyName == "Taskmaster") return QPixmap(":/assets/enemies/Taskmaster.png");
    if (enemyName == "TheChamp") return QPixmap(":/assets/enemies/TheChamp.png");
    if (enemyName == "ThreeSentries") return QPixmap(":/assets/enemies/ThreeSentries.png");

    return QPixmap(":/assets/enemy_default.png");
}

void MainWindow::rebuildEnemyUI() {
    for (auto& slot : enemySlots) {
        slot.wrapper->hide();
        slot.wrapper->deleteLater();
    }
    enemySlots.clear();

    const auto& enemies = battleManager->getEnemies();

    for (Enemy* enemy : enemies) {
        EnemyUISlot slot;
        slot.enemy = enemy;

        slot.wrapper = new QWidget(enemyAreaContainer);
        slot.wrapper->setFixedWidth(220);
        QVBoxLayout* vbox = new QVBoxLayout(slot.wrapper);
        vbox->setContentsMargins(6, 0, 6, 0);
        vbox->setSpacing(4);

        slot.nameLabel = new QLabel(slot.wrapper);
        slot.nameLabel->setAlignment(Qt::AlignCenter);
        slot.nameLabel->setFixedHeight(20);
        slot.nameLabel->setFixedWidth(205);
        slot.nameLabel->setStyleSheet(
            "color: white; font-weight: bold; font-size: 13px; "
            "background-color: rgba(0,0,0,140); border-radius: 4px; padding: 2px;");

        slot.intentLabel = new QLabel(slot.wrapper);
        slot.intentLabel->setAlignment(Qt::AlignCenter);
        slot.intentLabel->setFixedHeight(26);
        slot.intentLabel->setFixedWidth(205);
        slot.intentLabel->setStyleSheet(
            "background-color: rgba(20,20,20,190); color: white; border-radius: 8px; "
            "padding: 3px; font-weight: bold;");

        slot.sprite = new QLabel(slot.wrapper);
        slot.sprite->setFixedSize(220, 195);
        slot.sprite->setAlignment(Qt::AlignCenter);
        slot.sprite->setScaledContents(true);
        slot.sprite->setPixmap(getEnemyPixmap(enemy).scaled(220, 195, Qt::KeepAspectRatio, Qt::SmoothTransformation));

        slot.blockBadge = new QLabel(slot.sprite);
        slot.blockBadge->setFixedSize(28, 28);
        slot.blockBadge->setAlignment(Qt::AlignCenter);
        slot.blockBadge->setStyleSheet(
            "background-color: #2b3a55; color: #9fd8ff; border: 2px solid #5c85b0; "
            "border-radius: 14px; font-weight: bold; font-size: 12px;");
        slot.blockBadge->move(slot.sprite->width() - 30, slot.sprite->height() - 30);
        slot.blockBadge->hide();

        slot.hpBar = new QProgressBar(slot.wrapper);
        slot.hpBar->setTextVisible(true);
        slot.hpBar->setFixedHeight(20);
        slot.hpBar->setFixedWidth(205);
        slot.hpBar->setStyleSheet(playerHpBar->styleSheet());

        slot.statusRow = new QWidget(slot.wrapper);
        QHBoxLayout* srLayout = new QHBoxLayout(slot.statusRow);
        srLayout->setContentsMargins(0, 0, 0, 0);
        srLayout->setSpacing(4);
        srLayout->setAlignment(Qt::AlignCenter);

        vbox->addWidget(slot.nameLabel);
        vbox->addWidget(slot.intentLabel);
        vbox->addWidget(slot.sprite, 0, Qt::AlignCenter);
        vbox->addWidget(slot.hpBar);
        vbox->addWidget(slot.statusRow, 0, Qt::AlignCenter);

        QVariant enemyPtr = QVariant::fromValue<void*>(static_cast<void*>(enemy));
        slot.sprite->setProperty("enemyPtr", enemyPtr);
        slot.hpBar->setProperty("enemyPtr", enemyPtr);
        slot.intentLabel->setProperty("enemyPtr", enemyPtr);

        slot.sprite->installEventFilter(this);
        slot.hpBar->installEventFilter(this);
        slot.intentLabel->installEventFilter(this);

        enemyAreaLayout->addWidget(slot.wrapper);
        slot.wrapper->show();
        enemySlots.push_back(slot);
    }

    if (enemySlots.empty()) {
        targetedEnemyIndex = -1;
    } else {
        targetedEnemyIndex = qBound(0, targetedEnemyIndex, (int)enemySlots.size() - 1);

        EnemyUISlot& slot = enemySlots[targetedEnemyIndex];
        slot.wrapper->setProperty("isTargeted", true);
        QGraphicsDropShadowEffect* glow = new QGraphicsDropShadowEffect(slot.sprite);
        glow->setBlurRadius(45);
        glow->setColor(QColor(245, 197, 24, 180));
        glow->setOffset(0, 0);
        slot.sprite->setGraphicsEffect(glow);
    }
}

Enemy* MainWindow::enemyFromWidget(QObject* obj) {
    QWidget* w = qobject_cast<QWidget*>(obj);
    if (!w) return nullptr;
    QVariant v = w->property("enemyPtr");
    if (!v.isValid()) return nullptr;
    return static_cast<Enemy*>(v.value<void*>());
}

void MainWindow::updateAnimations() {
    angle += 0.2f;
    int floatOffset = static_cast<int>(std::sin(angle) * 6);

    playerSpriteLabel->setGeometry(currentStartX + 30, basePlayerY + floatOffset, 200, 300);

    if (!isAttackAnimating) {
        QRect base = enemyAreaContainer->geometry();
        enemyAreaContainer->move(base.x(), baseEnemyY - floatOffset);
    }
}

void MainWindow::checkGameOver() {
    if (isGameOver) return;

    if (playerObject->getHp() <= 0) {
        isGameOver = true;
        ui->EndTurnButton->setEnabled(false);
        disableAllCards();
        enemyTurnQueue.clear();
        enemyTurnQueueIndex = 0;
        showGameOverText("DEFEAT", QColor("#c0392b"));

        QTimer::singleShot(2500, this, [this]() {
            std::vector<std::string> emptyDeck;
            emit combatFinished(false, 0, playerObject->getMaxHp(),
                                playerObject->getGold(), emptyDeck);
            close();
        });
        return;
    }

    if (battleManager->getEnemies().empty()) {
        isGameOver = true;
        ui->EndTurnButton->setEnabled(false);
        disableAllCards();
        enemyTurnQueue.clear();
        enemyTurnQueueIndex = 0;
        bool won = battleManager->getAnyEnemyDied();
        if (won)
            showGameOverText("VICTORY", QColor("#f5c518"));
        else
            showGameOverText("ESCAPED", QColor("#8a8a8a"));

        QTimer::singleShot(2500, this, [this, won]() {
            std::vector<std::string> finalDeck;
            for (Card* c : playerObject->getFullDeck()) {
                if (c) finalDeck.push_back(c->getName());
            }
            emit combatFinished(won, playerObject->getHp(), playerObject->getMaxHp(),
                                playerObject->getGold(), finalDeck);
            close();
        });
        return;
    }
}

void MainWindow::showGameOverText(const QString& text, const QColor& color) {
    gameOverLabel->setText(text);
    gameOverLabel->setStyleSheet(QString("color: %1; font-size: 72px; font-weight: 900; background: transparent;").arg(color.name()));

    int labelW = 600, labelH = 120;
    QRect endRect(
        (this->width() - labelW) / 2,
        (this->height() - labelH) / 2,
        labelW, labelH);
    QRect startRect(
        endRect.x() - 40, endRect.y() - 20,
        labelW + 80, labelH + 40);

    gameOverLabel->setGeometry(startRect);
    gameOverLabel->show();
    gameOverLabel->raise();
    gameOverOpacityEffect->setOpacity(0.0);

    QPropertyAnimation* fadeAnim = new QPropertyAnimation(gameOverOpacityEffect, "opacity", this);
    fadeAnim->setDuration(900);
    fadeAnim->setStartValue(0.0);
    fadeAnim->setEndValue(1.0);
    fadeAnim->setEasingCurve(QEasingCurve::OutCubic);

    QPropertyAnimation* geomAnim = new QPropertyAnimation(gameOverLabel, "geometry", this);
    geomAnim->setDuration(900);
    geomAnim->setStartValue(startRect);
    geomAnim->setEndValue(endRect);
    geomAnim->setEasingCurve(QEasingCurve::OutBack);

    fadeAnim->start(QAbstractAnimation::DeleteWhenStopped);
    geomAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::showEnemyTooltip(Enemy* enemy, QWidget* anchorWidget) {
    if (!enemy) { customTooltipBox->hide(); return; }

    Hexaghost* hexa = dynamic_cast<Hexaghost*>(enemy);
    if (hexa && hexa->isDividerTurn()) {
        customTooltipBox->setText(
            "<div style='font-weight:bold; font-size:14px; color:#f5c518; margin-bottom:6px;'>Divider</div>"
            "<div>This enemy intends to unleash the <b>Divider</b> attack.</div>");
        customTooltipBox->adjustSize();
        QWidget* anchor = anchorWidget ? anchorWidget : this;
        QPoint anchorPos = anchor->mapTo(this, QPoint(0, 0));
        customTooltipBox->move(anchorPos.x() - customTooltipBox->width() - 20, anchorPos.y() + 40);
        customTooltipBox->show();
        customTooltipBox->raise();
        return;
    }

    QString title, desc;
    switch (enemy->getIntentType()) {
    case IntentType::Attack:
        title = "Attack";
        desc = QString("This enemy intends to <b><span style='color:#ff6b6b;'>Attack</span></b> for <b>%1</b> damage.").arg(enemy->getIntentValue());
        break;
    case IntentType::Defend:
        title = "Defend";
        desc = "This enemy intends to <b><span style='color:#5ec8ff;'>Defend</span></b> itself.";
        break;
    case IntentType::Buff:
        title = "Buff";
        desc = "This enemy intends to <b><span style='color:#ffd76a;'>strengthen</span></b> itself.";
        break;
    case IntentType::Debuff:
        title = "Debuff";
        desc = "This enemy intends to <b><span style='color:#c07af0;'>weaken</span></b> you.";
        break;
    case IntentType::Combined:
        title = "Attack + Defend";
        desc = QString("This enemy intends to <b><span style='color:#ff6b6b;'>Attack</span></b> for <b>%1</b> damage "
                       "and <b><span style='color:#5ec8ff;'>Defend</span></b> itself.").arg(enemy->getIntentValue());
        break;

    case IntentType::Special:
        title = "Special";
        desc = "This enemy intends to do something special.";
        break;
    case IntentType::AttackAddCard:
        title = "Attack + Add a card";
        desc = QString("This enemy intends to <b><span style='color:#ff6b6b;'>Attack</span></b> for <b>%1</b> damage "
                       "and <b><span style='color:#f5c518;'>add a card to your discard pile</span></b>.").arg(enemy->getIntentValue());
        break;
    case IntentType::AttackDebuff:
        title = "Attack + Add a card";
        desc = QString("This enemy intends to <b><span style='color:#ff6b6b;'>Attack</span></b> for <b>%1</b> damage "
                       "and <b><span style='color:#c07af0;'>weaken</span></b> you.").arg(enemy->getIntentValue());
        break;
    }

    customTooltipBox->setText(QString("<div style='font-weight:bold; font-size:14px; color:#f5c518; margin-bottom:6px;'>%1</div>""<div>%2</div>").arg(title, desc));
    customTooltipBox->adjustSize();

    QWidget* anchor = anchorWidget ? anchorWidget : this;
    QPoint anchorPos = anchor->mapTo(this, QPoint(0, 0));
    int tipX = anchorPos.x() - customTooltipBox->width() - 20;
    int tipY = anchorPos.y() + 40;
    customTooltipBox->move(tipX, tipY);
    customTooltipBox->show();
    customTooltipBox->raise();
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event) {
    QPushButton* cardBtn = qobject_cast<QPushButton*>(obj);
    if (cardBtn && cardBtn->property("cardImagePath").isValid()) {
        if (event->type() == QEvent::Enter && !isDraggingCard && highlightedCardIndex < 0) {
            showHoverCard(cardBtn);
            return true;
        }
        if (event->type() == QEvent::Leave && !isDraggingCard) {
            if (highlightedCardIndex < 0)
                hideHoverCard();
            return true;
        }
        if (event->type() == QEvent::MouseButtonPress) {
            isDraggingCard = true;
            draggedCardIndex = cardBtn->property("cardIndex").toInt();

            cardBtn->setStyleSheet("border: 3px solid #f5c518; border-radius: 6px;");

            cardPlaySoundPlayer->stop();
            cardPlaySoundPlayer->setPosition(500);
            cardPlaySoundPlayer->play();

            bool isAttack = cardBtn->property("cardIsAttack").toBool();

            if (isAttack) {
                hoverCardLabel->hide();
            } else {
                QString imagePath = cardBtn->property("cardImagePath").toString();
                QPixmap fullResPixmap(imagePath);
                int bigW = int(cardBtn->width() * 1.4);
                int bigH = int(cardBtn->height() * 1.4);

                hoverCardLabel->setPixmap(fullResPixmap.scaled(bigW, bigH, Qt::KeepAspectRatio, Qt::SmoothTransformation));

                QMouseEvent* me = static_cast<QMouseEvent*>(event);
                QPoint mouseInWindow = cardBtn->mapTo(this, me->pos());
                hoverCardLabel->setGeometry(mouseInWindow.x() - bigW/2, mouseInWindow.y() - bigH/2 - 40, bigW, bigH);
                hoverCardLabel->show();
                hoverCardLabel->raise();

                showPlayerTargetFrame();
            }

            return true;
        }
        if (event->type() == QEvent::MouseButtonRelease && isDraggingCard) {
            isDraggingCard = false;
            dragArrowLabel->hide();
            hidePlayerTargetFrame();
            hoverCardLabel->hide();
            cardBtn->setStyleSheet("border: none;");

            bool isAttack = cardBtn->property("cardIsAttack").toBool();

            QMouseEvent* me = static_cast<QMouseEvent*>(event);
            QPoint globalRelease = cardBtn->mapToGlobal(me->pos());
            QPoint localRelease = this->mapFromGlobal(globalRelease);

            if (isAttack) {
                if (dragHoverTarget)
                    playCardAtIndex(draggedCardIndex, dragHoverTarget);
            } else {
                QRect playerRect = playerSpriteLabel->geometry().adjusted(-60, -60, 60, 60);
                if (playerRect.contains(localRelease))
                    playCardAtIndex(draggedCardIndex);
            }
            dragHoverTarget = nullptr;
            return true;
        }
    }

    if (obj == ui->EndTurnButton && event->type() == QEvent::Enter) {
        if (ui->EndTurnButton->isEnabled()) {
            endTurnHoverSoundPlayer->setPosition(500);
            endTurnHoverSoundPlayer->play();
        }
    }

    QLabel* badge = qobject_cast<QLabel*>(obj);
    if (badge && badge->property("effectName").isValid()) {
        if (event->type() == QEvent::Enter) {
            showStatusEffectTooltip(badge);
        } else if (event->type() == QEvent::Leave) {
            customTooltipBox->hide();
        }
        return QMainWindow::eventFilter(obj, event);
    }

    if (obj == exhaustPileBadge && event->type() == QEvent::MouseButtonRelease) {
        showCardPileOverlay("Exhaust Pile", playerObject->getExhaustPile(), "#c07af0");
        return true;
    }

    if (obj == discardWrapper && event->type() == QEvent::MouseButtonRelease) {
        showCardPileOverlay("Discard Pile", playerObject->getDiscardPile(), "#e0c060");
        return true;
    }

    if (obj == drawPileIconLabel && event->type() == QEvent::MouseButtonRelease) {
        showCardPileOverlay("Draw Pile", playerObject->getDrawPile(), "#7fd0ff");
        return true;
    }

    if (obj == deckIconLabel && event->type() == QEvent::MouseButtonRelease) {
        showCardPileOverlay("Deck", playerObject->getFullDeck(), "#66ccff");
        return true;
    }

    if (obj == settingLabel && event->type() == QEvent::MouseButtonRelease) {
        settingsOverlayImage->setGeometry(0, 0, this->width(), this->height());
        settingsOverlayImage->setPixmap(QPixmap(":/assets/icons/settings_overlay.png").scaled(
            this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        settingsOverlayImage->show();
        settingsOverlayImage->raise();
        return true;
    }

    Enemy* clickedEnemy = enemyFromWidget(obj);
    if (clickedEnemy && event->type() == QEvent::MouseButtonRelease) {
        for (auto& s : enemySlots) {
            if (s.sprite && s.sprite->graphicsEffect()) {
                s.sprite->graphicsEffect()->deleteLater();
                s.sprite->setGraphicsEffect(nullptr);
            }
            s.wrapper->setProperty("isTargeted", false);
        }

        for (size_t i = 0; i < enemySlots.size(); ++i) {
            if (enemySlots[i].enemy == clickedEnemy) {
                targetedEnemyIndex = (int)i;
                enemySlots[i].wrapper->setProperty("isTargeted", true);

                QGraphicsDropShadowEffect* glow = new QGraphicsDropShadowEffect(enemySlots[i].sprite);
                glow->setBlurRadius(45);
                glow->setColor(QColor(245, 197, 24, 180));
                glow->setOffset(0, 0);
                enemySlots[i].sprite->setGraphicsEffect(glow);
                break;
            }
        }
        return true;
    }

    if (clickedEnemy && (event->type() == QEvent::Enter || event->type() == QEvent::Leave)) {
        if (event->type() == QEvent::Enter)
            showEnemyTooltip(clickedEnemy, qobject_cast<QWidget*>(obj));
        else
            customTooltipBox->hide();
        return QMainWindow::eventFilter(obj, event);
    }

    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::playHitEffect(QLabel* overlay, QGraphicsOpacityEffect* opacityEffect) {
    overlay->show();
    overlay->raise();

    QPropertyAnimation* flashIn = new QPropertyAnimation(opacityEffect, "opacity", this);
    flashIn->setDuration(60);
    flashIn->setStartValue(0.0);
    flashIn->setEndValue(1.0);

    QPropertyAnimation* flashOut = new QPropertyAnimation(opacityEffect, "opacity", this);
    flashOut->setDuration(250);
    flashOut->setStartValue(1.0);
    flashOut->setEndValue(0.0);

    QSequentialAnimationGroup* seq = new QSequentialAnimationGroup(this);
    seq->addAnimation(flashIn);
    seq->addAnimation(flashOut);
    connect(seq, &QSequentialAnimationGroup::finished, overlay, &QLabel::hide);
    seq->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::showCardPileOverlay(const QString& title, const vector<Card*>& cards, const QString& titleColor) {
    pileOpenSoundPlayer->setPosition(500);
    pileOpenSoundPlayer->play();

    QGridLayout* gridLayout = qobject_cast<QGridLayout*>(exhaustCardsContainer->layout());

    QLayoutItem* child;
    while ((child = gridLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            child->widget()->hide();
            delete child->widget();
        }
        delete child;
    }

    pileOverlayTitle->setText(title);
    pileOverlayTitle->setStyleSheet(QString("color: %1; font-size: 24px; font-weight: bold; background: transparent;").arg(titleColor));

    const int columns = 5;
    int row = 0, col = 0;

    for (Card* card : cards) {
        if (!card) continue;

        QLabel* cardImgLabel = new QLabel();
        cardImgLabel->setFixedSize(120, 160);
        QString cardName = QString::fromStdString(card->getName());
        cardImgLabel->setPixmap(QPixmap(":/assets/cards/" + cardName + ".png")
                                    .scaled(120, 160, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        gridLayout->addWidget(cardImgLabel, row, col);

        col++;
        if (col >= columns) {
            col = 0;
            row++;
        }
    }

    exhaustPileOverlay->setGeometry(0, 0, this->width(), this->height());
    exhaustPileOverlay->show();
    exhaustPileOverlay->raise();
}

void MainWindow::hidePileOverlay() {
    exhaustPileOverlay->hide();
}

static QString effectColor(const QString& name) {
    if (name == "Strength")    return "#e07b39";
    if (name == "Dexterity")   return "#4ea8de";
    if (name == "Vulnerable")  return "#e35d5d";
    if (name == "Weak")        return "#b98fda";
    if (name == "Frail")       return "#c9a24a";
    if (name == "Metallicize") return "#7d8fa3";
    if (name == "Entangled")   return "#6fae6f";
    if (name == "DemonForm")   return "#c0392b";
    if (name == "Brutality")   return "#8b0000";
    return "#666666";
}

void MainWindow::updateStatusEffectRow(QWidget* rowWidget, Character* character) {
    QLayout* layout = rowWidget->layout();

    QLayoutItem* child;
    while ((child = layout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            child->widget()->hide();
            delete child->widget();
        }
        delete child;
    }

    if (!character) return;

    for (auto* effect : character->getEffects()) {
        QString name = QString::fromStdString(effect->getName());
        int amount = effect->getAmount();

        QLabel* badge = new QLabel(rowWidget);
        badge->setFixedSize(60, 26);
        badge->setAlignment(Qt::AlignCenter);
        badge->setText(QString("%1 %2").arg(name).arg(amount));
        badge->setStyleSheet(QString("background-color: %1; color: white; border-radius: 6px; "
                                     "font-weight: bold; font-size: 10px;").arg(effectColor(name)));

        badge->setProperty("effectName", name);
        badge->setProperty("effectAmount", amount);
        badge->installEventFilter(this);

        layout->addWidget(badge);
    }
}

static QString effectDescription(const QString& name, int amount) {
    if (name == "Strength")    return QString("Increases damage dealt by Attack cards by %1.").arg(amount);
    if (name == "Dexterity")   return QString("Increases Block gained by %1.").arg(amount);
    if (name == "Vulnerable")  return "Take 50% more damage from attacks while this is active.";
    if (name == "Weak")        return "Deal 50% less damage while this is active.";
    if (name == "Frail")       return "Gain 25% less Block from cards while this is active.";
    if (name == "Metallicize") return QString("Gain %1 Block at the end of your turn.").arg(amount);
    if (name == "Entangled")   return "Cannot play Attack cards this turn.";
    if (name == "DemonForm")   return QString("Gain %1 Strength at the start of each turn.").arg(amount);
    if (name == "Brutality")   return "Lose 1 HP and draw 1 card at the start of each turn.";
    return "";
}

void MainWindow::showStatusEffectTooltip(QLabel* badge) {
    QString name = badge->property("effectName").toString();
    int amount = badge->property("effectAmount").toInt();
    QString desc = effectDescription(name, amount);

    customTooltipBox->setText(QString("<div style='font-weight:bold; font-size:14px; color:#f5c518; margin-bottom:6px;'>%1 (%2)</div>"
                                      "<div>%3</div>").arg(name).arg(amount).arg(desc));

    customTooltipBox->adjustSize();

    QPoint globalPos = badge->mapTo(this, QPoint(0, 0));
    customTooltipBox->move(globalPos.x(), globalPos.y() + badge->height() + 6);
    customTooltipBox->show();
    customTooltipBox->raise();
}

void MainWindow::disableAllCards() {
    QLayout* layout = ui->CardsContainer->layout();
    if (!layout) return;

    for (int i = 0; i < layout->count(); ++i) {
        QWidget* w = layout->itemAt(i)->widget();
        if (w)
            w->setEnabled(false);
    }
}

void MainWindow::showHoverCard(QPushButton* originalBtn) {
    if (!originalBtn) return;

    QString imagePath = originalBtn->property("cardImagePath").toString();
    QPixmap fullResPixmap(imagePath);
    if (fullResPixmap.isNull()) return;

    QPoint globalPos = originalBtn->mapTo(this, QPoint(0, 0));
    QRect smallRect(globalPos.x(), globalPos.y(), originalBtn->width(), originalBtn->height());
    hoverOriginalRect = smallRect;

    double scale = 1.55;
    int bigW = int(originalBtn->width() * scale);
    int bigH = int(originalBtn->height() * scale);
    int bigX = globalPos.x() - (bigW - originalBtn->width()) / 2;
    int bigY = globalPos.y() - (bigH - originalBtn->height()) / 2 - 50;
    QRect bigRect(bigX, bigY, bigW, bigH);

    hoverCardLabel->setPixmap(fullResPixmap.scaled(bigW, bigH, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    if (!hoverCardLabel->isVisible()) {
        hoverCardLabel->setGeometry(smallRect);
        hoverCardLabel->show();
    }
    hoverCardLabel->raise();

    if (hoverGeomAnim) {
        hoverGeomAnim->stop();
        delete hoverGeomAnim;
        hoverGeomAnim = nullptr;
    }

    hoverGeomAnim = new QPropertyAnimation(hoverCardLabel, "geometry", this);
    hoverGeomAnim->setDuration(180);
    hoverGeomAnim->setStartValue(hoverCardLabel->geometry());
    hoverGeomAnim->setEndValue(bigRect);
    hoverGeomAnim->setEasingCurve(QEasingCurve::OutCubic);
    hoverGeomAnim->start();
}

void MainWindow::hideHoverCard() {
    if (!hoverCardLabel || !hoverCardLabel->isVisible()) return;

    if (hoverGeomAnim) {
        hoverGeomAnim->stop();
        delete hoverGeomAnim;
        hoverGeomAnim = nullptr;
    }

    hoverGeomAnim = new QPropertyAnimation(hoverCardLabel, "geometry", this);
    hoverGeomAnim->setDuration(130);
    hoverGeomAnim->setStartValue(hoverCardLabel->geometry());
    hoverGeomAnim->setEndValue(hoverOriginalRect);
    hoverGeomAnim->setEasingCurve(QEasingCurve::InCubic);

    QLabel* labelPtr = hoverCardLabel;
    connect(hoverGeomAnim, &QPropertyAnimation::finished, labelPtr, &QLabel::hide);
    hoverGeomAnim->start();
}

void MainWindow::showFloatingDamage(QRect targetRect, int amount, const QColor& color) {
    if (amount <= 0) return;

    QLabel* dmgLabel = new QLabel(this);
    dmgLabel->setText(QString("-%1").arg(amount));
    dmgLabel->setStyleSheet(QString("color: %1; font-size: 30px; font-weight: 900; background: transparent;").arg(color.name()));
    dmgLabel->adjustSize();

    int startX = targetRect.x() + targetRect.width() / 2 - dmgLabel->width() / 2;
    int startY = targetRect.y() + 20;
    dmgLabel->move(startX, startY);
    dmgLabel->show();
    dmgLabel->raise();

    QGraphicsOpacityEffect* opacity = new QGraphicsOpacityEffect(dmgLabel);
    dmgLabel->setGraphicsEffect(opacity);
    opacity->setOpacity(1.0);

    QPropertyAnimation* moveAnim = new QPropertyAnimation(dmgLabel, "geometry", this);
    moveAnim->setDuration(750);
    moveAnim->setStartValue(dmgLabel->geometry());
    moveAnim->setEndValue(QRect(startX, startY - 60, dmgLabel->width(), dmgLabel->height()));
    moveAnim->setEasingCurve(QEasingCurve::OutCubic);

    QPropertyAnimation* fadeAnim = new QPropertyAnimation(opacity, "opacity", this);
    fadeAnim->setDuration(750);
    fadeAnim->setStartValue(1.0);
    fadeAnim->setEndValue(0.0);

    QParallelAnimationGroup* group = new QParallelAnimationGroup(this);
    group->addAnimation(moveAnim);
    group->addAnimation(fadeAnim);
    connect(group, &QParallelAnimationGroup::finished, dmgLabel, &QLabel::deleteLater);
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::showToastMessage(const QString& text) {
    toastLabel->setText(text);
    toastLabel->adjustSize();
    toastLabel->move(this->width()/2 - toastLabel->width()/2, 120);
    toastLabel->show();
    toastLabel->raise();
    QTimer::singleShot(1500, toastLabel, &QLabel::hide);
}

void MainWindow::playCardAtIndex(int index, Enemy* explicitTarget) {
    if (isGameOver || isAttackAnimating) return;

    const std::vector<Card*>& playerHand = playerObject->getHand();
    if (index < 0 || index >= (int)playerHand.size()) return;

    Card* card = playerHand[index];
    if (!card) return;

    const std::vector<Enemy*>& allEnemies = battleManager->getEnemies();
    if (allEnemies.empty()) return;

    string reason = card->getUnplayableReason(playerObject);
    if (!reason.empty()) {
        showToastMessage(QString::fromStdString(reason));
        return;
    }

    Enemy* targetEnemy = explicitTarget;
    if (!targetEnemy) {
        int idx = qBound(0, targetedEnemyIndex, (int)allEnemies.size() - 1);
        targetEnemy = allEnemies[idx];
    }

    bool isAttackCard = (card->getType() == CardType::Attack);
    int handSizeBefore = playerObject->getHandSize();
    int enemyHpBefore = targetEnemy->getHp();

    if (card->getCost(playerObject) > playerObject->getEnergy()) {
        showNotEnoughEnergy();
        return;
    }

    battleManager->playCardAction(card, targetEnemy);

    bool cardWasActuallyPlayed = (playerObject->getHandSize() < handSizeBefore);

    bool enemyStillAlive = false;
    for (Enemy* e : battleManager->getEnemies())
        if (e == targetEnemy) { enemyStillAlive = true; break; }

    int damageDealt = enemyStillAlive ? (enemyHpBefore - targetEnemy->getHp()) : enemyHpBefore;

    battleManager->cleanupDeadEnemies();
    checkGameOver();

    QRect enemyRectForFx = enemySpriteLabelRectFor(targetEnemy);

    if (isAttackCard && cardWasActuallyPlayed && !isGameOver) {
        QTimer::singleShot(150, this, [=]() {
            if (damageDealt > 0)
                showFloatingDamage(enemyRectForFx, damageDealt, QColor("#ff4d4d"));
            hitSoundPlayer->stop();
            hitSoundPlayer->setPosition(0);
            hitSoundPlayer->play();

            enemyHitOverlay->setGeometry(enemyRectForFx);
            playHitEffect(enemyHitOverlay, enemyHitOpacity);
        });
    } else if (damageDealt > 0) {
        showFloatingDamage(enemyRectForFx, damageDealt, QColor("#ff4d4d"));
    }

    highlightedCardIndex = -1;

    QTimer::singleShot(0, this, [=]() {
        updateHandUI();
        updateCharacterUI();
    });
}

void MainWindow::setupShortcuts() {
    QShortcut* endTurnKey = new QShortcut(QKeySequence(Qt::Key_E), this);
    connect(endTurnKey, &QShortcut::activated, this, [=]() {
        if (ui->EndTurnButton->isEnabled())
            on_EndTurnButton_clicked();
    });

    QShortcut* drawPileKey = new QShortcut(QKeySequence(Qt::Key_A), this);
    connect(drawPileKey, &QShortcut::activated, this, [=]() {
        showCardPileOverlay("Draw Pile", playerObject->getDrawPile(), "#7fd0ff");
    });

    QShortcut* discardPileKey = new QShortcut(QKeySequence(Qt::Key_S), this);
    connect(discardPileKey, &QShortcut::activated, this, [=]() {
        showCardPileOverlay("Discard Pile", playerObject->getDiscardPile(), "#e0c060");
    });

    QShortcut* exhaustPileKey = new QShortcut(QKeySequence(Qt::Key_X), this);
    connect(exhaustPileKey, &QShortcut::activated, this, [=]() {
        showCardPileOverlay("Exhaust Pile", playerObject->getExhaustPile(), "#c07af0");
    });

    QShortcut* deckKey = new QShortcut(QKeySequence(Qt::Key_D), this);
    connect(deckKey, &QShortcut::activated, this, [=]() {
        showCardPileOverlay("Deck", playerObject->getFullDeck(), "#66ccff");
    });

    QShortcut* escKey = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(escKey, &QShortcut::activated, this, [=]() {
        highlightedCardIndex = -1;
        updateCardHighlight();
        hidePileOverlay();
        if (settingsOverlayImage && settingsOverlayImage->isVisible())
            settingsOverlayImage->hide();
    });

    for (int i = 1; i <= 9; ++i) {
        QShortcut* numKey = new QShortcut(QKeySequence(QString::number(i)), this);
        int handIndex = i - 1;
        connect(numKey, &QShortcut::activated, this, [=]() {
            if (handIndex < playerObject->getHandSize()) {
                highlightedCardIndex = handIndex;
                updateCardHighlight();
            }
        });
    }
    QShortcut* zeroKey = new QShortcut(QKeySequence(Qt::Key_0), this);
    connect(zeroKey, &QShortcut::activated, this, [=]() {
        if (9 < playerObject->getHandSize()) {
            highlightedCardIndex = 9;
            updateCardHighlight();
        }
    });

    QShortcut* leftKey = new QShortcut(QKeySequence(Qt::Key_Left), this);
    connect(leftKey, &QShortcut::activated, this, [=]() {
        int handSize = playerObject->getHandSize();
        if (handSize == 0) return;
        highlightedCardIndex = (highlightedCardIndex <= 0) ? handSize - 1 : highlightedCardIndex - 1;
        updateCardHighlight();
    });

    QShortcut* rightKey = new QShortcut(QKeySequence(Qt::Key_Right), this);
    connect(rightKey, &QShortcut::activated, this, [=]() {
        int handSize = playerObject->getHandSize();
        if (handSize == 0) return;
        highlightedCardIndex = (highlightedCardIndex + 1) % handSize;
        updateCardHighlight();
    });

    QShortcut* enterKey = new QShortcut(QKeySequence(Qt::Key_Return), this);
    connect(enterKey, &QShortcut::activated, this, [=]() {
        if (highlightedCardIndex >= 0)
            playCardAtIndex(highlightedCardIndex);
    });

    QShortcut* peekKey = new QShortcut(QKeySequence(Qt::Key_Space), this);
    connect(peekKey, &QShortcut::activated, this, [=]() {
        QLayout* layout = ui->CardsContainer->layout();
        if (highlightedCardIndex >= 0 && highlightedCardIndex < layout->count()) {
            QPushButton* btn = qobject_cast<QPushButton*>(layout->itemAt(highlightedCardIndex)->widget());
            if (btn) showHoverCard(btn);
        }
    });
}

void MainWindow::updateCardHighlight() {
    QLayout* layout = ui->CardsContainer->layout();
    QPushButton* highlightedBtn = nullptr;

    for (int i = 0; i < layout->count(); ++i) {
        QPushButton* btn = qobject_cast<QPushButton*>(layout->itemAt(i)->widget());
        if (!btn) continue;
        if (i == highlightedCardIndex) {
            btn->setStyleSheet("border: 3px solid #f5c518; border-radius: 6px;");
            highlightedBtn = btn;
        } else
            btn->setStyleSheet("border: none;");
    }

    if (highlightedBtn)
        showHoverCard(highlightedBtn);
    else
        hideHoverCard();
}

void MainWindow::updateDragArrow(QPoint fromPoint, QPoint toPoint, bool isOverEnemy) {
    int padding = 40;
    QRect arrowRect(
        qMin(fromPoint.x(), toPoint.x()) - padding,
        qMin(fromPoint.y(), toPoint.y()) - padding,
        qAbs(toPoint.x() - fromPoint.x()) + padding * 2,
        qAbs(fromPoint.y() - toPoint.y()) + padding * 2
        );

    if (arrowRect.width() < 20 || arrowRect.height() < 20) {
        dragArrowLabel->hide();
        return;
    }

    QPixmap pixmap(arrowRect.size());
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    QPointF start = fromPoint - arrowRect.topLeft();
    QPointF end = toPoint - arrowRect.topLeft();

    QPointF mid = (start + end) / 2.0;
    double dx = end.x() - start.x();
    double dy = end.y() - start.y();
    double length = std::sqrt(dx * dx + dy * dy);
    if (length < 1) length = 1;

    double curveHeight = qMin(length * 0.2, 60.0);
    QPointF normal(-dy / length, dx / length);
    QPointF control = mid - normal * curveHeight;

    QPainterPath path;
    path.moveTo(start);
    path.quadTo(control, end);

    QColor mainColor = isOverEnemy ? QColor(255, 40, 40, 255) : QColor(230, 230, 230, 230);
    QColor lightColor = isOverEnemy ? QColor(255, 140, 140, 220) : QColor(255, 255, 255, 200);
    QColor glowColor = isOverEnemy ? QColor(255, 80, 80, 90) : QColor(200, 200, 200, 70);

    QPen glowPen(glowColor, 16, Qt::SolidLine, Qt::RoundCap);
    painter.strokePath(path, glowPen);

    QLinearGradient grad(start, end);
    grad.setColorAt(0, lightColor);
    grad.setColorAt(1, mainColor);
    QPen pen(QBrush(grad), 5, Qt::SolidLine, Qt::RoundCap);
    painter.strokePath(path, pen);

    double tangentAngle = std::atan2(end.y() - control.y(), end.x() - control.x());
    int arrowSize = 18;
    QPointF arrowP1(
        end.x() - arrowSize * std::cos(tangentAngle - M_PI / 6),
        end.y() - arrowSize * std::sin(tangentAngle - M_PI / 6)
        );
    QPointF arrowP2(
        end.x() - arrowSize * std::cos(tangentAngle + M_PI / 6),
        end.y() - arrowSize * std::sin(tangentAngle + M_PI / 6)
        );
    QPolygonF arrowHead;
    arrowHead << end << arrowP1 << arrowP2;
    painter.setBrush(mainColor);
    painter.setPen(Qt::NoPen);
    painter.drawPolygon(arrowHead);

    dragArrowLabel->setGeometry(arrowRect);
    dragArrowLabel->setPixmap(pixmap);
    dragArrowLabel->show();
    dragArrowLabel->raise();
}

void MainWindow::showPlayerTargetFrame() {
    QRect frameRect = playerSpriteLabel->geometry().adjusted(-15, -15, 15, 15);

    QPixmap pixmap(frameRect.size());
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    QPen pen(QColor(245, 197, 24, 255), 5, Qt::SolidLine, Qt::RoundCap);
    painter.setPen(pen);

    int w = pixmap.width(), h = pixmap.height();
    int cornerLen = 28;

    painter.drawLine(4, 4, 4 + cornerLen, 4);
    painter.drawLine(4, 4, 4, 4 + cornerLen);

    painter.drawLine(w - 4, 4, w - 4 - cornerLen, 4);
    painter.drawLine(w - 4, 4, w - 4, 4 + cornerLen);

    painter.drawLine(4, h - 4, 4 + cornerLen, h - 4);
    painter.drawLine(4, h - 4, 4, h - 4 - cornerLen);

    painter.drawLine(w - 4, h - 4, w - 4 - cornerLen, h - 4);
    painter.drawLine(w - 4, h - 4, w - 4, h - 4 - cornerLen);

    playerTargetFrame->setGeometry(frameRect);
    playerTargetFrame->setPixmap(pixmap);
    playerTargetFrame->show();
    playerTargetFrame->raise();
}

void MainWindow::hidePlayerTargetFrame() {
    playerTargetFrame->hide();
}

void MainWindow::showNotEnoughEnergy()
{
    QLabel* bubble = new QLabel(this);

    QPixmap pix(":/assets/not_enough_energy.png");

    qDebug() << pix.isNull();

    bubble->setPixmap(pix.scaled(260, 160, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    bubble->adjustSize();

    QRect playerRect = playerSpriteLabel->geometry();

    int startX = playerRect.center().x() - bubble->width()/2 + 150;
    int startY = playerRect.top() - bubble->height() + 70;

    bubble->move(startX, startY);
    bubble->show();
    bubble->raise();

    auto* effect = new QGraphicsOpacityEffect(bubble);
    bubble->setGraphicsEffect(effect);

    auto* fade = new QPropertyAnimation(effect, "opacity");
    fade->setDuration(2100);
    fade->setStartValue(1.0);
    fade->setEndValue(0.0);

    auto* move = new QPropertyAnimation(bubble, "pos");
    move->setDuration(2100);
    move->setStartValue(QPoint(startX, startY));
    move->setEndValue(QPoint(startX, startY - 45));
    move->setEasingCurve(QEasingCurve::OutQuad);

    auto* group = new QParallelAnimationGroup(this);

    group->addAnimation(move);
    group->addAnimation(fade);

    connect(group, &QParallelAnimationGroup::finished, bubble, &QLabel::deleteLater);

    group->start(QAbstractAnimation::DeleteWhenStopped);
}

MainWindow::EnemyUISlot* MainWindow::findSlotFor(Enemy* enemy) {
    for (auto& slot : enemySlots)
        if (slot.enemy == enemy) return &slot;
    return nullptr;
}

QRect MainWindow::enemySpriteLabelRectFor(Enemy* enemy) {
    EnemyUISlot* slot = findSlotFor(enemy);
    if (!slot || !slot->sprite) return QRect(0,0,0,0);
    QPoint topLeft = slot->sprite->mapTo(this, QPoint(0,0));
    return QRect(topLeft, slot->sprite->size());
}

void MainWindow::highlightAttackingEnemy(Enemy* enemy) {
    EnemyUISlot* slot = findSlotFor(enemy);
    if (slot && slot->sprite) {
        if (slot->sprite->graphicsEffect()) {
            slot->sprite->graphicsEffect()->deleteLater();
        }
        QGraphicsDropShadowEffect* glow = new QGraphicsDropShadowEffect(slot->sprite);
        glow->setBlurRadius(55);
        glow->setColor(QColor(255, 60, 40, 200));
        glow->setOffset(0, 0);
        slot->sprite->setGraphicsEffect(glow);
    }
}

void MainWindow::unhighlightAttackingEnemy(Enemy* enemy) {
    if (!enemy) return;
    if (isGameOver) return;

    EnemyUISlot* slot = findSlotFor(enemy);
    if (!slot || !slot->sprite) return;

    if (slot->sprite->graphicsEffect()) {
        slot->sprite->graphicsEffect()->deleteLater();
        slot->sprite->setGraphicsEffect(nullptr);
    }

    bool isTargeted = slot->wrapper->property("isTargeted").toBool();
    if (isTargeted) {
        QGraphicsDropShadowEffect* glow = new QGraphicsDropShadowEffect(slot->sprite);
        glow->setBlurRadius(45);
        glow->setColor(QColor(245, 197, 24, 180));
        glow->setOffset(0, 0);
        slot->sprite->setGraphicsEffect(glow);
    }
}

void MainWindow::processNextEnemyInQueue() {
    if (isGameOver) {
        isAttackAnimating = false;
        return;
    }
    while (enemyTurnQueueIndex < (int)enemyTurnQueue.size()) {
        Enemy* enemy = enemyTurnQueue[enemyTurnQueueIndex];
        bool stillInBattle = false;
        for (Enemy* e : battleManager->getEnemies()) {
            if (e == enemy) { stillInBattle = true; break; }
        }
        if (!stillInBattle || enemy->getHp() <= 0) {
            enemyTurnQueueIndex++;
        } else {
            break;
        }
    }
    if (enemyTurnQueueIndex >= (int)enemyTurnQueue.size()) {
        battleManager->endEnemyTurnPhase();
        battleManager->cleanupDeadEnemies();

        isAttackAnimating = false;
        checkGameOver();
        if (!isGameOver) {
            ui->EndTurnButton->setEnabled(true);
            updateHandUI();
            updateCharacterUI();
        }
        return;
    }

    Enemy* currentEnemy = enemyTurnQueue[enemyTurnQueueIndex];
    EnemyUISlot* slot = findSlotFor(currentEnemy);

    if (!slot) {
        rebuildEnemyUI();
        slot = findSlotFor(currentEnemy);
        if (!slot) {
            enemyTurnQueueIndex++;
            QTimer::singleShot(50, this, &MainWindow::processNextEnemyInQueue);
            return;
        }
    }

    highlightAttackingEnemy(currentEnemy);

    IntentType intent = currentEnemy->getIntentType();
    bool isAttackingIntent = (intent == IntentType::Attack || intent == IntentType::Combined ||intent == IntentType::AttackAddCard || intent == IntentType::AttackDebuff);

    if (isAttackingIntent && slot && slot->sprite) {
        int lungeDistance = 20;
        QRect startRect = slot->sprite->geometry();
        QRect lungeRect(startRect.x() - lungeDistance, startRect.y(), startRect.width(), startRect.height());

        QPropertyAnimation* forward = new QPropertyAnimation(slot->sprite, "geometry", this);
        forward->setDuration(250);
        forward->setStartValue(startRect);
        forward->setEndValue(lungeRect);
        forward->setEasingCurve(QEasingCurve::OutQuad);

        connect(forward, &QPropertyAnimation::finished, this, [=]() {
            int playerHpBefore = playerObject->getHp();

            battleManager->processSingleEnemyTurn(currentEnemy);
            battleManager->cleanupDeadEnemies();

            int damageDealt = playerHpBefore - playerObject->getHp();
            if (damageDealt > 0)
                showFloatingDamage(playerSpriteLabel->geometry(), damageDealt, QColor("#ff4d4d"));

            hitSoundPlayer->setPosition(0);
            hitSoundPlayer->play();
            playHitEffect(playerHitOverlay, playerHitOpacity);
            updateCharacterUI();

            EnemyUISlot* freshSlot = findSlotFor(currentEnemy);

            auto proceedToNext = [=]() {
                if (!isGameOver)
                    unhighlightAttackingEnemy(currentEnemy);
                enemyTurnQueueIndex++;
                checkGameOver();
                if (isGameOver) {
                    isAttackAnimating = false;
                    disableAllCards();
                    return;
                }
                processNextEnemyInQueue();
            };

            if (!freshSlot || !freshSlot->sprite) {
                QTimer::singleShot(1500, this, proceedToNext);
                return;
            }

            QPropertyAnimation* backward = new QPropertyAnimation(freshSlot->sprite, "geometry", this);
            backward->setDuration(300);
            backward->setStartValue(freshSlot->sprite->geometry());
            backward->setEndValue(startRect);
            backward->setEasingCurve(QEasingCurve::InQuad);

            connect(backward, &QPropertyAnimation::finished, this, [=]() {
                QTimer::singleShot(1500, this, proceedToNext);
            });
            backward->start(QAbstractAnimation::DeleteWhenStopped);
        });
        forward->start(QAbstractAnimation::DeleteWhenStopped);
    }
    else {
        battleManager->processSingleEnemyTurn(currentEnemy);
        battleManager->cleanupDeadEnemies();
        updateCharacterUI();

        QTimer::singleShot(600, this, [=]() {
            if (!isGameOver) {
                unhighlightAttackingEnemy(currentEnemy);
            }
            enemyTurnQueueIndex++;
            processNextEnemyInQueue();
        });
    }
}