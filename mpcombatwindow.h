#ifndef MPCOMBATWINDOW_H
#define MPCOMBATWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <cmath>
#include <vector>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QProgressBar>
#include <QGraphicsOpacityEffect>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QPushButton>
#include <QScrollArea>
#include <QShortcut>
#include <QMouseEvent>
#include <QJsonObject>
#include <QJsonArray>
#include <QStringList>
#include <QListWidget>
#include <QLineEdit>

QT_BEGIN_NAMESPACE
namespace Ui {
class MPCombatWindow;
}
QT_END_NAMESPACE


class MPCombatWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MPCombatWindow(QWidget *parent = nullptr, bool isLeader = true);
    ~MPCombatWindow() override;

signals:
    void combatFinished(bool victory);

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    Ui::MPCombatWindow *ui;

    bool m_isLeader = false;
    QString m_myUsername;
    QString m_teammateUsername;

    int m_myHp = 0, m_myMaxHp = 0, m_myEnergy = 0, m_myMaxEnergy = 0, m_myBlock = 0;
    int m_myGold = 0;
    bool m_iAmAlive = true;
    bool m_isPlayerTurn = false;
    bool m_hasEndedTurnLocally = false;
    QStringList m_myHand;
    QStringList m_drawPile, m_discardPile, m_exhaustPile;
    QJsonArray m_myEffects;

    bool m_hasTeammate = false;
    int m_teammateHp = 0, m_teammateMaxHp = 0, m_teammateBlock = 0;
    bool m_teammateAlive = true;

    struct EnemyData {
        QString name;
        int hp = 0, maxHp = 0, block = 0;
        QString intent;
        QJsonArray effects;
    };
    std::vector<EnemyData> m_enemies;
    std::vector<int> m_lastEnemyHps;

    struct EnemyUISlot {
        QWidget *wrapper = nullptr;
        QLabel *sprite = nullptr;
        QProgressBar *hpBar = nullptr;
        QLabel *blockBadge = nullptr;
        QLabel *intentLabel = nullptr;
        QLabel *nameLabel = nullptr;
        QWidget *statusRow = nullptr;
        QLabel *hitOverlay = nullptr;
        QGraphicsOpacityEffect *hitOpacity = nullptr;
    };
    QWidget *enemyAreaContainer;
    QHBoxLayout *enemyAreaLayout;
    std::vector<EnemyUISlot> enemySlots;
    int targetedEnemyIndex = 0;
    int dragHoverEnemyIndex = -1;

    void rebuildEnemyUI();
    QRect enemySpriteRectFor(int enemyIndex);
    void retargetEnemy(int enemyIndex);

    int m_lastMyHp = -1, m_lastTeammateHp = -1;

    bool m_isGameOver = false;
    float m_angle = 0;
    int m_basePlayerY = 0, m_baseTeammateY = 0, m_baseEnemyY = 0;
    int m_playerX = 0, m_teammateX = 0;
    int currentStartX = 0;

    QLabel *backgroundLabel;
    QLabel *playerSpriteLabel;
    QProgressBar *playerHpBar;
    QLabel *playerBlockBadge;
    QLabel *playerEnergyOrb;
    QLabel *energyOrbCountLabel;
    QWidget *playerStatusRow;
    QLabel *playerHitOverlay;
    QGraphicsOpacityEffect *playerHitOpacity;

    QLabel *playerHeartIcon;
    QLabel *playerHpTopLabel;

    QLabel *goldIconLabel;
    QLabel *goldCountLabel;
    QLabel *deckIconLabel;
    QLabel *deckCountLabel;
    QLabel *mapLabel;
    QLabel *settingLabel;
    QLabel *settingsOverlayImage;

    QLabel *teammateSpriteLabel;
    QProgressBar *teammateHpBar;
    QLabel *teammateNameLabel;
    QLabel *teammateBlockBadge;
    QLabel *teammateDownOverlay;
    QWidget *teammateStatusRow;
    QLabel *teammateHitOverlay;
    QGraphicsOpacityEffect *teammateHitOpacity;

    QLabel *drawPileIconLabel, *drawPileCountLabel;
    QLabel *discardPileIconLabel, *discardPileCountLabel;
    QLabel *exhaustPileBadge;
    QWidget *discardWrapper;

    QWidget *pileOverlay;
    QLabel *pileOverlayTitle;
    QWidget *pileCardsContainer;
    QScrollArea *pileScrollArea;
    QPushButton *closePileOverlayButton;

    QWidget *topHudBar;
    QLabel *turnIndicatorLabel;
    QLabel *spectatorLabel;
    QWidget *m_cardsContainer;
    QPushButton *m_endTurnBtn;

    // --- In-combat team chat ---
    QLabel *chatIconLabel;
    QLabel *chatUnreadBadge;
    QWidget *chatPanel;
    QListWidget *chatMessagesList;
    QLineEdit *chatInputField;
    QPushButton *chatSendButton;
    bool m_chatOpen = false;
    int m_unreadChatCount = 0;
    void toggleChatPanel();
    void sendChatMessage();
    void handleChatMessage(const QJsonObject &obj);
    void appendChatMessage(const QString &username, const QString &text, bool isMe);

    QLabel *gameOverLabel;
    QGraphicsOpacityEffect *gameOverOpacityEffect;

    QLabel *customTooltipBox;
    QLabel *toastLabel;

    QLabel *hoverCardLabel;
    QPropertyAnimation *hoverGeomAnim = nullptr;
    QRect hoverOriginalRect;

    // Drag-to-target
    bool isDraggingCard = false;
    int draggedCardIndex = -1;
    QLabel *dragArrowLabel;
    QLabel *playerTargetFrame;
    void updateDragArrow(QPoint fromPoint, QPoint toPoint, bool isOverEnemy);
    void showPlayerTargetFrame();
    void hidePlayerTargetFrame();
    void showNotEnoughEnergy();
    void setupShortcuts();

    // ============================================================
    // Keyboard highlight navigation (parity with MainWindow)
    // ============================================================
    int highlightedCardIndex = -1;
    void updateCardHighlight();

    // ============================================================
    // Audio
    // ============================================================
    QMediaPlayer *bgMusicPlayer; QAudioOutput *bgAudioOutput;
    QMediaPlayer *hitSoundPlayer; QAudioOutput *hitSoundOutput;
    QMediaPlayer *cardPlaySoundPlayer; QAudioOutput *cardPlaySoundOutput;
    QMediaPlayer *endTurnHoverSoundPlayer; QAudioOutput *endTurnHoverSoundOutput;
    QMediaPlayer *pileOpenSoundPlayer; QAudioOutput *pileOpenSoundOutput;

    QTimer *animationTimer;

    void buildUI();
    void setupAudio();

    void handleNetworkMessage(const QJsonObject &obj);
    void handleStateUpdate(const QJsonObject &obj);
    void handleCombatOver(const QJsonObject &obj);
    void handleLeaderChanged(const QJsonObject &obj);
    void sendPlayCard(const QString &cardName, int targetEnemyIndex = -1);
    void sendEndTurn();

    void updateHandUI();
    void updateCharacterUI();
    void updateStatusEffectRow(QWidget *rowWidget, const QJsonArray &effects);
    QPixmap getEnemyPixmap(const QString &enemyName);

    void showHoverCard(QPushButton *originalBtn);
    void hideHoverCard();
    void showFloatingDamage(QRect targetRect, int amount, const QColor &color);
    void showToastMessage(const QString &text);
    void showGameOverText(const QString &text, const QColor &color);
    void playHitEffect(QLabel *overlay, QGraphicsOpacityEffect *opacityEffect);
    void showEnemyTooltip(int enemyIndex, QWidget *anchorWidget = nullptr);
    void showStatusEffectTooltip(QLabel *badge);
    void updateAnimations();

    void showCardPileOverlay(const QString &title, const QStringList &cards, const QString &titleColor);
    void hidePileOverlay();
};

#endif // MPCOMBATWINDOW_H