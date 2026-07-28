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

    if (this->hp > 0 && this->hp <= (this->maxHp / 2) && !hasSplit) {
        hasSplit = true;
        int remainingHp = this->hp;

        LargeSlime* slime1 = new LargeSlime();
        LargeSlime* slime2 = new LargeSlime();
        slime1->setMaxHp(remainingHp);
        slime1->setHp(remainingHp);
        slime2->setMaxHp(remainingHp);
        slime2->setHp(remainingHp);

        myManager->spawnEnemy(slime1);
        myManager->spawnEnemy(slime2);
        myManager->removeEnemy(this);
    }
    return actualDamage;
}

//__________________________________________Hexaghost__________________________________________
Hexaghost::Hexaghost() : Enemy("Hexaghost", 250, 250), turnCount(1) {
    chooseAction();
}

void Hexaghost::chooseAction() {
    if (turnCount == 1)
        currentIntent = IntentType::Special;
    else if (turnCount == 2)
        currentIntent = IntentType::Special;
    else {
        int cycleIndex = (turnCount - 3) % 7;
        switch (cycleIndex) {
        case 0:
            currentIntent = IntentType::AttackAddCard;
            intentValue = 6;
            break;
        case 1:
            currentIntent = IntentType::Attack;
            intentValue = 5;
            break;
        case 2:
            currentIntent = IntentType::AttackAddCard;
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
            currentIntent = IntentType::AttackAddCard;
            intentValue = 6;
            break;
        case 6:
            currentIntent = IntentType::AttackAddCard;
            intentValue = 2;
            break;
        }
    }
}

void Hexaghost::executeAction(Character* target) {
    if (turnCount == 1) {
    }
    else if (turnCount == 2) {
        int dmgPerHit = target->getHp() / 12 + 1;
        for (int i = 0; i < 6; i++)
            target->takeDamage(calculateOutgoingDamage(dmgPerHit));
    }
    else if (currentIntent == IntentType::AttackAddCard) {
        if (intentValue == 6) {
            target->takeDamage(calculateOutgoingDamage(intentValue));
            Player* p = dynamic_cast<Player*>(target);
            if (p)
                p->addCardToDiscardPile(new BurnCard());
        }
        else {
            Player* p = dynamic_cast<Player*>(target);
            if (p)
                p->upgradeAllBurns();

            for (int i = 0; i < 6; i++)
                target->takeDamage(calculateOutgoingDamage(intentValue));

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
    else if (currentIntent == IntentType::Buff) {
        this->addBlock(intentBlock);
        this->applyStatus(new StrengthEffect(2));
    }

    turnCount++;
    chooseAction();
}

//__________________________________________TheChamp___________________________________________
TheChamp::TheChamp() : Enemy("TheChamp", 150, 150), isTauntTurn(false) {
    int randomHp = 150 + (rand() % 101);
    this->hp = randomHp;
    this->maxHp = randomHp;
    chooseAction();
}

void TheChamp::chooseAction() {
    int randVal = rand() % 100;
    if (isTauntTurn) {
        currentIntent = IntentType::Debuff;
        intentValue = 2;
    }
    else {
        if (randVal <= 15) {
            currentIntent = IntentType::Special;
            intentValue = 15;
        }
        else if (randVal <= 30) {
            currentIntent = IntentType::Buff;
            intentValue = 2;
        }
        else if (randVal <= 55) {
            currentIntent = IntentType::AttackDebuff;
            intentValue = 12;
        }
        else {
            currentIntent = IntentType::Attack;
            intentValue = 16;
        }
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
    else if (currentIntent == IntentType::AttackDebuff) {
        target->takeDamage(calculateOutgoingDamage(12));
        target->applyStatus(new FrailEffect(2));
        target->applyStatus(new VulnerableEffect(2));
    }
    else if (currentIntent == IntentType::Attack) {
        target->takeDamage(calculateOutgoingDamage(intentValue));
    }
    else {
        this->applyStatus(new StrengthEffect(2));
    }
    isTauntTurn = !isTauntTurn;

    chooseAction();
}