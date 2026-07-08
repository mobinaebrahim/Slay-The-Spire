#include "PowerCard.h"

//______________________________________PowerCard______________________________________
PowerCard::PowerCard(CardType type, string name, string description, int cost)
	: Card(type, name, description, cost) {}

void PowerCard::upgrade() {
    Card::upgrade();
}

void PowerCard::applyEffect(Character* caster, Character* target, BattleManager* bm) {}

//_____________________________________InflameCard_____________________________________
InflameCard::InflameCard(): PowerCard(CardType::Power, "Inflame",
    "Gain 2 Strength", 1) {}

void InflameCard::applyEffect(Character* caster, Character* target, BattleManager* bm) {
    if (caster) 
        caster->applyStatus(new StrengthEffect(isUpgraded ? 3 : 2));
}

//___________________________________MetallicizeCard___________________________________
MetallicizeCard::MetallicizeCard() : PowerCard(CardType::Power, "Metallicize",
    "At the end of your turn get 3 block", 1) {}

void MetallicizeCard::applyEffect(Character* caster, Character* target, BattleManager* bm) {
    if (caster) 
        caster->applyStatus(new MetallicizeEffect(isUpgraded ? 4 : 3));
}

//____________________________________DemonFormCard____________________________________
DemonFormCard::DemonFormCard() : PowerCard(CardType::Power, "DemonForm",
    "At the start of your turn gain 3 Strength", 3) {}

void DemonFormCard::applyEffect(Character* caster, Character* target, BattleManager* bm) {
    if (caster) 
        caster->applyStatus(new DemonFormEffect(isUpgraded ? 3 : 2));
}

//____________________________________BrutalityCard____________________________________
BrutalityCard::BrutalityCard() : PowerCard(CardType::Power, "Brutality",
    "At the start of your turn lose 1 HP and draw 1 card", 0) {}

void BrutalityCard::applyEffect(Character* caster, Character* target, BattleManager* bm) {
    if (caster) 
        caster->TurnStartEffect("Brutality");   
}

//____________________________________FeelNoPainCard___________________________________
FeelNoPainCard::FeelNoPainCard():PowerCard(CardType::Power, "FeelNoPain",
    "Every time a card is Exhausted, gain 3 block", 1) {}

void FeelNoPainCard::applyEffect(Character* caster, Character* target, BattleManager* bm) {
    //if(caster)
        //caster->applyStatus(new FeelNoPainEffect(isUpgraded ? 4 : 3));
}

//____________________________________BarricadeCard____________________________________
BarricadeCard::BarricadeCard():PowerCard(CardType::Power, "Barricade",
    "Block is not removed at the start of your turn", 3) {}

void BarricadeCard::applyEffect(Character* caster, Character* target, BattleManager* bm) {
    //if (caster) 
        //caster->applyStatus(new BarricadeEffect());
}