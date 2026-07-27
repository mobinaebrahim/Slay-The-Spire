#ifndef POTION_H
#define POTION_H

#include <string>
#include "character.h"
#include "BattleManager.h"
using namespace std;

class Potion {
protected:
    string name;
    string description;
public:
    Potion(string n, string d);
    virtual ~Potion() = default;
    virtual void applyEffect(Character* caster, Character* target, BattleManager* bm) = 0;
    string getName() const;
    string getDescription() const;
    virtual bool isPlayable() const;
};

class BlockPotion : public Potion { 
public:
    BlockPotion();
    void applyEffect(Character* caster, Character* target, BattleManager* bm) override;
};

class FirePotion : public Potion { 
public:
    FirePotion();
    void applyEffect(Character* caster, Character* target, BattleManager* bm) override;
};

class EnergyPotion : public Potion { 
public:
    EnergyPotion();
    void applyEffect(Character* caster, Character* target, BattleManager* bm) override;
};

class SwiftPotion : public Potion { 
public:
    SwiftPotion();
    void applyEffect(Character* caster, Character* target, BattleManager* bm) override;
};

class FairyInABottle : public Potion { 
public:
    FairyInABottle();
    void applyEffect(Character* caster, Character* target, BattleManager* bm) override;
};

#endif