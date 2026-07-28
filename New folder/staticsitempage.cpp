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

    main_size = this->size();
    sub_page_size = QSize(900, 650);

    ui->btnReturn->raise();
    ui->btnLeaderboard->raise();
    connect(ui->btnReturn,&QPushButton::clicked,this,[this](){
        close();
    });

    connect(ui->btnLeaderboard,&QPushButton::clicked,this,[this]{
        ui->stackedWidget->setCurrentWidget(ui->leaderboard);
        resize_and_center(sub_page_size);
    });

    connect(ui->btnHistory, &QPushButton::clicked, this, [this]{
        ui->stackedWidget->setCurrentWidget(ui->history);
        refresh_history();
        resize_and_center(sub_page_size);
    });

    connect(ui->btnCharStat, &QPushButton::clicked, this, [this]{
        ui->stackedWidget->setCurrentWidget(ui->charstats);
        refresh_character_stats();
        resize_and_center(sub_page_size);
    });

    //---charstats---

    connect(ui->btnReturnCharstats, &QPushButton::clicked, this, [this](){
        ui->stackedWidget->setCurrentWidget(ui->main);
        resize_and_center(main_size);
    });

    //---history---

    connect(ui->btnReturnHistory, &QPushButton::clicked, this, [this](){
        ui->stackedWidget->setCurrentWidget(ui->main);
        resize_and_center(main_size);
    });

    //---leaderboard---
    ui->btnRetun1->raise();
    connect(ui->btnRetun1, &QPushButton::clicked, this, [this](){
        ui->stackedWidget->setCurrentWidget(ui->main);
        resize_and_center(main_size);
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

void StaticsItemPage::resize_and_center(QSize newSize)
{
    this->resize(newSize);
    ui->stackedWidget->resize(newSize);

    if (parentWidget()) {
        QRect parentGeometry = parentWidget()->geometry();
        int x = parentGeometry.x() + (parentGeometry.width() - newSize.width()) / 2;
        int y = parentGeometry.y() + (parentGeometry.height() - newSize.height()) / 2;
        this->move(x, y);
    }
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

    ui->table_scores->clear();

    QWidget *headerWidget = new QWidget();
    headerWidget->setFixedHeight(40);
    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(15, 5, 25, 5);

    QLabel *headerUsername = new QLabel("Username");
    QLabel *headerValue = new QLabel(columnLabel);
    QString headerStyle = "color: #FFD700; font-size: 14px; font-weight: bold; background: transparent;";
    headerUsername->setStyleSheet(headerStyle);
    headerValue->setStyleSheet(headerStyle);
    headerUsername->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    headerLayout->addWidget(headerUsername);
    headerLayout->addWidget(headerValue);

    QListWidgetItem *headerItem = new QListWidgetItem();
    headerItem->setSizeHint(headerWidget->sizeHint());
    headerItem->setFlags(Qt::NoItemFlags);
    ui->table_scores-> addItem(headerItem);
    ui->table_scores->setItemWidget(headerItem, headerWidget);

    for (int i = 0; i < scores.size(); i++) {
        QWidget *itemWidget = new QWidget();
        itemWidget->setFixedHeight(50);

        QHBoxLayout *layout = new QHBoxLayout(itemWidget);
        layout->setContentsMargins(15, 5, 25, 5);
        layout->setSpacing(10);

        QLabel *usernameLabel = new QLabel(scores[i].username);
        usernameLabel->setStyleSheet(
            "color: #E8D2A0; font-size: 14px; font-weight: bold; background: transparent;"
            );
        usernameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

        QString valueText;
        if (sortBy == "floor") valueText = QString::number(scores[i].highest_floor);
        else if (sortBy == "wins") valueText = QString::number(scores[i].total_wins);
        else if (sortBy == "time") valueText = format_duration(scores[i].total_duration);
        else valueText = QString::number(scores[i].total_score);

        QLabel *valueLabel = new QLabel(valueText);
        valueLabel->setStyleSheet(
            "color: #E8D2A0; font-size: 14px; font-weight: bold; background: transparent;"
            );

        layout->addWidget(usernameLabel);
        layout->addWidget(valueLabel);
        layout->setAlignment(usernameLabel, Qt::AlignVCenter);
        layout->setAlignment(valueLabel, Qt::AlignVCenter);

        QListWidgetItem *listItem = new QListWidgetItem();
        listItem->setSizeHint(itemWidget->sizeHint());

        ui->table_scores->addItem(listItem);
        ui->table_scores->setItemWidget(listItem, itemWidget);
    }
}


void StaticsItemPage::refresh_history()
{
    QString currentUser = user_manager::instance().get_current_username();
    QList<Run_entry> history = ScoreManager::instance().get_run_history(currentUser);

    ui->table_history->clear();

    QWidget *headerWidget = new QWidget();
    headerWidget->setFixedHeight(40);
    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(15, 5, 25, 5);

    QStringList headers = {"Score", "Floor", "Time", "Result", "Date"};
    QString headerStyle = "color: #FFD700; font-size: 13px; font-weight: bold; background: transparent;";

    for (const QString &headerText : headers) {
        QLabel *headerLabel = new QLabel(headerText);
        headerLabel->setStyleSheet(headerStyle);
        headerLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        headerLayout->addWidget(headerLabel);
    }

    QListWidgetItem *headerItem = new QListWidgetItem();
    headerItem->setSizeHint(headerWidget->sizeHint());
    headerItem->setFlags(Qt::NoItemFlags);
    ui->table_history->addItem(headerItem);
    ui->table_history->setItemWidget(headerItem, headerWidget);

    for (int i = 0; i < history.size(); i++) {
        QWidget *itemWidget = new QWidget();
        itemWidget->setFixedHeight(50);

        QHBoxLayout *layout = new QHBoxLayout(itemWidget);
        layout->setContentsMargins(15, 5, 25, 5);
        layout->setSpacing(10);

        QString rowStyle = "color: #E8D2A0; font-size: 13px; font-weight: bold; background: transparent;";

        QLabel *scoreLabel = new QLabel(QString::number(history[i].score));
        QLabel *floorLabel = new QLabel(QString::number(history[i].floor_reached));
        QLabel *timeLabel = new QLabel(format_duration(history[i].play_duration));
        QLabel *resultLabel = new QLabel(history[i].is_victory ? "Victory" : "Defeat");
        QLabel *dateLabel = new QLabel(history[i].date_achieved);

        QList<QLabel*> labels = {scoreLabel, floorLabel, timeLabel, resultLabel, dateLabel};
        for (QLabel *label : labels) {
            label->setStyleSheet(rowStyle);
            label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
            layout->addWidget(label);
            layout->setAlignment(label, Qt::AlignVCenter);
        }

        QListWidgetItem *listItem = new QListWidgetItem();
        listItem->setSizeHint(itemWidget->sizeHint());

        ui->table_history->addItem(listItem);
        ui->table_history->setItemWidget(listItem, itemWidget);
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

void StaticsItemPage::refresh_character_stats()
{
    QString current_user = user_manager::instance().get_current_username();
    Score_entry stats = ScoreManager::instance().get_character_stats(current_user);

    ui->label_totalScore->setText("Total Score: " + QString::number(stats.total_score));
    ui->label_highestFloor->setText("Highest Floor: " + QString::number(stats.highest_floor));
    ui->label_totalWins->setText("Total Wins: " + QString::number(stats.total_wins));
    ui->label_totalTime->setText("Total Time: " + format_duration(stats.total_duration));
}

void StaticsItemPage::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QPixmap background;

    if (ui->stackedWidget->currentWidget() == ui->main) {
        background = QPixmap(":/assets/authpage/b.png");
    } else {
        background = QPixmap(":/assets/authpage/background.png");
    }

    painter.drawPixmap(rect(), background);
    QDialog::paintEvent(event);
}

StaticsItemPage::~StaticsItemPage()
{
    delete ui;
}