#include "Potion.h"

//_______________________________________Potion_______________________________________
Potion::Potion(string n, string d) : name(n), description(d){}

string Potion::getName() const { return name; }

string Potion::getDescription() const { return description; }

bool Potion::isPlayable() const { return true; }