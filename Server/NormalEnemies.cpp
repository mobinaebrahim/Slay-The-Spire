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
            int finalDamage = calculateOutgoingDamage(intentValue);
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
            int finalDamage = calculateOutgoingDamage(intentValue);
            target->takeDamage(finalDamage);
        }
    }
    else if (currentIntent == IntentType::Combined) {
        if (target) {
            int finalDamage = calculateOutgoingDamage(intentValue);
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

//__________________________________________Louse__________________________________________
Louse::Louse() : Enemy("Louse", 12, 12), hasCurledUp(false) {
    int randomHP = 10 + (rand() % 6);
    this->hp = randomHP;
    this->maxHp = randomHP;
    colorChance = 1 + (rand() % 2);
    chooseAction();
}

void Louse::chooseAction() {
    int randomIntent = 1 + (rand() % 100);

    if (randomIntent <= 75) {
        currentIntent = IntentType::Attack;
        intentValue = 5 + (rand() % 3);
        intentBlock = 0;
    }
    else {
        currentIntent = IntentType::Buff;
        intentValue = 3; 
        intentBlock = 0;
    }
}

void Louse::executeAction(Character* target) {
    if (currentIntent == IntentType::Attack) {
        if (target) {
            int finalDamage = calculateOutgoingDamage(intentValue);
            target->takeDamage(finalDamage);
        }
    }
    else if (currentIntent == IntentType::Buff) 
        this->applyStatus(new StrengthEffect(intentValue));
    chooseAction();
}

int Louse::takeDamage(int incomingDamage) {
    int actualDamage = Enemy::takeDamage(incomingDamage);

    if (this->hp > 0 && !hasCurledUp) {
        int randomBlock = 3 + (rand() % 5); 
        this->block += randomBlock;
        hasCurledUp = true; 
    }

    return actualDamage;
}

//________________________________________SmallSlime_______________________________________
SmallSlime::SmallSlime() : Enemy("SmallSlime", 10, 10) {
    int randomHP = 8 + (rand() % 5);
    this->hp = randomHP;
    this->maxHp = randomHP;
    chooseAction();
}

void SmallSlime::chooseAction() {
    int randomIntent = 1 + (rand() % 100);

    if (randomIntent <= 50) {
        currentIntent = IntentType::Attack;
        intentValue = 3;
        intentBlock = 0;
    }

    else {
        currentIntent = IntentType::Debuff;
        intentValue = 1; 
        intentBlock = 0;
    }
}

void SmallSlime::executeAction(Character* target) {
    if (currentIntent == IntentType::Attack) {
        if (target) {
            int finalDamage = calculateOutgoingDamage(intentValue);
            target->takeDamage(finalDamage);
        }
    }
    else if (currentIntent == IntentType::Debuff) {
        if (target) 
            target->applyStatus(new WeakEffect(intentValue));
    }
    chooseAction();
}

//_______________________________________MediumSlime_______________________________________
MediumSlime::MediumSlime() : Enemy("MediumSlime", 30, 30) {
    int randomHP = 28 + (rand() % 5);
    this->hp = randomHP;
    this->maxHp = randomHP;
    chooseAction();
}

void MediumSlime::chooseAction() {
    int randomIntent = 1 + (rand() % 100);

    if (randomIntent <= 30) {
        currentIntent = IntentType::Debuff; 
        intentValue = 7; 
    }
    else if (randomIntent <= 70) {
        currentIntent = IntentType::Attack; 
        intentValue = 10;
    }
    else {
        currentIntent = IntentType::Debuff; 
        intentValue = 1; 
    }
}

void MediumSlime::executeAction(Character* target) {
    if (currentIntent == IntentType::Attack) { 
        if (target) 
            target->takeDamage(calculateOutgoingDamage(intentValue));
    }
    else if (currentIntent == IntentType::Debuff) {
        if (target) {
            if (intentValue == 7) {
                target->takeDamage(calculateOutgoingDamage(7));
                Player* player = dynamic_cast<Player*>(target);
                if (player) 
                    player->addCardToDiscardPile(new SlimeCard());
            }
            else 
                target->applyStatus(new WeakEffect(1));
        }
    }
    chooseAction();
}

//_______________________________________LargeSlime________________________________________
LargeSlime::LargeSlime() : Enemy("LargeSlime", 70, 70), hasSplited(false) {
    int randomHP = 68 + (rand() % 5);
    this->hp = randomHP;
    this->maxHp = randomHP;
    chooseAction();
}

void LargeSlime::chooseAction() {
    if (this->hp <= (this->maxHp / 2) && !hasSplited) {
        currentIntent = IntentType::Combined;
        intentValue = 0;
        intentBlock = 0;
    }
    else {
        int randomIntent = 1 + (rand() % 100);
        if (randomIntent <= 30) {
            currentIntent = IntentType::Debuff; 
            intentValue = 7;
        }
        else if (randomIntent <= 70) {
            currentIntent = IntentType::Attack; 
            intentValue = 10;
        }
        else {
            currentIntent = IntentType::Debuff; 
            intentValue = 1; 
        }
    }
}

void LargeSlime::executeAction(Character* target) {
    if (currentIntent == IntentType::Combined) {
        hasSplited = true;
        //incomplete
    }
    else {
        if (currentIntent == IntentType::Attack) {
            if (target)
                target->takeDamage(calculateOutgoingDamage(intentValue));
        }
        else if (currentIntent == IntentType::Debuff) {
            if (target) {
                if (intentValue == 7) {
                    target->takeDamage(calculateOutgoingDamage(7));
                    Player* p = dynamic_cast<Player*>(target);
                    if (p)
                        p->addCardToDiscardPile(new SlimeCard());
                }
                else 
                    target->applyStatus(new WeakEffect(1));
            }
        }
    }
    chooseAction();
}

//__________________________________________Thief__________________________________________
Thief::Thief(string name) : Enemy(name, 48, 48), turnCounter(1) {
    this->hp = 44 + (rand() % 13);
    this->maxHp = this->hp;
    chooseAction();
}

void Thief::chooseAction() {
    if (turnCounter == 1 || turnCounter == 2) {
        currentIntent = IntentType::Attack; 
        intentValue = 10;    
    }
    else if (turnCounter == 3) {
        currentIntent = IntentType::Defend; 
        intentValue = 0;
        intentBlock = 6;
    }
    else {
        currentIntent = IntentType::Special; 
        intentValue = 0;
    }
}

void Thief::executeAction(Character* target) {
    if (currentIntent == IntentType::Attack) {
        if (target) {
            target->takeDamage(calculateOutgoingDamage(intentValue));
            Player* p = dynamic_cast<Player*>(target);
            if (p) {
                p->loseGold(15);
                stolenGold += 15;
            }
        }
    }
    else if (currentIntent == IntentType::Defend) 
        this->addBlock(intentBlock);
    else if (currentIntent == IntentType::Special) 
        hasScaped = true;

    turnCounter++; 
    chooseAction();
}

//_____________________________________SphericGuardian_____________________________________
SphericGuardian:: SphericGuardian() : Enemy("SphericGuardian", 0, 0), isFirstTurn(true) {
    int randomHP = 20 + (rand() % 21);
    this->hp = randomHP; 
    this->maxHp = randomHP;
    chooseAction();
}

void SphericGuardian::chooseAction() {
    if (isFirstTurn) {
        currentIntent = IntentType::Buff;
        intentBlock = 25;
    }
    else {
        int randVal = rand() % 100;
        if (randVal < 50) {
            currentIntent = IntentType::Combined;
            intentValue = 10;
            intentBlock = 15;
        }
        else {
            currentIntent = IntentType::Attack; // Slam (Attack twice)
            intentValue = 10;
            intentBlock = 0;
        }
    }
}

void SphericGuardian::executeAction(Character* target){
    if (isFirstTurn) {
        this->addBlock(intentBlock);
        target->takeDamage(calculateOutgoingDamage(10));
        target->applyStatus(new FrailEffect(5)); 
        isFirstTurn = false;
    }
    else if (currentIntent == IntentType::Combined) {
            target->takeDamage(calculateOutgoingDamage(intentValue));
            this->addBlock(intentBlock);
    }
    else {
        target->takeDamage(calculateOutgoingDamage(intentValue));
        target->takeDamage(calculateOutgoingDamage(intentValue));
    }
    chooseAction();
}

//_______________________________________BlueSlaver________________________________________
BlueSlaver:: BlueSlaver() : Enemy("BlueSlaver", 48, 48) {
    int hp = 46 + (rand() % 5);
    this->hp = hp; this->maxHp = hp;
    chooseAction();
}

void BlueSlaver::chooseAction() {
    int randVal = rand() % 100;
    if (randVal < 60) {
        currentIntent = IntentType::Attack;
        intentValue = 12;
    }
    else {
        currentIntent = IntentType::Combined; 
        intentValue = 7; 
    }
}

void BlueSlaver::executeAction(Character* target) {
    if (currentIntent == IntentType::Attack)
        target->takeDamage(calculateOutgoingDamage(intentValue));
    else if (currentIntent == IntentType::Combined) {
        target->takeDamage(calculateOutgoingDamage(intentValue));
        target->applyStatus(new WeakEffect(1)); 
    }
    chooseAction();
}

//________________________________________RedSlaver________________________________________
RedSlaver:: RedSlaver() : Enemy("RedSlaver", 48, 48), isFirstTurn(true), hasEntangled(false) {
    int hp = 46 + (rand() % 5);
    this->hp = hp; this->maxHp = hp;
    chooseAction();
}

void RedSlaver::chooseAction() {
    if (isFirstTurn) {
        currentIntent = IntentType::Attack; 
        intentValue = 13;
    }
    else {
        int randVal = rand() % 125;

        if (!hasEntangled && randVal < 25) {
            currentIntent = IntentType::Debuff;
            intentValue = 0; 
        }
        else if (randVal < 75) { 
            currentIntent = IntentType::Attack;
            intentValue = 13;
        }
        else { 
            currentIntent = IntentType::Combined;
            intentValue = 8; 
        }
    }
}

void RedSlaver:: executeAction(Character* target) {
    if (isFirstTurn)
        isFirstTurn = false;

    if (currentIntent == IntentType::Attack) {
        target->takeDamage(calculateOutgoingDamage(intentValue));
    }
    else if (currentIntent == IntentType::Debuff) {
            hasEntangled = true;
            target->applyStatus(new EntangledEffect(1));
    }
    else {
        target->takeDamage(calculateOutgoingDamage(intentValue));
        target->applyStatus(new VulnerableEffect(1));
    }
    chooseAction();
}