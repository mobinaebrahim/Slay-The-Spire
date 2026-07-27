#include "Potion.h"

//_______________________________________Potion_______________________________________
Potion::Potion(string n, string d) : name(n), description(d){}

string Potion::getName() const { return name; }

string Potion::getDescription() const { return description; }

bool Potion::isPlayable() const { return true; }

//_____________________________________BlockPotion____________________________________
BlockPotion::BlockPotion(): Potion("BlockPotion", "Gain 12 Block") {}

void BlockPotion::applyEffect(Character* caster, Character* target, BattleManager* bm) {
    if (caster)
        caster->addBlock(12);
}

//_____________________________________FirePotion_____________________________________
FirePotion::FirePotion() : Potion("FirePotion", "Deal 20 damage") {}

void FirePotion::applyEffect(Character* caster, Character* target, BattleManager* bm) {
    if (target)
        target->takeDamage(20);
}

//____________________________________EnergyPotion____________________________________
EnergyPotion::EnergyPotion() : Potion("EnergyPotion", "Gain 2 Energy") {}

void EnergyPotion::applyEffect(Character* caster, Character* target, BattleManager* bm) {
    Player* player = dynamic_cast<Player*>(caster);
    if (player)
        player->increaseEnergy(2);
}

//____________________________________SwiftPotion_____________________________________
SwiftPotion::SwiftPotion() : Potion("SwiftPotion", "Draw 3 cards") {}

void SwiftPotion::applyEffect(Character* caster, Character* target, BattleManager* bm) {
    Player* player = dynamic_cast<Player*>(caster);
    if (player)
        player->drawCards(3);
}

//___________________________________FairyInABottle___________________________________
FairyInABottle::FairyInABottle() : Potion("FairyInABottle", "When you would die, heal to 30% instead") {}

void FairyInABottle::applyEffect(Character* caster, Character* target, BattleManager* bm) {}

bool FairyInABottle::isPlayable() const { return false; }