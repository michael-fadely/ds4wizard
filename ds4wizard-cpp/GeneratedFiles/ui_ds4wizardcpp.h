/********************************************************************************
** Form generated from reading UI file 'ds4wizardcpp.ui'
**
** Created by: Qt User Interface Compiler version 5.11.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DS4WIZARDCPP_H
#define UI_DS4WIZARDCPP_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ds4wizardcppClass
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *ds4wizardcppClass)
    {
        if (ds4wizardcppClass->objectName().isEmpty())
            ds4wizardcppClass->setObjectName(QStringLiteral("ds4wizardcppClass"));
        ds4wizardcppClass->resize(600, 400);
        menuBar = new QMenuBar(ds4wizardcppClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        ds4wizardcppClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(ds4wizardcppClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        ds4wizardcppClass->addToolBar(mainToolBar);
        centralWidget = new QWidget(ds4wizardcppClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        ds4wizardcppClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(ds4wizardcppClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        ds4wizardcppClass->setStatusBar(statusBar);

        retranslateUi(ds4wizardcppClass);

        QMetaObject::connectSlotsByName(ds4wizardcppClass);
    } // setupUi

    void retranslateUi(QMainWindow *ds4wizardcppClass)
    {
        ds4wizardcppClass->setWindowTitle(QApplication::translate("ds4wizardcppClass", "ds4wizardcpp", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ds4wizardcppClass: public Ui_ds4wizardcppClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DS4WIZARDCPP_H
