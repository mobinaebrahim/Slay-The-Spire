#ifndef CAMPFIRESCREEN_H
#define CAMPFIRESCREEN_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QStackedWidget>
#include <QResizeEvent>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include "../Player.h"

class CampfireScreen : public QWidget {
    Q_OBJECT
public:
    explicit CampfireScreen(Player* player, QWidget* parent = nullptr);

signals:
    void finished();

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onRestClicked();
    void onSmithClicked();
    void onLiftClicked();
    void onCardUpgradeClicked(int index);
    void onRestContinue();
    void onCancelSmith();

private:
    Player* player;
    QLabel* backgroundLabel;
    QWidget* menuWidget;
    QPushButton* restBtn;
    QPushButton* smithBtn;
    QPushButton* liftBtn;

    QWidget* restWidget;
    QLabel* restBgLabel;
    QLabel* restInfoLabel;
    QPushButton* restContinueBtn;

    QWidget* smithWidget;
    QLabel* smithTitle;
    QWidget* smithCardsContainer;
    QPushButton* smithCancelBtn;

    void setupUI();
    void updateButtonStates();
    void showMenu();
    void showRest();
    void showSmith();
    void buildSmithCards();
};

#endif
