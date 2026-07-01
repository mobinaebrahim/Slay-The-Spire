#include "registerpage.h"
#include "ui_registerpage.h"

Dialogregisterpage::Dialogregisterpage(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialogregisterpage)
{
    ui->setupUi(this);
}

Dialogregisterpage::~Dialogregisterpage()
{
    delete ui;
}
