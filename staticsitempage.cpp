#include "staticsitempage.h"
#include "ui_staticsitempage.h"

StaticsItemPage::StaticsItemPage(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::StaticsItemPage)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);

    ui->btnReturn->raise();
    connect(ui->btnReturn,&QPushButton::clicked,this,[this](){
        close();
    });
}

StaticsItemPage::~StaticsItemPage()
{
    delete ui;
}
