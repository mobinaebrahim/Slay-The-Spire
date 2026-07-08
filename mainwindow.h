#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QGraphicsVideoItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QAudioOutput>
#include <QResizeEvent>
#include <QRandomGenerator>
#include <QMessageBox>
#include "authpage.h"
#include "staticsitempage.h"
#include "scoremanager.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
    void go_to_menu();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void on_exit_button_clicked();

private:
    Ui::MainWindow *ui;
    QMediaPlayer *player;
    QGraphicsVideoItem *videoItem;
    QGraphicsScene *scene;
    QGraphicsView *view;
    QAudioOutput *audioOutput;
};

#endif // MAINWINDOW_H