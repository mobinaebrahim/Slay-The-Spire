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
    bool isPlayerTurn;
public:
    void spawnEnemy(Enemy* newEnemy);
    void removeEnemy(Enemy* enemy); 
    void playerTurn();     
    void enemyTurn();
    void startCombat(); 
    bool isCombatOver();
    void playCardAction(Card* card, Enemy* target);
    void dealDamageToAllEnemies(int damage);
    int GetTotalDamageToAllEnemies(int damage);

    const vector<Enemy*>& getEnemies() const { return enemies; }
};

#endif