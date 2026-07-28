#ifndef ATTACKCARD_H
#define ATTACKCARD_H
#include "card.h"
#include "BattleManager.h"

class AttackCard: public Card {
protected:
	int baseDamage;
	int upgradedDamage;
public:
	AttackCard(CardType t, string n, string d, int e, int damage, int upgradeddamage);
	virtual ~AttackCard();
	virtual void upgrade() override;
	virtual void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

class ReaperCard: public AttackCard {
public:
	ReaperCard();
	void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

class FeedCard : public AttackCard {
public:
	FeedCard();
	void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

class ImmolateCard : public AttackCard {
public:
	ImmolateCard();
	void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

class WhirlwindCard : public AttackCard {
public:
	WhirlwindCard();
	void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

class Blood_for_BloodCard : public AttackCard {
public:
	Blood_for_BloodCard();
	int getCost(Character* caster) override;
	void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

class BashCard : public AttackCard {
public:
	BashCard();
	void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

class TwinStrikeCard : public AttackCard {
public:
	TwinStrikeCard();
	void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

class PerfectedStrike : public AttackCard {
public:
	PerfectedStrike();
	void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

class ClashCard : public AttackCard {
public:
	ClashCard();
	void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

#endif
