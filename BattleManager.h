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
    bool anyEnemyDiedThisCombat = false;
    bool isBossOrElite = false;
public:
    BattleManager() : player(nullptr), isPlayerTurn(true) {}

    void spawnEnemy(Enemy* newEnemy);
    void removeEnemy(Enemy* enemy);
    void cleanupDeadEnemies();
    void playerTurn();     
    void enemyTurn();
    void startCombat(); 
    bool isCombatOver();
    void playCardAction(Card* card, Enemy* target);
    void dealDamageToAllEnemies(int damage);
    int GetTotalDamageToAllEnemies(int damage);
    bool getAnyEnemyDied() const { return anyEnemyDiedThisCombat; }

    void beginEnemyTurnPhase();
    void processSingleEnemyTurn(Enemy* enemy);
    void endEnemyTurnPhase();

    void setPlayer(Player* p) { player = p; }
    const vector<Enemy*>& getEnemies() const { return enemies; }
    std::vector<Enemy*> enemiesToRemove;

    void setBossOrElite(bool value) { isBossOrElite = value; }
    bool isBossOrEliteCombat() const { return isBossOrElite; }
    std::function<void()> onCombatStartSoundCallback;
};

#endif