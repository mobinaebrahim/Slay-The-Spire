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
    void takeDamage(int amount) override; 
};

#endif