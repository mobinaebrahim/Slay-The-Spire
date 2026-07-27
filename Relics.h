#ifndef RELICS_H
#define RELICS_H

#include <string>
using namespace std;

class Character;
class Player;
class BattleManager;
class Card;

class Relic {
protected:
    string name;
    string description;

public:
    Relic(string n, string d);
    virtual ~Relic() = default;

    string getName() const;
    string getDescription() const;

    virtual void onCombatStart(Player* owner, BattleManager* bm) {}
    virtual void onCombatEnd(Player* owner, BattleManager* bm) {}
    virtual void onCardPlayed(Player* owner, Card* card) {}
    virtual void onTurnStart(Player* owner, BattleManager* bm) {}
    virtual void onTurnEnd(Player* owner) {}
    virtual void onCampsite(Player* owner) {}
    virtual void onObtain(Player* owner) {}
};

//_________________________________ Starter _________________________________
class BurningBlood : public Relic {
public:
    BurningBlood();
    void onCombatEnd(Player* owner, BattleManager* bm) override;
};

//_________________________________ Normal __________________________________
class Girya : public Relic {
private:
    int liftsRemaining = 3;
public:
    Girya();
    void onCampsite(Player* owner) override;
};

class IceCream : public Relic {
public:
    IceCream();
};

class Shuriken : public Relic {
private:
    int attacksPlayedThisTurn = 0;
public:
    Shuriken();
    void onCardPlayed(Player* owner, Card* card) override;
    void onTurnStart(Player* owner, BattleManager* bm) override;
};

class Kunai : public Relic {
private:
    int attacksPlayedThisTurn = 0;
public:
    Kunai();
    void onCardPlayed(Player* owner, Card* card) override;
    void onTurnStart(Player* owner, BattleManager* bm) override;
};

class PreservedInsect : public Relic {
public:
    PreservedInsect();
};

//__________________________________ Boss ___________________________________
class CallingBell : public Relic {
public:
    CallingBell();
    void onObtain(Player* owner) override;
};

class MarkOfPain : public Relic {
public:
    MarkOfPain();
    void onTurnStart(Player* owner, BattleManager* bm) override;
    void onCombatStart(Player* owner, BattleManager* bm) override;
};

class VelvetChoker : public Relic {
public:
    VelvetChoker();
    void onTurnStart(Player* owner, BattleManager* bm) override;
};

class SlaversCollar : public Relic {
public:
    SlaversCollar();
    void onTurnStart(Player* owner, BattleManager* bm) override;
};

//__________________________________ Event __________________________________
class WarpedTongs : public Relic {
public:
    WarpedTongs();
    void onCombatStart(Player* owner, BattleManager* bm) override;
};

class MutagenicStrength : public Relic {
public:
    MutagenicStrength();
    void onCombatStart(Player* owner, BattleManager* bm) override;
};

class CultistHeadpiece : public Relic {
public:
    CultistHeadpiece();
    void onCombatStart(Player* owner, BattleManager* bm) override;
};

class FaceOfTheCleric : public Relic {
public:
    FaceOfTheCleric();
    void onCombatEnd(Player* owner, BattleManager* bm) override;
};

#endif