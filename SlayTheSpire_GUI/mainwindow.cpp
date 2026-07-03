#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>
#include <QPushButton>
#include<QDebug>
#include<Qfile>

#include "../card.h"
#include "../AttackCard.h"
#include "../CurseCard.h"
#include "../SkillCard.h"
#include "../PowerCard.h"
#include "../StatusCard.h"
#include "../Player.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    std::srand(std::time(nullptr));
    playerObject = new Player("Dina", 80, 80, 3);
    initializePlayerDeck(15);
    playerObject->drawCards(5);
    updateHandUI();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete playerObject;
}

void MainWindow::on_cardButton_1_clicked() {
    const auto& hand = playerObject->getHand();
    if (0 < hand.size()) {
        playerObject->exhaustCard(hand[0]);
        updateHandUI();
    }
}

void MainWindow::on_cardButton_2_clicked() {
    const auto& hand = playerObject->getHand();
    if (1 < hand.size()) {
        playerObject->exhaustCard(hand[1]);
        updateHandUI();
    }
}

void MainWindow::on_cardButton_3_clicked() {
    const auto& hand = playerObject->getHand();
    if (4 < hand.size()) {
        playerObject->exhaustCard(hand[4]);
        updateHandUI();
    }
}

void MainWindow::on_cardButton_4_clicked() {
    const auto& hand = playerObject->getHand();
    if (2 < hand.size()) {
        playerObject->exhaustCard(hand[2]);
        updateHandUI();
    }
}

void MainWindow::on_cardButton_5_clicked() {
    const auto& hand = playerObject->getHand();
    if (3 < hand.size()) {
        playerObject->exhaustCard(hand[3]);
        updateHandUI();
    }
}

void MainWindow::on_EndTurnButton_clicked()
{
    const auto& hand = playerObject->getHand();
    for (Card* card : hand) {
        if (card && card->getName() == "Burn") {
            playerObject->decreaseHp(2);
        }
    }
    playerObject->endTurnCleanUp();

    // enemy

    playerObject->increaseEnergy(3);
    playerObject->drawCards(5);
    updateHandUI();
}

void MainWindow::updateHandUI() {
    QPushButton* buttons[] = { ui->cardButton_1, ui->cardButton_2, ui->cardButton_4, ui->cardButton_5, ui->cardButton_3 };

    const std::vector<Card*>& playerHand = playerObject->getHand();

    for (int i = 0; i < 5; ++i) {
        buttons[i]->setIcon(QIcon());
        if (i < playerHand.size() && playerHand[i] != nullptr) {
            buttons[i]->show();
            buttons[i]->setEnabled(playerHand[i]->isPlayable());
            buttons[i]->setText("");

            QString cardName = QString::fromStdString(playerHand[i]->getName());
            QString imgPath = ":/images/cards/" + cardName + ".png";

            QIcon cardIcon(imgPath);
            buttons[i]->setIcon(cardIcon);
            buttons[i]->setIconSize(buttons[i]->size());
        }
        else
            buttons[i]->hide();
    }
}

Card* createCardByName(const std::string& name) {
    if (name == "Bash")            return new BashCard();
    if (name == "Blood for Blood") return new Blood_for_BloodCard();
    if (name == "Clash")           return new ClashCard();
    if (name == "Feed")            return new FeedCard();
    if (name == "Immolate")        return new ImmolateCard();
    if (name == "PerfectedStrike") return new PerfectedStrike();
    if (name == "Reaper")          return new ReaperCard();
    if (name == "Strike")          return new AttackCard(CardType::Attack, "Strike", "Deal 6 damage", 1, 6);
    if (name == "Bludgeon")        return new AttackCard(CardType::Attack, "Bludgeon", "Deal 32 damage", 3, 32);
    if (name == "TwinStrike")      return new TwinStrikeCard();
    if (name == "Whirlwind")       return new WhirlwindCard();
    if (name == "Barricade")       return new BarricadeCard();
    if (name == "Bloodletting")    return new BloodlettingCard();
    if (name == "Brutality")       return new BrutalityCard();
    if (name == "Burn")            return new BurnCard();
    if (name == "CurseOfBell")     return new CurseOfBellCard();
    if (name == "DualWield")       return new DualWieldCard();
    if (name == "Daze")            return new DazeCard();
    if (name == "Defend")          return new DefendCard();
    if (name == "DemonForm")       return new DemonFormCard();
    if (name == "Disarm")          return new DisarmCard();
    if (name == "Entrench")        return new EntrenchCard();
    if (name == "Exhume")          return new ExhumeCard();
    if (name == "FeelNoPain")      return new FeelNoPainCard();
    if (name == "Impervious")      return new ImperviousCard();
    if (name == "Inflame")         return new InflameCard();
    if (name == "JAX")             return new JAXCard();
    if (name == "LimitBreak")      return new LimitBreakCard();
    if (name == "Metallicize")     return new MetallicizeCard();
    if (name == "Offering")        return new OfferingCard();
    if (name == "Regret")          return new RegretCard();
    if (name == "ShrugItOff")      return new ShrugItOffCard();
    if (name == "Slime")           return new SlimeCard();
    if (name == "Wound")           return new WoundCard();
    return nullptr;
}

void MainWindow::initializePlayerDeck(int totalCards) {
    std::vector<std::string> allCardNames = {
    "Bash", "Blood for Blood", "Clash", "Feed", "Immolate", "PerfectedStrike", "Reaper",
    "Strike", "Bludgeon", "TwinStrike", "Whirlwind", "Barricade", "Bloodletting", "Brutality",
    "DualWield", "Defend", "DemonForm", "Disarm", "Entrench",
    "Exhume", "FeelNoPain", "Impervious", "Inflame", "LimitBreak", "Metallicize",
    "Offering", "ShrugItOff"};

    for (int i = 0; i < totalCards; ++i) {
        int randomIndex = std::rand() % allCardNames.size();
        std::string randomName = allCardNames[randomIndex];

        Card* newCard = createCardByName(randomName);
        if (newCard)
            playerObject->addCardToDrawPile(newCard);
    }
}