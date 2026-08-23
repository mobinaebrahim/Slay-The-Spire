#ifndef CARDFACTORY_H
#define CARDFACTORY_H
#include "card.h"
#include <string>

Card* createCardByName(const std::string& name);

#endif