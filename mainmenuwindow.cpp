#include "mainmenuwindow.h"
#include "./ui_mainmenuwindow.h"

MainMenuWindow::MainMenuWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainMenuWindow)
{
    ui->setupUi(this);

    // ---start background music---
    AudioManager::instance().playMusic(":/assets/music/background.mp3");

    // ---play click sound for every button in this window---
    const QList<QPushButton*> allButtons = this->findChildren<QPushButton*>();
    for (QPushButton *btn : allButtons) {
        connect(btn, &QPushButton::clicked, this, [](){
            AudioManager::instance().playEffect(":/assets/music/menu_button.mp3");
        });
    }

    // ---set video on background---
    scene = new QGraphicsScene(this);
    view = new QGraphicsView(scene, ui->centralwidget);
    view->setGeometry(0, 0, ui->centralwidget->width(), ui->centralwidget->height());
    view->setStyleSheet("background: transparent; border: none;");
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    videoItem = new QGraphicsVideoItem();
    scene->addItem(videoItem);
    videoItem->setSize(QSizeF(ui->centralwidget->width(), ui->centralwidget->height()));

    // ---made video player
    player = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    player->setAudioOutput(audioOutput);
    player->setVideoOutput(videoItem);

    player->setSource(QUrl("qrc:/assets/assets/video/background.mp4"));

    // ---made loop for video
    connect(player, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status){
        if (status == QMediaPlayer::EndOfMedia) {
            player->setPosition(0);
            player->play();
        }
    });

    audioOutput->setMuted(true);
    player->play();

    view->lower();

    //---home_page---
    ui->btnLogin->raise();
    ui->btnRegister->raise();

    connect(ui->btnLogin, &QPushButton::clicked, this, [this](){
        AuthPage *authDlg = new AuthPage(this);
        authDlg->setAttribute(Qt::WA_DeleteOnClose);
        authDlg->show_login_page();
        authDlg->exec();
    });
    connect(ui->btnRegister, &QPushButton::clicked, this, [this](){
        AuthPage *authDlg = new AuthPage(this);
        authDlg->setAttribute(Qt::WA_DeleteOnClose);
        authDlg->show_register_page();
        authDlg->exec();
    });

    //---main_menu---
    ui->btnSetting->raise();
    ui->btnStatics->raise();
    ui->btnQuit->raise();
    ui->btnCompendium->raise();
    ui->btnContinue->hide();
    ui->btnAbandon->hide();

    connect(ui->btnQuit,&QPushButton::clicked, this, [this](){
        this->close();
    });


    connect(ui->btnStatics, &QPushButton::clicked, this, [this](){
        ui->page_menu->hide();

        StaticsItemPage *statsDlg = new StaticsItemPage(this);
        statsDlg->setAttribute(Qt::WA_DeleteOnClose);

        connect(statsDlg, &QDialog::finished, this, [this](){
            ui->page_menu->show();
        });

        statsDlg->exec();
    });

    connect(ui->btnFriends, &QPushButton::clicked, this, [this](){
        friendspage *friendsDlg = new friendspage(this);
        friendsDlg->setAttribute(Qt::WA_DeleteOnClose);
        friendsDlg->exec();
    });

    connect(ui->btnSetting, &QPushButton::clicked, this, [this](){
        SettingPage *settingDlg = new SettingPage(this);
        settingDlg->setAttribute(Qt::WA_DeleteOnClose);
        settingDlg->show();
    });

    connect(ui->btnPlay, &QPushButton::clicked, this, &MainMenuWindow::handle_play_button);

    connect(ui->btnContinue, &QPushButton::clicked, this, [this](){
        QString currentUser = user_manager::instance().get_current_username();
        QList<save_entry> saves = SaveManager::instance().get_save_list(currentUser);

        // Reset UI state for next time
        ui->btnContinue->hide();
        ui->btnAbandon->hide();
        ui->btnPlay->show();

        if (saves.isEmpty()) {
            QMessageBox::warning(this, "Continue", "No saved game found.");
            return;
        }

        int saveId = saves[0].id;
        QJsonObject mapData = SaveManager::instance().load_save(saveId);

        open_map_page(true, false, saveId, mapData);
    });

    connect(ui->btnAbandon, &QPushButton::clicked, this, [this](){
        QString currentUser = user_manager::instance().get_current_username();
        QList<save_entry> saves = SaveManager::instance().get_save_list(currentUser);

        if (!saves.isEmpty()) {
            SaveManager::instance().delete_save(saves[0].id);
        }

        QMessageBox::information(this, "New Game", "Old save deleted. Starting a new game!");

        ui->btnContinue->hide();
        ui->btnAbandon->hide();
        ui->btnPlay->show();
    });

    // ============================================================
    // Multiplayer: single, canonical place that opens MapPage when
    // a room is actually ready to play (both players present).
    // ------------------------------------------------------------
    // The server broadcasts "room_joined" to BOTH sockets in the
    // room the moment the second player joins (see
    // GameServer::handle_join_room -> broadcast_to_room). So this
    // one handler fires for the leader (who called create_room()
    // earlier, e.g. from friendspage's invite flow) AND for the
    // person who just joined. Nothing else in the app should open
    // MapPage in response to room_created/room_joined — this is
    // the only place, so we never get duplicate map windows.
    // ============================================================
    connect(&NetworkManager::instance(), &NetworkManager::room_joined, this, [this](const QString &roomCode){
        Q_UNUSED(roomCode);

        if (QWidget *activeModal = QApplication::activeModalWidget()) {
            activeModal->close();
        }

        open_map_page(NetworkManager::instance().isLeader(), true);
    });
}

void MainMenuWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    view->setGeometry(0, 0, ui->centralwidget->width(), ui->centralwidget->height());
    videoItem->setSize(QSizeF(ui->centralwidget->width(), ui->centralwidget->height()));
    scene->setSceneRect(0, 0, ui->centralwidget->width(), ui->centralwidget->height());
}

void MainMenuWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        return;
    }
    QMainWindow::keyPressEvent(event);
}

MainMenuWindow::~MainMenuWindow()
{
    delete ui;
}
void MainMenuWindow::on_exit_button_clicked()
{
    close();
}
void MainMenuWindow::go_to_menu()
{
    ui->stackedWidget->setCurrentWidget(ui->page_menu);
}

void MainMenuWindow::handle_play_button()
{
    QString currentUser = user_manager::instance().get_current_username();
    QList<save_entry> saves = SaveManager::instance().get_save_list(currentUser);

    if (!saves.isEmpty()) {
        ui->btnPlay->hide();
        ui->btnContinue->show();
        ui->btnAbandon->show();
        return;
    }

    QMessageBox modeBox(this);
    modeBox.setWindowTitle("Choose Mode");
    modeBox.setText("How do you want to play?");
    QPushButton *singleBtn = modeBox.addButton("Single Player", QMessageBox::AcceptRole);
    QPushButton *multiBtn  = modeBox.addButton("Multiplayer", QMessageBox::AcceptRole);
    modeBox.addButton(QMessageBox::Cancel);
    modeBox.exec();

    if (modeBox.clickedButton() == singleBtn) {
        open_map_page(true, false);
    }
    else if (modeBox.clickedButton() == multiBtn) {

        friendspage *friendsDlg = new friendspage(this);
        friendsDlg->setAttribute(Qt::WA_DeleteOnClose);
        friendsDlg->exec();
    }
}


void MainMenuWindow::open_map_page(bool isLeader, bool isMultiplayer, int existingSaveId, const QJsonObject &savedMapData)
{
    MapPage *mapDlg = new MapPage(nullptr, isLeader, isMultiplayer, existingSaveId, savedMapData);
    mapDlg->setAttribute(Qt::WA_DeleteOnClose);

    this->hide();

    connect(mapDlg, &MapPage::runAbandoned, this, [this](){
        this->show();
        this->raise();
        this->activateWindow();
    });

    connect(mapDlg, &QObject::destroyed, this, [this](){
        this->show();
    });

    mapDlg->showFullScreen();
}