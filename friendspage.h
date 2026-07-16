#ifndef FRIENDSPAGE_H
#define FRIENDSPAGE_H

#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QListWidgetItem>
#include <QPainter>
#include "friendmanager.h"
#include "usermanager.h"

namespace Ui {
class friendspage;
}

class friendspage : public QDialog
{
    Q_OBJECT

public:
    explicit friendspage(QWidget *parent = nullptr);
    ~friendspage();

private slots:
    void on_btnAddFriend_clicked();

private:
    Ui::friendspage *ui;
    void populate_pending_requests();
    void populate_friends_list();
    void showAddFriendError(const QString &message, bool isError = true);
protected:
    void paintEvent(QPaintEvent *event) override;
};

#endif // FRIENDSPAGE_H