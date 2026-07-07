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
        target->takeDamage(calculate_total_damage(intentValue));
    }
    else if (moveIndex == 2) { 
        Player* p = dynamic_cast<Player*>(target);
        if (p)
            p->addCardToDiscardPile(new SlimeCard()); 
    }

    moveIndex = (moveIndex + 1) % 3;
    chooseAction(); 
}

void KingSlime::takeDamage(int amount) {
    Enemy::takeDamage(amount);

    if (this->hp <= (this->maxHp / 2) && !hasSplit) {
        hasSplit = true;
        myManager->spawnEnemy(new LargeSlime());
        myManager->spawnEnemy(new LargeSlime());
        myManager->removeEnemy(this);
    }
}