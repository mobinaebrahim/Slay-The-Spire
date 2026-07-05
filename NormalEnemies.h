#ifndef NORMALENEMIES_H
#define NORMALENEMIES_H

#include "character.h"
#include "enemy.h"
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
    void takeDamage(int incomingDamage) override;
};

class SmallSlime : public Enemy {
public:
    SmallSlime();
    void chooseAction() override;
    void executeAction(Character* target) override;
};

#endif