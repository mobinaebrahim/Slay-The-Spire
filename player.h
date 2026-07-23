#ifndef PLAYER_H
#define PLAYER_H

#include "Character.h"
#include "StatusEffect.h"
#include "card.h"
#include "enemy.h"
#include <string>
#include <vector>
#include <iostream>
using namespace std;
class BattleManager;

static const int MAX_HAND_SIZE = 10;

class Player : public Character {
private:
    int currentEnergy;
    int maxEnergy;
    int feelNoPainStacks = 0;
    int gold;
    BattleManager* battleManagerPtr;
    vector<Card*> drawPile;
    vector<Card*> hand;
    vector<Card*> discardPile;
    vector<Card*> exhaustPile;

public:
    Player(string n, int h, int max, int en, int g, BattleManager* bm);

    int getGold() const { return gold; }
    int getDrawPileSize() const { return drawPile.size(); }
    int getDiscardPileSize() const { return discardPile.size(); }
    int getExhaustPileSize() const { return exhaustPile.size(); }
    int getEnergy() const {return currentEnergy;}
    int getMaxEnergy() const { return maxEnergy; }
    int getHandSize(){ return hand.size(); }
    const vector<Card*>& getHand() const { return hand; }
    const vector<Card*>& getExhaustPile() const { return exhaustPile; }
    const vector<Card*>& getDrawPile() const { return drawPile; }
    const vector<Card*>& getDiscardPile() const { return discardPile; }
    vector<Card*> getFullDeck() const;
    void resetEnergy() { currentEnergy = maxEnergy; }

    void decreaseEnergy(int amount);
    void increaseEnergy(int amount);
    void increaseMaxHP(int amount);
    void drawCards(int count);
    void addBurnToDiscard(int count);
    bool hasCardsInExhaustPile();
    void moveCardFromExhaustToHand();
    void exhaustCard(Card* card);
    void exhaust_card_automatically(Card* card);
    Card* chooseCardFromHand(); 
    void addCopiesToHand(Card* chosenCard, int count);
    void add_block_when_exhausted();
    void TurnStartEffect(string effect) override;
    void endTurnCleanUp();
    void addCardToDrawPile(Card* card);
    void addCardToDiscardPile(Card* card);
    void loseGold(int amount);
    void playCard(Card* card, Character* target);
    int countCardsByName(string name);
    bool isHandAllAttacks();
};

#endif