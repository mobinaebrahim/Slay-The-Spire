#ifndef STATUSCARD_H
#define STATUSCARD_H

#include "card.h"
#include "BattleManager.h"
#include <iostream>
#include <string>

using namespace std;

class StatusCard : public Card {
public:
    StatusCard(CardType type, string name, string desc, int cost = -1);
    virtual void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
    virtual bool isPlayable() const override;
};

class DazeCard : public StatusCard {
public:
    DazeCard();
    void upgrade() override;
    void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

class SlimeCard : public StatusCard {
public:
    SlimeCard();
    void upgrade() override;
    void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
    bool isPlayable() const override;
};

class WoundCard : public StatusCard {
public:
    WoundCard();
    void upgrade() override;
    void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

class BurnCard : public StatusCard {
public:
    BurnCard();
    void upgrade() override;
    void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

#endif