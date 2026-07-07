#include "StatusCard.h"

//______________________________________StatusCard______________________________________
StatusCard::StatusCard(CardType type, string name, string description, int cost)
	: Card(type, name, description, cost) {}

void StatusCard::applyEffect(Character* caster, Character* target, BattleManager* bm) {}

bool StatusCard::isPlayable() const { return false; }

//_______________________________________DazeCard_______________________________________
DazeCard::DazeCard(): StatusCard(CardType::Status, "Daze", 
    "Unplayable - Ethereal") {}

void DazeCard::upgrade() { Card::upgrade(); }

void DazeCard::applyEffect(Character* caster, Character* target, BattleManager* bm) {
    Player* player = dynamic_cast <Player*> (caster);
    player->exhaust_card_automatically(this);
}

//______________________________________SlimeCard_______________________________________
SlimeCard::SlimeCard() : StatusCard(CardType::Status, "Slime",
    "Has no effect - Exhaust", 1) {}

void SlimeCard::upgrade() { Card::upgrade(); }

void SlimeCard::applyEffect(Character* caster, Character* target, BattleManager* bm) {
    Player* player = dynamic_cast <Player*> (caster);
    if (player) {
        player->exhaustCard(this);
    }
}

bool SlimeCard::isPlayable() const { return true; }

//_______________________________________WoundCard______________________________________
WoundCard::WoundCard() : StatusCard(CardType::Status, "Wound",
    "Unplayable - Has no effect") {}

void WoundCard::upgrade() { Card::upgrade(); }

void WoundCard::applyEffect(Character* caster, Character* target, BattleManager* bm) {}

//_______________________________________BurnCard_______________________________________
BurnCard::BurnCard() : StatusCard(CardType::Status, "Burn",
    "Unplayable - At the end of your turn, if in hand: take 2 damage") {}

void BurnCard::upgrade() { Card::upgrade(); }

void BurnCard::applyEffect(Character* caster, Character* target, BattleManager* bm) {
    if (caster) {
        int damage = isUpgraded ? 4 : 2;
        caster->takeDamage(damage);
    }
}