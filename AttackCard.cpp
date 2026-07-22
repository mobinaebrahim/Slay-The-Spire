#include "AttackCard.h"
#include "character.h"
#include "player.h"

//___________________________________AttackCard______________________________________
AttackCard::AttackCard(CardType t, string n, string d, int e, int damage, int upgradeddamage) :
	Card(t, n, d, e), baseDamage(damage){}

AttackCard::~AttackCard() = default;

void AttackCard::upgrade() {
	Card::upgrade();
	baseDamage = upgradedDamage;
}

void AttackCard::applyEffect(Character* caster, Character* target, BattleManager* bm) {
    if (caster && target) {
        int baseDmg = isUpgraded ? upgradedDamage : baseDamage;
        int damageToDeal = caster->calculateOutgoingDamage(baseDmg);
        target->takeDamage(damageToDeal);
    }
}

//___________________________________ReaperCard______________________________________
ReaperCard::ReaperCard(): AttackCard(CardType::Attack, "Reaper", 
	"Deal 4 damage to all enemies - Heal HP equal to unblocked damage - Exhaust", 2, 4, 5) {}

void ReaperCard::applyEffect(class Character* caster, class Character* target, BattleManager* bm) {
    if (caster && target) {
        int baseDmg = isUpgraded ? upgradedDamage : this->baseDamage;
        int damageToDeal = caster->calculateOutgoingDamage(baseDmg);
        int totalActualDamage = bm->GetTotalDamageToAllEnemies(damageToDeal);
        caster->increaseHP(totalActualDamage);

        Player* player = dynamic_cast<Player*>(caster);
        if (player)
            player->exhaustCard(this);
    }
}

//____________________________________FeedCard_______________________________________
FeedCard::FeedCard(): AttackCard(CardType::Attack, "Feed",
	"Deal 10 damage - If fatal, raise max HP by 3 - Exhaust", 1, 10, 12) {}

void FeedCard::applyEffect(class Character* caster, class Character* target, BattleManager* bm) {
	if (caster && target) {
		int baseDamage = isUpgraded ? upgradedDamage : this->baseDamage;
		int damageToDeal = caster->calculateOutgoingDamage(baseDamage);
		int actualDamage = target->takeDamage(damageToDeal);
		if (actualDamage >= target->getHp()) {
			Player* player = dynamic_cast<Player*>(caster);
			if (player) player->increaseMaxHP(isUpgraded ? 4 : 3);
		}
	}
}

//__________________________________ImmolateCard_____________________________________
ImmolateCard::ImmolateCard(): AttackCard(CardType::Attack, "Immolate",
	"Deal 21 damage to all enemies - Add 2 BURN into discard pile", 2, 21, 28) {}

void ImmolateCard::applyEffect(class Character* caster, class Character* target, BattleManager* bm) {
	if (caster && target) {
		int baseDamage = isUpgraded ? upgradedDamage : this->baseDamage;
		int damageToDeal = caster->calculateOutgoingDamage(baseDamage);
		bm->GetTotalDamageToAllEnemies(damageToDeal);
		Player* player = dynamic_cast<Player*>(caster);
		if (player) player->addBurnToDiscard(2);
	}
}

//__________________________________WhirlwindCard_____________________________________
WhirlwindCard::WhirlwindCard() : AttackCard(CardType::Attack, "Whirlwind",
	"Deal 5 damage to all enemies X times (X = your current energy)", 0, 5, 8) {}

void WhirlwindCard::applyEffect(class Character* caster, class Character* target, BattleManager* bm) {
	if (caster && target) {
		Player* player = dynamic_cast<Player*>(caster);
		if (player) {
			int energySpent = player->getEnergy();
			player->decreaseEnergy(energySpent);
			int baseDamage = isUpgraded ? upgradedDamage : this->baseDamage;
			int damageToDeal = caster->calculateOutgoingDamage(baseDamage);
			for (int i = 0; i < energySpent; i++)
				bm->GetTotalDamageToAllEnemies(damageToDeal);
		}
	}
}

//__________________________________Blood_for_BloodCard_____________________________________
Blood_for_BloodCard::Blood_for_BloodCard() : AttackCard(CardType::Attack, "Blood for Blood",
	"Deal 18 damage - Costs 1 less for each time you lose HP this combat", 4, 18, 22) {} 

int Blood_for_BloodCard::getCost(Character* caster) {
	if (!caster) 
		return energyCost;
	int reduction = caster->getTimesDamagedThisCombat();
	int newCost = energyCost - reduction;
	return (newCost < 0) ? 0 : newCost;
}

void Blood_for_BloodCard::applyEffect(class Character* caster, class Character* target, BattleManager* bm) {
	if (caster && target) {
		int baseDmg = isUpgraded ? upgradedDamage : baseDamage;
		int damageToDeal = caster->calculateOutgoingDamage(baseDmg);
		target->takeDamage(damageToDeal);
	}
}

//________________________________________BashCard__________________________________________
BashCard::BashCard():AttackCard(CardType::Attack, "Bash",
	"Deal 8 damage - Apply 2 Vulnerable", 2, 8, 10) {}

void BashCard::applyEffect(Character* caster, Character* target, BattleManager* bm) {
	if (caster && target) {
		int baseDmg = isUpgraded ? upgradedDamage : baseDamage;
		int damageToDeal = caster->calculateOutgoingDamage(baseDmg);
		target->takeDamage(damageToDeal);
		target->applyStatus(new VulnerableEffect(isUpgraded ? 3 : 2));
	}
}

//_____________________________________TwinStrikeCard_______________________________________
TwinStrikeCard::TwinStrikeCard():AttackCard(CardType::Attack, "TwinStrike",
	"Deal 6 damage twice", 1, 6, 8) {}

void TwinStrikeCard::applyEffect(Character* caster, Character* target, BattleManager* bm) {
	if (caster && target) {
		int baseDmg = isUpgraded ? upgradedDamage : baseDamage;
		int damageToDeal = caster->calculateOutgoingDamage(baseDmg);
		target->takeDamage(damageToDeal);
		target->takeDamage(damageToDeal);
	}
}

//___________________________________PerfectedStrikeCard____________________________________
PerfectedStrike::PerfectedStrike():AttackCard(CardType::Attack, "PerfectedStrike",
	"Deal 6 damage + 2 additional for every Strike card in deck", 2, 6, 6) {}

void PerfectedStrike::applyEffect(Character* caster, Character* target, BattleManager* bm) {
	if (caster && target) {
		Player* player = dynamic_cast<Player*>(caster);
		int strikeCount = player->countCardsByName("Strike"); 
		int baseDmg = (isUpgraded ? upgradedDamage : baseDamage) + (isUpgraded ? strikeCount * 3 : strikeCount * 2);

		int damageToDeal = caster->calculateOutgoingDamage(baseDmg);
		target->takeDamage(damageToDeal);
	}
}

//________________________________________ClashCard_________________________________________
ClashCard::ClashCard():AttackCard(CardType::Attack, "Clash",
	"Deal 14 damage - Can only be played if every card in hand is an attack", 0, 14, 18) {}

void ClashCard::applyEffect(Character* caster, Character* target, BattleManager* bm) {
	Player* player = dynamic_cast<Player*>(caster);
	if (player && player->isHandAllAttacks()) { 
		int baseDmg = isUpgraded ? upgradedDamage : baseDamage;
		int damageToDeal = caster->calculateOutgoingDamage(baseDmg);
		target->takeDamage(damageToDeal);
	}
	else
		cout << "Cannot play Clash!" << endl;
}