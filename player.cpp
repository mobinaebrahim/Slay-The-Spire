#include "Player.h"
#include "BattleManager.h"
#include <algorithm>
#include <random>
#include <chrono>

extern Card* createCardByName(const std::string& name);

Player::Player(string n, int h, int max, int energy, int g, BattleManager* bm):
    Character(n, h, max), currentEnergy(energy), maxEnergy(energy), gold(99), battleManagerPtr(bm) {}

void Player::decreaseEnergy(int amount) {
    currentEnergy -= amount;
    if (currentEnergy < 0)
        currentEnergy = 0;
}

void Player::increaseEnergy(int amount) {
    currentEnergy += amount;
}

void Player::increaseMaxHP(int amount) {
    maxHp += amount;
    hp += amount;
}

void Player::drawCards(int count) {
    for (int i = 0; i < count; i++) {
        if (drawPile.empty()) {
            if (discardPile.empty())
                break;

            drawPile = discardPile;
            discardPile.clear();

            unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
            std::default_random_engine motor(seed);

            std::shuffle(drawPile.begin(), drawPile.end(), motor);
        }

        Card* topCard = drawPile.back();
        drawPile.pop_back();
        hand.push_back(topCard);
    }
}

bool Player::hasCardsInExhaustPile() {
    return !exhaustPile.empty();
}

void Player::moveCardFromExhaustToHand() {
    if (!exhaustPile.empty()) {
        Card* recoveredCard = exhaustPile.back();
        exhaustPile.pop_back();
        hand.push_back(recoveredCard);
    }
}

void Player::exhaustCard(Card* card) {
    for (auto it = hand.begin(); it != hand.end(); ++it) {
        if (*it == card) {
            exhaustPile.push_back(card);
            hand.erase(it);
            add_block_when_exhausted();

            for (auto* effect : effects)
                effect->onCardExhausted(this);

            break;
        }
    }
}

void Player::exhaust_card_automatically(Card* card) {
    exhaustCard(card); 
}

Card* Player::chooseCardFromHand() {
    if (hand.empty())
        return nullptr;

    int handSize = hand.size();
    int randomCard = rand() % (handSize + 1);

    return hand[randomCard];
}

void Player::addCopiesToHand(Card* chosenCard, int count) {
    if (!chosenCard)
        return;
    for (int i = 0; i < count; i++){
        Card* newCopy = createCardByName(chosenCard->getName());
        if (newCopy)
            hand.push_back(newCopy);
    }
}

void Player::add_block_when_exhausted() {
    if (feelNoPainStacks > 0) {
        int baseBlock = 3 * feelNoPainStacks;
        int blockToApply = calculate_total_block(baseBlock);
        addBlock(blockToApply);
    }
}

void Player::addBurnToDiscard(int count) {
    for (int i = 0; i < count; i++) {
        Card* burnCard = createCardByName("Burn");
        if (burnCard != nullptr) 
            discardPile.push_back(burnCard); 
    }
}

void Player::TurnStartEffect(string effect) {
    Character::TurnStartEffect(effect);
    if (effect == "Brutality") {
        decreaseHp(1);
        drawCards(1);
    }
        
}

void Player::endTurnCleanUp() {
    for (Card* card : hand) {
        if (card) 
            discardPile.push_back(card);
    }
    hand.clear(); 
}

void Player::addCardToDrawPile(Card* card) {
    if (card != nullptr) 
        drawPile.push_back(card);
}

void Player::addCardToDiscardPile(Card* card) {
    if (card != nullptr)
        discardPile.push_back(card);
}

void Player::loseGold(int amount) { 
    gold -= amount; 
    if (gold < 0) 
        gold = 0; 
}

void Player::playCard(Card* card, Character* target) {

    if (card->getType() == CardType::Attack && this->hasEffect("Entangled")) 
        return;

    int finalCost = card->getCost(this); 

    if (this->getEnergy() >= finalCost) {
        this->decreaseEnergy(finalCost);
        card->applyEffect(this, target, battleManagerPtr);

        if (target != nullptr) {
            Enemy* enemy = dynamic_cast<Enemy*>(target);
            if (enemy != nullptr)
                enemy->onPlayerPlayedCard(card);
        }
        auto it = std::find(hand.begin(), hand.end(), card);
        if (it != hand.end()) {
            hand.erase(it);
            discardPile.push_back(card);
        }
    }
}

int Player::countCardsByName(string name) {
    int count = 0;
    for (Card* c : hand) if (c->getName() == name) count++;
    for (Card* c : drawPile) if (c->getName() == name) count++;
    for (Card* c : discardPile) if (c->getName() == name) count++;

    return count;
}

bool Player::isHandAllAttacks() {
    if (hand.empty()) 
        return false;
    for (Card* c : hand) {
        if (c->getType() != CardType::Attack) 
            return false; 
    }
    return true; 
}

void Character::applyTurnStartEffects() {
    for (auto* effect : effects)
        effect->onTurnStart(this);
}

void Character::applyTurnEndEffects() {
    for (auto* effect : effects)
        effect->onTurnEnd(this);

    for (auto it = effects.begin(); it != effects.end(); ) {
        if ((*it)->isExpired()) {
            delete *it;
            it = effects.erase(it);
        } else {
            ++it;
        }
    }
}