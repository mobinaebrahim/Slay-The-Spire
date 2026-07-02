#include "Player.h"

extern Card* createCardByName(const std::string& name);

Player::Player(string n, int h, int max, int energy): Character(n, h, max), currentEnergy(energy), maxEnergy(energy) {}

void Player::decreaseEnergy(int amount) {
    currentEnergy -= amount;
    if (currentEnergy < 0)
        currentEnergy = 0;
}

void Player::increaseEnergy(int amount) {
    currentEnergy += amount;
    if (currentEnergy > maxEnergy)
        currentEnergy = maxEnergy; 
}

int Player::getEnergy() const {
    return currentEnergy;
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

    // incomplete

}

void Player::addCopiesToHand(Card* chosenCard, int count) {
    if (!chosenCard)
        return;
    for (int i = 0; i < count; i++) 
        hand.push_back(chosenCard);
    cout << "Added " << count << " copies of " << chosenCard->getName() << " to hand.\n";
}

int Player::getHandSize() {
    return hand.size();
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
    if (effect == "Brutality") 
        drawCards(1);
}