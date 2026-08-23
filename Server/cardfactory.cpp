#include "cardfactory.h"
#include "AttackCard.h"
#include "CurseCard.h"
#include "SkillCard.h"
#include "PowerCard.h"
#include "StatusCard.h"

Card* createCardByName(const std::string& name) {
    Card* card = nullptr;

    // Attack Cards
    if (name == "Bash")            card = new BashCard();
    else if (name == "Blood for Blood") card = new Blood_for_BloodCard();
    else if (name == "Clash")      card = new ClashCard();
    else if (name == "Feed")       card = new FeedCard();
    else if (name == "Immolate")   card = new ImmolateCard();
    else if (name == "PerfectedStrike") card = new PerfectedStrike();
    else if (name == "Reaper")     card = new ReaperCard();
    else if (name == "Strike")     card = new AttackCard(CardType::Attack, "Strike", "Deal 6 damage", 1, 6, 9);
    else if (name == "Bludgeon")   card = new AttackCard(CardType::Attack, "Bludgeon", "Deal 32 damage", 3, 32, 42);
    else if (name == "TwinStrike") card = new TwinStrikeCard();
    else if (name == "Whirlwind")  card = new WhirlwindCard();

    // Power Cards
    else if (name == "Barricade")  card = new BarricadeCard();
    else if (name == "Bloodletting") card = new BloodlettingCard();
    else if (name == "Brutality")  card = new BrutalityCard();
    else if (name == "DemonForm")  card = new DemonFormCard();
    else if (name == "Disarm")     card = new DisarmCard();
    else if (name == "Entrench")   card = new EntrenchCard();
    else if (name == "Exhume")     card = new ExhumeCard();
    else if (name == "FeelNoPain") card = new FeelNoPainCard();
    else if (name == "Impervious") card = new ImperviousCard();
    else if (name == "Inflame")    card = new InflameCard();
    else if (name == "JAX")        card = new JAXCard();
    else if (name == "LimitBreak") card = new LimitBreakCard();
    else if (name == "Metallicize")card = new MetallicizeCard();
    else if (name == "Offering")   card = new OfferingCard();

    // Skill Cards
    else if (name == "DualWield")  card = new DualWieldCard();
    else if (name == "Defend")     card = new DefendCard();
    else if (name == "ShrugItOff") card = new ShrugItOffCard();

    // Status / Curse
    else if (name == "Burn")       card = new BurnCard();
    else if (name == "CurseOfBell")card = new CurseOfBellCard();
    else if (name == "Daze")       card = new DazeCard();
    else if (name == "Regret")     card = new RegretCard();
    else if (name == "Slime")      card = new SlimeCard();
    else if (name == "Wound")      card = new WoundCard();

    // NEW: Set Rarity for Revive system and card classification
    if (card) {
        if (name == "Strike" || name == "Defend") {
            card->setRarity(CardRarity::Basic);
        }
        else if (name == "Reaper" || name == "Feed" || name == "Immolate" ||
                 name == "Barricade" || name == "DemonForm" || name == "Brutality" ||
                 name == "Exhume" || name == "Impervious" || name == "Offering" ||
                 name == "Bludgeon" || name == "LimitBreak" || name == "Metallicize") {
            card->setRarity(CardRarity::Rare);
        }
        else {
            // Everything else is Common (treated as "Normal" in Revive 50% chance)
            card->setRarity(CardRarity::Common);
        }
    }

    return card;
}