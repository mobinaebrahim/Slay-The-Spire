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
    vector<Player*> players;
    bool isPlayerTurn;

public:
    BattleManager() : isPlayerTurn(true) {}

    void spawnEnemy(Enemy* newEnemy);
    void removeEnemy(Enemy* enemy);
    void cleanupDeadEnemies();

    void playerTurn();
    void enemyTurn();
    void startCombat();
    bool isCombatOver();

    void playCardAction(Player* actingPlayer, Card* card, Enemy* target);

    void dealDamageToAllEnemies(int damage);
    int GetTotalDamageToAllEnemies(int damage);

    void addPlayer(Player* p) { players.push_back(p); }
    const vector<Player*>& getPlayers() const { return players; }
    Player* getPlayerAt(int index) const {
        return (index >= 0 && index < (int)players.size()) ? players[index] : nullptr;
    }
    bool areAllPlayersDeadOrGone() const;

    const vector<Enemy*>& getEnemies() const { return enemies; }
    std::vector<Enemy*> enemiesToRemove;
};
#endif