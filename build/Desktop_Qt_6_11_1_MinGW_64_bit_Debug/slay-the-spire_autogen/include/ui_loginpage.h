/********************************************************************************
** Form generated from reading UI file 'loginpage.ui'
**
** Created by: Qt User Interface Compiler version 6.11.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOGINPAGE_H
#define UI_LOGINPAGE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>

QT_BEGIN_NAMESPACE

class Ui_Dialogloginpage
{
public:

    void setupUi(QDialog *Dialogloginpage)
    {
        if (Dialogloginpage->objectName().isEmpty())
            Dialogloginpage->setObjectName("Dialogloginpage");
        Dialogloginpage->resize(400, 300);

        retranslateUi(Dialogloginpage);

        QMetaObject::connectSlotsByName(Dialogloginpage);
    } // setupUi

    void retranslateUi(QDialog *Dialogloginpage)
    {
        Dialogloginpage->setWindowTitle(QCoreApplication::translate("Dialogloginpage", "Dialog", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Dialogloginpage: public Ui_Dialogloginpage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOGINPAGE_H
