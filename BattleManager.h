#ifndef BATTLEMANAGER_H
#define BATTLEMANAGER_H

#include "character.h"
#include "enemy.h"
#include "player.h"
#include "StatusCard.h"
#include <string>
#include <vector>
using namespace std;

class BattleManager {
private:
    vector<Enemy*> enemies;
    Player* player;

public:
    void spawnEnemy(Enemy* newEnemy);
    void removeEnemy(Enemy* enemy); 
    void startCombat(); 
    void dealDamageToAllEnemies(int damage);
    int GetTotalDamageToAllEnemies(int damage);
};

#endif