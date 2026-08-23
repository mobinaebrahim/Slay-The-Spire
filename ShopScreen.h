#ifndef SHOPSCREEN_H
#define SHOPSCREEN_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QScrollArea>
#include "player.h"

class Card;
class Potion;
class Relic;

struct ShopOffer
{
    enum Type
    {
        CardType,
        PotionType,
        RelicType
    };

    Type type;

    int price = 0;

    Card* card = nullptr;
    Potion* potion = nullptr;
    Relic* relic = nullptr;

    QPushButton* btn = nullptr;
    QLabel* priceLabel = nullptr;
};

class ShopScreen : public QWidget {
    Q_OBJECT
public:
    explicit ShopScreen(Player* player, QWidget* parent = nullptr);
    ~ShopScreen();

signals:
    void finished();

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onBuyOffer(int index);
    void onCardRemoval();
    void onCancelRemoval();
    void onRemoveCardFromDeck(Card* card);
    void onLeaveShop();

private:
    Player* player;
    QLabel* backgroundLabel;
    QLabel* goldLabel;
    QPushButton* leaveBtn;
    QPushButton* removalBtn;
    QWidget* removalOverlay;
    QWidget* removalCardsContainer;

    std::vector<ShopOffer> allOffers;
    int cardRemovalCost = 50;

    void setupUI();
    void refreshGoldDisplay();
    void generateOffers();
    void buildRemovalOverlay();
    void updateOfferButtons();
    QWidget* createCardOfferWidget(Card* card, int price, int index);
    QWidget* createPotionOfferWidget(Potion* potion, int price, int index);
};

#endif