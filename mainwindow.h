#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QGraphicsVideoItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QAudioOutput>
#include <QResizeEvent>
#include "loginpage.h"
#include "registerpage.h"

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

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    Ui::MainWindow *ui;
    QMediaPlayer *player;
    QGraphicsVideoItem *videoItem;
    QGraphicsScene *scene;
    QGraphicsView *view;
    QAudioOutput *audioOutput;
};

#endif // MAINWINDOW_H