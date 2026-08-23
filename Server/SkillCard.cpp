#include "SkillCard.h"

//______________________________________SkillCard______________________________________
SkillCard::SkillCard(CardType type, string name, string description, int cost, int baseB)
    : Card(type, name, description, cost), baseBlock(baseB) {}

void SkillCard::applyEffect(Character* caster, Character* target, BattleManager* bm) {}

int SkillCard::getBaseBlock() const { return baseBlock; } 

//______________________________________DefendCard_____________________________________
DefendCard::DefendCard(): SkillCard(CardType::Skill, "Defend",
    "Gain 5 block", 1, 5){}

void DefendCard::upgrade() {
    Card::upgrade(); 
    baseBlock = 8; 
}

void DefendCard::applyEffect(Character * caster, Character * target, BattleManager * bm) {
    if (caster) {
        int finalBlock = caster->calculate_total_block(baseBlock);
        caster->addBlock(finalBlock);
    }
}

//_____________________________________ExhumeCard______________________________________
ExhumeCard::ExhumeCard():SkillCard(CardType::Skill, "Exhume",
    "Put a card from exhaust pile into hand - Exhaust", 1, 0) {}

void ExhumeCard::upgrade() {
    Card::upgrade();
}

int ExhumeCard::getCost(Character* caster) {
    return isUpgraded ? 0 : 1;
}

void ExhumeCard::applyEffect(Character* caster, Character* target, BattleManager* bm) {
    Player* player = dynamic_cast <Player*> (caster);
    if (player) {
        if (player->hasCardsInExhaustPile()) 
            player->moveCardFromExhaustToHand();
        else 
            cout << " -> Exhaust pile is empty!\n";
         player->exhaustCard(this);
    }
}

//___________________________________LimitBreakCard____________________________________
LimitBreakCard::LimitBreakCard():SkillCard(CardType::Skill, "LimitBreak",
    "Double your Strength - Exhaust", 1, 0) {}

void LimitBreakCard::upgrade() {
    Card::upgrade();
}

void LimitBreakCard::applyEffect(Character* caster, Character* target, BattleManager* bm) {
    if (caster) {
        int currentStr = caster->getStatusValue("Strength");
        caster->applyStatus(new StrengthEffect(currentStr));
        if (!isUpgraded) {
            Player* player = dynamic_cast <Player*> (caster);
            if (player) player->exhaustCard(this);
        }
    }
}

//____________________________________OfferingCard_____________________________________
OfferingCard::OfferingCard():SkillCard(CardType::Skill, "Offering",
    "Lose 6 HP - Gain 2 Energy - Draw 3 cards - Exhaust", 0, 0) {}

void OfferingCard::upgrade() {
    Card::upgrade();
}

void OfferingCard::applyEffect(Character* caster, Character* target, BattleManager* bm) {
    Player* player = dynamic_cast<Player*>(caster);
    if (player) {
        caster->decreaseHp(6);
        player->increaseEnergy(2);
        player->drawCards(isUpgraded ? 5 : 3);
        player->exhaustCard(this);
    }
}

//____________________________________ImperviousCard___________________________________
ImperviousCard::ImperviousCard(): SkillCard(CardType::Skill, "Impervious",
    "Gain 30 block - Exhaust", 2, 30) {}

void ImperviousCard::upgrade() {
    Card::upgrade();
    baseBlock = 40;
}

void ImperviousCard::applyEffect(Character* caster, Character* target, BattleManager* bm) {
    if (caster) {
        caster->addBlock(caster->calculate_total_block(baseBlock));
        Player* player = dynamic_cast<Player*>(caster);
        if(player) player->exhaustCard(this);
    }
}

//____________________________________DualWieldCard____________________________________
DualWieldCard::DualWieldCard(): SkillCard(CardType::Skill, "DualWield",
    "Choose a card in hand - Add 2 copies into hand", 1, 0) {}

void DualWieldCard::upgrade() {
    Card::upgrade();
}

void DualWieldCard::applyEffect(Character* caster, Character* target, BattleManager* bm) {
    Player* player = dynamic_cast<Player*>(caster);
    if (player) {
        Card* chosen = player->chooseCardFromHand();
        player->addCopiesToHand(chosen, isUpgraded ? 3 : 2);
    }
}

//____________________________________EntrenchCard_____________________________________
EntrenchCard::EntrenchCard(): SkillCard(CardType::Skill, "Entrench",
    "Double your Block", 2, 0) {}

void EntrenchCard::upgrade() {
    Card::upgrade();
}

int EntrenchCard::getCost(Character* caster) {
    return isUpgraded ? 1 : 2;
}

void EntrenchCard::applyEffect(Character* caster, Character* target, BattleManager* bm) {
    if (caster) {
        int current = caster->getBlock();
        caster->addBlock(current); 
    }
}

//___________________________________ShrugItOffCard____________________________________
ShrugItOffCard::ShrugItOffCard(): SkillCard(CardType::Skill, "ShrugItOff",
    "Draw 1 card - Gain 8 block", 1, 8) {}

void ShrugItOffCard::upgrade() {
    Card::upgrade();
    baseBlock = 11;
}

void ShrugItOffCard::applyEffect(Character* caster, Character* target, BattleManager* bm) {
    if (caster) {
        caster->addBlock(caster->calculate_total_block(baseBlock));
        Player* player = dynamic_cast<Player*>(caster);
        if (player) player->drawCards(1);
    }
}

//___________________________________BloodlettingCard__________________________________
BloodlettingCard::BloodlettingCard(): SkillCard(CardType::Skill, "Bloodletting",
    "Lose 3 HP - Gain 2 Energy", 0, 0) {}

void BloodlettingCard::upgrade() {
    Card::upgrade();
}

void BloodlettingCard::applyEffect(Character* caster, Character* target, BattleManager* bm) {
    if (caster) {
        caster->decreaseHp(3);
        Player* player = dynamic_cast<Player*>(caster);
        if (player) player->increaseEnergy(isUpgraded ? 3 : 2);
    }
}

//_____________________________________DisarmCard______________________________________
DisarmCard::DisarmCard(): SkillCard(CardType::Skill, "Disarm",
    "Enemy loses 2 Strength - Exhaust", 1, 0) {}

void DisarmCard::upgrade() {
    Card::upgrade();
}
void DisarmCard::applyEffect(Character* caster, Character* target, BattleManager* bm) {
    if (target) {
        target->applyStatus(new StrengthEffect(isUpgraded ? -3 : -2));
        Player* player = dynamic_cast<Player*>(caster);
        if (player) player->exhaustCard(this);
    }
}