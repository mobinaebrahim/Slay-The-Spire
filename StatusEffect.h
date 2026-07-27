#ifndef STATUSEFFECT_H
#define STATUSEFFECT_H
#include <string>
class Character;
using namespace std;

class StatusEffect {
protected:
    string name;
    int amount;

public:
    StatusEffect(string name, int amount);
    virtual ~StatusEffect() = default;

    string getName() const;
    int getAmount() const;
    void setAmount(int value);

    virtual int modifyOutgoingDamage(int baseDamage);
    virtual int modifyIncomingDamage(int baseDamage);
    virtual int modifyBlock(int baseBlock);
    virtual void onTurnEnd(Character* owner) {}
    virtual void onTurnStart(Character* owner) {}
    virtual void onCardExhausted(Character* owner) {}
    virtual bool isExpired() const { return false; }
};

class StrengthEffect : public StatusEffect {
public:
    StrengthEffect(int increaseDamageBy);
    int modifyOutgoingDamage(int baseDamage) override;
};

class DexterityEffect : public StatusEffect {
public:
    DexterityEffect(int increaseBlockBy);
    int modifyBlock(int baseBlock) override;
};

class VulnerableEffect : public StatusEffect {
public:
    VulnerableEffect(int turns);
    int modifyIncomingDamage(int baseDamage) override;
    void onTurnEnd(Character* owner) override;
    bool isExpired() const override { return amount <= 0; }
};

class WeakEffect : public StatusEffect {
public:
    WeakEffect(int turns);
    int modifyOutgoingDamage(int baseDamage) override;
    void onTurnEnd(Character* owner) override;
    bool isExpired() const override { return amount <= 0; }
};

class FrailEffect : public StatusEffect {
public:
    FrailEffect(int turns);
    int modifyBlock(int baseBlock) override;
    void onTurnEnd(Character* owner) override;
    bool isExpired() const override { return amount <= 0; }
};

class MetallicizeEffect : public StatusEffect {
public:
    MetallicizeEffect(int increaseBlockBy);
    void onTurnEnd(Character* owner) override;
};

class EntangledEffect : public StatusEffect {
public:
    EntangledEffect(int turns);
    void onTurnEnd(Character* owner) override;
    bool isExpired() const override { return amount <= 0; }
};

class DemonFormEffect : public StatusEffect {
public:
    DemonFormEffect(int amount);
    void onTurnStart(Character* owner) override;
};

class BrutalityEffect : public StatusEffect {
public:
    BrutalityEffect();
    void onTurnStart(Character* owner) override;
};

class FeelNoPainEffect : public StatusEffect {
public:
    FeelNoPainEffect(int blockAmount);
    void onCardExhausted(Character* owner) override;
};

#endif