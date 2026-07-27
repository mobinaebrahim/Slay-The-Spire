#include "Relics.h"
#include "Player.h"
#include "BattleManager.h"
#include "card.h"
#include "StatusEffect.h"
#include "StatusCard.h"
#include "CurseCard.h"
#include <cstdlib>

//_________________________________ Relic (base) _________________________________
Relic::Relic(string n, string d) : name(n), description(d) {}
string Relic::getName() const { return name; }
string Relic::getDescription() const { return description; }

//_________________________________ BurningBlood _________________________________
BurningBlood::BurningBlood() : Relic("Burning Blood", "At the end of combat, heal 6 HP.") {}
void BurningBlood::onCombatEnd(Player* owner, BattleManager* bm) {
    if (owner)
        owner->increaseHP(6);
}

//_____________________________________ Girya ____________________________________
Girya::Girya() : Relic("Girya", "You may lift at campsites (up to 3 times) to gain 1 permanent Strength.") {}
void Girya::onCampsite(Player* owner) {
    if (owner && liftsRemaining > 0) {
        owner->applyStatus(new StrengthEffect(1));
        liftsRemaining--;
    }
}

//___________________________________ IceCream ___________________________________
IceCream::IceCream() : Relic("Ice Cream", "Energy is now conserved between turns.") {}
// will complete in Player::resetEnergy 

//___________________________________ Shuriken ___________________________________
Shuriken::Shuriken() : Relic("Shuriken", "Every time you play 3 attacks in a turn, gain 1 Strength.") {}
void Shuriken::onCardPlayed(Player* owner, Card* card) {
    if (!owner || !card)
        return;
    if (card->getType() == CardType::Attack) {
        attacksPlayedThisTurn++;
        if (attacksPlayedThisTurn % 3 == 0)
            owner->applyStatus(new StrengthEffect(1));
    }
}
void Shuriken::onTurnStart(Player* owner, BattleManager* bm) {
    attacksPlayedThisTurn = 0;
}

//_____________________________________ Kunai ____________________________________
Kunai::Kunai() : Relic("Kunai", "Every time you play 3 attacks in a turn, gain 1 Dexterity.") {}
void Kunai::onCardPlayed(Player* owner, Card* card) {
    if (!owner || !card)
        return;
    if (card->getType() == CardType::Attack) {
        attacksPlayedThisTurn++;
        if (attacksPlayedThisTurn % 3 == 0)
            owner->applyStatus(new DexterityEffect(1));
    }
}
void Kunai::onTurnStart(Player* owner, BattleManager* bm) {
    attacksPlayedThisTurn = 0;
}

//_______________________________ PreservedInsect ________________________________
PreservedInsect::PreservedInsect() : Relic("Preserved Insect", "Elites start with 25% less HP.") {}
// will complete in BattleManager::spawnEnemy 

//_________________________________ CallingBell __________________________________
CallingBell::CallingBell() : Relic("Calling Bell", "Obtain the Curse of the Bell and 3 random normal relics.") {}
void CallingBell::onObtain(Player* owner) {
    if (!owner)
        return;
    owner->addCardToDrawPile(new CurseOfBellCard());
    // incomplete
}

//__________________________________ MarkOfPain __________________________________
MarkOfPain::MarkOfPain() : Relic("Mark of Pain", "Gain 1 extra energy per turn. Start combat with 2 Wounds in draw pile.") {}
void MarkOfPain::onTurnStart(Player* owner, BattleManager* bm) {
    if (owner)
        owner->increaseEnergy(1);
}
void MarkOfPain::onCombatStart(Player* owner, BattleManager* bm) {
    if (owner) {
        owner->addCardToDrawPile(new WoundCard());
        owner->addCardToDrawPile(new WoundCard());
    }
}

//_________________________________ VelvetChoker _________________________________
VelvetChoker::VelvetChoker() : Relic("Velvet Choker", "Gain 1 extra energy per turn. Cannot play more than 6 cards per turn.") {}
void VelvetChoker::onTurnStart(Player* owner, BattleManager* bm) {
    if (owner)
        owner->increaseEnergy(1);
    //incomplete
}

//_________________________________ SlaversCollar ________________________________
SlaversCollar::SlaversCollar() : Relic("Slaver's Collar", "During Boss and Elite combats, gain 1 Energy at the start of your turn.") {}
void SlaversCollar::onTurnStart(Player* owner, BattleManager* bm) {
    //if (owner && bm && bm->isBossOrEliteCombat())
    //    owner->increaseEnergy(1);
}