#include "card.h"

Card::Card(CardType t, string n, string d, int e): type(t), name(n), description(d), energyCost(e){}

Card::~Card() = default;

string Card::getName() const { return name; }

string Card::getDescription() const { return description; }

int Card::getCost(Character* caster) { return energyCost; }

CardType Card::getType() const { return type; }

bool Card::isPlayable() const { return true; }

void Card::upgrade() { isUpgraded = true; }