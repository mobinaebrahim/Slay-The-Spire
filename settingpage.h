#ifndef SETTINGPAGE_H
#define SETTINGPAGE_H

#include <QWidget>
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

private:
    Ui::SettingPage *ui;

    void handleChangePassword();
};

#endif // SETTINGPAGE_H
