#ifndef MAINMENUWINDOW_H
#define MAINMENUWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QGraphicsVideoItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QAudioOutput>
#include <QResizeEvent>
#include <QRandomGenerator>
#include <QMessageBox>
#include <QJsonObject>
#include <QApplication>

#include "authpage.h"
#include "staticsitempage.h"
#include "scoremanager.h"
#include "friendspage.h"
#include "savemanager.h"
#include "usermanager.h"
#include "gamemap.h"
#include "mapview.h"
#include "mappage.h"
#include "audiomanager.h"
#include "settingpage.h"
#include "networkmanager.h"


QT_BEGIN_NAMESPACE
namespace Ui {
class MainMenuWindow;
}
QT_END_NAMESPACE

class MainMenuWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainMenuWindow(QWidget *parent = nullptr);
    ~MainMenuWindow() override;
    void go_to_menu();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void on_exit_button_clicked();

private:
    Ui::MainMenuWindow *ui;
    QMediaPlayer *player;
    QGraphicsVideoItem *videoItem;
    QGraphicsScene *scene;
    QGraphicsView *view;
    QAudioOutput *audioOutput;

    QList<QPushButton*> m_hiddenMenuButtons;

    void hide_menu_buttons_for_dialog();
    void restore_menu_buttons_after_dialog();

    void handle_play_button();
    void open_map_page(bool isLeader, bool isMultiplayer, int existingSaveId = -1, const QJsonObject &savedMapData = QJsonObject());
};

#endif // MAINMENUWINDOW_H