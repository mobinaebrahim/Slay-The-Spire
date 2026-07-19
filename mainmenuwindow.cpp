#include "mainmenuwindow.h"
#include "./ui_mainmenuwindow.h"

MainMenuWindow::MainMenuWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainMenuWindow)
{
    ui->setupUi(this);

    // ---start background music---
    //AudioManager::instance().playMusic(":/assets/music/background.mp3");

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
        QMessageBox::information(this, "Continue", "Continuing your saved game!");
        // TODO: add actual continue game logic here using load_save

        // Reset UI state for next time
        ui->btnContinue->hide();
        ui->btnAbandon->hide();
        ui->btnPlay->show();
    });

    connect(ui->btnAbandon, &QPushButton::clicked, this, [this](){
        QString currentUser = user_manager::instance().get_current_username();
        QList<save_entry> saves = SaveManager::instance().get_save_list(currentUser);

        if (!saves.isEmpty()) {
            SaveManager::instance().delete_save(saves[0].id);
        }

        QMessageBox::information(this, "New Game", "Old save deleted. Starting a new game!");
        // TODO: add actual new game start logic here

        ui->btnContinue->hide();
        ui->btnAbandon->hide();
        ui->btnPlay->show();
    });

    //---test the map ---
    //console test
    GameMap testMap;
    testMap.generate();
    testMap.printToConsole();
    testMap.startRun();

    qDebug() << "=== Path test started ===";

    int steps = 0;
    while (!testMap.isAtBoss() && steps < 20) {
        // Find every available room across the whole map
        QVector<MapNode*> available;
        for (int f = 0; f < testMap.floorCount(); ++f) {
            for (int i = 0; i < testMap.roomCountAt(f); ++i) {
                MapNode *n = testMap.nodeAt(f, i);
                if (n->available())
                    available.append(n);
            }
        }

        if (available.isEmpty()) {
            qDebug() << "ERROR: no available rooms, but haven't reached the boss yet!";
            break;
        }

        // Pick one at random (simulating a player click)
        int pick = QRandomGenerator::global()->bounded(available.size());
        MapNode *choice = available[pick];

        bool ok = testMap.selectRoom(choice);
        qDebug() << "Step" << steps << ": floor" << (choice->floor() + 1)
                 << "type" << choice->typeLetter() << "->" << (ok ? "OK" : "FAILED");

        steps++;
    }

    qDebug() << "=== Reached boss?" << (testMap.isAtBoss() ? "YES" : "NO") << "===";

    /*//grafic test
    GameMap *testMap1 = new GameMap();
    testMap1->generate();
    testMap1->startRun();

    MapView *testView = new MapView(this);
    testView->resize(600, 600);
    testView->buildScene(testMap1);
    testView->show();

*/


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

    if (saves.isEmpty()) {
        MapPage *mapDlg = new MapPage(nullptr);
        mapDlg->setAttribute(Qt::WA_DeleteOnClose);

        this->hide();

        connect(mapDlg, &QObject::destroyed, this, [this](){
            this->show();
        });

        mapDlg->showFullScreen();
    }
    else {
        ui->btnPlay->hide();
        ui->btnContinue->show();
        ui->btnAbandon->show();
    }
}