#ifndef CARD_H
#define CARD_H

#include <string>
#include <iostream>
using namespace std;
class BattleManager;
class character;

enum class CardType{ Attack, Skill, Power, Status, Curse};

class Card {
protected:
	CardType type;
	string name;
	string description;
	int energyCost;
	bool isUpgraded = false;
public:
	Card(CardType t, string n, string d, int e);
	virtual ~Card();
	virtual void applyEffect(class Character* caster, class Character* target, BattleManager* bm) = 0;
	string getName() const;
	string getDescription() const;
	virtual int getCost(Character* caster);
	CardType getType() const;
	virtual bool isPlayable() const; 
	virtual void upgrade();
    virtual string getUnplayableReason(Character* caster) const { return ""; }
};

#endif
