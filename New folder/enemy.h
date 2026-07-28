#ifndef ENEMY_H
#define ENEMY_H
#include "Character.h"
#include <string>
#include <vector>
using namespace std;
class BattleManager;
class Card;
enum class IntentType { Attack, Defend, Buff, Debuff, Combined, AttackDebuff, AttackAddCard, Special};
class Enemy : public Character {
protected:
    IntentType currentIntent;
    int intentValue;
    int intentBlock;
    BattleManager* battleManagerPtr = nullptr;
public:
    Enemy(std::string n, int h, int max);
    virtual ~Enemy() = default;
    virtual void chooseAction();
    virtual void executeAction(Character* target);
    virtual void onPlayerPlayedCard(Card* card) {}
    virtual bool wantsToFlee() const { return false; }
    IntentType getIntentType() const;
    int getIntentValue() const;
    string getIntentString() const;
    void setBattleManager(BattleManager* bm) { battleManagerPtr = bm; }
    void setHp(int newHp) { hp = newHp; }
    void setMaxHp(int newMax) { maxHp = newMax; }
};
#endif