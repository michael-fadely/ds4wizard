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
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ds4wizardcpp
{
public:
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout_2;
    QTabWidget *tabWidget;
    QWidget *tab_1;
    QVBoxLayout *verticalLayout_3;
    QTableWidget *deviceTable;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushButton_DeviceProperties;
    QWidget *tab_3;
    QVBoxLayout *verticalLayout_4;
    QListWidget *profileList;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *profileEdit;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *profileAdd;
    QPushButton *profileDelete;
    QWidget *tab_4;
    QVBoxLayout *verticalLayout_6;
    QCheckBox *checkStartMinimized;
    QCheckBox *checkMinimizeToTray;
    QVBoxLayout *verticalLayout_5;
    QLabel *label;
    QComboBox *comboConnectionType;
    QSpacerItem *verticalSpacer;

    void setupUi(QMainWindow *ds4wizardcpp)
    {
        if (ds4wizardcpp->objectName().isEmpty())
            ds4wizardcpp->setObjectName(QStringLiteral("ds4wizardcpp"));
        ds4wizardcpp->resize(384, 400);
        centralWidget = new QWidget(ds4wizardcpp);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        verticalLayout_2 = new QVBoxLayout(centralWidget);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        tabWidget = new QTabWidget(centralWidget);
        tabWidget->setObjectName(QStringLiteral("tabWidget"));
        tab_1 = new QWidget();
        tab_1->setObjectName(QStringLiteral("tab_1"));
        verticalLayout_3 = new QVBoxLayout(tab_1);
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setContentsMargins(11, 11, 11, 11);
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        deviceTable = new QTableWidget(tab_1);
        if (deviceTable->columnCount() < 2)
            deviceTable->setColumnCount(2);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        deviceTable->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        deviceTable->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        deviceTable->setObjectName(QStringLiteral("deviceTable"));
        deviceTable->setColumnCount(2);

        verticalLayout_3->addWidget(deviceTable);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        pushButton_DeviceProperties = new QPushButton(tab_1);
        pushButton_DeviceProperties->setObjectName(QStringLiteral("pushButton_DeviceProperties"));

        horizontalLayout->addWidget(pushButton_DeviceProperties);


        verticalLayout_3->addLayout(horizontalLayout);

        tabWidget->addTab(tab_1, QString());
        tab_3 = new QWidget();
        tab_3->setObjectName(QStringLiteral("tab_3"));
        verticalLayout_4 = new QVBoxLayout(tab_3);
        verticalLayout_4->setSpacing(6);
        verticalLayout_4->setContentsMargins(11, 11, 11, 11);
        verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
        profileList = new QListWidget(tab_3);
        profileList->setObjectName(QStringLiteral("profileList"));

        verticalLayout_4->addWidget(profileList);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        profileEdit = new QPushButton(tab_3);
        profileEdit->setObjectName(QStringLiteral("profileEdit"));

        horizontalLayout_2->addWidget(profileEdit);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_2);

        profileAdd = new QPushButton(tab_3);
        profileAdd->setObjectName(QStringLiteral("profileAdd"));

        horizontalLayout_2->addWidget(profileAdd);

        profileDelete = new QPushButton(tab_3);
        profileDelete->setObjectName(QStringLiteral("profileDelete"));

        horizontalLayout_2->addWidget(profileDelete);


        verticalLayout_4->addLayout(horizontalLayout_2);

        tabWidget->addTab(tab_3, QString());
        tab_4 = new QWidget();
        tab_4->setObjectName(QStringLiteral("tab_4"));
        verticalLayout_6 = new QVBoxLayout(tab_4);
        verticalLayout_6->setSpacing(6);
        verticalLayout_6->setContentsMargins(11, 11, 11, 11);
        verticalLayout_6->setObjectName(QStringLiteral("verticalLayout_6"));
        checkStartMinimized = new QCheckBox(tab_4);
        checkStartMinimized->setObjectName(QStringLiteral("checkStartMinimized"));

        verticalLayout_6->addWidget(checkStartMinimized);

        checkMinimizeToTray = new QCheckBox(tab_4);
        checkMinimizeToTray->setObjectName(QStringLiteral("checkMinimizeToTray"));

        verticalLayout_6->addWidget(checkMinimizeToTray);

        verticalLayout_5 = new QVBoxLayout();
        verticalLayout_5->setSpacing(6);
        verticalLayout_5->setObjectName(QStringLiteral("verticalLayout_5"));
        label = new QLabel(tab_4);
        label->setObjectName(QStringLiteral("label"));

        verticalLayout_5->addWidget(label);

        comboConnectionType = new QComboBox(tab_4);
        comboConnectionType->addItem(QString());
        comboConnectionType->addItem(QString());
        comboConnectionType->setObjectName(QStringLiteral("comboConnectionType"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(comboConnectionType->sizePolicy().hasHeightForWidth());
        comboConnectionType->setSizePolicy(sizePolicy);

        verticalLayout_5->addWidget(comboConnectionType);


        verticalLayout_6->addLayout(verticalLayout_5);

        verticalSpacer = new QSpacerItem(20, 226, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_6->addItem(verticalSpacer);

        tabWidget->addTab(tab_4, QString());

        verticalLayout_2->addWidget(tabWidget);

        ds4wizardcpp->setCentralWidget(centralWidget);

        retranslateUi(ds4wizardcpp);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(ds4wizardcpp);
    } // setupUi

    void retranslateUi(QMainWindow *ds4wizardcpp)
    {
        ds4wizardcpp->setWindowTitle(QApplication::translate("ds4wizardcpp", "ds4wizardcpp", nullptr));
        QTableWidgetItem *___qtablewidgetitem = deviceTable->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QApplication::translate("ds4wizardcpp", "Device", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = deviceTable->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QApplication::translate("ds4wizardcpp", "Battery", nullptr));
        pushButton_DeviceProperties->setText(QApplication::translate("ds4wizardcpp", "&Properties", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_1), QApplication::translate("ds4wizardcpp", "Devices", nullptr));
        profileEdit->setText(QApplication::translate("ds4wizardcpp", "Edit", nullptr));
        profileAdd->setText(QApplication::translate("ds4wizardcpp", "Add", nullptr));
        profileDelete->setText(QApplication::translate("ds4wizardcpp", "Delete", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_3), QApplication::translate("ds4wizardcpp", "Profiles", nullptr));
        checkStartMinimized->setText(QApplication::translate("ds4wizardcpp", "Start minimized", nullptr));
        checkMinimizeToTray->setText(QApplication::translate("ds4wizardcpp", "Minimize to tray", nullptr));
        label->setText(QApplication::translate("ds4wizardcpp", "Preferred connection type:", nullptr));
        comboConnectionType->setItemText(0, QApplication::translate("ds4wizardcpp", "USB", nullptr));
        comboConnectionType->setItemText(1, QApplication::translate("ds4wizardcpp", "Bluetooth", nullptr));

        tabWidget->setTabText(tabWidget->indexOf(tab_4), QApplication::translate("ds4wizardcpp", "Settings", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ds4wizardcpp: public Ui_ds4wizardcpp {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DS4WIZARDCPP_H
