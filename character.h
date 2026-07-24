/*#ifndef CHARACTER_H
#define CHARACTER_H

#include <string>
#include <vector>
#include "StatusEffect.h"
#include <iostream>
using namespace std;

class Character {
protected:
    string name;
    int hp;
    int maxHp;
    int block;
    int timesDamagedThisCombat; 
    bool retainBlock = false;
    vector<StatusEffect*> effects;

public:
    Character(string name, int hp, int maxhp);
    virtual ~Character() = default;

    void decreaseHp(int amount);
    virtual int takeDamage(int incomingDamage);
    void addBlock(int amount);
    void increaseHP(int amount);
    void applyStatus(StatusEffect* newEffect);
    void keep_block_until_next_turn();
    virtual void TurnStartEffect(string effect);
    void setBlock(int amount);
    bool hasEffect(string effectName);

    int getStatusValue(string status);
    int calculate_total_block(int amount);
    int getTimesDamagedThisCombat() const;

    int calculateOutgoingDamage(int baseDamage);
    int calculateIncomingDamage(int baseDamage);

    void applyTurnStartEffects();
    void applyTurnEndEffects();

    int getHp() const { return hp; }
    int getBlock() const { return block; }
    string getName() const { return name; }
    int getMaxHp() const { return maxHp; }
    const std::vector<StatusEffect*>& getEffects() const { return effects; }
};

#endif*/

#ifndef CHARACTER_H
#define CHARACTER_H

#include <string>
#include <vector>
#include "StatusEffect.h"
#include <iostream>
using namespace std;

class Character {
protected:
    string name;
    int hp;
    int maxHp;
    int block;
    int timesDamagedThisCombat;
    bool retainBlock = false;
    vector<StatusEffect*> effects;

public:
    Character(string name, int hp, int maxhp);
    virtual ~Character() = default;

    void decreaseHp(int amount);
    virtual int takeDamage(int incomingDamage);
    void addBlock(int amount);
    void increaseHP(int amount);
    void applyStatus(StatusEffect* newEffect);
    void keep_block_until_next_turn();
    virtual void TurnStartEffect(string effect);
    void setBlock(int amount);
    bool hasEffect(string effectName);

    int getStatusValue(string status);
    int calculate_total_block(int amount);
    int getTimesDamagedThisCombat() const;

    int calculateOutgoingDamage(int baseDamage);
    int calculateIncomingDamage(int baseDamage);

    void applyTurnStartEffects();
    void applyTurnEndEffects();

    int getHp() const { return hp; }
    int getBlock() const { return block; }
    string getName() const { return name; }
    int getMaxHp() const { return maxHp; }
    const std::vector<StatusEffect*>& getEffects() const { return effects; }

    // ====== NEW: Setter methods for multiplayer ======
    void setHp(int newHp) { hp = newHp; if (hp < 0) hp = 0; if (hp > maxHp) hp = maxHp; }
    void setMaxHp(int newMaxHp) { maxHp = newMaxHp; if (hp > maxHp) hp = maxHp; }
};

#endif