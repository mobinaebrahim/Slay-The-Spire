#include "StatusEffect.h"
#include "character.h"
#include "Player.h"

//_______________________________________StatusEffect_______________________________________
StatusEffect:: StatusEffect(string name, int amount) : name(name), amount(amount) {}

string StatusEffect::getName() const { return name; }
int StatusEffect::getAmount() const { return amount; }
void StatusEffect::setAmount(int value) { amount = value; }

int StatusEffect::modifyOutgoingDamage(int baseDamage) { return baseDamage; }
int StatusEffect::modifyIncomingDamage(int baseDamage) { return baseDamage; }
int StatusEffect::modifyBlock(int baseBlock) { return baseBlock; }

//______________________________________StrengthEffect______________________________________
StrengthEffect::StrengthEffect(int increaseDamageBy) : StatusEffect("Strength", increaseDamageBy) {}

int StrengthEffect::modifyOutgoingDamage(int baseDamage) { return baseDamage + amount; }

//______________________________________DexterityEffect_____________________________________
DexterityEffect::DexterityEffect(int increaseBlockBy) : StatusEffect("Dexterity", increaseBlockBy) {}

int DexterityEffect::modifyBlock(int baseBlock) { return baseBlock + amount; }

//_____________________________________VulnerableEffect_____________________________________
VulnerableEffect::VulnerableEffect(int turns) : StatusEffect("Vulnerable", turns) {}

int VulnerableEffect::modifyIncomingDamage(int baseDamage) {
    if (amount > 0) 
        return baseDamage * 1.5; 
    return baseDamage;
}

void VulnerableEffect::onTurnEnd(Character* owner) {
    if (amount > 0)
        amount--;
}

//________________________________________WeakEffect________________________________________
WeakEffect::WeakEffect(int turns) : StatusEffect("Weak", turns) {}

int WeakEffect::modifyOutgoingDamage(int baseDamage) {
    if (amount > 0)
        return baseDamage * 0.5;
    return baseDamage;
}

void WeakEffect::onTurnEnd(Character* owner) {
    if (amount > 0)
        amount--;
}

//________________________________________FrailEffect_______________________________________
FrailEffect::FrailEffect(int turns) : StatusEffect("Frail", turns) {}

int FrailEffect::modifyBlock(int baseBlock) {
    if (amount > 0) 
        return baseBlock * 0.75; 
    return baseBlock;
}

void FrailEffect::onTurnEnd(Character* owner) {
    if (amount > 0) 
        amount--; 
}

//_____________________________________MetallicizeEffect____________________________________
MetallicizeEffect::MetallicizeEffect(int increaseBlockBy) : StatusEffect("Metallicize", increaseBlockBy) {}

void MetallicizeEffect::onTurnEnd(Character* owner) {
    if (amount > 0)
        owner->addBlock(amount);
}

//_____________________________________EntangledEffect______________________________________
EntangledEffect::EntangledEffect(int turns) : StatusEffect("Entangled", turns) {}

void EntangledEffect:: onTurnEnd(Character* owner) {
    if (amount > 0) 
        amount--;
}

//______________________________________DemonFormEffect_____________________________________
DemonFormEffect::DemonFormEffect(int amount) : StatusEffect("DemonForm", amount) {}

void DemonFormEffect::onTurnStart(Character* owner) {
    owner->applyStatus(new StrengthEffect(amount));
}

//______________________________________BrutalityEffect_____________________________________
BrutalityEffect::BrutalityEffect() : StatusEffect("Brutality", 1) {}

void BrutalityEffect::onTurnStart(Character* owner) {
    owner->decreaseHp(1);
    Player* player = dynamic_cast<Player*>(owner);
    if (player)
        player->drawCards(1);
}