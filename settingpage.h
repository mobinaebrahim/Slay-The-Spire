#ifndef SETTINGPAGE_H
#define SETTINGPAGE_H

#include <QWidget>
#include <QPainter>
#include <QTimer>

#include "audiomanager.h"
#include "cursormanager.h"
#include "usermanager.h"

namespace Ui {
class SettingPage;
}

class SettingPage : public QWidget
{
    Q_OBJECT

public:
    explicit SettingPage(QWidget *parent = nullptr);
    ~SettingPage();

private slots:
    void updateButtonVisibility(int index);

private:
    Ui::SettingPage *ui;

protected:
    void paintEvent(QPaintEvent *event) override;
};

#endif // SETTINGPAGE_H