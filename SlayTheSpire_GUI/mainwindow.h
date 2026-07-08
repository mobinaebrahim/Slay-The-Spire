#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <vector>
#include <string>
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
    void on_cardButton_1_clicked();
    void on_cardButton_2_clicked();
    void on_cardButton_3_clicked();
    void on_cardButton_4_clicked();
    void on_cardButton_5_clicked();
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

protected:
    void resizeEvent(QResizeEvent* event) override;
};

Card* createCardByName(const std::string& name);

#endif