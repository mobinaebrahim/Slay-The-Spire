#include "Enemy.h"
#include "BattleManager.h"
#include "player.h" // NEW: for Player in AOE fallback
#include <cstdlib>

Enemy::Enemy(string n, int h, int max)
    : Character(n, h, max), currentIntent(IntentType::Attack), intentValue(0), intentBlock(0) {}

void Enemy::chooseAction() {}

void Enemy::executeAction(Character* target) {}

// NEW: default single-target fallback
void Enemy::executeActionOnAllPlayers(const std::vector<Player*>& players) {
    for (Player* p : players) {
        if (p && p->getHp() > 0) {
            executeAction(p);
            break;
        }
    }
}

IntentType Enemy::getIntentType() const { return currentIntent; }
int Enemy::getIntentValue() const { return intentValue; }

string Enemy::getIntentString() const {
    if (currentIntent == IntentType::Attack)
        return "Intent: Attack for " + to_string(intentValue) + " DMG";
    if (currentIntent == IntentType::Defend)
        return "Intent: Defend for " + to_string(intentBlock) + " BLOCK";
    if (currentIntent == IntentType::Buff)
        return "Intent: Buff (getting stronger by " + to_string(intentValue) + " )";
    if (currentIntent == IntentType::Debuff)
        return "Intent: Debuff (wants to have negative effect on you) ";
    if(currentIntent == IntentType::Combined)
        return "Intent: Attack for " + to_string(intentValue) + " DMG + Defend for " + to_string(intentBlock) + " BLOCK";
    if (currentIntent == IntentType::Special)
        return "Intent: Special";
    return "Intent: Unknown";
}