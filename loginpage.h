#ifndef LOGINPAGE_H
#define LOGINPAGE_H

#include <QDialog>
#include"usermanager.h"
#include"mainwindow.h"
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
    void login_check();
};

#endif // LOGINPAGE_H
