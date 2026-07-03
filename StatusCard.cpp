#include "StatusCard.h"

//______________________________________StatusCard______________________________________
StatusCard::StatusCard(CardType type, string name, string description, int cost)
	: Card(type, name, description, cost) {}

void StatusCard::applyEffect(class Character* caster, class Character* target) {}

bool StatusCard::isPlayable() const { return false; }

//_______________________________________DazeCard_______________________________________
DazeCard::DazeCard(): StatusCard(CardType::Status, "Daze", 
    "Unplayable - Ethereal") {}

void DazeCard::applyEffect(class Character* caster, class Character* target) {
    if (caster) {
        //cout << "Daze card checked in hand!\n";
        // caster->exhaust_card_automatically(this);
        cout << " -> Daze is Ethereal! (It'll be automatically Exhausted at the end of your turn if not played)\n";
    }
}

//______________________________________SlimeCard_______________________________________
SlimeCard::SlimeCard() : StatusCard(CardType::Status, "Slime",
    "Has no effect - Exhaust", 1) {}

void SlimeCard::applyEffect(class Character* caster, class Character* target) {
    if (caster) {
        //cout << "Slime card played but hey it has no effect!\n";
        // caster->exhaustCard(this);
        cout << " -> Slime has been Exhausted and moved to the exhaust pile\n";
    }
}

bool SlimeCard::isPlayable() const { return true; }

//_______________________________________WoundCard______________________________________
WoundCard::WoundCard() : StatusCard(CardType::Status, "Wound",
    "Unplayable - Has no effect") {}

void WoundCard::applyEffect(class Character* caster, class Character* target) {
    //cout << "Wound card is in your hand... It just takes up space without any effects!\n";
}

//_______________________________________BurnCard_______________________________________
BurnCard::BurnCard() : StatusCard(CardType::Status, "Burn",
    "Unplayable - At the end of your turn, if in hand: take 2 damage") {}

void BurnCard::applyEffect(class Character* caster, class Character* target) {
    if (caster) {
        //cout << "Burn card detected in hand!\n";
        // target->decreaseHp(2);
        //cout << " -> Burn effect active! (You will take 2 damage if this stays in your hand)\n";
    }
}