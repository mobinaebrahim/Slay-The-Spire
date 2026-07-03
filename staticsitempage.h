#ifndef STATICSITEMPAGE_H
#define STATICSITEMPAGE_H

#include <QDialog>

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
};

#endif // STATICSITEMPAGE_H
