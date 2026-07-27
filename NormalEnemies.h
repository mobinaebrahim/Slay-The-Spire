#ifndef NORMALENEMIES_H
#define NORMALENEMIES_H

#include "character.h"
#include "enemy.h"
#include "player.h"
#include "StatusCard.h"
#include <string>
#include <vector>
using namespace std;

class Cultist : public Enemy {
private:
    bool isFirstTurn;

public:
    Cultist();
    void chooseAction() override;
    void executeAction(Character* target) override;
};

class JawWorm : public Enemy {
private:
    bool isFirstTurn;

public:
    JawWorm();
    void chooseAction() override;
    void executeAction(Character* target) override;
};

class Louse : public Enemy {
private:
    bool hasCurledUp;
    int colorChance;

public:
    Louse();
    void chooseAction() override;
    void executeAction(Character* target) override;
    int takeDamage(int incomingDamage) override;
    int getColorChance() const { return colorChance; }
};

class SmallSlime : public Enemy {
public:
    SmallSlime();
    void chooseAction() override;
    void executeAction(Character* target) override;
};

class MediumSlime : public Enemy {
public:
    MediumSlime();
    void chooseAction() override;
    void executeAction(Character* target) override;
};

class LargeSlime : public Enemy {
private:
    bool hasSplited;

public:
    LargeSlime();
    void chooseAction() override;
    void executeAction(Character* target) override;
    int takeDamage(int incomingDamage) override;
};

class Thief : public Enemy {
private:
    int turnCounter;
    bool hasScaped = false;
    int stolenGold = 0;

public:
    Thief(string name);
    void chooseAction() override;
    void executeAction(Character* target) override;
    bool wantsToFlee() const { return hasScaped; }
    int getStolenGold() const { return stolenGold; }
};

class SphericGuardian : public Enemy {
private:
    bool isFirstTurn;

public:
    SphericGuardian();
    void chooseAction() override;
    void executeAction(Character* target) override;
};

class BlueSlaver : public Enemy {
public:
    BlueSlaver();
    void chooseAction() override;
    void executeAction(Character* target) override;
};

class RedSlaver : public Enemy {
private:
    bool isFirstTurn;
    bool hasEntangled;

public:
    RedSlaver();
    void chooseAction() override;
    void executeAction(Character* target) override;
};

#endif