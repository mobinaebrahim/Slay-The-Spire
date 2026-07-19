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
    populate_game_invites();

    connect(ui->btnBack, &QPushButton::clicked, this, &QDialog::close);

    connect(ui->btnRefreshInvites, &QPushButton::clicked, this, [this](){
        populate_pending_requests();
        populate_friends_list();
        populate_game_invites();
    });
}

void friendspage::populate_pending_requests()
{
    ui->listPendingRequests->clear();

    QListWidgetItem *spacerItem = new QListWidgetItem();
    spacerItem->setSizeHint(QSize(0, 10));
    spacerItem->setFlags(Qt::NoItemFlags);
    ui->listPendingRequests->addItem(spacerItem);

    QString currentUser = user_manager::instance().get_current_username();
    QStringList requests = FriendManager::instance().getPendingRequests(currentUser);

    qDebug() << "Current user:" << currentUser;
    qDebug() << "Pending requests count:" << requests.size();
    qDebug() << "Requests:" << requests;

    for (const QString &requester : requests) {
        QWidget *itemWidget = new QWidget();
        itemWidget->setFixedHeight(50);

        QHBoxLayout *layout = new QHBoxLayout(itemWidget);
        layout->setContentsMargins(15, 5, 25, 5);
        layout->setSpacing(10);

        QLabel *nameLabel = new QLabel(requester);
        nameLabel->setContentsMargins(20, 0, 0, 0);
        nameLabel->setStyleSheet(
            "color: #E8D2A0;"
            "font-size: 14px;"
            "font-weight: bold;"
            "background: transparent;"
            );
        nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

        QPushButton *acceptBtn = new QPushButton();
        acceptBtn->setIcon(QIcon(":/assets/mainmenu/friend/accept.png"));
        acceptBtn->setIconSize(QSize(40, 40));
        acceptBtn->setFixedSize(48, 48);
        acceptBtn->setStyleSheet(
            "QPushButton{"
            "border:none;"
            "background:transparent;"
            "}"
            );

        QPushButton *rejectBtn = new QPushButton();
        rejectBtn->setIcon(QIcon(":/assets/mainmenu/friend/reject.png"));
        rejectBtn->setIconSize(QSize(40, 40));
        rejectBtn->setFixedSize(48, 48);
        rejectBtn->setStyleSheet(
            "QPushButton{"
            "border:none;"
            "background:transparent;"
            "}"
            );

        layout->addWidget(nameLabel);
        layout->addWidget(acceptBtn);
        layout->addWidget(rejectBtn);
        layout->setAlignment(nameLabel, Qt::AlignVCenter);
        layout->setAlignment(acceptBtn, Qt::AlignVCenter);
        layout->setAlignment(rejectBtn, Qt::AlignVCenter);

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

    QListWidgetItem *spacerItem = new QListWidgetItem();
    spacerItem->setSizeHint(QSize(0, 10));
    spacerItem->setFlags(Qt::NoItemFlags);
    ui->listFriends->addItem(spacerItem);

    QString currentUser = user_manager::instance().get_current_username();
    QStringList friendsList = FriendManager::instance().getFriendsList(currentUser);

    for (const QString &friendName : friendsList) {
        QWidget *itemWidget = new QWidget();
        itemWidget->setFixedHeight(50);

        QHBoxLayout *layout = new QHBoxLayout(itemWidget);
        layout->setContentsMargins(15, 5, 25, 5);
        layout->setSpacing(10);

        QLabel *nameLabel = new QLabel(friendName);
        nameLabel->setContentsMargins(20, 0, 0, 0);
        nameLabel->setStyleSheet(
            "color: #E8D2A0;"
            "font-size: 14px;"
            "font-weight: bold;"
            "background: transparent;"
            );
        nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

        QPushButton *inviteBtn = new QPushButton();
        inviteBtn->setIcon(QIcon(":/assets/mainmenu/friend/remove.png"));
        inviteBtn->setIconSize(QSize(40, 40));
        inviteBtn->setFixedSize(48, 48);
        inviteBtn->setStyleSheet(
            "QPushButton{"
            "border:none;"
            "background:transparent;"
            "}"
            );

        QPushButton *removeBtn = new QPushButton();
        removeBtn->setIcon(QIcon(":/assets/mainmenu/friend/remove.png"));
        removeBtn->setIconSize(QSize(40, 40));
        removeBtn->setFixedSize(48, 48);
        removeBtn->setStyleSheet(
            "QPushButton{"
            "border:none;"
            "background:transparent;"
            "}"
            );

        layout->addWidget(nameLabel);
        layout->addWidget(inviteBtn);
        layout->addWidget(removeBtn);
        layout->setAlignment(nameLabel, Qt::AlignVCenter);
        layout->setAlignment(inviteBtn, Qt::AlignVCenter);
        layout->setAlignment(removeBtn, Qt::AlignVCenter);

        QListWidgetItem *listItem = new QListWidgetItem();
        listItem->setSizeHint(itemWidget->sizeHint());

        ui->listFriends->addItem(listItem);
        ui->listFriends->setItemWidget(listItem, itemWidget);

        connect(removeBtn, &QPushButton::clicked, this, [this, friendName](){
            FriendManager::instance().removeFriend(user_manager::instance().get_current_username(), friendName);
            populate_friends_list();
        });

        connect(inviteBtn, &QPushButton::clicked, this, [this, friendName](){
            QString currentUser = user_manager::instance().get_current_username();

            connect(&NetworkManager::instance(), &NetworkManager::room_created, this,
                    [this, currentUser, friendName](const QString &roomCode){
                        NetworkManager::instance().setIsLeader(true);
                        FriendManager::instance().sendGameInvite(currentUser, friendName, roomCode);
                        QMessageBox::information(this, "Invite Sent", "Game invite sent to " + friendName);
                    });

            NetworkManager::instance().create_room();
        });
    }
}

void friendspage::populate_game_invites()
{
    ui->listGameInvites->clear();

    QString currentUser = user_manager::instance().get_current_username();
    QStringList inviters = FriendManager::instance().getPendingGameInvites(currentUser);

    for (const QString &inviter : inviters) {
        QWidget *itemWidget = new QWidget();
        itemWidget->setFixedHeight(50);

        QHBoxLayout *layout = new QHBoxLayout(itemWidget);
        layout->setContentsMargins(15, 5, 25, 5);
        layout->setSpacing(10);

        QLabel *nameLabel = new QLabel(inviter);
        nameLabel->setStyleSheet(
            "color: #E8D2A0; font-size: 14px; font-weight: bold; background: transparent;"
            );
        nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

        QPushButton *acceptBtn = new QPushButton();
        acceptBtn->setIcon(QIcon(":/assets/mainmenu/friend/accept.png"));
        acceptBtn->setIconSize(QSize(40, 40));
        acceptBtn->setFixedSize(48, 48);
        acceptBtn->setStyleSheet("QPushButton{border:none; background:transparent;}");

        QPushButton *rejectBtn = new QPushButton();
        rejectBtn->setIcon(QIcon(":/assets/mainmenu/friend/reject.png"));
        rejectBtn->setIconSize(QSize(40, 40));
        rejectBtn->setFixedSize(48, 48);
        rejectBtn->setStyleSheet("QPushButton{border:none; background:transparent;}");

        layout->addWidget(nameLabel);
        layout->addWidget(acceptBtn);
        layout->addWidget(rejectBtn);
        layout->setAlignment(nameLabel, Qt::AlignVCenter);
        layout->setAlignment(acceptBtn, Qt::AlignVCenter);
        layout->setAlignment(rejectBtn, Qt::AlignVCenter);

        QListWidgetItem *listItem = new QListWidgetItem();
        listItem->setSizeHint(itemWidget->sizeHint());

        ui->listGameInvites->addItem(listItem);
        ui->listGameInvites->setItemWidget(listItem, itemWidget);

        connect(acceptBtn, &QPushButton::clicked, this, [this, inviter](){
            QString currentUser = user_manager::instance().get_current_username();
            QString roomCode;

            if (FriendManager::instance().acceptGameInvite(currentUser, inviter, roomCode)) {
                NetworkManager::instance().setIsLeader(false);
                NetworkManager::instance().join_room(roomCode);
                QMessageBox::information(this, "Joining Game", "Joining " + inviter + "'s game...");
                populate_game_invites();
            }
        });

        connect(rejectBtn, &QPushButton::clicked, this, [this, inviter](){
            FriendManager::instance().rejectGameInvite(user_manager::instance().get_current_username(), inviter);
            populate_game_invites();
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