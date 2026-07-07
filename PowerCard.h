#ifndef POWERCARD_H
#define POWERCARD_H

#include "Card.h"
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
    void upgrade() override;
    void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

class MetallicizeCard : public PowerCard {
public:
    MetallicizeCard();
    void upgrade() override;
    void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

class DemonFormCard : public PowerCard {
public:
    DemonFormCard();
    void upgrade() override;
    void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

class BrutalityCard : public PowerCard {
public:
    BrutalityCard();
    void upgrade() override;
    void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

class FeelNoPainCard : public PowerCard {
public:
    FeelNoPainCard();
    void upgrade() override;
    void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

class BarricadeCard : public PowerCard {
public:
    BarricadeCard();
    void upgrade() override;
    void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

#endif