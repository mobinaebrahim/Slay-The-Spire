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
	virtual void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

class ReaperCard: public AttackCard {
public:
	ReaperCard();
	void upgrade() override;
	void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

class FeedCard : public AttackCard {
public:
	FeedCard();
	void upgrade() override;
	void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

class ImmolateCard : public AttackCard {
public:
	ImmolateCard();
	void upgrade() override;
	void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

class WhirlwindCard : public AttackCard {
public:
	WhirlwindCard();
	void upgrade() override;
	void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

class Blood_for_BloodCard : public AttackCard {
public:
	Blood_for_BloodCard();
	void upgrade() override;
	int getCost(Character* caster) override;
	void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

class BashCard : public AttackCard {
public:
	BashCard();
	void upgrade() override;
	void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

class TwinStrikeCard : public AttackCard {
public:
	TwinStrikeCard();
	void upgrade() override;
	void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

class PerfectedStrike : public AttackCard {
public:
	PerfectedStrike();
	void upgrade() override;
	void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

class ClashCard : public AttackCard {
public:
	ClashCard();
	void upgrade() override;
	void applyEffect(class Character* caster, class Character* target, BattleManager* bm) override;
};

#endif
