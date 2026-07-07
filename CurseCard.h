#ifndef CURSECARD_H
#define CURSECARD_H

#include "Card.h"
#include "BattleManager.h"
#include <iostream>
#include <string>

using namespace std;

class CurseCard : public Card {
public:
    CurseCard(CardType type, string name, string desc, int cost = -1);
    virtual void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
    virtual bool isPlayable() const override;
};

class JAXCard : public CurseCard {
public:
    JAXCard();
    void upgrade() override;
    void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
    bool isPlayable() const override;
};

class CurseOfBellCard : public CurseCard {
public:
    CurseOfBellCard();
    void upgrade() override;
    void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

class RegretCard : public CurseCard {
public:
    RegretCard();
    void upgrade() override;
    void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

#endif