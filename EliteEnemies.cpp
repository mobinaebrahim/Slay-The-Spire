#include "EliteEnemies.h"
#include "BattleManager.h"

//_______________________________________GremlinKnob_______________________________________
GremlinKnob::GremlinKnob() : Enemy("GremlinKnob", 82, 86) {
    int hp = 82 + (rand() % 5);
    this->hp = hp;
    this->maxHp = hp;
    chooseAction();
}

void GremlinKnob::onPlayerPlayedCard(Card* card) {
    if (card->getType() == CardType::Skill) {
        this->applyStatus(new StrengthEffect(2));
    }
}

void GremlinKnob::chooseAction() {
    int randVal = rand() % 100;

    if (randVal < 67) {
        currentIntent = IntentType::Attack;
        intentValue = 14; 
    }
    else {
        currentIntent = IntentType::Combined; 
        intentValue = 6; 
    }
}

void GremlinKnob::executeAction(Character* target) {
    if (currentIntent == IntentType::Attack) 
        target->takeDamage(calculateOutgoingDamage(intentValue));

    else {
        target->takeDamage(calculateOutgoingDamage(intentValue));
        target->applyStatus(new VulnerableEffect(2));
    }
    chooseAction();
}

//______________________________________ThreeSentries______________________________________
ThreeSentries::ThreeSentries(string name, int minHp, int maxHp) : Enemy("ThreeSentries", 38, 42) {
    int hp = minHp + (rand() % (maxHp - minHp + 1));
    this->hp = hp;
    this->maxHp = hp;
}

void ThreeSentries::chooseAction() {
    if (isNextMoveBeam) {
        currentIntent = IntentType::Attack;
        intentValue = 9;
    }
    else {
        currentIntent = IntentType::Debuff;
        intentValue = 0;
    }
    isNextMoveBeam = !isNextMoveBeam;
}

void ThreeSentries::executeAction(Character* target) {
    if (currentIntent == IntentType::Attack) 
        target->takeDamage(calculateOutgoingDamage(intentValue));
    else {
        Player* player = dynamic_cast <Player*> (target);
        player->addCardToDiscardPile(new DazeCard());
    }
    chooseAction(); 
}

MiddleSentry::MiddleSentry() : ThreeSentries("MiddleSentry", 38, 42) {
    isNextMoveBeam = true; 
    chooseAction();
}

SideSentry::SideSentry() : ThreeSentries("SideSentry", 38, 42) {
    isNextMoveBeam = false; 
    chooseAction();
}

void ThreeSentries::spawnGroup(BattleManager* bm) {
    if (!bm) return;
    bm->spawnEnemy(new SideSentry());
    bm->spawnEnemy(new MiddleSentry());
    bm->spawnEnemy(new SideSentry());
}

//______________________________________BookOfStabbing_____________________________________
BookOfStabbing::BookOfStabbing() : Enemy("BookOfStabbing", 160, 162) {
    int hp = 160 + (rand() % 3);
    this->hp = hp;
    this->maxHp = hp;
    this->stabCount = 0; 
    chooseAction();
}

void BookOfStabbing::chooseAction() {
    int randVal = rand() % 100;

    if (randVal < 85) {
        currentIntent = IntentType::Attack;
        intentValue = stabCount + 2;
    }
    else {
        currentIntent = IntentType::Attack;
        intentValue = 21;
    }
}

void BookOfStabbing::executeAction(Character* target) {
    if (intentValue == 21) {
        target->takeDamage(calculateOutgoingDamage(21));
    }
    else {
        for (int i = 0; i < intentValue; i++) 
            target->takeDamage(calculateOutgoingDamage(6)); 
        stabCount++;
    }
    chooseAction();
}

//_______________________________________Taskmaster________________________________________
Taskmaster::Taskmaster() : Enemy("Taskmaster", 54, 60) {
    int hp = 54 + (rand() % 7); 
    this->hp = hp;
    this->maxHp = hp;
    chooseAction();
}

void Taskmaster::chooseAction() {
    currentIntent = IntentType::Combined;
    intentValue = 7;
}

void Taskmaster::executeAction(Character* target) {
    target->takeDamage(calculateOutgoingDamage(intentValue));

    Player* player = dynamic_cast<Player*>(target);
    if (player) 
        player->addCardToDiscardPile(new WoundCard());

    chooseAction();
}