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
    this->setMouseTracking(true);

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

    settingLabel = new QLabel(this);
    settingLabel->setFixedSize(46, 46);
    settingLabel->setPixmap(QPixmap(":/images/icons/settings.png").scaled(46, 46, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    settingLabel->move(this->width() - 60, 3);
    settingLabel->installEventFilter(this);

    settingsOverlayImage = new QLabel(this);
    settingsOverlayImage->setAlignment(Qt::AlignCenter);
    settingsOverlayImage->setStyleSheet("background-color: rgba(0,0,0,220);");
    settingsOverlayImage->setPixmap(QPixmap(":/images/icons/settings_overlay.png").scaled(
        this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    settingsOverlayImage->setGeometry(0, 0, this->width(), this->height());
    settingsOverlayImage->hide();
    settingsOverlayImage->setAttribute(Qt::WA_TransparentForMouseEvents, false);

    mapLabel = new QLabel(this);
    mapLabel->setFixedSize(46, 46);
    mapLabel->setPixmap(QPixmap(":/images/icons/map.png").scaled(46, 46, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    mapLabel->move(this->width() - 172, 3);
    mapLabel->installEventFilter(this);

    deckIconLabel = new QLabel(this);
    deckIconLabel->setFixedSize(46, 46);
    deckIconLabel->setPixmap(QPixmap(":/images/icons/deck.png").scaled(46, 46, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    deckIconLabel->move(this->width() - 116, 3);
    deckIconLabel->installEventFilter(this);

    deckCountLabel = new QLabel(deckIconLabel);
    deckCountLabel->setGeometry(14, 30, 32, 16);
    deckCountLabel->setAlignment(Qt::AlignCenter);
    deckCountLabel->setStyleSheet("color: white; font-weight: bold; font-size: 14px; background: transparent;");

    drawPileIconLabel = new QLabel(this);
    drawPileIconLabel->setFixedSize(84, 84);
    drawPileIconLabel->setPixmap(QPixmap(":/images/icons/draw_pile.png").scaled(84, 84, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    drawPileIconLabel->installEventFilter(this);

    drawPileCountLabel = new QLabel(drawPileIconLabel);
    drawPileCountLabel->setGeometry(12, 28, 28, 16);
    drawPileCountLabel->setAlignment(Qt::AlignCenter);
    drawPileCountLabel->setStyleSheet(
        "background-color: rgba(0,0,0,170); color: white; font-weight: bold; "
        "font-size: 11px; border-radius: 4px;");

    discardWrapper = new QWidget(this);
    discardWrapper->setFixedSize(90, 90);

    discardPileIconLabel = new QLabel(discardWrapper);
    discardPileIconLabel->setGeometry(16, 16, 74, 74);
    discardPileIconLabel->setPixmap(QPixmap(":/images/icons/discard_pile.png").scaled(74, 74, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    discardPileCountLabel = new QLabel(discardPileIconLabel);
    discardPileCountLabel->setGeometry(12, 28, 28, 16);
    discardPileCountLabel->setAlignment(Qt::AlignCenter);
    discardPileCountLabel->setStyleSheet(
        "background-color: rgba(0,0,0,170); color: white; font-weight: bold; "
        "font-size: 11px; border-radius: 4px;");

    exhaustPileBadge = new QLabel(discardWrapper);
    exhaustPileBadge->setFixedSize(26, 26);
    exhaustPileBadge->move(0, 0);
    exhaustPileBadge->setAlignment(Qt::AlignCenter);
    exhaustPileBadge->setStyleSheet(
        "background-color: #6a3fa0; color: white; border: 2px solid #9b6fd6; "
        "border-radius: 13px; font-weight: bold; font-size: 11px;");
    exhaustPileBadge->hide();

    exhaustPileOverlay = new QWidget(this);
    exhaustPileOverlay->setStyleSheet("background-color: rgba(0,0,0,190);");
    exhaustPileOverlay->hide();

    QVBoxLayout* overlayLayout = new QVBoxLayout(exhaustPileOverlay);
    overlayLayout->setAlignment(Qt::AlignCenter);

    pileOverlayTitle = new QLabel("", exhaustPileOverlay);
    pileOverlayTitle->setAlignment(Qt::AlignCenter);
    pileOverlayTitle->setStyleSheet("font-size: 24px; font-weight: bold; background: transparent;");
    overlayLayout->addWidget(pileOverlayTitle);

    exhaustCardsContainer = new QWidget();
    QGridLayout* cardsGridLayout = new QGridLayout(exhaustCardsContainer);
    cardsGridLayout->setSpacing(12);
    cardsGridLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    pileScrollArea = new QScrollArea(exhaustPileOverlay);
    pileScrollArea->setWidget(exhaustCardsContainer);
    pileScrollArea->setWidgetResizable(true);
    pileScrollArea->setStyleSheet("background: transparent; border: none;");
    pileScrollArea->setFixedHeight(500);
    pileScrollArea->setMinimumWidth(700);
    overlayLayout->addWidget(pileScrollArea, 0, Qt::AlignCenter);

    closeExhaustOverlayButton = new QPushButton("Close", exhaustPileOverlay);
    closeExhaustOverlayButton->setFixedSize(120, 40);
    closeExhaustOverlayButton->setStyleSheet(
        "QPushButton { background-color: #6a3fa0; color: white; border-radius: 6px; font-weight: bold; }"
        "QPushButton:hover { background-color: #7d4bb5; }");
    overlayLayout->addWidget(closeExhaustOverlayButton, 0, Qt::AlignCenter);

    connect(closeExhaustOverlayButton, &QPushButton::clicked, this, &MainWindow::hidePileOverlay);
    discardWrapper->installEventFilter(this);
    exhaustPileBadge->installEventFilter(this);
    ui->EndTurnButton->installEventFilter(this);

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
    enemyBlockBadge->setFixedSize(36, 36);
    enemyBlockBadge->setStyleSheet(
        "background-color: #2b3a55; color: #9fd8ff; border: 2px solid #5c85b0; "
        "border-radius: 18px; font-weight: bold; font-size: 13px;"
        );
    enemyBlockBadge->hide();

    enemyIntentLabel = new QLabel(this);
    enemyIntentLabel->setAlignment(Qt::AlignCenter);
    enemyIntentLabel->setFixedHeight(28);
    enemyIntentLabel->setStyleSheet(
        "background-color: rgba(20,20,20,190); color: white; border-radius: 8px; "
        "padding: 3px; font-weight: bold;"
        );

    customTooltipBox = new QLabel(this);
    customTooltipBox->setStyleSheet(
        "background-color: rgba(20,20,25,235); color: white; "
        "border: 1px solid rgba(255,255,255,40); border-radius: 6px; "
        "padding: 10px; font-size: 13px;");
    customTooltipBox->setWordWrap(true);
    customTooltipBox->setFixedWidth(260);
    customTooltipBox->hide();
    customTooltipBox->setAttribute(Qt::WA_TransparentForMouseEvents);

    enemyNameLabel = new QLabel(this);
    enemyNameLabel->setAlignment(Qt::AlignCenter);
    enemyNameLabel->setFixedHeight(20);
    enemyNameLabel->setStyleSheet(
        "color: white; font-weight: bold; font-size: 13px; "
        "background-color: rgba(0,0,0,140); border-radius: 4px; padding: 2px;");

    playerSpriteLabel = new QLabel(this);
    playerSpriteLabel->setPixmap(QPixmap(":/images/characters/IronClad.png").scaled(200, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    playerSpriteLabel->setAlignment(Qt::AlignCenter);
    playerSpriteLabel->setScaledContents(true);

    enemySpriteLabel = new QLabel(this);
    enemySpriteLabel->setAlignment(Qt::AlignCenter);
    enemySpriteLabel->setScaledContents(true);

    enemySpriteLabel->installEventFilter(this);
    enemyHpBar->installEventFilter(this);
    enemyIntentLabel->installEventFilter(this);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    mainLayout->addStretch(1);
    mainLayout->addStretch(3);

    QHBoxLayout* bottomBarLayout = new QHBoxLayout();
    bottomBarLayout->addWidget(playerEnergyOrb, 0, Qt::AlignBottom);
    bottomBarLayout->addWidget(ui->CardsContainer, 1);
    bottomBarLayout->addWidget(drawPileIconLabel, 0, Qt::AlignBottom);
    bottomBarLayout->addWidget(discardWrapper, 0, Qt::AlignBottom);
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
    mapLabel->raise();
    deckIconLabel->raise();
    settingLabel->raise();
    ui->EndTurnButton->raise();

    gameOverLabel = new QLabel(this);
    gameOverLabel->setAlignment(Qt::AlignCenter);
    gameOverLabel->hide();

    gameOverOpacityEffect = new QGraphicsOpacityEffect(gameOverLabel);
    gameOverLabel->setGraphicsEffect(gameOverOpacityEffect);
    gameOverOpacityEffect->setOpacity(0.0);

    QPixmap slashPixmap(200, 200);
    slashPixmap.fill(Qt::transparent);
    {
        QPainter painter(&slashPixmap);
        painter.setRenderHint(QPainter::Antialiasing);

        QPen glowPen(QColor(255, 120, 120, 140));
        glowPen.setWidth(14);
        glowPen.setCapStyle(Qt::RoundCap);
        painter.setPen(glowPen);
        painter.drawLine(30, 30, 170, 170);

        QPen corePen(QColor(255, 20, 20, 235));
        corePen.setWidth(6);
        corePen.setCapStyle(Qt::RoundCap);
        painter.setPen(corePen);
        painter.drawLine(30, 30, 170, 170);
    }

    playerHitOverlay = new QLabel(this);
    playerHitOverlay->setFixedSize(200, 200);
    playerHitOverlay->setPixmap(slashPixmap);
    playerHitOverlay->hide();
    playerHitOpacity = new QGraphicsOpacityEffect(playerHitOverlay);
    playerHitOverlay->setGraphicsEffect(playerHitOpacity);
    playerHitOpacity->setOpacity(0.0);

    enemyHitOverlay = new QLabel(this);
    enemyHitOverlay->setFixedSize(200, 200);
    enemyHitOverlay->setPixmap(slashPixmap);
    enemyHitOverlay->hide();
    enemyHitOpacity = new QGraphicsOpacityEffect(enemyHitOverlay);
    enemyHitOverlay->setGraphicsEffect(enemyHitOpacity);
    enemyHitOpacity->setOpacity(0.0);

    bgAudioOutput = new QAudioOutput(this);
    bgMusicPlayer = new QMediaPlayer(this);
    bgMusicPlayer->setAudioOutput(bgAudioOutput);
    bgMusicPlayer->setSource(QUrl("qrc:/audio/Exordium.mp3"));
    bgAudioOutput->setVolume(0.4);

    connect(bgMusicPlayer, &QMediaPlayer::mediaStatusChanged, this, [=](QMediaPlayer::MediaStatus status){
        if (status == QMediaPlayer::EndOfMedia) {
            bgMusicPlayer->setPosition(0);
            bgMusicPlayer->play();
        }
    });
    bgMusicPlayer->play();

    hitSoundOutput = new QAudioOutput(this);
    hitSoundPlayer = new QMediaPlayer(this);
    hitSoundPlayer->setAudioOutput(hitSoundOutput);
    hitSoundPlayer->setSource(QUrl("qrc:/audio/hit.mp3"));
    hitSoundOutput->setVolume(0.8);

    cardPlaySoundOutput = new QAudioOutput(this);
    cardPlaySoundPlayer = new QMediaPlayer(this);
    cardPlaySoundPlayer->setAudioOutput(cardPlaySoundOutput);
    cardPlaySoundPlayer->setSource(QUrl("qrc:/audio/Card.mp3"));
    cardPlaySoundOutput->setVolume(3);

    endTurnHoverSoundOutput = new QAudioOutput(this);
    endTurnHoverSoundPlayer = new QMediaPlayer(this);
    endTurnHoverSoundPlayer->setAudioOutput(endTurnHoverSoundOutput);
    endTurnHoverSoundPlayer->setSource(QUrl("qrc:/audio/End Turn.mp3"));
    endTurnHoverSoundOutput->setVolume(3);

    pileOpenSoundOutput = new QAudioOutput(this);
    pileOpenSoundPlayer = new QMediaPlayer(this);
    pileOpenSoundPlayer->setAudioOutput(pileOpenSoundOutput);
    pileOpenSoundPlayer->setSource(QUrl("qrc:/audio/DrawPile&DiscardPile.mp3"));
    pileOpenSoundOutput->setVolume(3);

    playerStatusRow = new QWidget(this);
    QHBoxLayout* playerStatusLayout = new QHBoxLayout(playerStatusRow);
    playerStatusLayout->setContentsMargins(0, 0, 0, 0);
    playerStatusLayout->setSpacing(4);

    enemyStatusRow = new QWidget(this);
    QHBoxLayout* enemyStatusLayout = new QHBoxLayout(enemyStatusRow);
    enemyStatusLayout->setContentsMargins(0, 0, 0, 0);
    enemyStatusLayout->setSpacing(4);

    hoverCardLabel = new QLabel(this);
    hoverCardLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    hoverCardLabel->setStyleSheet("background: transparent; border: none;");
    hoverCardLabel->hide();

    QGraphicsDropShadowEffect* hoverShadow = new QGraphicsDropShadowEffect(hoverCardLabel);
    hoverShadow->setBlurRadius(45);
    hoverShadow->setOffset(0, 14);
    hoverShadow->setColor(QColor(0, 0, 0, 210));
    hoverCardLabel->setGraphicsEffect(hoverShadow);

    toastLabel = new QLabel(this);
    toastLabel->setAlignment(Qt::AlignCenter);
    toastLabel->setStyleSheet(
        "background-color: rgba(180,30,30,220); color: white; font-weight: bold; "
        "font-size: 16px; border-radius: 8px; padding: 10px;");
    toastLabel->hide();

    dragArrowLabel = new QLabel(this);
    dragArrowLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    dragArrowLabel->hide();

    playerTargetFrame = new QLabel(this);
    playerTargetFrame->setAttribute(Qt::WA_TransparentForMouseEvents);
    playerTargetFrame->hide();

    std::srand(std::time(nullptr));
    battleManager = new BattleManager();
    playerObject = new Player("Dina", 80, 80, 3, 99, battleManager);
    battleManager->setPlayer(playerObject);
    battleManager->spawnEnemy(new SphericGuardian());
    initializePlayerDeck(15);
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
                bool overEnemy = enemySpriteLabel->geometry().adjusted(-60, -60, 60, 60).contains(mousePos);
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
        QPixmap original(":/images/scene.png");
        backgroundLabel->setPixmap(original.scaled(this->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    }

    int playerW = 200, playerH = 300;
    int enemyW  = 200, enemyH  = 200;
    int gap = 550;

    currentStartX = (this->width() - (playerW + enemyW + gap)) / 2;

    basePlayerY   = (this->height() / 2) - (playerH / 2) - 90;
    baseEnemyY    = (this->height() / 2) - (enemyH / 2) - 80;

    playerSpriteLabel->setGeometry(currentStartX, basePlayerY, playerW, playerH);
    enemySpriteLabel->setGeometry(currentStartX + playerW + gap, baseEnemyY, enemyW, enemyH);

    int barWidth = 180, barHeight = 22;

    topHudBar->setGeometry(0, 0, this->width(), 48);

    playerHpBar->setGeometry(
        currentStartX + playerW / 2 - barWidth / 2 + 20,
        basePlayerY + 280,
        barWidth, barHeight);

    playerBlockBadge->setGeometry(currentStartX + playerW - 30, basePlayerY - 8, 30, 30);

    int enemyX2 = currentStartX + playerW + gap;

    enemyHpBar->setGeometry(
        enemyX2 + enemyW / 2 - barWidth / 2,
        baseEnemyY - 32,
        barWidth, barHeight);

    enemyBlockBadge->setGeometry(enemyX2 - 5, baseEnemyY + enemyH - 35, 36, 36);

    enemyIntentLabel->setGeometry(enemyX2 - 20, baseEnemyY - 65, enemyW + 40, 28);
    enemyNameLabel->setGeometry(enemyX2 - 20, baseEnemyY - 90, enemyW + 40, 20);

    playerHitOverlay->setGeometry(currentStartX, basePlayerY, playerW, playerH);
    enemyHitOverlay->setGeometry(enemyX2, baseEnemyY, enemyW, enemyH);

    int endTurnW = 230, endTurnH = 82;
    ui->EndTurnButton->setGeometry(
        this->width() - endTurnW + 30,
        this->height() - endTurnH - 100,
        endTurnW, endTurnH);

    if (exhaustPileOverlay->isVisible())
        exhaustPileOverlay->setGeometry(0, 0, this->width(), this->height());

    deckIconLabel->move(this->width() - 116, 3);
    mapLabel->move(this->width() - 172, 3);
    settingLabel->move(this->width() - 60, 3);

    if (settingsOverlayImage && settingsOverlayImage->isVisible()) {
        settingsOverlayImage->setGeometry(0, 0, this->width(), this->height());
        settingsOverlayImage->setPixmap(QPixmap(":/images/settings_overlay.png").scaled(
            this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    playerStatusRow->setGeometry(currentStartX + 10, basePlayerY + playerH + 22, playerW, 30);
    enemyStatusRow->setGeometry(enemyX2, baseEnemyY - 5, enemyW, 30);
}

void MainWindow::on_EndTurnButton_clicked()
{
    if (isGameOver)
        return;

    const auto& hand = playerObject->getHand();
    for (Card* card : hand) {
        if (card && card->getName() == "Burn")
            playerObject->decreaseHp(2);
    }

    checkGameOver();
    if (isGameOver) {
        updateCharacterUI();
        return;
    }

    ui->EndTurnButton->setEnabled(false);

    const auto& enemies = battleManager->getEnemies();
    bool enemyWillAttack = false;
    if (!enemies.empty()) {
        IntentType intent = enemies[0]->getIntentType();
        enemyWillAttack = (intent == IntentType::Attack || intent == IntentType::Combined);
    }

    if (enemyWillAttack)
        playEnemyAttack();
    else
        playEnemyNonAttackTurn();
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
        QString cardImagePath = ":/images/cards/" + cardName + ".png";

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
    bool hasEnemy = !enemies.empty();

    enemySpriteLabel->setVisible(hasEnemy);
    enemyHpBar->setVisible(hasEnemy);
    enemyIntentLabel->setVisible(hasEnemy);
    enemyNameLabel->setVisible(hasEnemy);
    enemyBlockBadge->setVisible(false);

    if (!hasEnemy){
        customTooltipBox->hide();
        updateStatusEffectRow(enemyStatusRow, nullptr);
    }

    if (hasEnemy) {
        Enemy* enemy = enemies[0];
        QString enemyName = QString::fromStdString(enemy->getName());

        QPixmap enemyPixmap = getEnemyPixmap(enemyName);
        enemySpriteLabel->setPixmap(enemyPixmap.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        enemyNameLabel->setText(enemyName);

        enemyHpBar->setMaximum(enemy->getMaxHp());
        enemyHpBar->setValue(enemy->getHp());
        enemyHpBar->setFormat(QString("%1 / %2").arg(enemy->getHp()).arg(enemy->getMaxHp()));
        updateStatusEffectRow(enemyStatusRow, enemy);

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

    playerSpriteLabel->setGeometry(currentStartX, basePlayerY + floatOffset, 200, 300);
    if (!isAttackAnimating)
        enemySpriteLabel->setGeometry(currentStartX + 200 + 550, baseEnemyY - floatOffset, 200, 200);
}

void MainWindow::checkGameOver() {
    if (isGameOver) return;

    if (playerObject->getHp() <= 0) {
        isGameOver = true;
        ui->EndTurnButton->setEnabled(false);
        disableAllCards();
        showGameOverText("DEFEAT", QColor("#c0392b"));
        return;
    }

    if (battleManager->getEnemies().empty()) {
        isGameOver = true;
        ui->EndTurnButton->setEnabled(false);
        disableAllCards();
        showGameOverText("VICTORY", QColor("#f5c518"));
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

void MainWindow::showEnemyTooltip(Enemy* enemy) {
    if (!enemy) { customTooltipBox->hide(); return; }

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
    }

    customTooltipBox->setText(QString("<div style='font-weight:bold; font-size:14px; color:#f5c518; margin-bottom:6px;'>%1</div>""<div>%2</div>").arg(title, desc));

    customTooltipBox->adjustSize();

    int tipX = enemySpriteLabel->x() - customTooltipBox->width() - 20;
    int tipY = enemySpriteLabel->y() + 40;
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
                QRect enemyRect = enemySpriteLabel->geometry().adjusted(-60, -60, 60, 60);
                if (enemyRect.contains(localRelease))
                    playCardAtIndex(draggedCardIndex);
            } else {
                QRect playerRect = playerSpriteLabel->geometry().adjusted(-60, -60, 60, 60);
                if (playerRect.contains(localRelease))
                    playCardAtIndex(draggedCardIndex);
            }

            draggedCardIndex = -1;
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
        settingsOverlayImage->setPixmap(QPixmap(":/images/icons/settings_overlay.png").scaled(
            this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        settingsOverlayImage->show();
        settingsOverlayImage->raise();
        return true;
    }

    if (obj == enemySpriteLabel || obj == enemyHpBar || obj == enemyIntentLabel) {
        if (event->type() == QEvent::Enter) {
            const auto& enemies = battleManager->getEnemies();
            if (!enemies.empty())
                showEnemyTooltip(enemies[0]);
        } else if (event->type() == QEvent::Leave) {
            customTooltipBox->hide();
        }
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

void MainWindow::playEnemyAttack() {
    isAttackAnimating = true;

    int lungeDistance = 60;
    QRect startRect = enemySpriteLabel->geometry();
    QRect lungeRect(startRect.x() - lungeDistance, startRect.y(), startRect.width(), startRect.height());

    QPropertyAnimation* forward = new QPropertyAnimation(enemySpriteLabel, "geometry", this);
    forward->setDuration(300);
    forward->setStartValue(startRect);
    forward->setEndValue(lungeRect);
    forward->setEasingCurve(QEasingCurve::OutQuad);

    connect(forward, &QPropertyAnimation::finished, this, [=]() {
        QTimer::singleShot(150, this, [=]() {
            int playerHpBefore = playerObject->getHp();

            battleManager->enemyTurn();
            battleManager->cleanupDeadEnemies();

            int damageDealt = playerHpBefore - playerObject->getHp();
            if (damageDealt > 0)
                showFloatingDamage(playerSpriteLabel->geometry(), damageDealt, QColor("#ff4d4d"));

            hitSoundPlayer->setPosition(0);
            hitSoundPlayer->play();

            playHitEffect(playerHitOverlay, playerHitOpacity);
            updateCharacterUI();

            QPropertyAnimation* backward = new QPropertyAnimation(enemySpriteLabel, "geometry", this);
            backward->setDuration(350);
            backward->setStartValue(lungeRect);
            backward->setEndValue(startRect);
            backward->setEasingCurve(QEasingCurve::InQuad);

            connect(backward, &QPropertyAnimation::finished, this, [=]() {
                QTimer::singleShot(200, this, [=]() {
                    isAttackAnimating = false;
                    checkGameOver();
                    if (!isGameOver)
                        ui->EndTurnButton->setEnabled(true);
                    updateHandUI();
                    updateCharacterUI();
                });
            });

            backward->start(QAbstractAnimation::DeleteWhenStopped);
        });
    });

    forward->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::playEnemyNonAttackTurn() {
    isAttackAnimating = true;

    battleManager->enemyTurn();
    battleManager->cleanupDeadEnemies();
    updateCharacterUI();

    QTimer::singleShot(400, this, [=]() {
        isAttackAnimating = false;
        checkGameOver();
        if (!isGameOver)
            ui->EndTurnButton->setEnabled(true);
        updateHandUI();
        updateCharacterUI();
    });
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
    pileOverlayTitle->setStyleSheet(QString(
                                        "color: %1; font-size: 24px; font-weight: bold; background: transparent;").arg(titleColor));

    const int columns = 5;
    int row = 0, col = 0;

    for (Card* card : cards) {
        if (!card) continue;

        QLabel* cardImgLabel = new QLabel();
        cardImgLabel->setFixedSize(120, 160);
        QString cardName = QString::fromStdString(card->getName());
        cardImgLabel->setPixmap(QPixmap(":/images/cards/" + cardName + ".png")
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
        badge->setFixedSize(90, 26);
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

    customTooltipBox->setText(QString(
                                  "<div style='font-weight:bold; font-size:14px; color:#f5c518; margin-bottom:6px;'>%1 (%2)</div>"
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
    dmgLabel->setStyleSheet(QString(
                                "color: %1; font-size: 30px; font-weight: 900; background: transparent;").arg(color.name()));
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

void MainWindow::playCardAtIndex(int index) {
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

    bool isAttackCard = (card->getType() == CardType::Attack);
    int handSizeBefore = playerObject->getHandSize();

    Enemy* targetEnemy = allEnemies[0];
    int enemyHpBefore = targetEnemy->getHp();

    if(card->getCost(playerObject) > playerObject->getEnergy())
    {
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

    if (isAttackCard && cardWasActuallyPlayed && !isGameOver) {
        QTimer::singleShot(150, this, [=]() {
            if (damageDealt > 0)
                showFloatingDamage(enemySpriteLabel->geometry(), damageDealt, QColor("#ff4d4d"));
            hitSoundPlayer->stop();
            hitSoundPlayer->setPosition(0);
            hitSoundPlayer->play();
            playHitEffect(enemyHitOverlay, enemyHitOpacity);
        });
    } else if (damageDealt > 0) {
        showFloatingDamage(enemySpriteLabel->geometry(), damageDealt, QColor("#ff4d4d"));
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

    QPixmap pix(":/images/not_enough_energy.png");

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