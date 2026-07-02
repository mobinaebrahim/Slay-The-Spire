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

    ui->btnLogin->raise();
    ui->btnRegister->raise();

    // ---connect video to loginpage/registerpage---
    connect(ui->btnLogin, &QPushButton::clicked, this, [this](){
        Dialogloginpage *loginDlg = new Dialogloginpage(this);
        loginDlg->setAttribute(Qt::WA_DeleteOnClose);
        loginDlg->exec();
    });

    connect(ui->btnRegister, &QPushButton::clicked, this, [this](){
        Dialogregisterpage *registerDlg = new Dialogregisterpage(this);
        registerDlg->setAttribute(Qt::WA_DeleteOnClose);
        registerDlg->exec();
    });
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    view->setGeometry(0, 0, ui->centralwidget->width(), ui->centralwidget->height());
    videoItem->setSize(QSizeF(ui->centralwidget->width(), ui->centralwidget->height()));
    scene->setSceneRect(0, 0, ui->centralwidget->width(), ui->centralwidget->height());
}

MainWindow::~MainWindow()
{
    delete ui;
}