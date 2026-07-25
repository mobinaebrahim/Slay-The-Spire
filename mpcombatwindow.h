#ifndef MPCOMBATPAGE_H
#define MPCOMBATPAGE_H

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <cmath>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QProgressBar>
#include <QGraphicsOpacityEffect>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QPushButton>
#include <QScrollArea>
#include <QJsonObject>
#include <QJsonArray>
#include <QStringList>

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

private:
    bool m_isLeader = false;
    QString m_myUsername;
    QString m_teammateUsername;

    int m_myHp = 0, m_myMaxHp = 0, m_myEnergy = 0, m_myMaxEnergy = 0, m_myBlock = 0;
    bool m_iAmAlive = true;
    bool m_isPlayerTurn = false;
    bool m_hasEndedTurnLocally = false;
    QStringList m_myHand;
    QStringList m_drawPile, m_discardPile, m_exhaustPile;
    QJsonArray m_myEffects;

    bool m_hasTeammate = false;
    int m_teammateHp = 0, m_teammateMaxHp = 0, m_teammateBlock = 0;
    bool m_teammateAlive = true;

    QString m_enemyName;
    int m_enemyHp = 0, m_enemyMaxHp = 0, m_enemyBlock = 0;
    QString m_enemyIntent;
    QJsonArray m_enemyEffects;

    int m_lastMyHp = -1, m_lastTeammateHp = -1, m_lastEnemyHp = -1;

    bool m_isGameOver = false;
    float m_angle = 0;
    int m_basePlayerY = 0, m_baseTeammateY = 0, m_baseEnemyY = 0;
    int m_playerX = 0, m_teammateX = 0, m_enemyX = 0;

    QLabel *backgroundLabel;
    QLabel *playerSpriteLabel;
    QProgressBar *playerHpBar;
    QLabel *playerBlockBadge;
    QLabel *playerEnergyOrb;
    QLabel *energyOrbCountLabel;
    QWidget *playerStatusRow;
    QLabel *playerHitOverlay;
    QGraphicsOpacityEffect *playerHitOpacity;

    QLabel *teammateSpriteLabel;
    QProgressBar *teammateHpBar;
    QLabel *teammateNameLabel;
    QLabel *teammateBlockBadge;
    QLabel *teammateDownOverlay;
    QWidget *teammateStatusRow;
    QLabel *teammateHitOverlay;
    QGraphicsOpacityEffect *teammateHitOpacity;

    QLabel *enemySpriteLabel;
    QProgressBar *enemyHpBar;
    QLabel *enemyBlockBadge;
    QLabel *enemyIntentLabel;
    QLabel *enemyNameLabel;
    QWidget *enemyStatusRow;
    QLabel *enemyHitOverlay;
    QGraphicsOpacityEffect *enemyHitOpacity;

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

    QLabel *gameOverLabel;
    QGraphicsOpacityEffect *gameOverOpacityEffect;

    QLabel *customTooltipBox;
    QLabel *toastLabel;

    QLabel *hoverCardLabel;
    QPropertyAnimation *hoverGeomAnim = nullptr;
    QRect hoverOriginalRect;

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
    void sendPlayCard(const QString &cardName);
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
    void showEnemyTooltip();
    void showStatusEffectTooltip(QLabel *badge);
    void updateAnimations();

    void showCardPileOverlay(const QString &title, const QStringList &cards, const QString &titleColor);
    void hidePileOverlay();
};

#endif // MPCOMBATPAGE_H