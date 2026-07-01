#include "loginpage.h"
#include "ui_loginpage.h"

Dialogloginpage::Dialogloginpage(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialogloginpage)
{
    ui->setupUi(this);
}

Dialogloginpage::~Dialogloginpage()
{
    delete ui;
}
