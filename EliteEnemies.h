#ifndef ELITEENEMIES_H
#define ELITEENEMIES_H

#include "character.h"
#include "enemy.h"
#include "player.h"
#include "StatusCard.h"
#include <string>
#include <vector>
using namespace std;

class GremlinKnob : public Enemy {
public:
    GremlinKnob();
    void chooseAction() override; 
    void executeAction(Character* target) override;
    void onPlayerPlayedCard(Card* card);
};

class ThreeSentries : public Enemy {
protected:
    bool isNextMoveBeam;
public:
    ThreeSentries(string name, int minHp, int maxHp);
    void chooseAction() override;
    void executeAction(Character* target) override;
};

class MiddleSentry : public ThreeSentries {
public:
    MiddleSentry();
};

class SideSentry : public ThreeSentries {
public:
    SideSentry();
};

#endif