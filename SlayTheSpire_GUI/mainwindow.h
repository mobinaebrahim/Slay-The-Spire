#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QTimer>
#include <cmath>
#include <vector>
#include <string>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QProgressBar>
#include <QGraphicsOpacityEffect>
#include <QGraphicsDropShadowEffect>
#include <QSoundEffect>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QSequentialAnimationGroup>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QPainter>
#include <QPen>
#include <QPushButton>
#include <QScrollArea>
#include <QGridLayout>
#include <QParallelAnimationGroup>
#include <QShortcut>
#include "../Player.h"

class Card;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void on_EndTurnButton_clicked();

private:
    Ui::MainWindow *ui;
    std::vector<Card*> playerHand;
    BattleManager* battleManager;
    Player* playerObject;
    void updateHandUI();
    void drawRandomCards(int numberOfCards);
    void initializePlayerDeck(int totalCards);
    bool isGameOver = false;
    void checkGameOver();

    QLabel* backgroundLabel;
    QLabel* enemyLabel;
    QLabel* playerSpriteLabel;
    QLabel* enemySpriteLabel;
    QProgressBar* playerHpBar;
    QLabel* playerBlockBadge;
    QWidget* topHudBar;
    QLabel* playerHeartIcon;
    QLabel* playerHpTopLabel;

    QProgressBar* enemyHpBar;
    QLabel* enemyBlockBadge;
    QLabel* enemyIntentLabel;
    QLabel* enemyNameLabel;

    QLabel* playerEnergyOrb;
    QLabel* energyOrbCountLabel;

    QLabel* goldIconLabel;
    QLabel* goldCountLabel;

    QLabel* drawPileIconLabel;
    QLabel* drawPileCountLabel;
    QLabel* discardPileIconLabel;
    QLabel* discardPileCountLabel;
    QLabel* exhaustPileBadge;

    QScrollArea* pileScrollArea;
    QWidget* exhaustCardsContainer;
    QLabel* pileOverlayTitle;
    QWidget* exhaustPileOverlay;
    QPushButton* closeExhaustOverlayButton;
    QWidget* discardWrapper;

    void showCardPileOverlay(const QString& title, const vector<Card*>& cards, const QString& titleColor);
    void hidePileOverlay();

    QWidget* playerStatusRow;
    QWidget* enemyStatusRow;
    void updateStatusEffectRow(QWidget* rowWidget, Character* character);
    void showStatusEffectTooltip(QLabel* badge);

    QPropertyAnimation* hoverGeomAnim = nullptr;
    QRect hoverOriginalRect;
    QLabel* hoverCardLabel = nullptr;
    QPixmap hoverCardPixmap;
    QPropertyAnimation* hoverAnim = nullptr;
    void showHoverCard(QPushButton* originalBtn);
    void hideHoverCard();

    void showFloatingDamage(QRect targetRect, int amount, const QColor& color);

    void showToastMessage(const QString& text);
    QLabel* toastLabel;

    QMediaPlayer* cardPlaySoundPlayer;
    QAudioOutput* cardPlaySoundOutput;

    QMediaPlayer* endTurnHoverSoundPlayer;
    QAudioOutput* endTurnHoverSoundOutput;

    QMediaPlayer* pileOpenSoundPlayer;
    QAudioOutput* pileOpenSoundOutput;

    QLabel* deckIconLabel;
    QLabel* deckCountLabel;
    QLabel* settingLabel;
    QLabel* mapLabel;

    int highlightedCardIndex = -1;
    void updateCardHighlight();
    void playCardAtIndex(int index);
    void setupShortcuts();

protected:

    QLabel* gameOverLabel;
    QGraphicsOpacityEffect* gameOverOpacityEffect;
    void showGameOverText(const QString& text, const QColor& color);

    QLabel* customTooltipBox;
    void showEnemyTooltip(Enemy* enemy);

    QMediaPlayer* bgMusicPlayer;
    QAudioOutput* bgAudioOutput;
    QMediaPlayer* hitSoundPlayer;
    QAudioOutput* hitSoundOutput;

    QLabel* playerHitOverlay;
    QGraphicsOpacityEffect* playerHitOpacity;
    QLabel* enemyHitOverlay;
    QGraphicsOpacityEffect* enemyHitOpacity;

    void playHitEffect(QLabel* overlay, QGraphicsOpacityEffect* opacityEffect);
    bool isAttackAnimating = false;

    void playEnemyAttack();

    QTimer* animationTimer;
    float angle = 0;
    int basePlayerY;
    int baseEnemyY;
    int currentStartX;
    int playerX;
    int enemyX;

    void updateCharacterUI();
    QPixmap getEnemyPixmap(const QString& enemyName);
    void updateAnimations();

    void playEnemyNonAttackTurn();

    void disableAllCards();

protected:
    void resizeEvent(QResizeEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;
};

Card* createCardByName(const std::string& name);

#endif