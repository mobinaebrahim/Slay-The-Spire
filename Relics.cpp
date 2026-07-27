#include "Relics.h"
#include "Player.h"
#include "BattleManager.h"
#include "card.h"
#include "StatusEffect.h"
#include "StatusCard.h"
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