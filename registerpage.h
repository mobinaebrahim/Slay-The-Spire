#ifndef REGISTERPAGE_H
#define REGISTERPAGE_H

#include <QDialog>

namespace Ui {
class Dialogregisterpage;
}

class Dialogregisterpage : public QDialog
{
    Q_OBJECT

public:
    explicit Dialogregisterpage(QWidget *parent = nullptr);
    ~Dialogregisterpage();

private:
    Ui::Dialogregisterpage *ui;
};

#endif // REGISTERPAGE_H
