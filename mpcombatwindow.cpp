#include "mpcombatwindow.h"
#include "ui_mpcombatwindow.h"
#include "networkmanager.h"
#include "usermanager.h"
#include <QMouseEvent>
#include <QMessageBox>
#include <QDebug>

MPCombatWindow::MPCombatWindow(QWidget *parent, bool isLeader)
    : QWidget(parent)
    , ui(new Ui::MPCombatWindow)
    , m_isLeader(isLeader)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Window);
    setWindowTitle("Combat");
    setMinimumSize(1280, 720);
    setMouseTracking(true);

    m_myUsername = user_manager::instance().get_current_username();

    buildUI();
    setupAudio();

    connect(&NetworkManager::instance(), &NetworkManager::game_action_received,
            this, &MPCombatWindow::handleNetworkMessage);

    animationTimer = new QTimer(this);
    connect(animationTimer, &QTimer::timeout, this, &MPCombatWindow::updateAnimations);
    animationTimer->start(50);
}

MPCombatWindow::~MPCombatWindow()
{
    delete ui;
}

// ================================================================
// UI Setup
// ================================================================
void MPCombatWindow::buildUI()
{
    backgroundLabel = new QLabel(this);
    backgroundLabel->setPixmap(QPixmap(":/assets/scene.png").scaled(this->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    backgroundLabel->setGeometry(0, 0, this->width(), this->height());
    backgroundLabel->lower();

    // --- Top HUD ---
    topHudBar = new QWidget(this);
    topHudBar->setStyleSheet(
        "background-color: rgba(20, 20, 25, 180); border-bottom: 2px solid rgba(255, 215, 130, 60);");

    turnIndicatorLabel = new QLabel(topHudBar);
    turnIndicatorLabel->setAlignment(Qt::AlignCenter);
    turnIndicatorLabel->setStyleSheet("color: white; font-weight: bold; font-size: 16px; background: transparent;");

    spectatorLabel = new QLabel(this);
    spectatorLabel->setAlignment(Qt::AlignCenter);
    spectatorLabel->setStyleSheet(
        "background-color: rgba(0,0,0,160); color: #cccccc; font-style: italic; "
        "font-size: 14px; border-radius: 6px; padding: 6px;");
    spectatorLabel->hide();

    // --- Player ---
    playerSpriteLabel = new QLabel(this);
    playerSpriteLabel->setPixmap(QPixmap(":/assets/characters/IronClad.png").scaled(200, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    playerSpriteLabel->setScaledContents(true);

    playerHpBar = new QProgressBar(this);
    playerHpBar->setTextVisible(true);
    playerHpBar->setStyleSheet(
        "QProgressBar { border: 2px solid #3a1f1f; border-radius: 6px; background: #2b1414; "
        "color: white; font-weight: bold; text-align: center; }"
        "QProgressBar::chunk { background-color: qlineargradient(x1:0,y1:0,x2:1,y2:0, "
        "stop:0 #8e0e0e, stop:1 #d94040); border-radius: 4px; }");

    playerBlockBadge = new QLabel(this);
    playerBlockBadge->setAlignment(Qt::AlignCenter);
    playerBlockBadge->setFixedSize(30, 30);
    playerBlockBadge->setStyleSheet(
        "background-color: #2b3a55; color: #9fd8ff; border: 2px solid #5c85b0; "
        "border-radius: 15px; font-weight: bold;");
    playerBlockBadge->hide();

    playerEnergyOrb = new QLabel(this);
    playerEnergyOrb->setFixedSize(70, 70);
    playerEnergyOrb->setPixmap(QPixmap(":/assets/icons/energy.png").scaled(70, 70, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    energyOrbCountLabel = new QLabel(playerEnergyOrb);
    energyOrbCountLabel->setGeometry(0, 0, 70, 70);
    energyOrbCountLabel->setAlignment(Qt::AlignCenter);
    energyOrbCountLabel->setStyleSheet("background: transparent; color: white; font-weight: 900; font-size: 20px;");

    playerStatusRow = new QWidget(this);
    QHBoxLayout *psl = new QHBoxLayout(playerStatusRow);
    psl->setContentsMargins(0, 0, 0, 0);
    psl->setSpacing(4);

    playerHitOverlay = new QLabel(this);
    playerHitOverlay->setFixedSize(200, 200);
    playerHitOverlay->hide();
    playerHitOpacity = new QGraphicsOpacityEffect(playerHitOverlay);
    playerHitOverlay->setGraphicsEffect(playerHitOpacity);
    playerHitOpacity->setOpacity(0.0);

    // --- Teammate ---
    teammateSpriteLabel = new QLabel(this);
    teammateSpriteLabel->setPixmap(QPixmap(":/assets/characters/teammate.png").scaled(200, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    teammateSpriteLabel->setScaledContents(true);
    teammateSpriteLabel->hide();

    teammateHpBar = new QProgressBar(this);
    teammateHpBar->setTextVisible(true);
    teammateHpBar->setStyleSheet(playerHpBar->styleSheet());
    teammateHpBar->hide();

    teammateNameLabel = new QLabel(this);
    teammateNameLabel->setAlignment(Qt::AlignCenter);
    teammateNameLabel->setStyleSheet(
        "color: white; font-weight: bold; font-size: 13px; background-color: rgba(0,0,0,140); "
        "border-radius: 4px; padding: 2px;");
    teammateNameLabel->hide();

    teammateBlockBadge = new QLabel(this);
    teammateBlockBadge->setAlignment(Qt::AlignCenter);
    teammateBlockBadge->setFixedSize(30, 30);
    teammateBlockBadge->setStyleSheet(playerBlockBadge->styleSheet());
    teammateBlockBadge->hide();

    teammateDownOverlay = new QLabel("DOWN", this);
    teammateDownOverlay->setAlignment(Qt::AlignCenter);
    teammateDownOverlay->setStyleSheet(
        "background-color: rgba(120,0,0,190); color: white; font-weight: 900; "
        "font-size: 18px; border-radius: 6px;");
    teammateDownOverlay->hide();

    teammateStatusRow = new QWidget(this);
    QHBoxLayout *tsl = new QHBoxLayout(teammateStatusRow);
    tsl->setContentsMargins(0, 0, 0, 0);
    tsl->setSpacing(4);

    teammateHitOverlay = new QLabel(this);
    teammateHitOverlay->setFixedSize(200, 200);
    teammateHitOverlay->hide();
    teammateHitOpacity = new QGraphicsOpacityEffect(teammateHitOverlay);
    teammateHitOverlay->setGraphicsEffect(teammateHitOpacity);
    teammateHitOpacity->setOpacity(0.0);

    // --- Enemy ---
    enemySpriteLabel = new QLabel(this);
    enemySpriteLabel->setAlignment(Qt::AlignCenter);
    enemySpriteLabel->setScaledContents(true);
    enemySpriteLabel->installEventFilter(this);

    enemyHpBar = new QProgressBar(this);
    enemyHpBar->setTextVisible(true);
    enemyHpBar->setStyleSheet(playerHpBar->styleSheet());
    enemyHpBar->installEventFilter(this);

    enemyBlockBadge = new QLabel(this);
    enemyBlockBadge->setAlignment(Qt::AlignCenter);
    enemyBlockBadge->setFixedSize(36, 36);
    enemyBlockBadge->setStyleSheet(
        "background-color: #2b3a55; color: #9fd8ff; border: 2px solid #5c85b0; "
        "border-radius: 18px; font-weight: bold; font-size: 13px;");
    enemyBlockBadge->hide();

    enemyIntentLabel = new QLabel(this);
    enemyIntentLabel->setAlignment(Qt::AlignCenter);
    enemyIntentLabel->setFixedHeight(28);
    enemyIntentLabel->setStyleSheet(
        "background-color: rgba(20,20,20,190); color: white; border-radius: 8px; padding: 3px; font-weight: bold;");
    enemyIntentLabel->installEventFilter(this);

    enemyNameLabel = new QLabel(this);
    enemyNameLabel->setAlignment(Qt::AlignCenter);
    enemyNameLabel->setFixedHeight(20);
    enemyNameLabel->setStyleSheet(
        "color: white; font-weight: bold; font-size: 13px; background-color: rgba(0,0,0,140); "
        "border-radius: 4px; padding: 2px;");

    enemyStatusRow = new QWidget(this);
    QHBoxLayout *esl = new QHBoxLayout(enemyStatusRow);
    esl->setContentsMargins(0, 0, 0, 0);
    esl->setSpacing(4);

    enemyHitOverlay = new QLabel(this);
    enemyHitOverlay->setFixedSize(200, 200);
    enemyHitOverlay->hide();
    enemyHitOpacity = new QGraphicsOpacityEffect(enemyHitOverlay);
    enemyHitOverlay->setGraphicsEffect(enemyHitOpacity);
    enemyHitOpacity->setOpacity(0.0);

    // --- Piles ---
    drawPileIconLabel = new QLabel(this);
    drawPileIconLabel->setFixedSize(84, 84);
    drawPileIconLabel->setPixmap(QPixmap(":/assets/icons/draw_pile.png").scaled(84, 84, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    drawPileIconLabel->installEventFilter(this);

    drawPileCountLabel = new QLabel(drawPileIconLabel);
    drawPileCountLabel->setGeometry(12, 28, 28, 16);
    drawPileCountLabel->setAlignment(Qt::AlignCenter);
    drawPileCountLabel->setStyleSheet(
        "background-color: rgba(0,0,0,170); color: white; font-weight: bold; font-size: 11px; border-radius: 4px;");

    discardWrapper = new QWidget(this);
    discardWrapper->setFixedSize(90, 90);
    discardWrapper->installEventFilter(this);

    discardPileIconLabel = new QLabel(discardWrapper);
    discardPileIconLabel->setGeometry(16, 16, 74, 74);
    discardPileIconLabel->setPixmap(QPixmap(":/assets/icons/discard_pile.png").scaled(74, 74, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    discardPileCountLabel = new QLabel(discardPileIconLabel);
    discardPileCountLabel->setGeometry(12, 28, 28, 16);
    discardPileCountLabel->setAlignment(Qt::AlignCenter);
    discardPileCountLabel->setStyleSheet(
        "background-color: rgba(0,0,0,170); color: white; font-weight: bold; font-size: 11px; border-radius: 4px;");

    exhaustPileBadge = new QLabel(discardWrapper);
    exhaustPileBadge->setFixedSize(26, 26);
    exhaustPileBadge->move(0, 0);
    exhaustPileBadge->setAlignment(Qt::AlignCenter);
    exhaustPileBadge->setStyleSheet(
        "background-color: #6a3fa0; color: white; border: 2px solid #9b6fd6; "
        "border-radius: 13px; font-weight: bold; font-size: 11px;");
    exhaustPileBadge->hide();
    exhaustPileBadge->installEventFilter(this);

    // --- Pile overlay ---
    pileOverlay = new QWidget(this);
    pileOverlay->setStyleSheet("background-color: rgba(0,0,0,190);");
    pileOverlay->hide();

    QVBoxLayout *overlayLayout = new QVBoxLayout(pileOverlay);
    overlayLayout->setAlignment(Qt::AlignCenter);

    pileOverlayTitle = new QLabel("", pileOverlay);
    pileOverlayTitle->setAlignment(Qt::AlignCenter);
    pileOverlayTitle->setStyleSheet("font-size: 24px; font-weight: bold; background: transparent;");
    overlayLayout->addWidget(pileOverlayTitle);

    pileCardsContainer = new QWidget();
    QGridLayout *cardsGrid = new QGridLayout(pileCardsContainer);
    cardsGrid->setSpacing(12);
    cardsGrid->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    pileScrollArea = new QScrollArea(pileOverlay);
    pileScrollArea->setWidget(pileCardsContainer);
    pileScrollArea->setWidgetResizable(true);
    pileScrollArea->setStyleSheet("background: transparent; border: none;");
    pileScrollArea->setFixedHeight(500);
    pileScrollArea->setMinimumWidth(700);
    overlayLayout->addWidget(pileScrollArea, 0, Qt::AlignCenter);

    closePileOverlayButton = new QPushButton("Close", pileOverlay);
    closePileOverlayButton->setFixedSize(120, 40);
    closePileOverlayButton->setStyleSheet(
        "QPushButton { background-color: #6a3fa0; color: white; border-radius: 6px; font-weight: bold; }"
        "QPushButton:hover { background-color: #7d4bb5; }");
    overlayLayout->addWidget(closePileOverlayButton, 0, Qt::AlignCenter);
    connect(closePileOverlayButton, &QPushButton::clicked, this, &MPCombatWindow::hidePileOverlay);

    // --- Cards + End Turn ---
    m_cardsContainer = ui->CardsContainer;
    QHBoxLayout *cardLayout = new QHBoxLayout(m_cardsContainer);

    m_endTurnBtn = ui->EndTurnButton;
    m_endTurnBtn->setFixedSize(150, 60);
    m_endTurnBtn->setStyleSheet(
        "QPushButton { background-color: #7a1f1f; color: white; font-weight: bold; border-radius: 8px; }"
        "QPushButton:hover { background-color: #9a2a2a; }"
        "QPushButton:disabled { background-color: #555; color: #999; }");
    m_endTurnBtn->setEnabled(false);
    m_endTurnBtn->installEventFilter(this);
    connect(m_endTurnBtn, &QPushButton::clicked, this, &MPCombatWindow::sendEndTurn);

    // --- Game over text ---
    gameOverLabel = new QLabel(this);
    gameOverLabel->setAlignment(Qt::AlignCenter);
    gameOverLabel->hide();
    gameOverOpacityEffect = new QGraphicsOpacityEffect(gameOverLabel);
    gameOverLabel->setGraphicsEffect(gameOverOpacityEffect);
    gameOverOpacityEffect->setOpacity(0.0);

    // --- Tooltip / toast / hover card ---
    customTooltipBox = new QLabel(this);
    customTooltipBox->setStyleSheet(
        "background-color: rgba(20,20,25,235); color: white; border: 1px solid rgba(255,255,255,40); "
        "border-radius: 6px; padding: 10px; font-size: 13px;");
    customTooltipBox->setWordWrap(true);
    customTooltipBox->setFixedWidth(260);
    customTooltipBox->hide();
    customTooltipBox->setAttribute(Qt::WA_TransparentForMouseEvents);

    toastLabel = new QLabel(this);
    toastLabel->setAlignment(Qt::AlignCenter);
    toastLabel->setStyleSheet(
        "background-color: rgba(180,30,30,220); color: white; font-weight: bold; "
        "font-size: 16px; border-radius: 8px; padding: 10px;");
    toastLabel->hide();

    hoverCardLabel = new QLabel(this);
    hoverCardLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    hoverCardLabel->setStyleSheet("background: transparent; border: none;");
    hoverCardLabel->hide();

    // z-order
    topHudBar->raise();
    m_endTurnBtn->raise();
}

void MPCombatWindow::setupAudio()
{
    bgAudioOutput = new QAudioOutput(this);
    bgMusicPlayer = new QMediaPlayer(this);
    bgMusicPlayer->setAudioOutput(bgAudioOutput);
    bgMusicPlayer->setSource(QUrl("qrc:/assets/audio/Exordium.mp3"));
    bgAudioOutput->setVolume(0.4);
    connect(bgMusicPlayer, &QMediaPlayer::mediaStatusChanged, this, [=](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::EndOfMedia) {
            bgMusicPlayer->setPosition(0);
            bgMusicPlayer->play();
        }
    });
    bgMusicPlayer->play();

    hitSoundOutput = new QAudioOutput(this);
    hitSoundPlayer = new QMediaPlayer(this);
    hitSoundPlayer->setAudioOutput(hitSoundOutput);
    hitSoundPlayer->setSource(QUrl("qrc:/assets/audio/hit.mp3"));
    hitSoundOutput->setVolume(0.8);

    cardPlaySoundOutput = new QAudioOutput(this);
    cardPlaySoundPlayer = new QMediaPlayer(this);
    cardPlaySoundPlayer->setAudioOutput(cardPlaySoundOutput);
    cardPlaySoundPlayer->setSource(QUrl("qrc:/assets/audio/Card.mp3"));
    cardPlaySoundOutput->setVolume(1.0);

    endTurnHoverSoundOutput = new QAudioOutput(this);
    endTurnHoverSoundPlayer = new QMediaPlayer(this);
    endTurnHoverSoundPlayer->setAudioOutput(endTurnHoverSoundOutput);
    endTurnHoverSoundPlayer->setSource(QUrl("qrc:/assets/audio/End Turn.mp3"));
    endTurnHoverSoundOutput->setVolume(1.0);

    pileOpenSoundOutput = new QAudioOutput(this);
    pileOpenSoundPlayer = new QMediaPlayer(this);
    pileOpenSoundPlayer->setAudioOutput(pileOpenSoundOutput);
    pileOpenSoundPlayer->setSource(QUrl("qrc:/assets/audio/DrawPile&DiscardPile.mp3"));
    pileOpenSoundOutput->setVolume(1.0);
}

// ================================================================
// Layout on resize
// ================================================================
void MPCombatWindow::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    backgroundLabel->resize(this->size());
    backgroundLabel->setPixmap(QPixmap(":/assets/scene.png").scaled(this->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));

    topHudBar->setGeometry(0, 0, this->width(), 48);
    turnIndicatorLabel->setGeometry(0, 0, this->width(), 48);
    spectatorLabel->setGeometry(this->width() / 2 - 220, 55, 440, 30);

    int playerW = 180, playerH = 260;
    int teammateW = 160, teammateH = 230;
    int enemyW = 200, enemyH = 200;

    m_playerX = 60;
    m_teammateX = 60;
    m_enemyX = this->width() - enemyW - 120;

    m_basePlayerY = this->height() / 2 - playerH / 2;
    m_baseTeammateY = m_basePlayerY - teammateH - 20;
    m_baseEnemyY = this->height() / 2 - enemyH / 2 - 40;

    playerSpriteLabel->setGeometry(m_playerX, m_basePlayerY, playerW, playerH);
    playerHpBar->setGeometry(m_playerX, m_basePlayerY + playerH + 8, 180, 22);
    playerBlockBadge->setGeometry(m_playerX + playerW - 20, m_basePlayerY - 8, 30, 30);
    playerEnergyOrb->setGeometry(m_playerX - 20, m_basePlayerY + playerH + 40, 70, 70);
    playerStatusRow->setGeometry(m_playerX, m_basePlayerY + playerH + 70, 200, 30);
    playerHitOverlay->setGeometry(m_playerX, m_basePlayerY, playerW, playerH);

    teammateSpriteLabel->setGeometry(m_teammateX, m_baseTeammateY, teammateW, teammateH);
    teammateHpBar->setGeometry(m_teammateX, m_baseTeammateY - 26, 160, 20);
    teammateNameLabel->setGeometry(m_teammateX, m_baseTeammateY - 48, 160, 20);
    teammateBlockBadge->setGeometry(m_teammateX + teammateW - 20, m_baseTeammateY - 8, 30, 30);
    teammateDownOverlay->setGeometry(m_teammateX, m_baseTeammateY + teammateH / 2 - 15, teammateW, 30);
    teammateStatusRow->setGeometry(m_teammateX, m_baseTeammateY + teammateH + 4, 200, 26);
    teammateHitOverlay->setGeometry(m_teammateX, m_baseTeammateY, teammateW, teammateH);

    enemySpriteLabel->setGeometry(m_enemyX, m_baseEnemyY, enemyW, enemyH);
    enemyHpBar->setGeometry(m_enemyX, m_baseEnemyY + enemyH + 12, 180, 22);
    enemyBlockBadge->setGeometry(m_enemyX + enemyW - 10, m_baseEnemyY + enemyH - 35, 36, 36);
    enemyIntentLabel->setGeometry(m_enemyX - 20, m_baseEnemyY - 65, enemyW + 40, 28);
    enemyNameLabel->setGeometry(m_enemyX - 20, m_baseEnemyY - 90, enemyW + 40, 20);
    enemyStatusRow->setGeometry(m_enemyX, m_baseEnemyY + enemyH + 40, enemyW, 30);
    enemyHitOverlay->setGeometry(m_enemyX, m_baseEnemyY, enemyW, enemyH);

    drawPileIconLabel->move(this->width() - 190, this->height() - 110);
    discardWrapper->move(this->width() - 100, this->height() - 110);

    m_cardsContainer->setGeometry(220, this->height() - 200, this->width() - 440, 180);
    m_endTurnBtn->move(this->width() - 190, this->height() - 280);

    if (pileOverlay->isVisible())
        pileOverlay->setGeometry(0, 0, this->width(), this->height());
}

// ================================================================
// Network dispatch
// ================================================================
void MPCombatWindow::handleNetworkMessage(const QJsonObject &obj)
{
    const QString type = obj["type"].toString();

    if (type == "state_update")
        handleStateUpdate(obj);
    else if (type == "combat_over")
        handleCombatOver(obj);
    else if (type == "leader_changed")
        handleLeaderChanged(obj);
}

// ================================================================
// state_update
// ================================================================
void MPCombatWindow::handleStateUpdate(const QJsonObject &obj)
{
    m_isPlayerTurn = obj["is_player_turn"].toBool();

    QJsonArray players = obj["players"].toArray();
    m_hasTeammate = false;

    for (const QJsonValue &v : players) {
        QJsonObject p = v.toObject();
        QString username = p["username"].toString();

        if (username == m_myUsername) {
            m_myHp = p["hp"].toInt();
            m_myMaxHp = p["max_hp"].toInt();
            m_myEnergy = p["energy"].toInt();
            m_myMaxEnergy = p["max_energy"].toInt();
            m_myBlock = p["block"].toInt();
            m_iAmAlive = p["is_alive"].toBool();
            m_myEffects = p["effects"].toArray();

            m_myHand.clear();
            for (const QJsonValue &c : p["hand"].toArray()) m_myHand.append(c.toString());

            m_drawPile.clear();
            for (const QJsonValue &c : p["draw_pile"].toArray()) m_drawPile.append(c.toString());

            m_discardPile.clear();
            for (const QJsonValue &c : p["discard_pile"].toArray()) m_discardPile.append(c.toString());

            m_exhaustPile.clear();
            for (const QJsonValue &c : p["exhaust_pile"].toArray()) m_exhaustPile.append(c.toString());
        } else {
            m_hasTeammate = true;
            m_teammateUsername = username;
            m_teammateHp = p["hp"].toInt();
            m_teammateMaxHp = p["max_hp"].toInt();
            m_teammateBlock = p["block"].toInt();
            m_teammateAlive = p["is_alive"].toBool();
        }
    }

    QJsonArray enemies = obj["enemies"].toArray();
    if (!enemies.isEmpty()) {
        QJsonObject e = enemies[0].toObject();
        m_enemyName = e["name"].toString();
        m_enemyHp = e["hp"].toInt();
        m_enemyMaxHp = e["max_hp"].toInt();
        m_enemyBlock = e["block"].toInt();
        m_enemyIntent = e["intent"].toString();
        m_enemyEffects = e["effects"].toArray();
    }

    if (m_isPlayerTurn)
        m_hasEndedTurnLocally = false;

    if (m_lastMyHp >= 0 && m_myHp < m_lastMyHp) {
        int dmg = m_lastMyHp - m_myHp;
        showFloatingDamage(playerSpriteLabel->geometry(), dmg, QColor(255, 77, 77));
        playHitEffect(playerHitOverlay, playerHitOpacity);
        hitSoundPlayer->setPosition(0);
        hitSoundPlayer->play();
    }
    if (m_lastTeammateHp >= 0 && m_teammateHp < m_lastTeammateHp) {
        int dmg = m_lastTeammateHp - m_teammateHp;
        showFloatingDamage(teammateSpriteLabel->geometry(), dmg, QColor(255, 77, 77));
        playHitEffect(teammateHitOverlay, teammateHitOpacity);
    }
    if (m_lastEnemyHp >= 0 && m_enemyHp < m_lastEnemyHp) {
        int dmg = m_lastEnemyHp - m_enemyHp;
        showFloatingDamage(enemySpriteLabel->geometry(), dmg, QColor(255, 221, 85));
        playHitEffect(enemyHitOverlay, enemyHitOpacity);
        hitSoundPlayer->setPosition(0);
        hitSoundPlayer->play();
    }
    m_lastMyHp = m_myHp;
    m_lastTeammateHp = m_teammateHp;
    m_lastEnemyHp = m_enemyHp;

    updateCharacterUI();
    updateHandUI();
}

// ================================================================
// combat_over
// ================================================================
void MPCombatWindow::handleCombatOver(const QJsonObject &obj)
{
    if (m_isGameOver) return;
    m_isGameOver = true;

    bool victory = obj["victory"].toBool();
    m_endTurnBtn->setEnabled(false);
    m_cardsContainer->setEnabled(false);

    showGameOverText(victory ? "VICTORY" : "DEFEAT", victory ? QColor(245, 197, 24) : QColor(192, 57, 43));

    QTimer::singleShot(2200, this, [this, victory]() {
        emit combatFinished(victory);
    });
}

// ================================================================
// leader_changed
// ================================================================
void MPCombatWindow::handleLeaderChanged(const QJsonObject &obj)
{
    m_isLeader = obj["you_are_leader"].toBool();
    if (m_isLeader)
        showToastMessage("You are now the Leader");
}

// ================================================================
// UI update
// ================================================================
void MPCombatWindow::updateCharacterUI()
{
    playerHpBar->setMaximum(m_myMaxHp);
    playerHpBar->setValue(qMax(0, m_myHp));
    playerHpBar->setFormat(QString("%1 / %2").arg(m_myHp).arg(m_myMaxHp));
    updateStatusEffectRow(playerStatusRow, m_myEffects);

    playerBlockBadge->setVisible(m_myBlock > 0);
    if (m_myBlock > 0) playerBlockBadge->setText(QString::number(m_myBlock));

    energyOrbCountLabel->setText(QString("%1/%2").arg(m_myEnergy).arg(m_myMaxEnergy));

    drawPileCountLabel->setText(QString::number(m_drawPile.size()));
    discardPileCountLabel->setText(QString::number(m_discardPile.size()));

    exhaustPileBadge->setVisible(!m_exhaustPile.isEmpty());
    if (!m_exhaustPile.isEmpty())
        exhaustPileBadge->setText(QString::number(m_exhaustPile.size()));

    teammateSpriteLabel->setVisible(m_hasTeammate);
    teammateHpBar->setVisible(m_hasTeammate);
    teammateNameLabel->setVisible(m_hasTeammate);
    if (m_hasTeammate) {
        teammateNameLabel->setText(m_teammateUsername);
        teammateHpBar->setMaximum(m_teammateMaxHp);
        teammateHpBar->setValue(qMax(0, m_teammateHp));
        teammateHpBar->setFormat(QString("%1 / %2").arg(m_teammateHp).arg(m_teammateMaxHp));

        teammateBlockBadge->setVisible(m_teammateBlock > 0);
        if (m_teammateBlock > 0) teammateBlockBadge->setText(QString::number(m_teammateBlock));

        teammateDownOverlay->setVisible(!m_teammateAlive);
    }

    bool hasEnemy = !m_enemyName.isEmpty();
    enemySpriteLabel->setVisible(hasEnemy);
    enemyHpBar->setVisible(hasEnemy);
    enemyIntentLabel->setVisible(hasEnemy);
    enemyNameLabel->setVisible(hasEnemy);

    if (hasEnemy) {
        enemySpriteLabel->setPixmap(getEnemyPixmap(m_enemyName).scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        enemyNameLabel->setText(m_enemyName);
        enemyHpBar->setMaximum(m_enemyMaxHp);
        enemyHpBar->setValue(qMax(0, m_enemyHp));
        enemyHpBar->setFormat(QString("%1 / %2").arg(m_enemyHp).arg(m_enemyMaxHp));
        enemyIntentLabel->setText(m_enemyIntent);
        updateStatusEffectRow(enemyStatusRow, m_enemyEffects);

        enemyBlockBadge->setVisible(m_enemyBlock > 0);
        if (m_enemyBlock > 0) enemyBlockBadge->setText(QString::number(m_enemyBlock));
    }

    if (!m_iAmAlive) {
        spectatorLabel->setText("You have fallen — spectating");
        spectatorLabel->show();
        turnIndicatorLabel->setText("");
        m_endTurnBtn->setEnabled(false);
        m_cardsContainer->setEnabled(false);
    } else {
        spectatorLabel->hide();
        turnIndicatorLabel->setText(m_isPlayerTurn ? "Your Turn" : "Enemy Turn...");
        m_endTurnBtn->setEnabled(m_isPlayerTurn && !m_hasEndedTurnLocally);
        m_cardsContainer->setEnabled(m_isPlayerTurn);
    }
}

void MPCombatWindow::updateStatusEffectRow(QWidget *rowWidget, const QJsonArray &effects)
{
    QLayout *layout = rowWidget->layout();
    QLayoutItem *child;
    while ((child = layout->takeAt(0)) != nullptr) {
        if (child->widget()) { child->widget()->hide(); delete child->widget(); }
        delete child;
    }

    for (const QJsonValue &v : effects) {
        QJsonObject eff = v.toObject();
        QString name = eff["name"].toString();
        int amount = eff["amount"].toInt();

        QLabel *badge = new QLabel(rowWidget);
        badge->setFixedSize(90, 26);
        badge->setAlignment(Qt::AlignCenter);
        badge->setText(QString("%1 %2").arg(name).arg(amount));
        badge->setStyleSheet(
            "background-color: #4a4a4a; color: white; border-radius: 6px; font-weight: bold; font-size: 10px;");
        badge->setProperty("effectName", name);
        badge->setProperty("effectAmount", amount);
        badge->installEventFilter(this);

        layout->addWidget(badge);
    }
}

void MPCombatWindow::updateHandUI()
{
    hideHoverCard();

    QLayout *layout = m_cardsContainer->layout();
    QLayoutItem *child;
    while ((child = layout->takeAt(0)) != nullptr) {
        if (child->widget()) { child->widget()->hide(); delete child->widget(); }
        delete child;
    }

    for (int i = 0; i < m_myHand.size(); ++i) {
        QString cardName = m_myHand[i];
        QPushButton *btn = new QPushButton();
        btn->setFixedSize(120, 160);
        btn->setIcon(QIcon(":/assets/cards/" + cardName + ".png"));
        btn->setIconSize(btn->size());
        btn->setStyleSheet("QPushButton { border: none; background: transparent; }");
        btn->setProperty("cardName", cardName);
        btn->setProperty("cardImagePath", ":/assets/cards/" + cardName + ".png");
        btn->installEventFilter(this);

        connect(btn, &QPushButton::clicked, this, [this, cardName]() { sendPlayCard(cardName); });
        layout->addWidget(btn);

        QGraphicsOpacityEffect *fadeEffect = new QGraphicsOpacityEffect(btn);
        btn->setGraphicsEffect(fadeEffect);
        fadeEffect->setOpacity(0.0);
        QPropertyAnimation *fadeIn = new QPropertyAnimation(fadeEffect, "opacity", this);
        fadeIn->setDuration(500);
        fadeIn->setStartValue(0.0);
        fadeIn->setEndValue(1.0);
        fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

QPixmap MPCombatWindow::getEnemyPixmap(const QString &enemyName)
{
    QPixmap p(":/assets/enemies/" + enemyName + ".png");
    if (p.isNull())
        return QPixmap(":/assets/enemy_default.png");
    return p;
}

// ================================================================
// Actions
// ================================================================
void MPCombatWindow::sendPlayCard(const QString &cardName)
{
    if (!m_isPlayerTurn || !m_iAmAlive) return;

    cardPlaySoundPlayer->stop();
    cardPlaySoundPlayer->setPosition(0);
    cardPlaySoundPlayer->play();

    QJsonObject msg;
    msg["type"] = "play_card";
    msg["card_name"] = cardName;
    NetworkManager::instance().send_game_action(msg);
}

void MPCombatWindow::sendEndTurn()
{
    if (!m_isPlayerTurn || !m_iAmAlive || m_hasEndedTurnLocally) return;

    m_hasEndedTurnLocally = true;
    m_endTurnBtn->setEnabled(false);

    QJsonObject msg;
    msg["type"] = "end_turn";
    NetworkManager::instance().send_game_action(msg);
}

// ================================================================
// Animations & effects
// ================================================================
void MPCombatWindow::updateAnimations()
{
    m_angle += 0.2f;
    int floatOffset = static_cast<int>(std::sin(m_angle) * 6);

    playerSpriteLabel->move(playerSpriteLabel->x(), m_basePlayerY + floatOffset);
    if (m_hasTeammate)
        teammateSpriteLabel->move(teammateSpriteLabel->x(), m_baseTeammateY + floatOffset);
    enemySpriteLabel->move(enemySpriteLabel->x(), m_baseEnemyY - floatOffset);
}

void MPCombatWindow::playHitEffect(QLabel *overlay, QGraphicsOpacityEffect *opacityEffect)
{
    overlay->show();
    overlay->raise();

    QPropertyAnimation *flashIn = new QPropertyAnimation(opacityEffect, "opacity", this);
    flashIn->setDuration(60);
    flashIn->setStartValue(0.0);
    flashIn->setEndValue(0.7);

    QPropertyAnimation *flashOut = new QPropertyAnimation(opacityEffect, "opacity", this);
    flashOut->setDuration(250);
    flashOut->setStartValue(0.7);
    flashOut->setEndValue(0.0);

    QSequentialAnimationGroup *seq = new QSequentialAnimationGroup(this);
    seq->addAnimation(flashIn);
    seq->addAnimation(flashOut);
    connect(seq, &QSequentialAnimationGroup::finished, overlay, &QLabel::hide);
    seq->start(QAbstractAnimation::DeleteWhenStopped);
}

void MPCombatWindow::showFloatingDamage(QRect targetRect, int amount, const QColor &color)
{
    if (amount <= 0) return;

    QLabel *dmgLabel = new QLabel(this);
    dmgLabel->setText(QString("-%1").arg(amount));
    dmgLabel->setStyleSheet(QString("color: %1; font-size: 28px; font-weight: 900; background: transparent;").arg(color.name()));
    dmgLabel->adjustSize();

    int startX = targetRect.x() + targetRect.width() / 2 - dmgLabel->width() / 2;
    int startY = targetRect.y() + 20;
    dmgLabel->move(startX, startY);
    dmgLabel->show();
    dmgLabel->raise();

    QGraphicsOpacityEffect *opacity = new QGraphicsOpacityEffect(dmgLabel);
    dmgLabel->setGraphicsEffect(opacity);

    QPropertyAnimation *moveAnim = new QPropertyAnimation(dmgLabel, "geometry", this);
    moveAnim->setDuration(750);
    moveAnim->setStartValue(dmgLabel->geometry());
    moveAnim->setEndValue(QRect(startX, startY - 60, dmgLabel->width(), dmgLabel->height()));

    QPropertyAnimation *fadeAnim = new QPropertyAnimation(opacity, "opacity", this);
    fadeAnim->setDuration(750);
    fadeAnim->setStartValue(1.0);
    fadeAnim->setEndValue(0.0);

    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);
    group->addAnimation(moveAnim);
    group->addAnimation(fadeAnim);
    connect(group, &QParallelAnimationGroup::finished, dmgLabel, &QLabel::deleteLater);
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void MPCombatWindow::showToastMessage(const QString &text)
{
    toastLabel->setText(text);
    toastLabel->adjustSize();
    toastLabel->move(this->width() / 2 - toastLabel->width() / 2, 120);
    toastLabel->show();
    toastLabel->raise();
    QTimer::singleShot(1500, toastLabel, &QLabel::hide);
}

void MPCombatWindow::showGameOverText(const QString &text, const QColor &color)
{
    gameOverLabel->setText(text);
    gameOverLabel->setStyleSheet(QString("color: %1; font-size: 72px; font-weight: 900; background: transparent;").arg(color.name()));

    int labelW = 600, labelH = 120;
    QRect endRect((this->width() - labelW) / 2, (this->height() - labelH) / 2, labelW, labelH);
    gameOverLabel->setGeometry(endRect);
    gameOverLabel->show();
    gameOverLabel->raise();
    gameOverOpacityEffect->setOpacity(0.0);

    QPropertyAnimation *fadeAnim = new QPropertyAnimation(gameOverOpacityEffect, "opacity", this);
    fadeAnim->setDuration(900);
    fadeAnim->setStartValue(0.0);
    fadeAnim->setEndValue(1.0);
    fadeAnim->setEasingCurve(QEasingCurve::OutCubic);
    fadeAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void MPCombatWindow::showHoverCard(QPushButton *originalBtn)
{
    if (!originalBtn) return;
    QString imagePath = originalBtn->property("cardImagePath").toString();
    QPixmap fullResPixmap(imagePath);
    if (fullResPixmap.isNull()) return;

    QPoint globalPos = originalBtn->mapTo(this, QPoint(0, 0));
    QRect smallRect(globalPos.x(), globalPos.y(), originalBtn->width(), originalBtn->height());
    hoverOriginalRect = smallRect;

    double scale = 1.5;
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

    if (hoverGeomAnim) { hoverGeomAnim->stop(); delete hoverGeomAnim; hoverGeomAnim = nullptr; }
    hoverGeomAnim = new QPropertyAnimation(hoverCardLabel, "geometry", this);
    hoverGeomAnim->setDuration(150);
    hoverGeomAnim->setStartValue(hoverCardLabel->geometry());
    hoverGeomAnim->setEndValue(bigRect);
    hoverGeomAnim->setEasingCurve(QEasingCurve::OutCubic);
    hoverGeomAnim->start();
}

void MPCombatWindow::hideHoverCard()
{
    if (!hoverCardLabel || !hoverCardLabel->isVisible()) return;
    if (hoverGeomAnim) { hoverGeomAnim->stop(); delete hoverGeomAnim; hoverGeomAnim = nullptr; }

    hoverGeomAnim = new QPropertyAnimation(hoverCardLabel, "geometry", this);
    hoverGeomAnim->setDuration(120);
    hoverGeomAnim->setStartValue(hoverCardLabel->geometry());
    hoverGeomAnim->setEndValue(hoverOriginalRect);
    hoverGeomAnim->setEasingCurve(QEasingCurve::InCubic);

    QLabel *labelPtr = hoverCardLabel;
    connect(hoverGeomAnim, &QPropertyAnimation::finished, labelPtr, &QLabel::hide);
    hoverGeomAnim->start();
}

void MPCombatWindow::showEnemyTooltip()
{
    if (m_enemyName.isEmpty()) { customTooltipBox->hide(); return; }

    customTooltipBox->setText(QString(
                                  "<div style='font-weight:bold; font-size:14px; color:#f5c518; margin-bottom:6px;'>%1</div>"
                                  "<div>%2</div>").arg(m_enemyName, m_enemyIntent));
    customTooltipBox->adjustSize();

    int tipX = enemySpriteLabel->x() - customTooltipBox->width() - 20;
    int tipY = enemySpriteLabel->y() + 40;
    customTooltipBox->move(tipX, tipY);
    customTooltipBox->show();
    customTooltipBox->raise();
}

void MPCombatWindow::showStatusEffectTooltip(QLabel *badge)
{
    QString name = badge->property("effectName").toString();
    int amount = badge->property("effectAmount").toInt();

    customTooltipBox->setText(QString(
                                  "<div style='font-weight:bold; font-size:14px; color:#f5c518; margin-bottom:6px;'>%1 (%2)</div>")
                                  .arg(name).arg(amount));
    customTooltipBox->adjustSize();

    QPoint globalPos = badge->mapTo(this, QPoint(0, 0));
    customTooltipBox->move(globalPos.x(), globalPos.y() + badge->height() + 6);
    customTooltipBox->show();
    customTooltipBox->raise();
}

// ================================================================
// Pile overlay
// ================================================================
void MPCombatWindow::showCardPileOverlay(const QString &title, const QStringList &cards, const QString &titleColor)
{
    pileOpenSoundPlayer->setPosition(0);
    pileOpenSoundPlayer->play();

    QGridLayout *gridLayout = qobject_cast<QGridLayout *>(pileCardsContainer->layout());
    QLayoutItem *child;
    while ((child = gridLayout->takeAt(0)) != nullptr) {
        if (child->widget()) { child->widget()->hide(); delete child->widget(); }
        delete child;
    }

    pileOverlayTitle->setText(title);
    pileOverlayTitle->setStyleSheet(QString("color: %1; font-size: 24px; font-weight: bold; background: transparent;").arg(titleColor));

    const int columns = 5;
    int row = 0, col = 0;
    for (const QString &cardName : cards) {
        QLabel *cardImgLabel = new QLabel();
        cardImgLabel->setFixedSize(120, 160);
        cardImgLabel->setPixmap(QPixmap(":/assets/cards/" + cardName + ".png").scaled(120, 160, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        gridLayout->addWidget(cardImgLabel, row, col);
        col++;
        if (col >= columns) { col = 0; row++; }
    }

    pileOverlay->setGeometry(0, 0, this->width(), this->height());
    pileOverlay->show();
    pileOverlay->raise();
}

void MPCombatWindow::hidePileOverlay()
{
    pileOverlay->hide();
}

// ================================================================
// Event filter — hover states, pile clicks, tooltips
// ================================================================
bool MPCombatWindow::eventFilter(QObject *obj, QEvent *event)
{
    QPushButton *cardBtn = qobject_cast<QPushButton *>(obj);
    if (cardBtn && cardBtn->property("cardImagePath").isValid()) {
        if (event->type() == QEvent::Enter) { showHoverCard(cardBtn); return false; }
        if (event->type() == QEvent::Leave) { hideHoverCard(); return false; }
    }

    if (obj == m_endTurnBtn && event->type() == QEvent::Enter && m_endTurnBtn->isEnabled()) {
        endTurnHoverSoundPlayer->setPosition(0);
        endTurnHoverSoundPlayer->play();
    }

    QLabel *badge = qobject_cast<QLabel *>(obj);
    if (badge && badge->property("effectName").isValid()) {
        if (event->type() == QEvent::Enter) showStatusEffectTooltip(badge);
        else if (event->type() == QEvent::Leave) customTooltipBox->hide();
        return false;
    }

    if (obj == exhaustPileBadge && event->type() == QEvent::MouseButtonRelease) {
        showCardPileOverlay("Exhaust Pile", m_exhaustPile, "#c07af0");
        return true;
    }
    if (obj == discardWrapper && event->type() == QEvent::MouseButtonRelease) {
        showCardPileOverlay("Discard Pile", m_discardPile, "#e0c060");
        return true;
    }
    if (obj == drawPileIconLabel && event->type() == QEvent::MouseButtonRelease) {
        showCardPileOverlay("Draw Pile", m_drawPile, "#7fd0ff");
        return true;
    }

    if (obj == enemySpriteLabel || obj == enemyHpBar || obj == enemyIntentLabel) {
        if (event->type() == QEvent::Enter) showEnemyTooltip();
        else if (event->type() == QEvent::Leave) customTooltipBox->hide();
    }

    return QWidget::eventFilter(obj, event);
}