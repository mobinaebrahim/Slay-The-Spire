#include "CurseCard.h"

//_______________________________________CurseCard________________________________________
CurseCard::CurseCard(CardType type, string name, string description, int cost)
	: Card(type, name, description, cost) {}

void CurseCard::applyEffect(Character* caster, Character* target, BattleManager* bm) {}

bool CurseCard::isPlayable() const { return false; }

//________________________________________J.A.XCard_______________________________________
JAXCard::JAXCard():CurseCard(CardType::Curse, "JAX",
	"Special event card: Lose 3 HP - Gain 2 Strength", 0) {}

void JAXCard::upgrade() {
    Card::upgrade();
}

void JAXCard::applyEffect(Character* caster, Character* target, BattleManager* bm) {
    if (caster) {
        caster->decreaseHp(3);
        int strGain = isUpgraded ? 3 : 2;
        caster->applyStatus(new StrengthEffect(strGain));
    }
}

bool JAXCard::isPlayable() const { return true; }

//____________________________________CurseOfBellCard___________________________________
CurseOfBellCard::CurseOfBellCard() :CurseCard(CardType::Curse, "CurseOfBell",
	"Unplayable - You cannot remove this card from your deck") {}

void CurseOfBellCard::upgrade() { 
    Card::upgrade(); 
}

void CurseOfBellCard::applyEffect(Character* caster, Character* target, BattleManager* bm) {}

//______________________________________RegretCard______________________________________
RegretCard::RegretCard() :CurseCard(CardType::Curse, "Regret",
	"Unplayable - Take 1 damage for every card in hand after your turn ends") {}

void RegretCard::upgrade() { 
    Card::upgrade();
}

void RegretCard::applyEffect(Character* caster, Character* target, BattleManager* bm) {
    Player* player = dynamic_cast<Player*>(caster);
    if (player) {
        int damage = player->getHandSize();
        player->takeDamage(damage);
    }
}