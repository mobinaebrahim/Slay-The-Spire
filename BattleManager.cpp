#include "BattleManager.h"

void BattleManager:: spawnEnemy(Enemy* newEnemy) {
    enemies.push_back(newEnemy);
}

void BattleManager::removeEnemy(Enemy* enemy) {
    for (auto it = enemies.begin(); it != enemies.end(); ++it) {
        if (*it == enemy) {
            enemies.erase(it);
            delete enemy;
            break;
        }
    }
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