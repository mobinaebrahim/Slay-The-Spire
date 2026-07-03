#ifndef STATUSEFFECT_H
#define STATUSEFFECT_H
#include <string>
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
    virtual void onTurnEnd() {}
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
    void onTurnEnd() override; 
};

class WeakEffect : public StatusEffect {
public:
    WeakEffect(int turns);
    int modifyOutgoingDamage(int baseDamage) override;
    void onTurnEnd() override;
};

class FrailEffect : public StatusEffect {
public:
    FrailEffect(int turns);
    int modifyBlock(int baseBlock) override;
    void onTurnEnd() override;
};

class MetallicizeEffect : public StatusEffect {
public:
    MetallicizeEffect(int increaseBlockBy);
    void onTurnEnd() override;
};

#endif