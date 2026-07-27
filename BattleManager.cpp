/*#include "BattleManager.h"
#include <algorithm>
#include <cstdlib>

void BattleManager::spawnEnemy(Enemy* newEnemy) {
    enemies.push_back(newEnemy);
}

void BattleManager::removeEnemy(Enemy* enemy) {
    for (auto it = enemies.begin(); it != enemies.end(); ++it) {
        if (*it == enemy) {
            enemiesToRemove.push_back(enemy);
            break;
        }
    }
}

void BattleManager::cleanupDeadEnemies() {
    for (Enemy* enemy : enemiesToRemove) {
        auto it = std::find(enemies.begin(), enemies.end(), enemy);
        if (it != enemies.end()) {
            enemies.erase(it);
            delete enemy;
        }
    }
    enemiesToRemove.clear();
}

void BattleManager::dealDamageToAllEnemies(int damage) {
    for (Enemy* enemy : enemies) {
        enemy->takeDamage(damage);
    }
}

int BattleManager::GetTotalDamageToAllEnemies(int damage) {
    int totalDealt = 0;
    for (Enemy* enemy : enemies) {
        totalDealt += enemy->takeDamage(damage);
    }
    return totalDealt;
}

void BattleManager::playerTurn() {
    isPlayerTurn = true;
    for (Player* p : players) {
        if (!p || p->getHp() <= 0) continue;
        p->resetEnergy();
        p->applyTurnStartEffects();
        p->drawCards(5);
    }
}

void BattleManager::enemyTurn() {
    isPlayerTurn = false;

    for (Player* p : players) {
        if (!p || p->getHp() <= 0) continue;
        p->applyTurnEndEffects();
        p->endTurnCleanUp();
    }

    for (Enemy* enemy : enemies) {
        if (enemy->getHp() <= 0) continue;

        vector<Player*> alive;
        for (Player* p : players) {
            if (p && p->getHp() > 0) alive.push_back(p);
        }
        if (alive.empty()) break;

        Player* target = alive[std::rand() % alive.size()];
        enemy->executeAction(target);
        enemy->applyTurnEndEffects();
    }

    playerTurn();
}

void BattleManager::playCardAction(Player* actingPlayer, Card* card, Enemy* target) {
    if (!isPlayerTurn || !actingPlayer)
        return;

    actingPlayer->playCard(card, target);

    if (target && target->getHp() <= 0)
        removeEnemy(target);
}

void BattleManager::startCombat() {
    playerTurn();
}

bool BattleManager::isCombatOver() {
    if (enemies.empty())
        return true;
    return areAllPlayersDeadOrGone();
}

bool BattleManager::areAllPlayersDeadOrGone() const {
    for (Player* p : players) {
        if (p && p->getHp() > 0)
            return false;
    }
    return true;
}*/

#include "BattleManager.h"
#include <algorithm>

void BattleManager:: spawnEnemy(Enemy* newEnemy) {
    newEnemy->setBattleManager(this);
    enemies.push_back(newEnemy);
}

void BattleManager::removeEnemy(Enemy* enemy) {
    for (auto it = enemies.begin(); it != enemies.end(); ++it) {
        if (*it == enemy) {
            enemiesToRemove.push_back(enemy);
            break;
        }
    }
}

void BattleManager::cleanupDeadEnemies() {
    for (Enemy* enemy : enemiesToRemove) {
        auto it = std::find(enemies.begin(), enemies.end(), enemy);
        if (it != enemies.end()) {
            enemies.erase(it);
            delete enemy;
        }
    }
    enemiesToRemove.clear();
}

void BattleManager::dealDamageToAllEnemies(int damage) {
    for (Enemy* enemy : enemies) {
        enemy->takeDamage(damage);
    }
}

int BattleManager::GetTotalDamageToAllEnemies(int damage) {
    int totalDealt = 0;
    for (Enemy* enemy : enemies) {
        totalDealt += enemy ->takeDamage(damage);
    }
    return totalDealt;
}

void BattleManager::playerTurn() {
    isPlayerTurn = true;
    player->resetEnergy();
    player->applyTurnStartEffects();
    player->drawCards(5);
}

void BattleManager::enemyTurn() {
    isPlayerTurn = false;
    player->applyTurnEndEffects();
    player->endTurnCleanUp();

    std::vector<Enemy*> enemiesSnapshot = enemies;
    for (Enemy* enemy : enemiesSnapshot) {
        if (enemy->getHp() > 0) {
            enemy->executeAction(player);
            enemy->applyTurnEndEffects();
        }
    }
    playerTurn();
}

void BattleManager::playCardAction(Card* card, Enemy* target) {
    if (!isPlayerTurn)
        return;
    player->playCard(card, target);
    if (target && target->getHp() <= 0)
        removeEnemy(target);
}

void BattleManager::startCombat() {
    playerTurn();
}

bool BattleManager::isCombatOver() {
    if (player->getHp() <= 0)
        return true;

    if (enemies.empty())
        return true;

    return false;
}

void BattleManager::beginEnemyTurnPhase() {
    isPlayerTurn = false;
    player->applyTurnEndEffects();
    player->endTurnCleanUp();
}

void BattleManager::processSingleEnemyTurn(Enemy* enemy) {
    if (enemy->getHp() > 0) {
        enemy->executeAction(player);
        enemy->applyTurnEndEffects();
    }
}

void BattleManager::endEnemyTurnPhase() {
    playerTurn();
}