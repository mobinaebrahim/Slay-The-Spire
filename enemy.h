#ifndef ENEMY_H
#define ENEMY_H

#include "Character.h"
#include <string>
#include <vector>
using namespace std;

enum class IntentType { Attack, Defend, Buff, Debuff };

class Enemy : public Character {
protected:
    IntentType currentIntent;
    int intentValue;

public:
    Enemy(std::string n, int h, int max);
    virtual ~Enemy() = default;

    virtual void chooseAction();
    virtual void executeAction(Character* target);

    IntentType getIntentType() const;
    int getIntentValue() const;
    std::string getIntentString() const;
};

#endif