#include "BattleManager.h"
#include "NormalEnemies.h"
#include <algorithm>
#include <cstdlib>

void BattleManager::spawnEnemy(Enemy* newEnemy) {
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
    for (Enemy* e : enemies) {
        if (e->getHp() <= 0) {
            if (std::find(enemiesToRemove.begin(), enemiesToRemove.end(), e) == enemiesToRemove.end())
                enemiesToRemove.push_back(e);
        }
    }
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

void BattleManager::setPlayer(Player* p) {
    // Single-player: this IS the whole roster.
    players.clear();
    if (p) players.push_back(p);
}

void BattleManager::addPlayer(Player* p) {
    // Multiplayer: add another combatant to the fight (co-op, up to N players).
    if (p) players.push_back(p);
}

Player* BattleManager::pickEnemyTarget() const {
    // Picks a random alive player for enemy AI to attack. In single-player there's
    // only ever one entry, so this always resolves to that player.
    std::vector<Player*> alive;
    for (Player* p : players) {
        if (p && p->getHp() > 0) alive.push_back(p);
    }
    if (alive.empty()) return players.empty() ? nullptr : players[0];
    return alive[rand() % alive.size()];
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
        if (!p) continue;
        p->applyTurnEndEffects();
        p->endTurnCleanUp();
    }

    std::vector<Enemy*> enemiesSnapshot = enemies;
    for (Enemy* enemy : enemiesSnapshot) {
        if (enemy->getHp() > 0) {
            // NEW: AOE support
            if (enemy->isAOE()) {
                enemy->executeActionOnAllPlayers(players);
            } else {
                Player* target = pickEnemyTarget();
                if (!target) break;
                enemy->executeAction(target);
            }
            enemy->applyTurnEndEffects();
            if (enemy->wantsToFlee())
                removeEnemy(enemy);
        }
    }
    // Note: does NOT call playerTurn() here — GameServer drives the next player-turn
    // step itself (per-player draw) after calling enemyTurn(). Single-player never
    // calls enemyTurn() at all; it uses beginEnemyTurnPhase()/processSingleEnemyTurn()/
    // endEnemyTurnPhase() instead, and endEnemyTurnPhase() does call playerTurn().
}

void BattleManager::playCardAction(Card* card, Enemy* target) {
    // Single-player path: acts on the single registered player.
    playCardAction(getPlayer(), card, target);
}

void BattleManager::playCardAction(Player* actingPlayer, Card* card, Enemy* target) {
    if (!isPlayerTurn || !actingPlayer)
        return;
    actingPlayer->playCard(card, target);
    if (target && target->getHp() <= 0) {
        anyEnemyDiedThisCombat = true;
        Thief* thief = dynamic_cast<Thief*>(target);
        if (thief && thief->getStolenGold() > 0)
            actingPlayer->increaseGold(thief->getStolenGold());
        removeEnemy(target);
    }
}

void BattleManager::startCombat() {
    playerTurn();
}

bool BattleManager::isCombatOver() {
    if (enemies.empty())
        return true;

    for (Player* p : players) {
        if (p && p->getHp() > 0)
            return false;
    }
    return true;
}

void BattleManager::beginEnemyTurnPhase() {
    isPlayerTurn = false;
    Player* p = getPlayer();
    if (p) {
        p->applyTurnEndEffects();
        p->endTurnCleanUp();
    }
}

void BattleManager::processSingleEnemyTurn(Enemy* enemy) {
    Player* p = getPlayer();
    if (!p) return;
    if (enemy->getHp() > 0) {
        enemy->executeAction(p);
        enemy->applyTurnEndEffects();
        if (enemy->wantsToFlee())
            removeEnemy(enemy);
    }
}

void BattleManager::endEnemyTurnPhase() {
    playerTurn();
}