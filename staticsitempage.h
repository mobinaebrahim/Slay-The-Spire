#ifndef STATICSITEMPAGE_H
#define STATICSITEMPAGE_H

#include <QDialog>
#include <QButtonGroup>

namespace Ui {
class StaticsItemPage;
}

class StaticsItemPage : public QDialog
{
    Q_OBJECT

public:
    explicit StaticsItemPage(QWidget *parent = nullptr);
    ~StaticsItemPage();

private:
    Ui::StaticsItemPage *ui;
    QButtonGroup *regionGroup;
    QButtonGroup *typeGroup;

    void refresh_leaderboard();
    QString format_duration(int totalSeconds);
    void refresh_history();

};

#endif // STATICSITEMPAGE_H
