#ifndef TREASURESCREEN_H
#define TREASURESCREEN_H
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QResizeEvent>
#include "Player.h"
class TreasureScreen : public QWidget {
    Q_OBJECT
public:
    explicit TreasureScreen(Player* player, QWidget* parent = nullptr);
signals:
    void finished();
protected:
    void resizeEvent(QResizeEvent* event) override;
private slots:
    void onContinueClicked();
private:
    Player* player;
    QLabel* backgroundLabel;
    QLabel* rewardLabel;
    QPushButton* continueBtn;
    QMediaPlayer* soundPlayer;
    QAudioOutput* soundOutput;

    void setupUI();
    void grantReward();
};
#endif // TREASURESCREEN_H