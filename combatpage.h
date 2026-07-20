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
#include <QStringList>
#include "networkmanager.h"

class CombatPage : public QWidget
{
    Q_OBJECT
public:
    explicit CombatPage(QWidget *parent = nullptr, bool isLeader = true);
    ~CombatPage();

private:
    bool m_isLeader;

    // --- وضعیت خودم (از سرور میاد) ---
    int m_myHp = 0;
    int m_myMaxHp = 0;
    int m_myEnergy = 0;
    int m_myMaxEnergy = 0;
    QStringList m_myHand;

    // --- وضعیت هم‌تیمی (از سرور میاد) ---
    int m_teammateHp = 0;
    int m_teammateMaxHp = 0;

    // --- وضعیت دشمن (از سرور میاد) ---
    QString m_enemyName;
    int m_enemyHp = 0;
    int m_enemyMaxHp = 0;

    // UI - خودم
    QProgressBar *m_myHpBar;
    QLabel *m_myEnergyLabel;
    QWidget *m_myCardsContainer;

    // UI - هم‌تیمی
    QProgressBar *m_teammateHpBar;
    QLabel *m_teammateNameLabel;

    // UI - دشمن
    QProgressBar *m_enemyHpBar;
    QLabel *m_enemyNameLabel;

    QPushButton *m_endTurnBtn;

    void setupCombat();
    void buildUI();
    void updateAllUI();
    void updateMyHandUI();

    void sendPlayCard(const QString &cardName);

    void handleNetworkMessage(const QJsonObject &obj);

signals:
    void combatFinished(bool victory);
};

#endif // COMBATPAGE_H