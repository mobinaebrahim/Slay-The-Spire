#include "EliteEnemies.h"

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
        target->takeDamage(calculate_total_damage(intentValue));

    else {
        target->takeDamage(calculate_total_damage(intentValue));
        target->applyStatus(new VulnerableEffect(2));
    }
    chooseAction();
}

//______________________________________ThreeSentries______________________________________
ThreeSentries::ThreeSentries(string name, int minHp, int maxHp) : Enemy("ThreeSentries", 38, 42) {
    int hp = 38 + (rand() % 4);
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
        target->takeDamage(calculate_total_damage(intentValue));
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