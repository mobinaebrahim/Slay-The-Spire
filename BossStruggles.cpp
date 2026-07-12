#include "BossStruggles.h"

//__________________________________________KingSlime__________________________________________
KingSlime::KingSlime(BattleManager* manager) : Enemy("KingSlime", 140, 140), myManager(manager) {
    this->moveIndex = 0;
    this->hasSplit = false;
    chooseAction();
}

void KingSlime::chooseAction() {
    if (moveIndex == 0) 
        currentIntent = IntentType::Special;

    else if (moveIndex == 1) {
        currentIntent = IntentType::Attack;
        intentValue = 35; 
    }
    else 
        currentIntent = IntentType::Debuff; 
}

void KingSlime::executeAction(Character* target) {
    if (moveIndex == 1) { 
        target->takeDamage(calculateOutgoingDamage(intentValue));
    }
    else if (moveIndex == 2) { 
        Player* p = dynamic_cast<Player*>(target);
        if (p)
            p->addCardToDiscardPile(new SlimeCard()); 
    }

    moveIndex = (moveIndex + 1) % 3;
    chooseAction(); 
}

int KingSlime::takeDamage(int amount) {
    int actualDamage = Enemy::takeDamage(amount);

    if (this->hp <= (this->maxHp / 2) && !hasSplit) {
        hasSplit = true;
        myManager->spawnEnemy(new LargeSlime());
        myManager->spawnEnemy(new LargeSlime());
        myManager->removeEnemy(this);
    }
    return actualDamage;
}

//__________________________________________Hexaghost__________________________________________
Hexaghost::Hexaghost() : Enemy("Hexaghost",250 ,250), turnCount(0) {
    chooseAction();
}

void Hexaghost::chooseAction() {
    if (turnCount == 1)
        currentIntent = IntentType::Special;
    else if (turnCount == 2) 
        currentIntent = IntentType::Attack;
    else {
        int cycleIndex = (turnCount - 3) % 7;

        switch (cycleIndex) {
        case 0: 
            currentIntent = IntentType::Combined;
            intentValue = 6;
            break;
        case 1: 
            currentIntent = IntentType::Attack;
            intentValue = 5;
            break;
        case 2: 
            currentIntent = IntentType::Combined;
            intentValue = 6;
            break;
        case 3: 
            currentIntent = IntentType::Buff; 
            intentBlock = 12;
            break;
        case 4: 
            currentIntent = IntentType::Attack;
            intentValue = 5;
            break;
        case 5: 
            currentIntent = IntentType::Combined;
            intentValue = 6;
            break;
        case 6: 
            currentIntent = IntentType::Combined;
            intentValue = 2;
            break;
        }
    }
}

void Hexaghost::executeAction(Character* target) {
    if (turnCount == 2) {
        for (int i = 0; i < 6; i++)
            target->takeDamage(calculateOutgoingDamage(getHp() / 12 + 1));
    }
    else if (currentIntent == IntentType::Combined ) {
        if (intentValue == 6) {
            target->takeDamage(calculateOutgoingDamage(intentValue));
            Player* p = dynamic_cast<Player*>(target);
            if (p)
                p->addCardToDiscardPile(new BurnCard());
        }
        else {
            for (int i = 0; i < 6; i++)
                target->takeDamage(calculateOutgoingDamage(intentValue));
            Player* p = dynamic_cast<Player*>(target);
            if (p) {
                for (int i = 0; i < 3; i++)
                    p->addCardToDiscardPile(new BurnCard());
            }
        }
    }
    else if (currentIntent == IntentType::Attack) {
        target->takeDamage(calculateOutgoingDamage(intentValue));
        target->takeDamage(calculateOutgoingDamage(intentValue));
    }
    else {
        this->addBlock(intentBlock);
        this->applyStatus(new StrengthEffect(2));
    }

    turnCount++;
}

//__________________________________________TheChamp___________________________________________
TheChamp::TheChamp() : Enemy("TheChamp", 150, 150), isTauntTurn(false) {
    int randomHp = 150 + (rand() % 101);
    this->hp = randomHp;
    this->maxHp = randomHp;
}

void TheChamp::chooseAction() {
    int randVal = rand() % 100;

    if (isTauntTurn) 
        currentIntent = IntentType::Debuff;
    else {
        if(randVal <= 15)
            currentIntent = IntentType::Special;
        if(randVal > 15 && randVal <= 30)
            currentIntent = IntentType::Buff;
        if(randVal > 30 && randVal <= 55)
            currentIntent = IntentType::Combined;
        else
            currentIntent = IntentType::Attack;
    }
}

void TheChamp::executeAction(Character* target) {
    if (currentIntent == IntentType::Debuff) {
        target->applyStatus(new WeakEffect(2));
        target->applyStatus(new VulnerableEffect(2));
    }
    else if (currentIntent == IntentType::Special) {
        this->addBlock(15);
        this->applyStatus(new MetallicizeEffect(5));
    }
    else if (currentIntent == IntentType::Combined) {
        target->takeDamage(calculateOutgoingDamage(12));
        target->applyStatus(new FrailEffect(2));
        target->applyStatus(new VulnerableEffect(2));
    }
    else if (currentIntent == IntentType::Attack) {
        target->takeDamage(calculateOutgoingDamage(8));
        target->takeDamage(calculateOutgoingDamage(8));
    }
    else {
        this->applyStatus(new StrengthEffect(2));
    }
    isTauntTurn = !isTauntTurn;

    chooseAction();
}