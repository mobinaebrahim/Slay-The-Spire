#ifndef VERIFYPASSWORD_H
#define VERIFYPASSWORD_H

#include <QDialog>

namespace Ui {
class verifypassword;
}

class verifypassword : public QDialog
{
    Q_OBJECT

public:
    explicit verifypassword(QWidget *parent = nullptr);
    ~verifypassword();

private:
    Ui::verifypassword *ui;
};

#endif // VERIFYPASSWORD_H
