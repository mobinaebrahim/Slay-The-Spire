#include "staticsitempage.h"
#include "ui_staticsitempage.h"
#include "scoremanager.h"
#include "friendmanager.h"
#include "usermanager.h"

StaticsItemPage::StaticsItemPage(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::StaticsItemPage)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);

    ui->btnReturn->raise();
    ui->btnLeaderboard->raise();
    connect(ui->btnReturn,&QPushButton::clicked,this,[this](){
        close();
    });

    connect(ui->btnLeaderboard,&QPushButton::clicked,this,[this]{
        ui->stackedWidget->setCurrentWidget(ui->leaderboard);
    });

    //---leaderboard---
    ui->btnRetun1->raise();
    connect(ui->btnRetun1, &QPushButton::clicked, this, [this](){
        ui->stackedWidget->setCurrentWidget(ui->main);
    });

    regionGroup = new QButtonGroup(this);
    regionGroup->addButton(ui->btnglobal);
    regionGroup->addButton(ui->btnfriends);
    regionGroup->setExclusive(true);

    typeGroup = new QButtonGroup(this);
    typeGroup->addButton(ui->btnTotalScore);
    typeGroup->addButton(ui->btnHighestFloor);
    typeGroup->addButton(ui->btnTotalWins);
    typeGroup->addButton(ui->btnTotalTime);
    typeGroup->setExclusive(true);

    connect(regionGroup, &QButtonGroup::buttonClicked, this, &StaticsItemPage::refresh_leaderboard);
    connect(typeGroup, &QButtonGroup::buttonClicked, this, &StaticsItemPage::refresh_leaderboard);

    //connect(ui->btnShowScoreboard, &QPushButton::clicked, this, &StaticsItemPage::refresh_leaderboard);
}

void StaticsItemPage::refresh_leaderboard()
{
    QString sortBy = "score";
    QString columnLabel = "Score";
    if (ui->btnHighestFloor->isChecked()) { sortBy = "floor"; columnLabel = "Floor"; }
    else if (ui->btnTotalWins->isChecked()) { sortBy = "wins"; columnLabel = "Wins"; }
    else if (ui->btnTotalTime->isChecked()) { sortBy = "time"; columnLabel = "Time"; }

    QList<Score_entry> scores = ScoreManager::instance().get_scores(sortBy);

    if (ui->btnfriends->isChecked()) {
        QString currentUser = user_manager::instance().get_current_username();
        QStringList friendsList = FriendManager::instance().getFriendsList(currentUser);
        friendsList.append(currentUser);

        QList<Score_entry> filtered;
        for (const Score_entry &entry : scores) {
            if (friendsList.contains(entry.username)) {
                filtered.append(entry);
            }
        }
        scores = filtered;
    }

    ui->table_scores->setColumnCount(2);
    ui->table_scores->setHorizontalHeaderLabels({"Username", columnLabel});

    ui->table_scores->setRowCount(scores.size());
    for (int i = 0; i < scores.size(); i++) {
        ui->table_scores->setItem(i, 0, new QTableWidgetItem(scores[i].username));

        QString valueText;
        if (sortBy == "floor") valueText = QString::number(scores[i].highest_floor);
        else if (sortBy == "wins") valueText = QString::number(scores[i].total_wins);
        else if (sortBy == "time") valueText = format_duration(scores[i].total_duration);
        else valueText = QString::number(scores[i].total_score);

        ui->table_scores->setItem(i, 1, new QTableWidgetItem(valueText));
    }
}

QString StaticsItemPage::format_duration(int totalSeconds)
{
    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    int seconds = totalSeconds % 60;

    if (hours > 0) {
        return QString("%1:%2:%3")
        .arg(hours)
            .arg(minutes, 2, 10, QChar('0'))
            .arg(seconds, 2, 10, QChar('0'));
    }
    return QString("%1:%2")
        .arg(minutes)
        .arg(seconds, 2, 10, QChar('0'));
}

StaticsItemPage::~StaticsItemPage()
{
    delete ui;
}