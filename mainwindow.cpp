#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

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

    //---test leaderboard---
    /*connect(ui->btnTestAddScore, &QPushButton::clicked, this, [this](){
        QString currentUser = user_manager::instance().get_current_username();
        int randomScore = QRandomGenerator::global()->bounded(100, 1000);
        int randomFloor = QRandomGenerator::global()->bounded(1, 20);

        ScoreManager::instance().add_scores(currentUser, "Ironclad", randomScore, randomFloor);
        QMessageBox::information(this, "Test", "Added score: " + QString::number(randomScore));
    });*/


    connect(ui->btnTestAddScore, &QPushButton::clicked, this, [this](){
        QString currentUser = user_manager::instance().get_current_username();
        int random_score = QRandomGenerator::global()->bounded(100, 1000);
        int random_floor = QRandomGenerator::global()->bounded(1, 20);
        int random_duration = QRandomGenerator::global()->bounded(300, 3600);

        ScoreManager::instance().add_score(currentUser, "Ironclad", random_score, random_floor, random_duration);

        QMessageBox::information(this, "Test", "Added score: " + QString::number(random_score));
    });

}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    view->setGeometry(0, 0, ui->centralwidget->width(), ui->centralwidget->height());
    videoItem->setSize(QSizeF(ui->centralwidget->width(), ui->centralwidget->height()));
    scene->setSceneRect(0, 0, ui->centralwidget->width(), ui->centralwidget->height());
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        return;
    }
    QMainWindow::keyPressEvent(event);
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::on_exit_button_clicked()
{
    close();
}
void MainWindow::go_to_menu()
{
    ui->stackedWidget->setCurrentWidget(ui->page_menu);
}



