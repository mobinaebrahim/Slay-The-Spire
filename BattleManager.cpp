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