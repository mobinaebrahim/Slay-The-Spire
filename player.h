#ifndef PLAYER_H
#define PLAYER_H

#include "Character.h"
#include "StatusEffect.h"
#include "card.h"
#include <string>
#include <vector>
#include <iostream>
using namespace std;

class Player : public Character {
private:
    int currentEnergy;
    int maxEnergy;
    int feelNoPainStacks = 0;
    int gold;
    vector<Card*> drawPile;
    vector<Card*> hand;
    vector<Card*> discardPile;
    vector<Card*> exhaustPile;

public:
    Player(string n, int h, int max, int en);

    void decreaseEnergy(int amount);
    void increaseEnergy(int amount);
    int getEnergy() const;
    void increaseMaxHP(int amount);
    void drawCards(int count);
    void addBurnToDiscard(int count);
    bool hasCardsInExhaustPile();
    void moveCardFromExhaustToHand();
    void exhaustCard(Card* card);
    void exhaust_card_automatically(Card* card);
    Card* chooseCardFromHand(); 
    void addCopiesToHand(Card* chosenCard, int count);
    int getHandSize();
    void add_block_when_exhausted();
    void TurnStartEffect(string effect) override;
    const vector<Card*>& getHand() const { return hand; }
    void endTurnCleanUp();
    void addCardToDrawPile(Card* card);
    void addCardToDiscardPile(Card* card);
    void loseGold(int amount);
};

#endif