#ifndef SKILLCARD_H
#define SKILLCARD_H

#include "Card.h"
#include "BattleManager.h"
#include <iostream>
#include <string>

using namespace std;

class SkillCard : public Card {
protected:
    int baseBlock; 
public:
    SkillCard(CardType type, string name, string description, int cost, int baseBlock);
    virtual ~SkillCard() = default;
    virtual void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
    int getBaseBlock() const;
};

class DefendCard : public SkillCard {
public:
    DefendCard();
    void upgrade() override;
    void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

class ExhumeCard : public SkillCard {
public:
    ExhumeCard();
    void upgrade() override;
    int getCost(Character* caster) override;
    void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

class LimitBreakCard : public SkillCard {
public:
    LimitBreakCard();
    void upgrade() override;
    void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

class OfferingCard : public SkillCard {
public:
    OfferingCard();
    void upgrade() override;
    void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

class ImperviousCard : public SkillCard {
public:
    ImperviousCard();
    void upgrade() override;
    void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

class DualWieldCard : public SkillCard {
public:
    DualWieldCard();
    void upgrade() override;
    void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

class EntrenchCard : public SkillCard {
public:
    EntrenchCard();
    void upgrade() override;
    int getCost(Character* caster) override;
    void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

class ShrugItOffCard : public SkillCard {
public:
    ShrugItOffCard();
    void upgrade() override;
    void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

class BloodlettingCard : public SkillCard {
public:
    BloodlettingCard();
    void upgrade() override;
    void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

class DisarmCard : public SkillCard {
public:
    DisarmCard();
    void upgrade() override;
    void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

#endif