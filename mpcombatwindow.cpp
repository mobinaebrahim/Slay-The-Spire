#include "mpcombatwindow.h"
#include "ui_mpcombatwindow.h"
#include "networkmanager.h"
#include "usermanager.h"
#include <QMouseEvent>
#include <QMessageBox>
#include <QDebug>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ================================================================
// Helper: determine if a card name is an Attack (client-side)
// ================================================================
static bool isAttackCard(const QString &name)
{
    static const QStringList attacks = {
        "Strike", "Bash", "Blood for Blood", "Clash", "Feed", "Immolate",
        "PerfectedStrike", "Reaper", "Bludgeon", "TwinStrike", "Whirlwind",
        "Disarm", "Uppercut", "Carnage", "Pummel", "Sever Soul", "Dropkick",
        "Hemokinesis", "Rampage", "Searing Blow", "Thunderclap", "Pommel Strike",
        "Headbutt", "Clothesline", "Iron Wave", "Sword Boomerang", "Anger",
        "Body Slam", "Cleave", "Heavy Blade", "Fiend Fire", "Twin Strike",
        "Bloodletting", "Offering"
    };
    return attacks.contains(name);
}

// ================================================================
// Helper: status-effect colours (parity with MainWindow)
// ================================================================
static QString effectColor(const QString& name)
{
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

// ================================================================
// Helper: status-effect descriptions (parity with MainWindow)
// ================================================================
static QString effectDescription(const QString& name, int amount)
{
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
    setupShortcuts();

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
    backgroundLabel->setPixmap(QPixmap(":/images/scene.png").scaled(this->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    backgroundLabel->setGeometry(0, 0, this->width(), this->height());
    backgroundLabel->lower();

    topHudBar = new QWidget(this);
    topHudBar->setStyleSheet(
        "background-color: rgba(20, 20, 25, 180); border-bottom: 2px solid rgba(255, 215, 130, 60);");

    turnIndicatorLabel = new QLabel(topHudBar);
    turnIndicatorLabel->setAlignment(Qt::AlignCenter);
    turnIndicatorLabel->setStyleSheet("color: white; font-weight: bold; font-size: 16px; background: transparent;");

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
    settingLabel->installEventFilter(this);

    settingsOverlayImage = new QLabel(this);
    settingsOverlayImage->setAlignment(Qt::AlignCenter);
    settingsOverlayImage->setStyleSheet("background-color: rgba(0,0,0,220);");
    settingsOverlayImage->setPixmap(QPixmap(":/images/icons/settings_overlay.png").scaled(
        this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    settingsOverlayImage->setGeometry(0, 0, this->width(), this->height());
    settingsOverlayImage->hide();
    settingsOverlayImage->installEventFilter(this);

    mapLabel = new QLabel(this);
    mapLabel->setFixedSize(46, 46);
    mapLabel->setPixmap(QPixmap(":/images/icons/map.png").scaled(46, 46, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    mapLabel->installEventFilter(this);

    deckIconLabel = new QLabel(this);
    deckIconLabel->setFixedSize(46, 46);
    deckIconLabel->setPixmap(QPixmap(":/images/icons/deck.png").scaled(46, 46, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    deckIconLabel->installEventFilter(this);

    deckCountLabel = new QLabel(deckIconLabel);
    deckCountLabel->setGeometry(14, 30, 32, 16);
    deckCountLabel->setAlignment(Qt::AlignCenter);
    deckCountLabel->setStyleSheet("color: white; font-weight: bold; font-size: 14px; background: transparent;");

    spectatorLabel = new QLabel(this);
    spectatorLabel->setAlignment(Qt::AlignCenter);
    spectatorLabel->setStyleSheet(
        "background-color: rgba(0,0,0,160); color: #cccccc; font-style: italic; "
        "font-size: 14px; border-radius: 6px; padding: 6px;");
    spectatorLabel->hide();

    playerSpriteLabel = new QLabel(this);
    playerSpriteLabel->setPixmap(QPixmap(":/images/characters/IronClad.png").scaled(200, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    playerSpriteLabel->setAlignment(Qt::AlignCenter);
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

    playerNameLabel = new QLabel(this);
    playerNameLabel->setAlignment(Qt::AlignCenter);
    playerNameLabel->setStyleSheet(
        "color: white; font-weight: bold; font-size: 13px; background-color: rgba(0,0,0,140); "
        "border-radius: 4px; padding: 2px;");
    playerNameLabel->hide();

    playerEnergyOrb = new QLabel(this);
    playerEnergyOrb->setFixedSize(70, 70);
    playerEnergyOrb->setPixmap(QPixmap(":/images/icons/energy.png").scaled(70, 70, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    energyOrbCountLabel = new QLabel(playerEnergyOrb);
    energyOrbCountLabel->setGeometry(0, 0, 70, 70);
    energyOrbCountLabel->setAlignment(Qt::AlignCenter);
    energyOrbCountLabel->setStyleSheet("background: transparent; color: white; font-weight: 900; font-size: 20px;");

    playerStatusRow = new QWidget(this);
    QHBoxLayout *psl = new QHBoxLayout(playerStatusRow);
    psl->setContentsMargins(0, 0, 0, 0);
    psl->setSpacing(4);

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

    teammateSpriteLabel = new QLabel(this);
    teammateSpriteLabel->setPixmap(QPixmap(":/images/characters/teammate.png").scaled(200, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
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
    teammateHitOverlay->setPixmap(slashPixmap);
    teammateHitOverlay->hide();
    teammateHitOpacity = new QGraphicsOpacityEffect(teammateHitOverlay);
    teammateHitOverlay->setGraphicsEffect(teammateHitOpacity);
    teammateHitOpacity->setOpacity(0.0);

    enemyAreaContainer = new QWidget(this);
    enemyAreaLayout = new QHBoxLayout(enemyAreaContainer);
    enemyAreaLayout->setContentsMargins(0, 0, 0, 0);
    enemyAreaLayout->setSpacing(30);
    enemyAreaLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    customTooltipBox = new QLabel(this);
    customTooltipBox->setStyleSheet(
        "background-color: rgba(20,20,25,235); color: white; border: 1px solid rgba(255,255,255,40); "
        "border-radius: 6px; padding: 10px; font-size: 13px;");
    customTooltipBox->setWordWrap(true);
    customTooltipBox->setFixedWidth(260);
    customTooltipBox->hide();
    customTooltipBox->setAttribute(Qt::WA_TransparentForMouseEvents);

    drawPileIconLabel = new QLabel(this);
    drawPileIconLabel->setFixedSize(84, 84);
    drawPileIconLabel->setPixmap(QPixmap(":/images/icons/draw_pile.png").scaled(84, 84, Qt::KeepAspectRatio, Qt::SmoothTransformation));
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
    discardPileIconLabel->setPixmap(QPixmap(":/images/icons/discard_pile.png").scaled(74, 74, Qt::KeepAspectRatio, Qt::SmoothTransformation));

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

    m_cardsContainer = ui->CardsContainer;
    // FIX: avoid creating a duplicate layout if the .ui file already assigned one
    QHBoxLayout *cardLayout = qobject_cast<QHBoxLayout*>(m_cardsContainer->layout());
    if (!cardLayout) {
        cardLayout = new QHBoxLayout(m_cardsContainer);
    }
    cardLayout->setContentsMargins(0, 0, 0, 0);
    cardLayout->setSpacing(8);

    m_endTurnBtn = ui->EndTurnButton;
    m_endTurnBtn->setFixedSize(150, 60);
    m_endTurnBtn->setStyleSheet(
        "QPushButton { background-color: #7a1f1f; color: white; font-weight: bold; border-radius: 8px; }"
        "QPushButton:hover { background-color: #9a2a2a; }"
        "QPushButton:disabled { background-color: #555; color: #999; }");
    m_endTurnBtn->setEnabled(false);
    m_endTurnBtn->installEventFilter(this);
    connect(m_endTurnBtn, &QPushButton::clicked, this, &MPCombatWindow::sendEndTurn);

    chatIconLabel = new QLabel(this);
    chatIconLabel->setFixedSize(46, 46);
    chatIconLabel->setPixmap(QPixmap(":/images/icons/chatbox.png").scaled(46, 46, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    chatIconLabel->installEventFilter(this);

    chatUnreadBadge = new QLabel(chatIconLabel);
    chatUnreadBadge->setFixedSize(20, 20);
    chatUnreadBadge->setAlignment(Qt::AlignCenter);
    chatUnreadBadge->setStyleSheet(
        "background-color: #c0392b; color: white; border-radius: 10px; font-weight: bold; font-size: 11px;");
    chatUnreadBadge->move(chatIconLabel->width() - 16, -4);
    chatUnreadBadge->hide();

    chatPanel = new QWidget(this);
    chatPanel->setStyleSheet(
        "background-color: rgba(20,20,25,235); border: 1px solid rgba(255,255,255,40); border-radius: 10px;");
    chatPanel->hide();

    QVBoxLayout *chatLayout = new QVBoxLayout(chatPanel);
    chatLayout->setContentsMargins(10, 10, 10, 10);
    chatLayout->setSpacing(8);

    QLabel *chatTitle = new QLabel("Team Chat", chatPanel);
    chatTitle->setStyleSheet("color: #f5c518; font-weight: bold; font-size: 14px; background: transparent;");
    chatLayout->addWidget(chatTitle);

    chatMessagesList = new QListWidget(chatPanel);
    chatMessagesList->setStyleSheet(
        "QListWidget { background: rgba(0,0,0,120); border: none; border-radius: 6px; color: white; font-size: 12px; }"
        "QListWidget::item { padding: 3px 6px; }");
    chatMessagesList->setWordWrap(true);
    chatMessagesList->setSelectionMode(QAbstractItemView::NoSelection);
    chatLayout->addWidget(chatMessagesList, 1);

    QHBoxLayout *chatInputRow = new QHBoxLayout();
    chatInputField = new QLineEdit(chatPanel);
    chatInputField->setPlaceholderText("Message your teammate...");
    chatInputField->setStyleSheet(
        "QLineEdit { background: rgba(255,255,255,15); border: 1px solid rgba(255,255,255,60); "
        "border-radius: 6px; color: white; padding: 6px; font-size: 12px; }");
    connect(chatInputField, &QLineEdit::returnPressed, this, &MPCombatWindow::sendChatMessage);
    chatInputRow->addWidget(chatInputField, 1);

    chatSendButton = new QPushButton("Send", chatPanel);
    chatSendButton->setFixedWidth(60);
    chatSendButton->setStyleSheet(
        "QPushButton { background-color: #6a3fa0; color: white; border-radius: 6px; font-weight: bold; }"
        "QPushButton:hover { background-color: #7d4bb5; }");
    connect(chatSendButton, &QPushButton::clicked, this, &MPCombatWindow::sendChatMessage);
    chatInputRow->addWidget(chatSendButton);

    chatLayout->addLayout(chatInputRow);

    dragArrowLabel = new QLabel(this);
    dragArrowLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    dragArrowLabel->hide();

    playerTargetFrame = new QLabel(this);
    playerTargetFrame->setAttribute(Qt::WA_TransparentForMouseEvents);
    playerTargetFrame->hide();

    gameOverLabel = new QLabel(this);
    gameOverLabel->setAlignment(Qt::AlignCenter);
    gameOverLabel->hide();
    gameOverOpacityEffect = new QGraphicsOpacityEffect(gameOverLabel);
    gameOverLabel->setGraphicsEffect(gameOverOpacityEffect);
    gameOverOpacityEffect->setOpacity(0.0);

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

    QGraphicsDropShadowEffect *hoverShadow = new QGraphicsDropShadowEffect(hoverCardLabel);
    hoverShadow->setBlurRadius(45);
    hoverShadow->setOffset(0, 14);
    hoverShadow->setColor(QColor(0, 0, 0, 210));
    hoverCardLabel->setGraphicsEffect(hoverShadow);

    topHudBar->raise();
    playerHeartIcon->raise();
    playerHpTopLabel->raise();
    goldIconLabel->raise();
    goldCountLabel->raise();
    mapLabel->raise();
    deckIconLabel->raise();
    settingLabel->raise();
    m_endTurnBtn->raise();
    chatIconLabel->raise();
    chatUnreadBadge->raise();
}

void MPCombatWindow::setupAudio()
{
    bgAudioOutput = new QAudioOutput(this);
    bgMusicPlayer = new QMediaPlayer(this);
    bgMusicPlayer->setAudioOutput(bgAudioOutput);
    bgMusicPlayer->setSource(QUrl("qrc:/audio/Exordium.mp3"));
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
    hitSoundPlayer->setSource(QUrl("qrc:/audio/hit.mp3"));
    hitSoundOutput->setVolume(0.8);

    cardPlaySoundOutput = new QAudioOutput(this);
    cardPlaySoundPlayer = new QMediaPlayer(this);
    cardPlaySoundPlayer->setAudioOutput(cardPlaySoundOutput);
    cardPlaySoundPlayer->setSource(QUrl("qrc:/audio/Card.mp3"));
    cardPlaySoundOutput->setVolume(1.0);

    endTurnHoverSoundOutput = new QAudioOutput(this);
    endTurnHoverSoundPlayer = new QMediaPlayer(this);
    endTurnHoverSoundPlayer->setAudioOutput(endTurnHoverSoundOutput);
    endTurnHoverSoundPlayer->setSource(QUrl("qrc:/audio/End Turn.mp3"));
    endTurnHoverSoundOutput->setVolume(1.0);

    pileOpenSoundOutput = new QAudioOutput(this);
    pileOpenSoundPlayer = new QMediaPlayer(this);
    pileOpenSoundPlayer->setAudioOutput(pileOpenSoundOutput);
    pileOpenSoundPlayer->setSource(QUrl("qrc:/audio/DrawPile&DiscardPile.mp3"));
    pileOpenSoundOutput->setVolume(1.0);
}

// ================================================================
// Keyboard shortcuts — full parity with single-player MainWindow
// ================================================================
void MPCombatWindow::setupShortcuts()
{
    QShortcut *endTurnKey = new QShortcut(QKeySequence(Qt::Key_E), this);
    connect(endTurnKey, &QShortcut::activated, this, [=]() {
        if (m_endTurnBtn->isEnabled()) sendEndTurn();
    });

    QShortcut *drawPileKey = new QShortcut(QKeySequence(Qt::Key_A), this);
    connect(drawPileKey, &QShortcut::activated, this, [=]() {
        showCardPileOverlay("Draw Pile", m_drawPile, "#7fd0ff");
    });

    QShortcut *discardPileKey = new QShortcut(QKeySequence(Qt::Key_S), this);
    connect(discardPileKey, &QShortcut::activated, this, [=]() {
        showCardPileOverlay("Discard Pile", m_discardPile, "#e0c060");
    });

    QShortcut *exhaustPileKey = new QShortcut(QKeySequence(Qt::Key_X), this);
    connect(exhaustPileKey, &QShortcut::activated, this, [=]() {
        showCardPileOverlay("Exhaust Pile", m_exhaustPile, "#c07af0");
    });

    QShortcut *deckKey = new QShortcut(QKeySequence(Qt::Key_D), this);
    connect(deckKey, &QShortcut::activated, this, [=]() {
        QStringList fullDeck = m_drawPile + m_discardPile + m_exhaustPile + m_myHand;
        showCardPileOverlay("Deck", fullDeck, "#66ccff");
    });

    QShortcut *escKey = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(escKey, &QShortcut::activated, this, [=]() {
        highlightedCardIndex = -1;
        updateCardHighlight();
        hidePileOverlay();
        hideHoverCard();
        if (settingsOverlayImage && settingsOverlayImage->isVisible())
            settingsOverlayImage->hide();
    });

    for (int i = 1; i <= 9; ++i) {
        QShortcut *numKey = new QShortcut(QKeySequence(QString::number(i)), this);
        int handIndex = i - 1;
        connect(numKey, &QShortcut::activated, this, [=]() {
            if (handIndex < m_myHand.size()) {
                highlightedCardIndex = handIndex;
                updateCardHighlight();
            }
        });
    }
    QShortcut *zeroKey = new QShortcut(QKeySequence(Qt::Key_0), this);
    connect(zeroKey, &QShortcut::activated, this, [=]() {
        if (9 < m_myHand.size()) {
            highlightedCardIndex = 9;
            updateCardHighlight();
        }
    });

    QShortcut *leftKey = new QShortcut(QKeySequence(Qt::Key_Left), this);
    connect(leftKey, &QShortcut::activated, this, [=]() {
        int handSize = m_myHand.size();
        if (handSize == 0) return;
        highlightedCardIndex = (highlightedCardIndex <= 0) ? handSize - 1 : highlightedCardIndex - 1;
        updateCardHighlight();
    });

    QShortcut *rightKey = new QShortcut(QKeySequence(Qt::Key_Right), this);
    connect(rightKey, &QShortcut::activated, this, [=]() {
        int handSize = m_myHand.size();
        if (handSize == 0) return;
        highlightedCardIndex = (highlightedCardIndex + 1) % handSize;
        updateCardHighlight();
    });

    QShortcut *enterKey = new QShortcut(QKeySequence(Qt::Key_Return), this);
    connect(enterKey, &QShortcut::activated, this, [=]() {
        if (highlightedCardIndex >= 0 && highlightedCardIndex < m_myHand.size()) {
            QString cardName = m_myHand[highlightedCardIndex];
            if (isAttackCard(cardName))
                sendPlayCard(cardName, targetedEnemyIndex);
            else
                sendPlayCard(cardName, -1);
        }
    });

    QShortcut *peekKey = new QShortcut(QKeySequence(Qt::Key_Space), this);
    connect(peekKey, &QShortcut::activated, this, [=]() {
        QLayout *layout = m_cardsContainer->layout();
        if (highlightedCardIndex >= 0 && highlightedCardIndex < layout->count()) {
            QPushButton *btn = qobject_cast<QPushButton*>(layout->itemAt(highlightedCardIndex)->widget());
            if (btn) showHoverCard(btn);
        }
    });
}

// ================================================================
// Layout on resize
// ================================================================
void MPCombatWindow::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    backgroundLabel->resize(this->size());
    backgroundLabel->setPixmap(QPixmap(":/images/scene.png").scaled(this->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));

    topHudBar->setGeometry(0, 0, this->width(), 48);
    turnIndicatorLabel->setGeometry(0, 0, this->width(), 48);
    spectatorLabel->setGeometry(this->width() / 2 - 220, 55, 440, 30);

    settingLabel->move(this->width() - 60, 3);
    mapLabel->move(this->width() - 172, 3);
    deckIconLabel->move(this->width() - 116, 3);

    if (settingsOverlayImage && settingsOverlayImage->isVisible()) {
        settingsOverlayImage->setGeometry(0, 0, this->width(), this->height());
        settingsOverlayImage->setPixmap(QPixmap(":/images/icons/settings_overlay.png").scaled(
            this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    int playerW = 180, playerH = 260;
    int teammateW = 160, teammateH = 230;

    currentStartX = 110;
    m_playerX = currentStartX + 40;
    m_teammateX = currentStartX + 130;

    m_basePlayerY = this->height() / 2 - playerH / 2;
    m_baseTeammateY = m_basePlayerY - teammateH + 30;

    playerSpriteLabel->setGeometry(m_playerX, m_basePlayerY, playerW, playerH);
    playerHpBar->setGeometry(m_playerX, m_basePlayerY + playerH + 8, 180, 22);
    playerBlockBadge->setGeometry(m_playerX + playerW - 20, m_basePlayerY - 8, 30, 30);
    playerEnergyOrb->setGeometry(m_playerX - 20, m_basePlayerY + playerH + 40, 70, 70);
    playerStatusRow->setGeometry(m_playerX, m_basePlayerY + playerH + 70, 200, 30);
    playerHitOverlay->setGeometry(m_playerX, m_basePlayerY, playerW, playerH);
    playerNameLabel->setGeometry(m_playerX, m_basePlayerY - 48, playerW, 20);

    teammateSpriteLabel->setGeometry(m_teammateX, m_baseTeammateY, teammateW, teammateH);
    teammateHpBar->setGeometry(m_teammateX, m_baseTeammateY - 26, 160, 20);
    teammateNameLabel->setGeometry(m_teammateX, m_baseTeammateY - 48, 160, 20);
    teammateBlockBadge->setGeometry(m_teammateX + teammateW - 20, m_baseTeammateY - 8, 30, 30);
    teammateDownOverlay->setGeometry(m_teammateX, m_baseTeammateY + teammateH / 2 - 15, teammateW, 30);
    teammateStatusRow->setGeometry(m_teammateX, m_baseTeammateY + teammateH + 4, 200, 26);
    teammateHitOverlay->setGeometry(m_teammateX, m_baseTeammateY, teammateW, teammateH);

    int spriteCenterOffsetInWrapper = 20 + 4 + 26 + 4 + 75;
    int playerSpriteCenterY = m_basePlayerY + playerH / 2;
    int enemyAreaH = 320;
    int enemyContainerTop = playerSpriteCenterY - spriteCenterOffsetInWrapper - 35;
    int enemyAreaX = currentStartX + playerW + 150;
    int enemyAreaW = this->width() - enemyAreaX - 60;
    enemyAreaContainer->setGeometry(enemyAreaX, enemyContainerTop, enemyAreaW, enemyAreaH);
    m_baseEnemyY = enemyContainerTop;

    drawPileIconLabel->move(this->width() - 190, this->height() - 110);
    discardWrapper->move(this->width() - 100, this->height() - 110);

    m_cardsContainer->setGeometry(220, this->height() - 200, this->width() - 440, 180);
    m_endTurnBtn->move(this->width() - 190, this->height() - 280);
    chatIconLabel->move(this->width() - 190, this->height() - 340);

    int chatPanelW = 320, chatPanelH = 380;
    chatPanel->setGeometry(this->width() - chatPanelW - 20,
                           this->height() - 340 - chatPanelH - 12,
                           chatPanelW, chatPanelH);

    if (pileOverlay->isVisible())
        pileOverlay->setGeometry(0, 0, this->width(), this->height());

    // FIX: keep overlays centered if visible during resize
    if (gameOverLabel && gameOverLabel->isVisible()) {
        int labelW = 600, labelH = 120;
        gameOverLabel->setGeometry((this->width() - labelW) / 2, (this->height() - labelH) / 2, labelW, labelH);
    }
    if (toastLabel && toastLabel->isVisible()) {
        toastLabel->move(this->width() / 2 - toastLabel->width() / 2, 120);
    }
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
    else if (type == "chat_message")
        handleChatMessage(obj);
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
            m_myGold = p["gold"].toInt(m_myGold);
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

    QJsonArray enemiesJson = obj["enemies"].toArray();
    std::vector<EnemyData> newEnemies;
    newEnemies.reserve(enemiesJson.size());
    for (const QJsonValue &v : enemiesJson) {
        QJsonObject e = v.toObject();
        EnemyData d;
        d.name = e["name"].toString();
        d.hp = e["hp"].toInt();
        d.maxHp = e["max_hp"].toInt();
        d.block = e["block"].toInt();
        d.intent = e["intent"].toString();
        d.effects = e["effects"].toArray();
        newEnemies.push_back(d);
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

    bool needsRebuild = (newEnemies.size() != m_enemies.size());
    if (!needsRebuild) {
        for (size_t i = 0; i < newEnemies.size(); ++i) {
            if (newEnemies[i].name != m_enemies[i].name) { needsRebuild = true; break; }
        }
    }

    for (size_t i = 0; i < newEnemies.size() && i < m_lastEnemyHps.size(); ++i) {
        if (m_lastEnemyHps[i] >= 0 && newEnemies[i].hp < m_lastEnemyHps[i]) {
            int dmg = m_lastEnemyHps[i] - newEnemies[i].hp;
            m_totalDamageDealtToEnemies += dmg;
            QRect r = enemySpriteRectFor((int)i);
            if (!r.isNull()) {
                showFloatingDamage(r, dmg, QColor(255, 221, 85));
                if (i < enemySlots.size() && enemySlots[i].hitOverlay) {
                    enemySlots[i].hitOverlay->setGeometry(r);
                    playHitEffect(enemySlots[i].hitOverlay, enemySlots[i].hitOpacity);
                }
                hitSoundPlayer->setPosition(0);
                hitSoundPlayer->play();
            }
        }
    }

    m_lastMyHp = m_myHp;
    m_lastTeammateHp = m_teammateHp;

    m_enemies = newEnemies;
    m_lastEnemyHps.assign(m_enemies.size(), -1);
    for (size_t i = 0; i < m_enemies.size(); ++i) m_lastEnemyHps[i] = m_enemies[i].hp;

    if (needsRebuild)
        rebuildEnemyUI();

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
        emit combatFinished(victory, m_totalDamageDealtToEnemies);
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
// Team chat
// ================================================================
void MPCombatWindow::toggleChatPanel()
{
    m_chatOpen = !m_chatOpen;
    chatPanel->setVisible(m_chatOpen);
    if (m_chatOpen) {
        chatPanel->raise();
        chatInputField->setFocus();
        m_unreadChatCount = 0;
        chatUnreadBadge->hide();
    }
}

void MPCombatWindow::sendChatMessage()
{
    QString text = chatInputField->text().trimmed();
    if (text.isEmpty()) return;

    QJsonObject msg;
    msg["type"] = "chat_message";
    msg["text"] = text;
    msg["username"] = m_myUsername;
    NetworkManager::instance().send_game_action(msg);

    appendChatMessage(m_myUsername, text, true);
    chatInputField->clear();
}

void MPCombatWindow::handleChatMessage(const QJsonObject &obj)
{
    QString username = obj["username"].toString();
    QString text = obj["text"].toString();
    if (username == m_myUsername) return;

    appendChatMessage(username, text, false);

    if (!m_chatOpen) {
        m_unreadChatCount++;
        chatUnreadBadge->setText(QString::number(m_unreadChatCount));
        chatUnreadBadge->show();
        chatUnreadBadge->raise();
    }
}

void MPCombatWindow::appendChatMessage(const QString &username, const QString &text, bool isMe)
{
    QListWidgetItem *item = new QListWidgetItem(chatMessagesList);
    QLabel *bubble = new QLabel(chatMessagesList);
    bubble->setWordWrap(true);
    bubble->setText(QString("<span style='color:%1; font-weight:bold;'>%2:</span> %3")
                        .arg(isMe ? "#7fd0ff" : "#f5c518", username.toHtmlEscaped(), text.toHtmlEscaped()));
    bubble->setStyleSheet("background: transparent; color: white; font-size: 12px; padding: 2px;");
    bubble->setMaximumWidth(chatMessagesList->width() - 20);

    item->setSizeHint(bubble->sizeHint());
    chatMessagesList->setItemWidget(item, bubble);
    chatMessagesList->scrollToBottom();
}

// ================================================================
// UI update
// ================================================================
void MPCombatWindow::updateCharacterUI()
{
    playerHpBar->setMaximum(m_myMaxHp);
    playerHpBar->setValue(qMax(0, m_myHp));
    // FIX: clamp displayed HP so it never shows negative
    playerHpBar->setFormat(QString("%1 / %2").arg(qMax(0, m_myHp)).arg(m_myMaxHp));
    updateStatusEffectRow(playerStatusRow, m_myEffects);

    playerHpTopLabel->setText(QString("%1/%2").arg(qMax(0, m_myHp)).arg(m_myMaxHp));
    goldCountLabel->setText(QString::number(m_myGold));

    playerBlockBadge->setVisible(m_myBlock > 0);
    if (m_myBlock > 0) playerBlockBadge->setText(QString::number(m_myBlock));

    energyOrbCountLabel->setText(QString("%1/%2").arg(m_myEnergy).arg(m_myMaxEnergy));

    drawPileCountLabel->setText(QString::number(m_drawPile.size()));
    discardPileCountLabel->setText(QString::number(m_discardPile.size()));
    deckCountLabel->setText(QString::number(m_drawPile.size() + m_discardPile.size() + m_exhaustPile.size() + m_myHand.size()));

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
        // FIX: clamp teammate HP display
        teammateHpBar->setFormat(QString("%1 / %2").arg(qMax(0, m_teammateHp)).arg(m_teammateMaxHp));

        teammateBlockBadge->setVisible(m_teammateBlock > 0);
        if (m_teammateBlock > 0) teammateBlockBadge->setText(QString::number(m_teammateBlock));

        teammateDownOverlay->setVisible(!m_teammateAlive);
    }

    if (enemySlots.size() == m_enemies.size()) {
        for (size_t i = 0; i < m_enemies.size(); ++i) {
            const EnemyData &d = m_enemies[i];
            EnemyUISlot &slot = enemySlots[i];

            slot.sprite->setPixmap(getEnemyPixmap(d.name).scaled(220, 195, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            slot.nameLabel->setText(d.name);
            slot.hpBar->setMaximum(d.maxHp);
            slot.hpBar->setValue(qMax(0, d.hp));
            // FIX: clamp enemy HP display
            slot.hpBar->setFormat(QString("%1 / %2").arg(qMax(0, d.hp)).arg(d.maxHp));
            slot.intentLabel->setText(d.intent);
            updateStatusEffectRow(slot.statusRow, d.effects);

            slot.blockBadge->setVisible(d.block > 0);
            if (d.block > 0) slot.blockBadge->setText(QString::number(d.block));
        }
    }

    playerNameLabel->setVisible(true);
    playerNameLabel->setText(m_isLeader ? m_myUsername + " ★" : m_myUsername);

    if (!m_iAmAlive) {
        spectatorLabel->setText("You have fallen — spectating");
        spectatorLabel->show();
        turnIndicatorLabel->setText("");
        m_endTurnBtn->setEnabled(false);
        m_cardsContainer->setEnabled(false);
    } else {
        spectatorLabel->hide();
        if (m_isPlayerTurn) {
            turnIndicatorLabel->setText(QString("★ %1's Turn  |  HP: %2 / %3")
                                            .arg(m_myUsername).arg(qMax(0, m_myHp)).arg(m_myMaxHp));
        } else {
            turnIndicatorLabel->setText("Enemy Turn...");
        }
        m_endTurnBtn->setEnabled(m_isPlayerTurn && !m_hasEndedTurnLocally);
        m_cardsContainer->setEnabled(m_isPlayerTurn);
    }
}

// ================================================================
// Multi-enemy UI construction — mirrors MainWindow::rebuildEnemyUI
// ================================================================
void MPCombatWindow::rebuildEnemyUI()
{
    // FIX: delete hit overlays explicitly (they are parented to 'this', not wrapper)
    for (auto &slot : enemySlots) {
        if (slot.hitOverlay) {
            slot.hitOverlay->deleteLater();
            slot.hitOverlay = nullptr;
        }
    }

    // FIX: remove old wrappers from layout before clearing vector
    while (QLayoutItem *child = enemyAreaLayout->takeAt(0)) {
        if (child->widget()) {
            child->widget()->deleteLater();
        }
        delete child;
    }
    enemySlots.clear();

    for (size_t i = 0; i < m_enemies.size(); ++i) {
        EnemyUISlot slot;

        slot.wrapper = new QWidget(enemyAreaContainer);
        slot.wrapper->setFixedWidth(253);
        QVBoxLayout *vbox = new QVBoxLayout(slot.wrapper);
        vbox->setContentsMargins(6, 0, 6, 0);
        vbox->setSpacing(4);

        slot.nameLabel = new QLabel(slot.wrapper);
        slot.nameLabel->setAlignment(Qt::AlignCenter);
        slot.nameLabel->setFixedHeight(20);
        slot.nameLabel->setFixedWidth(236);
        slot.nameLabel->setStyleSheet(
            "color: white; font-weight: bold; font-size: 13px; "
            "background-color: rgba(0,0,0,140); border-radius: 4px; padding: 2px;");

        slot.intentLabel = new QLabel(slot.wrapper);
        slot.intentLabel->setAlignment(Qt::AlignCenter);
        slot.intentLabel->setFixedHeight(26);
        slot.intentLabel->setFixedWidth(236);
        slot.intentLabel->setStyleSheet(
            "background-color: rgba(20,20,20,190); color: white; border-radius: 8px; "
            "padding: 3px; font-weight: bold;");

        slot.sprite = new QLabel(slot.wrapper);
        slot.sprite->setFixedSize(253, 224);
        slot.sprite->setAlignment(Qt::AlignCenter);
        slot.sprite->setScaledContents(true);
        slot.sprite->setPixmap(getEnemyPixmap(m_enemies[i].name).scaled(253, 224, Qt::KeepAspectRatio, Qt::SmoothTransformation));

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
        slot.hpBar->setFixedWidth(236);
        slot.hpBar->setStyleSheet(playerHpBar->styleSheet());

        slot.statusRow = new QWidget(slot.wrapper);
        QHBoxLayout *srLayout = new QHBoxLayout(slot.statusRow);
        srLayout->setContentsMargins(0, 0, 0, 0);
        srLayout->setSpacing(4);
        srLayout->setAlignment(Qt::AlignCenter);

        vbox->addWidget(slot.nameLabel);
        vbox->addWidget(slot.intentLabel);
        vbox->addWidget(slot.sprite, 0, Qt::AlignCenter);
        vbox->addWidget(slot.hpBar);
        vbox->addWidget(slot.statusRow, 0, Qt::AlignCenter);

        slot.hitOverlay = new QLabel(this);
        slot.hitOverlay->setFixedSize(253, 224);
        slot.hitOverlay->hide();
        slot.hitOpacity = new QGraphicsOpacityEffect(slot.hitOverlay);
        slot.hitOverlay->setGraphicsEffect(slot.hitOpacity);
        slot.hitOpacity->setOpacity(0.0);

        slot.sprite->setProperty("enemyIndex", (int)i);
        slot.hpBar->setProperty("enemyIndex", (int)i);
        slot.intentLabel->setProperty("enemyIndex", (int)i);

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
        retargetEnemy(targetedEnemyIndex);
    }
}

QRect MPCombatWindow::enemySpriteRectFor(int enemyIndex)
{
    if (enemyIndex < 0 || enemyIndex >= (int)enemySlots.size() || !enemySlots[enemyIndex].sprite)
        return QRect();
    QPoint topLeft = enemySlots[enemyIndex].sprite->mapTo(this, QPoint(0, 0));
    return QRect(topLeft, enemySlots[enemyIndex].sprite->size());
}

void MPCombatWindow::retargetEnemy(int enemyIndex)
{
    for (auto &s : enemySlots) {
        if (s.sprite && s.sprite->graphicsEffect()) {
            s.sprite->graphicsEffect()->deleteLater();
            s.sprite->setGraphicsEffect(nullptr);
        }
    }
    if (enemyIndex < 0 || enemyIndex >= (int)enemySlots.size()) return;

    targetedEnemyIndex = enemyIndex;
    QGraphicsDropShadowEffect *glow = new QGraphicsDropShadowEffect(enemySlots[enemyIndex].sprite);
    glow->setBlurRadius(45);
    glow->setColor(QColor(245, 197, 24, 180));
    glow->setOffset(0, 0);
    enemySlots[enemyIndex].sprite->setGraphicsEffect(glow);
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

void MPCombatWindow::updateHandUI()
{
    hideHoverCard();
    highlightedCardIndex = -1;

    QLayout *layout = m_cardsContainer->layout();
    // FIX: null-safe layout check
    if (!layout) return;

    QLayoutItem *child;
    while ((child = layout->takeAt(0)) != nullptr) {
        if (child->widget()) { child->widget()->hide(); delete child->widget(); }
        delete child;
    }

    for (int i = 0; i < m_myHand.size(); ++i) {
        QString cardName = m_myHand[i];
        QPushButton *btn = new QPushButton();
        btn->setFixedSize(140, 180);
        btn->setIcon(QIcon(":/images/cards/" + cardName + ".png"));
        btn->setIconSize(btn->size());
        btn->setProperty("cardName", cardName);
        btn->setProperty("cardImagePath", ":/images/cards/" + cardName + ".png");
        btn->setProperty("cardIndex", i);
        btn->setProperty("cardIsAttack", isAttackCard(cardName));
        btn->installEventFilter(this);

        layout->addWidget(btn);

        QGraphicsOpacityEffect *fadeEffect = new QGraphicsOpacityEffect(btn);
        btn->setGraphicsEffect(fadeEffect);
        fadeEffect->setOpacity(0.0);
        QPropertyAnimation *fadeIn = new QPropertyAnimation(fadeEffect, "opacity", this);
        fadeIn->setDuration(800);
        fadeIn->setStartValue(0.0);
        fadeIn->setEndValue(1.0);
        fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

QPixmap MPCombatWindow::getEnemyPixmap(const QString &enemyName)
{
    QPixmap p(":/images/enemies/" + enemyName + ".png");
    if (p.isNull())
        return QPixmap(":/images/enemy_default.png");
    return p;
}

// ================================================================
// Actions
// ================================================================
void MPCombatWindow::sendPlayCard(const QString &cardName, int targetEnemyIndex)
{
    if (!m_isPlayerTurn || !m_iAmAlive) return;

    cardPlaySoundPlayer->stop();
    cardPlaySoundPlayer->setPosition(0);
    cardPlaySoundPlayer->play();

    QJsonObject msg;
    msg["type"] = "play_card";
    msg["card_name"] = cardName;
    if (targetEnemyIndex >= 0)
        msg["target_enemy_index"] = targetEnemyIndex;
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

    QRect base = enemyAreaContainer->geometry();
    enemyAreaContainer->move(base.x(), m_baseEnemyY - floatOffset);
}

void MPCombatWindow::playHitEffect(QLabel *overlay, QGraphicsOpacityEffect *opacityEffect)
{
    // FIX: stop any existing animation on this overlay to prevent overlap
    QObject *existingAnim = overlay->property("hitAnim").value<QObject*>();
    if (existingAnim) {
        QSequentialAnimationGroup *seq = qobject_cast<QSequentialAnimationGroup*>(existingAnim);
        if (seq) seq->stop();
        existingAnim->deleteLater();
    }

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

    // Store reference so we can stop it if triggered again before finishing
    overlay->setProperty("hitAnim", QVariant::fromValue<QObject*>(seq));

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
    QRect startRect(endRect.x() - 40, endRect.y() - 20, labelW + 80, labelH + 40);

    gameOverLabel->setGeometry(startRect);
    gameOverLabel->show();
    gameOverLabel->raise();
    gameOverOpacityEffect->setOpacity(0.0);

    QPropertyAnimation *fadeAnim = new QPropertyAnimation(gameOverOpacityEffect, "opacity", this);
    fadeAnim->setDuration(900);
    fadeAnim->setStartValue(0.0);
    fadeAnim->setEndValue(1.0);
    fadeAnim->setEasingCurve(QEasingCurve::OutCubic);
    fadeAnim->start(QAbstractAnimation::DeleteWhenStopped);

    QPropertyAnimation *geomAnim = new QPropertyAnimation(gameOverLabel, "geometry", this);
    geomAnim->setDuration(900);
    geomAnim->setStartValue(startRect);
    geomAnim->setEndValue(endRect);
    geomAnim->setEasingCurve(QEasingCurve::OutBack);
    geomAnim->start(QAbstractAnimation::DeleteWhenStopped);
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

    if (hoverGeomAnim) { hoverGeomAnim->stop(); delete hoverGeomAnim; hoverGeomAnim = nullptr; }
    hoverGeomAnim = new QPropertyAnimation(hoverCardLabel, "geometry", this);
    hoverGeomAnim->setDuration(180);
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
    hoverGeomAnim->setDuration(130);
    hoverGeomAnim->setStartValue(hoverCardLabel->geometry());
    hoverGeomAnim->setEndValue(hoverOriginalRect);
    hoverGeomAnim->setEasingCurve(QEasingCurve::InCubic);

    QLabel *labelPtr = hoverCardLabel;
    connect(hoverGeomAnim, &QPropertyAnimation::finished, labelPtr, &QLabel::hide);
    hoverGeomAnim->start();
}

void MPCombatWindow::showEnemyTooltip(int enemyIndex, QWidget *anchorWidget)
{
    if (enemyIndex < 0 || enemyIndex >= (int)m_enemies.size()) {
        customTooltipBox->hide();
        return;
    }

    const EnemyData &d = m_enemies[enemyIndex];

    QString title, desc;
    QString intent = d.intent;

    if (intent.contains("Divider")) {
        title = "Divider";
        desc = "This enemy intends to unleash the <b>Divider</b> attack.";
    } else if (intent.contains("Attack") && intent.contains("Block")) {
        title = "Attack + Defend";
        desc = QString("This enemy intends to <b><span style='color:#ff6b6b;'>Attack</span></b> "
                       "and <b><span style='color:#5ec8ff;'>Defend</span></b> itself.");
    } else if (intent.contains("Attack") && intent.contains("Debuff")) {
        title = "Attack + Debuff";
        desc = QString("This enemy intends to <b><span style='color:#ff6b6b;'>Attack</span></b> "
                       "and <b><span style='color:#c07af0;'>weaken</span></b> you.");
    } else if (intent.contains("Attack") && intent.contains("Add")) {
        title = "Attack + Add Card";
        desc = QString("This enemy intends to <b><span style='color:#ff6b6b;'>Attack</span></b> "
                       "and <b><span style='color:#f5c518;'>add a card to your discard pile</span></b>.");
    } else if (intent.contains("Attack")) {
        title = "Attack";
        desc = QString("This enemy intends to <b><span style='color:#ff6b6b;'>Attack</span></b>.");
    } else if (intent.contains("Defend") || intent.contains("Block")) {
        title = "Defend";
        desc = "This enemy intends to <b><span style='color:#5ec8ff;'>Defend</span></b> itself.";
    } else if (intent.contains("Buff")) {
        title = "Buff";
        desc = "This enemy intends to <b><span style='color:#ffd76a;'>strengthen</span></b> itself.";
    } else if (intent.contains("Debuff")) {
        title = "Debuff";
        desc = "This enemy intends to <b><span style='color:#c07af0;'>weaken</span></b> you.";
    } else if (intent.contains("Special")) {
        title = "Special";
        desc = "This enemy intends to do something special.";
    } else {
        title = d.name;
        desc = d.intent;
    }

    customTooltipBox->setText(QString(
                                  "<div style='font-weight:bold; font-size:14px; color:#f5c518; margin-bottom:6px;'>%1</div>"
                                  "<div>%2</div>").arg(title, desc));
    customTooltipBox->adjustSize();

    QWidget *anchor = anchorWidget ? anchorWidget : this;
    QPoint anchorPos = anchor->mapTo(this, QPoint(0, 0));
    int tipX = anchorPos.x() - customTooltipBox->width() - 20;
    int tipY = anchorPos.y() + 40;
    customTooltipBox->move(tipX, tipY);
    customTooltipBox->show();
    customTooltipBox->raise();
}

void MPCombatWindow::showStatusEffectTooltip(QLabel *badge)
{
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

void MPCombatWindow::updateDragArrow(QPoint fromPoint, QPoint toPoint, bool isOverEnemy)
{
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

void MPCombatWindow::showPlayerTargetFrame()
{
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

void MPCombatWindow::hidePlayerTargetFrame()
{
    playerTargetFrame->hide();
}

void MPCombatWindow::showNotEnoughEnergy()
{
    QLabel *bubble = new QLabel(this);
    QPixmap pix(":/images/not_enough_energy.png");
    bubble->setPixmap(pix.scaled(260, 160, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    bubble->adjustSize();

    QRect playerRect = playerSpriteLabel->geometry();
    int startX = playerRect.center().x() - bubble->width()/2 + 150;
    int startY = playerRect.top() - bubble->height() + 70;
    bubble->move(startX, startY);
    bubble->show();
    bubble->raise();

    auto *effect = new QGraphicsOpacityEffect(bubble);
    bubble->setGraphicsEffect(effect);

    auto *fade = new QPropertyAnimation(effect, "opacity");
    fade->setDuration(2100);
    fade->setStartValue(1.0);
    fade->setEndValue(0.0);

    auto *move = new QPropertyAnimation(bubble, "pos");
    move->setDuration(2100);
    move->setStartValue(QPoint(startX, startY));
    move->setEndValue(QPoint(startX, startY - 45));
    move->setEasingCurve(QEasingCurve::OutQuad);

    auto *group = new QParallelAnimationGroup(this);
    group->addAnimation(move);
    group->addAnimation(fade);
    connect(group, &QParallelAnimationGroup::finished, bubble, &QLabel::deleteLater);
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void MPCombatWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (isDraggingCard && draggedCardIndex >= 0) {
        QPoint mousePos = event->pos();

        bool isAttack = (draggedCardIndex < m_myHand.size()) && isAttackCard(m_myHand[draggedCardIndex]);

        if (isAttack) {
            QLayout *layout = m_cardsContainer->layout();
            QPushButton *originBtn = (draggedCardIndex < layout->count())
                                         ? qobject_cast<QPushButton *>(layout->itemAt(draggedCardIndex)->widget()) : nullptr;

            if (originBtn) {
                QPoint cardTop = originBtn->mapTo(this, QPoint(originBtn->width() / 2, 0));
                dragHoverEnemyIndex = -1;
                for (size_t i = 0; i < enemySlots.size(); ++i) {
                    QRect r = enemySpriteRectFor((int)i);
                    if (r.adjusted(-40, -40, 40, 40).contains(mousePos)) {
                        dragHoverEnemyIndex = (int)i;
                        break;
                    }
                }
                bool overEnemy = (dragHoverEnemyIndex >= 0);
                updateDragArrow(cardTop, mousePos, overEnemy);
            }
        } else if (hoverCardLabel && hoverCardLabel->isVisible()) {
            int w = hoverCardLabel->width();
            int h = hoverCardLabel->height();
            hoverCardLabel->move(mousePos.x() - w / 2, mousePos.y() - h / 2 - 40);
            hoverCardLabel->raise();
        }
    }
    QWidget::mouseMoveEvent(event);
}

void MPCombatWindow::showCardPileOverlay(const QString &title, const QStringList &cards, const QString &titleColor)
{
    pileOpenSoundPlayer->setPosition(0);
    pileOpenSoundPlayer->play();

    QGridLayout *gridLayout = qobject_cast<QGridLayout *>(pileCardsContainer->layout());
    // FIX: null-safe
    if (!gridLayout) return;

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
        cardImgLabel->setPixmap(QPixmap(":/images/cards/" + cardName + ".png").scaled(120, 160, Qt::KeepAspectRatio, Qt::SmoothTransformation));
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

void MPCombatWindow::updateCardHighlight()
{
    QLayout *layout = m_cardsContainer->layout();
    if (!layout) return;
    QPushButton *highlightedBtn = nullptr;

    for (int i = 0; i < layout->count(); ++i) {
        QPushButton *btn = qobject_cast<QPushButton*>(layout->itemAt(i)->widget());
        if (!btn) continue;
        if (i == highlightedCardIndex) {
            btn->setStyleSheet("border: 3px solid #f5c518; border-radius: 6px;");
            highlightedBtn = btn;
        } else {
            btn->setStyleSheet("border: none;");
        }
    }

    if (highlightedBtn)
        showHoverCard(highlightedBtn);
    else
        hideHoverCard();
}

// ================================================================
// Event filter — hover states, drag-to-target, pile clicks, tooltips, HUD buttons
// ================================================================
bool MPCombatWindow::eventFilter(QObject *obj, QEvent *event)
{
    QPushButton *cardBtn = qobject_cast<QPushButton *>(obj);
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
            if (!m_isPlayerTurn || !m_iAmAlive) return true;

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

                QMouseEvent *me = static_cast<QMouseEvent *>(event);
                QPoint mouseInWindow = cardBtn->mapTo(this, me->pos());
                hoverCardLabel->setGeometry(mouseInWindow.x() - bigW / 2, mouseInWindow.y() - bigH / 2 - 40, bigW, bigH);
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
            QString cardName = cardBtn->property("cardName").toString();

            QMouseEvent *me = static_cast<QMouseEvent *>(event);
            QPoint globalRelease = cardBtn->mapToGlobal(me->pos());
            QPoint localRelease = this->mapFromGlobal(globalRelease);

            if (isAttack) {
                if (dragHoverEnemyIndex >= 0)
                    sendPlayCard(cardName, dragHoverEnemyIndex);
            } else {
                QRect playerRect = playerSpriteLabel->geometry().adjusted(-60, -60, 60, 60);
                if (playerRect.contains(localRelease))
                    sendPlayCard(cardName, -1);
            }
            dragHoverEnemyIndex = -1;
            return true;
        }
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
    if (obj == deckIconLabel && event->type() == QEvent::MouseButtonRelease) {
        QStringList fullDeck = m_drawPile + m_discardPile + m_exhaustPile + m_myHand;
        showCardPileOverlay("Deck", fullDeck, "#66ccff");
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

    if (obj == settingsOverlayImage && event->type() == QEvent::MouseButtonRelease) {
        settingsOverlayImage->hide();
        return true;
    }

    if (obj == mapLabel && event->type() == QEvent::MouseButtonRelease) {
        // No-op — parity with MainWindow (MainWindow does not handle mapLabel click)
        return false;
    }

    if (obj == chatIconLabel && event->type() == QEvent::MouseButtonRelease) {
        toggleChatPanel();
        return true;
    }

    // Enemy click-to-target + hover tooltip + hover glow
    for (size_t i = 0; i < enemySlots.size(); ++i) {
        EnemyUISlot &s = enemySlots[i];
        if (obj == s.sprite || obj == s.hpBar || obj == s.intentLabel) {
            if (event->type() == QEvent::MouseButtonRelease) {
                retargetEnemy((int)i);
                return true;
            }
            if (event->type() == QEvent::Enter) {
                showEnemyTooltip((int)i, qobject_cast<QWidget *>(obj));
                if ((int)i != targetedEnemyIndex && s.sprite) {
                    QGraphicsDropShadowEffect *glow = new QGraphicsDropShadowEffect(s.sprite);
                    glow->setBlurRadius(25);
                    glow->setColor(QColor(200, 200, 200, 150));
                    glow->setOffset(0, 0);
                    s.sprite->setGraphicsEffect(glow);
                }
                return false;
            }
            if (event->type() == QEvent::Leave) {
                customTooltipBox->hide();
                if ((int)i != targetedEnemyIndex) {
                    if (s.sprite && s.sprite->graphicsEffect()) {
                        s.sprite->graphicsEffect()->deleteLater();
                        s.sprite->setGraphicsEffect(nullptr);
                    }
                } else {
                    retargetEnemy(targetedEnemyIndex);
                }
                return false;
            }
        }
    }

    return QWidget::eventFilter(obj, event);
}