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