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