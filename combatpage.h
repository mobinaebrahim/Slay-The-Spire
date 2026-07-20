#ifndef COMBATPAGE_H
#define COMBATPAGE_H

#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QJsonObject>
#include <QJsonArray>
#include "networkmanager.h"
#include "BattleManager.h"
#include "player.h"
#include "enemy.h"
#include "NormalEnemies.h"

class CombatPage : public QWidget
{
    Q_OBJECT
public:
    explicit CombatPage(QWidget *parent = nullptr, bool isLeader = true);
    ~CombatPage();

private:
    bool m_isLeader;
    BattleManager *m_battleManager;
    Player *m_localPlayer;   // بازیکن خودم (این کلاینت)

    // UI - خودم
    QProgressBar *m_myHpBar;
    QLabel *m_myEnergyLabel;
    QWidget *m_myCardsContainer;

    // UI - هم‌تیمی (فقط نمایشی)
    QProgressBar *m_teammateHpBar;
    QLabel *m_teammateNameLabel;

    // UI - دشمن (مشترک)
    QProgressBar *m_enemyHpBar;
    QLabel *m_enemyNameLabel;

    QPushButton *m_endTurnBtn;

    void setupCombat();
    void buildUI();
    void updateAllUI();
    void updateMyHandUI();

    void handleNetworkMessage(const QJsonObject &obj);

signals:
    void combatFinished(bool victory);
};

#endif // COMBATPAGE_H