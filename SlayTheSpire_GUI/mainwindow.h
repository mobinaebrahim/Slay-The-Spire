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
    QLabel* backgroundLabel;
    QLabel* enemyLabel;
    void updateHandUI();
    void drawRandomCards(int numberOfCards);
    void initializePlayerDeck(int totalCards);

    QGroupBox* playerInfoGroup;
    QGroupBox* enemyInfoGroup;
    QVBoxLayout* enemyListLayout;
    QLabel* playerSpriteLabel;
    QLabel* enemySpriteLabel;
    QLabel* playerNameLabel;
    QLabel* playerHpLabel;
    QLabel* playerBlockLabel;
    QLabel* playerEnergyLabel;
    QLabel* playerStatusLabel;

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

protected:
    void resizeEvent(QResizeEvent* event) override;
};

Card* createCardByName(const std::string& name);

#endif