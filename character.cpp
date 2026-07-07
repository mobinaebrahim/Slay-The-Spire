#include "Character.h"

Character::Character(string n, int h, int max): name(n), hp(h), maxHp(max), block(0), timesDamagedThisCombat(0) {}

void Character::decreaseHp(int amount) {
    hp -= amount;
    if (hp < 0)
        hp = 0;
}

int Character::takeDamage(int incomingDamage) {
    int finalDamage = calculateIncomingDamage(incomingDamage);

    int damageToHp = 0;
    if (finalDamage <= block) {
        block -= finalDamage;
    }
    else {
        damageToHp = finalDamage - block;
        block = 0;
        hp -= damageToHp;
    }

    return damageToHp;
}

void Character::addBlock(int amount) {
    block += amount;
}

void Character::increaseHP(int amount) {
    hp += amount;
    if (hp > maxHp)
        hp = maxHp;
}

int Character::calculate_total_block(int amount) {
    int finalBlock = amount;

    int dexterity = getStatusValue("Dexterity");
    finalBlock += dexterity;

    int frail = getStatusValue("Frail");
    if (frail > 0) 
        finalBlock = finalBlock * 3 / 4; 

    return finalBlock;
}

int Character::getStatusValue(string status) {
    for (auto effect : effects) {
        if (effect->getName() == status) 
            return effect->getAmount(); 
    }
    return 0; 
}

void Character::applyStatus(StatusEffect* newEffect) {
    if (!newEffect)
        return;

    for (auto effect : effects) {
        if (effect->getName() == newEffect->getName()) {
            effect->setAmount(effect->getAmount() + newEffect->getAmount());
            delete newEffect; 
            return;
        }
    }
    effects.push_back(newEffect);
}

void Character::keep_block_until_next_turn() {
    retainBlock = true; 
}

void Character::TurnStartEffect(string effect) {
    if (effect == "DemonForm") 
        applyStatus(new StrengthEffect(3));
    if(effect == "Brutality")
        decreaseHp(1);
}

void Character::setBlock(int amount) {
    block = amount;
}

bool Character::hasEffect(string effectName) {
    for (auto* effect : effects) {
        if (effect->getName() == effectName && effect->getAmount() > 0)
            return true;
    }
    return false;
}

int Character::calculateOutgoingDamage(int baseDamage) {
    for (auto* effect : effects)
        baseDamage = effect->modifyOutgoingDamage(baseDamage);
    return baseDamage;
}

int Character::calculateIncomingDamage(int baseDamage) {
    for (auto* effect : effects)
        baseDamage = effect->modifyIncomingDamage(baseDamage);
    return baseDamage;
}

int Character::getTimesDamagedThisCombat() const {
    return timesDamagedThisCombat;
}