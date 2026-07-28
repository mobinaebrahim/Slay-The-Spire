#ifndef BOSSSTRUGGLES_H
#define BOSSSTRUGGLES_H

#include "character.h"
#include "enemy.h"
#include "player.h"
#include "BattleManager.h"
#include "NormalEnemies.h"
#include <string>
#include <vector>
using namespace std;

class KingSlime : public Enemy {
private:
    int moveIndex; 
    bool hasSplit; 
    BattleManager* myManager;
public:
    KingSlime(BattleManager* manager);
    void chooseAction() override;
    void executeAction(Character* target) override;
    int takeDamage(int amount) override;
    bool isBoss() const override { return true; }
};

class Hexaghost : public Enemy {
private:
    int turnCount;
    BattleManager* myManager;
public:
    Hexaghost();
    void chooseAction() override;
    void executeAction(Character* target) override;
    bool isDividerTurn() const { return turnCount == 2; }
    bool isBoss() const override { return true; }
};

class TheChamp : public Enemy {
private:
    bool isTauntTurn;
public:
    TheChamp();
    void chooseAction() override;
    void executeAction(Character* target) override;
    bool isBoss() const override { return true; }
};

#endif