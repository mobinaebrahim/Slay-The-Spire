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
#include <QMouseEvent>
#include <QPainterPath>
#include "player.h"
#include "combattype.h"
#include "mappage.h"

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

    MainWindow(QWidget *parent, int initialHp, int maxHp,
               int initialGold, const std::vector<std::string>& deckNames,
               const std::vector<std::string>& potionNames = {},
               CombatType combatType = CombatType::Normal);

    ~MainWindow() override;

signals:
    void combatFinished(bool victory, int finalHp, int maxHp,
                        int finalGold, const std::vector<std::string>& finalDeck,
                        const std::vector<std::string>& finalPotions);
private slots:
    void on_EndTurnButton_clicked();

private:
    Ui::MainWindow *ui;

    struct EnemyUISlot {
        Enemy* enemy = nullptr;
        QWidget* wrapper = nullptr;
        QLabel* sprite = nullptr;
        QProgressBar* hpBar = nullptr;
        QLabel* blockBadge = nullptr;
        QLabel* intentLabel = nullptr;
        QLabel* nameLabel = nullptr;
        QWidget* statusRow = nullptr;
    };

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
    void playCardAtIndex(int index, Enemy* explicitTarget = nullptr);
    void setupShortcuts();

    bool isDraggingCard = false;
    int draggedCardIndex = -1;
    QPoint dragPreviewOffset;
    QLabel* dragArrowLabel;
    void updateDragArrow(QPoint fromPoint, QPoint toPoint);

    QLabel* playerTargetFrame;
    void showPlayerTargetFrame();
    void hidePlayerTargetFrame();

    QLabel* settingsOverlayImage;

    void showNotEnoughEnergy();

    QWidget* enemyAreaContainer;
    QHBoxLayout* enemyAreaLayout;
    std::vector<EnemyUISlot> enemySlots;
    int targetedEnemyIndex = 0;
    Enemy* dragHoverTarget = nullptr;

    void rebuildEnemyUI();
    Enemy* enemyFromWidget(QObject* obj);
    Enemy* findEnemySlotAt(const QPoint& windowPos);

    std::vector<Enemy*> enemyTurnQueue;
    int enemyTurnQueueIndex = 0;

    void processNextEnemyInQueue();
    void highlightAttackingEnemy(Enemy* enemy);
    void unhighlightAttackingEnemy(Enemy* enemy);
    EnemyUISlot* findSlotFor(Enemy* enemy);
    QRect enemySpriteLabelRectFor(Enemy* enemy);

    void spawnNormalEncounter();
    void spawnEliteEncounter();
    void spawnBossEncounter();

    QWidget* potionRow = nullptr;
    void updatePotionUI();
    void usePotionAtIndex(int index);

protected:
    void mouseMoveEvent(QMouseEvent* event) override;

    QLabel* gameOverLabel;
    QGraphicsOpacityEffect* gameOverOpacityEffect;
    void showGameOverText(const QString& text, const QColor& color);

    QLabel* customTooltipBox;
    void showEnemyTooltip(Enemy* enemy, QWidget* anchorWidget = nullptr);

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

    QTimer* animationTimer;
    float angle = 0;
    int basePlayerY;
    int baseEnemyY;
    int currentStartX;
    int playerX;
    int enemyX;

    void updateCharacterUI();
    QPixmap getEnemyPixmap(Enemy* enemy);
    void updateAnimations();

    void disableAllCards();

    void resizeEvent(QResizeEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;
    void updateDragArrow(QPoint fromPoint, QPoint toPoint, bool isOverEnemy);
};

Card* createCardByName(const std::string& name);

#endif