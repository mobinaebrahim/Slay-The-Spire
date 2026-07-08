#ifndef POWERCARD_H
#define POWERCARD_H

#include "card.h"
#include "BattleManager.h"
#include <string>
#include <iostream>

using namespace std;

class PowerCard : public Card {
public:
    PowerCard(CardType type, string name, string desc, int cost);
    void upgrade() override;
    virtual void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

class InflameCard : public PowerCard {
public:
    InflameCard();
    void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

class MetallicizeCard : public PowerCard {
public:
    MetallicizeCard();
    void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

class DemonFormCard : public PowerCard {
public:
    DemonFormCard();
    void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

class BrutalityCard : public PowerCard {
public:
    BrutalityCard();
    void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

class FeelNoPainCard : public PowerCard {
public:
    FeelNoPainCard();
    void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

class BarricadeCard : public PowerCard {
public:
    BarricadeCard();
    void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

#endif