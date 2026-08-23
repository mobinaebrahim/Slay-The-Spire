#ifndef EVENTSCREEN_H
#define EVENTSCREEN_H
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <vector>
#include <functional>
#include "Player.h"
#include "Enemy.h"
#include "Relics.h"
#include "Potion.h"
extern Card* createCardByName(const std::string& name);
extern Relic* createRelicByName(const std::string& name);
extern Enemy* createEliteEnemy(const std::string& name);

enum class EventType {
    OminousForge, GoldenIdol, Augmenter, FaceTrader,
    TheColosseum, GoldenShrine, Lab, ShiningLight, TheSerpent
};

struct EventOption {
    QString buttonText;
    QString tooltip;
    std::function<void()> action;
};

class EventScreen : public QWidget {
    Q_OBJECT
public:
    explicit EventScreen(Player* player, QWidget* parent = nullptr);
    void setEvent(EventType type);

signals:
    void finished();
    void requestCombat(Enemy* enemy);
    void requestCardUpgrade();
    void requestCardTransform(int count);
    void requestCardSelection(std::function<void(Card*)> callback);

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onOptionClicked(int index);
    void onLeave();

private:
    Player* player;
    EventType currentType;

    QWidget* cardContainer;
    QLabel* frameLabel;
    QLabel* eventImageLabel;
    QLabel* titleLabel;
    QLabel* descLabel;
    QWidget* optionsContainer;
    std::vector<QPushButton*> optionButtons;

    std::vector<EventOption> currentOptions;

    void setupUI();
    void clearOptions();
    void addOption(const QString& text, const QString& tooltip, std::function<void()> action);
    void layoutCard();
    QString imagePathForEvent(EventType type) const;

    void setupOminousForge();
    void setupGoldenIdol();
    void setupAugmenter();
    void setupFaceTrader();
    void setupTheColosseum();
    void setupGoldenShrine();
    void setupLab();
    void setupShiningLight();
    void setupTheSerpent();
};
#endif // EVENTSCREEN_H