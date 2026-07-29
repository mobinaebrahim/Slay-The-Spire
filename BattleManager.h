#ifndef BATTLEMANAGER_H
#define BATTLEMANAGER_H

#include "character.h"
#include "enemy.h"
#include "player.h"
#include "StatusCard.h"
#include <string>
#include <vector>
using namespace std;

// BattleManager supports both:
//  - single-player (MainWindow): one Player set via setPlayer(), turn flow driven
//    manually via beginEnemyTurnPhase()/processSingleEnemyTurn()/endEnemyTurnPhase()
//    and the 2-arg playCardAction(card, target).
//  - multiplayer (GameServer): one or more Players added via addPlayer(), turn flow
//    driven via startCombat()/enemyTurn(), and the 3-arg
//    playCardAction(actingPlayer, card, target) so the server can say *who* played it.
class BattleManager {
private:
    vector<Enemy*> enemies;
    vector<Player*> players;
    bool isPlayerTurn;
    bool anyEnemyDiedThisCombat = false;

    Player* pickEnemyTarget() const;

public:
    BattleManager() : isPlayerTurn(true) {}
    // FIX: BattleManager owns every Enemy*/Player* added via spawnEnemy()/
    // addPlayer(), but had no destructor — `delete game.battleManager` in
    // GameServer::handle_start_combat only freed the vectors themselves,
    // leaking every Enemy and Player object from every previous combat.
    ~BattleManager();

    void spawnEnemy(Enemy* newEnemy);
    void removeEnemy(Enemy* enemy);
    void cleanupDeadEnemies();

    void playerTurn();
    void enemyTurn();
    void startCombat();
    bool isCombatOver();

    void playCardAction(Card* card, Enemy* target);
    void playCardAction(Player* actingPlayer, Card* card, Enemy* target);

    void dealDamageToAllEnemies(int damage);
    int GetTotalDamageToAllEnemies(int damage);
    bool getAnyEnemyDied() const { return anyEnemyDiedThisCombat; }

    void beginEnemyTurnPhase();
    void processSingleEnemyTurn(Enemy* enemy);
    void endEnemyTurnPhase();

    void setPlayer(Player* p);
    void addPlayer(Player* p);
    const vector<Player*>& getPlayers() const { return players; }
    Player* getPlayer() const { return players.empty() ? nullptr : players[0]; }

    const vector<Enemy*>& getEnemies() const { return enemies; }
    std::vector<Enemy*> enemiesToRemove;
};

#endif