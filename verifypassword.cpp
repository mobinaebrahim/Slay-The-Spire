#include "verifypassword.h"
#include "ui_verifypassword.h"

verifypassword::verifypassword(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::verifypassword)
{
    ui->setupUi(this);
}

verifypassword::~verifypassword()
{
    delete ui;
}
