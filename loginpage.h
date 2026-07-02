#ifndef LOGINPAGE_H
#define LOGINPAGE_H

#include <QDialog>

namespace Ui {
class Dialogloginpage;
}

class Dialogloginpage : public QDialog
{
    Q_OBJECT

public:
    explicit Dialogloginpage(QWidget *parent = nullptr);
    ~Dialogloginpage();

private:
    Ui::Dialogloginpage *ui;
};

#endif // LOGINPAGE_H
