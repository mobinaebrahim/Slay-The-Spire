#include "Enemy.h"
#include <cstdlib>

Enemy::Enemy(string n, int h, int max)
    : Character(n, h, max), currentIntent(IntentType::Attack), intentValue(0), intentBlock(0) {}

void Enemy::chooseAction() {}

void Enemy::executeAction(Character* target) {}

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
    else if(currentIntent == IntentType::Combined)
        return "Intent: Attack for " + to_string(intentValue) + " DMG + Defend for " + to_string(intentBlock) + " BLOCK";
}