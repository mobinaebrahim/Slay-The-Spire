/********************************************************************************
** Form generated from reading UI file 'registerpage.ui'
**
** Created by: Qt User Interface Compiler version 6.11.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_REGISTERPAGE_H
#define UI_REGISTERPAGE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>

QT_BEGIN_NAMESPACE

class Ui_Dialogregisterpage
{
public:

    void setupUi(QDialog *Dialogregisterpage)
    {
        if (Dialogregisterpage->objectName().isEmpty())
            Dialogregisterpage->setObjectName("Dialogregisterpage");
        Dialogregisterpage->resize(400, 300);

        retranslateUi(Dialogregisterpage);

        QMetaObject::connectSlotsByName(Dialogregisterpage);
    } // setupUi

    void retranslateUi(QDialog *Dialogregisterpage)
    {
        Dialogregisterpage->setWindowTitle(QCoreApplication::translate("Dialogregisterpage", "Dialog", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Dialogregisterpage: public Ui_Dialogregisterpage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_REGISTERPAGE_H
