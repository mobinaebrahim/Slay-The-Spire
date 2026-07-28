#ifndef ELITEENEMIES_H
#define ELITEENEMIES_H

#include "character.h"
#include "enemy.h"
#include "player.h"
#include "StatusCard.h"
#include "NormalEnemies.h"
#include <string>
#include <vector>
using namespace std;

class BattleManager;

class GremlinKnob : public Enemy {
public:
    GremlinKnob();
    void chooseAction() override; 
    void executeAction(Character* target) override;
    void onPlayerPlayedCard(Card* card);
    bool isElite() const override { return true; }
};

class ThreeSentries : public Enemy {
protected:
    bool isNextMoveBeam;
public:
    ThreeSentries(string name, int minHp, int maxHp);
    void chooseAction() override;
    void executeAction(Character* target) override;
    static void spawnGroup(BattleManager* bm);
    bool isElite() const override { return true; }
};

class MiddleSentry : public ThreeSentries {
public:
    MiddleSentry();
};

class SideSentry : public ThreeSentries {
public:
    SideSentry();
};

class BookOfStabbing : public Enemy {
private:
    int stabCount; 
public:
    BookOfStabbing();
    void chooseAction() override;
    void executeAction(Character* target) override;
    bool isElite() const override { return true; }
};

class Taskmaster : public Enemy {
public:
    Taskmaster();
    void chooseAction() override;
    void executeAction(Character* target) override;
    static void spawnGroup(BattleManager* bm);
    bool isElite() const override { return true; }
};

#endif