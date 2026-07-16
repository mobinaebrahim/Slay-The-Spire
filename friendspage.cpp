#include "friendspage.h"
#include "ui_friendspage.h"

friendspage::friendspage(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::friendspage)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    ui->label_addFriendError->clear();

    populate_pending_requests();
    populate_friends_list();
    connect(ui->btnBack, &QPushButton::clicked, this, &QDialog::close);
}

friendspage::~friendspage()
{
    delete ui;
}

void friendspage::on_btnAddFriend_clicked()
{
    QString targetUsername = ui->lineEditSearchUsername->text().trimmed();
    QString currentUser = user_manager::instance().get_current_username();

    if (targetUsername.isEmpty()) {
        showAddFriendError("Please enter a username.");
        return;
    }

    if (targetUsername == currentUser) {
        showAddFriendError("You cannot add yourself.");
        return;
    }

    if (!user_manager::instance().usernmae_exist(targetUsername)) {
        showAddFriendError("User not found.");
        return;
    }

    if (FriendManager::instance().areFriends(currentUser, targetUsername)) {
        showAddFriendError("Already friends or request pending.");
        return;
    }

    if (FriendManager::instance().sendFriendRequest(currentUser, targetUsername)) {
        showAddFriendError("Friend request sent!", false);
        ui->lineEditSearchUsername->clear();
    } else {
        showAddFriendError("Could not send friend request.");
    }
}

void friendspage::showAddFriendError(const QString &message, bool isError)
{
    ui->label_addFriendError->setText(message);
    ui->label_addFriendError->setStyleSheet(isError ? "color: red;" : "color: green;");
}

void friendspage::populate_friends_list()
{
    ui->listFriends->clear();

    QString currentUser = user_manager::instance().get_current_username();
    QStringList friendsList = FriendManager::instance().getFriendsList(currentUser);

    for (const QString &friendName : friendsList) {
        QWidget *itemWidget = new QWidget();
        QHBoxLayout *layout = new QHBoxLayout(itemWidget);

        QLabel *nameLabel = new QLabel(friendName);
        /*QFont font("Cinzel", 12, QFont::Bold);
        nameLabel->setFont(font);
        nameLabel->setStyleSheet(
            "QLabel {"
            "color: #EFD7A0;"
            "font-size: 14px;"
            "font-weight: bold;"
            "background: transparent;"
            "}"
            );*/

        nameLabel->setStyleSheet(
            "color:#E8D2A0;"
            );

        QPushButton *removeBtn = new QPushButton();
        removeBtn->setIcon(QIcon(":/assets/mainmenu/friend/remove.png"));
        removeBtn->setIconSize(QSize(32,32));
        removeBtn->setFixedSize(40,40);
        removeBtn->setStyleSheet(
            "QPushButton{"
            "border:none;"
            "background:transparent;"
            "}"
            );

        nameLabel->setSizePolicy(
            QSizePolicy::Expanding,
            QSizePolicy::Preferred
            );
        layout->addWidget(nameLabel);
        layout->addWidget(removeBtn);
        layout->setContentsMargins(5, 5, 5, 5);
        /*itemWidget->setStyleSheet(
            "background: transparent;"
            );*/

        QListWidgetItem *listItem = new QListWidgetItem();
        listItem->setSizeHint(itemWidget->sizeHint());

        ui->listFriends->addItem(listItem);
        ui->listFriends->setItemWidget(listItem, itemWidget);

        connect(removeBtn, &QPushButton::clicked, this, [this, friendName](){
            FriendManager::instance().removeFriend(user_manager::instance().get_current_username(), friendName);
            populate_friends_list();
        });
    }
}

void friendspage::populate_pending_requests()
{
    ui->listPendingRequests->clear();

    QString currentUser = user_manager::instance().get_current_username();
    QStringList requests = FriendManager::instance().getPendingRequests(currentUser);

    for (const QString &requester : requests) {
        QWidget *itemWidget = new QWidget();
        QHBoxLayout *layout = new QHBoxLayout(itemWidget);

        QLabel *nameLabel = new QLabel(requester);

        QPushButton *acceptBtn = new QPushButton();
        acceptBtn->setIcon(QIcon(":/assets/mainmenu/friend/accept.png"));
        acceptBtn->setIconSize(QSize(32,32));
        acceptBtn->setFixedSize(40,40);
        acceptBtn->setStyleSheet(
            "QPushButton{"
            "border:none;"
            "background:transparent;"
            "}"
            );

        QPushButton *rejectBtn = new QPushButton();
        rejectBtn->setIcon(QIcon(":/assets/mainmenu/friend/reject.png"));
        rejectBtn->setIconSize(QSize(32,32));
        rejectBtn->setFixedSize(40,40);
        rejectBtn->setStyleSheet(
            "QPushButton{"
            "border:none;"
            "background:transparent;"
            "}"
            );

        layout->addWidget(nameLabel);
        layout->addWidget(acceptBtn);
        layout->addWidget(rejectBtn);
        layout->setContentsMargins(5, 5, 5, 5);
        itemWidget->setLayout(layout);

        QListWidgetItem *listItem = new QListWidgetItem();
        listItem->setSizeHint(itemWidget->sizeHint());

        ui->listPendingRequests->addItem(listItem);
        ui->listPendingRequests->setItemWidget(listItem, itemWidget);

        connect(acceptBtn, &QPushButton::clicked, this, [this, requester](){
            FriendManager::instance().acceptFriendRequest(user_manager::instance().get_current_username(), requester);
            populate_pending_requests();
            populate_friends_list();
        });

        connect(rejectBtn, &QPushButton::clicked, this, [this, requester](){
            FriendManager::instance().rejectFriendRequest(user_manager::instance().get_current_username(), requester);
            populate_pending_requests();
        });
    }
}

void friendspage::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    QPixmap background(":/assets/authpage/background.png");

    painter.drawPixmap(rect(), background);

    QDialog::paintEvent(event);
}
