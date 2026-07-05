#include "NormalEnemies.h"
#include <cstdlib> 
#include <ctime>   

//_________________________________________Cultist_________________________________________
Cultist::Cultist() : Enemy("Cultist", 50, 50), isFirstTurn(true) {
    int randomHP = 48 + (rand() % 7);
    this->hp = randomHP;
    this->maxHp = randomHP;
    chooseAction();
}

void Cultist::chooseAction() {
    if (isFirstTurn) {
        currentIntent = IntentType::Buff;
        intentValue = 3; 
    }
    else {
        currentIntent = IntentType::Attack;
        intentValue = 6; 
    }
}

void Cultist::executeAction(Character* target) {
    if (currentIntent == IntentType::Buff) {
        this->applyStatus(new StrengthEffect(intentValue));
        isFirstTurn = false; 
    }
    else if (currentIntent == IntentType::Attack) {
        if (target) {
            int finalDamage = calculate_total_damage(intentValue);
            target->takeDamage(finalDamage);
        }
    }
    chooseAction();
}

//_________________________________________JawWorm_________________________________________
JawWorm::JawWorm() : Enemy("JawWorm", 44, 44), isFirstTurn(true) {
    int randomHP = 42 + (rand() % 5);
    this->hp = randomHP;
    this->maxHp = randomHP;
    chooseAction();
}

void JawWorm::chooseAction() {
    if (isFirstTurn) {
        currentIntent = IntentType::Attack;
        intentValue = 11;
        intentBlock = 0;
    }
    else {
        int randomIntent = 1 + (rand() % 100);

        if (randomIntent >= 1 && randomIntent <= 25) {
            currentIntent = IntentType::Attack;
            intentValue = 11;
            intentBlock = 0;
        }
        if (randomIntent >= 26 && randomIntent <= 55) {
            currentIntent = IntentType::Combined;
            intentValue = 7;  
            intentBlock = 5;
        }
        else if (randomIntent >= 56 && randomIntent <= 100) {
            currentIntent = IntentType::Buff;
            intentValue = 3;   
            intentBlock = 6; 
        }
    }
}

void JawWorm::executeAction(Character* target) {
    if (currentIntent == IntentType::Attack) {
        if (target) {
            int finalDamage = calculate_total_damage(intentValue);
            target->takeDamage(finalDamage);
        }
    }
    else if (currentIntent == IntentType::Combined) {
        if (target) {
            int finalDamage = calculate_total_damage(intentValue);
            target->takeDamage(finalDamage);
        }
        this->block += intentBlock;
    }
    else if (currentIntent == IntentType::Buff) {
        this->block += intentBlock;
        this->applyStatus(new StrengthEffect(intentValue));
    }
    isFirstTurn = false;
    chooseAction();
}