/********************************************************************************
** Form generated from reading UI file 'devicepropertiesdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.11.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DEVICEPROPERTIESDIALOG_H
#define UI_DEVICEPROPERTIESDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DevicePropertiesDialog
{
public:
    QVBoxLayout *verticalLayout;
    QTabWidget *tabWidget;
    QWidget *tab;
    QVBoxLayout *verticalLayout_3;
    QGridLayout *gridLayout_2;
    QLabel *label;
    QLineEdit *lineEdit_DeviceName;
    QGridLayout *gridLayout_3;
    QLabel *label_2;
    QComboBox *comboBox_Profile;
    QPushButton *pushButton_Edit;
    QGridLayout *gridLayout_4;
    QCheckBox *checkBox_AutoLIghtColor;
    QCheckBox *checkBox_UseProfileLIght;
    QGridLayout *gridLayout;
    QSlider *horizontalSlider_Red;
    QSlider *horizontalSlider_Green;
    QSlider *horizontalSlider_Blue;
    QLabel *label_5;
    QLabel *label_4;
    QLabel *label_3;
    QPushButton *pushButton_CustomColor;
    QCheckBox *checkBox_UseProfileIdle;
    QGroupBox *groupBox_IdleOptions;
    QVBoxLayout *verticalLayout_2;
    QCheckBox *checkBox_IdleDisconnect;
    QGridLayout *gridLayout_5;
    QDoubleSpinBox *doubleSpinBox;
    QComboBox *comboBox_2;
    QSpacerItem *verticalSpacer;
    QWidget *tab_2;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *DevicePropertiesDialog)
    {
        if (DevicePropertiesDialog->objectName().isEmpty())
            DevicePropertiesDialog->setObjectName(QStringLiteral("DevicePropertiesDialog"));
        DevicePropertiesDialog->resize(400, 430);
        verticalLayout = new QVBoxLayout(DevicePropertiesDialog);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        tabWidget = new QTabWidget(DevicePropertiesDialog);
        tabWidget->setObjectName(QStringLiteral("tabWidget"));
        tab = new QWidget();
        tab->setObjectName(QStringLiteral("tab"));
        verticalLayout_3 = new QVBoxLayout(tab);
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        gridLayout_2 = new QGridLayout();
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        label = new QLabel(tab);
        label->setObjectName(QStringLiteral("label"));
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy);

        gridLayout_2->addWidget(label, 0, 0, 1, 1);

        lineEdit_DeviceName = new QLineEdit(tab);
        lineEdit_DeviceName->setObjectName(QStringLiteral("lineEdit_DeviceName"));

        gridLayout_2->addWidget(lineEdit_DeviceName, 1, 0, 1, 1);


        verticalLayout_3->addLayout(gridLayout_2);

        gridLayout_3 = new QGridLayout();
        gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
        label_2 = new QLabel(tab);
        label_2->setObjectName(QStringLiteral("label_2"));

        gridLayout_3->addWidget(label_2, 0, 0, 1, 1);

        comboBox_Profile = new QComboBox(tab);
        comboBox_Profile->addItem(QString());
        comboBox_Profile->addItem(QString());
        comboBox_Profile->addItem(QString());
        comboBox_Profile->addItem(QString());
        comboBox_Profile->addItem(QString());
        comboBox_Profile->addItem(QString());
        comboBox_Profile->addItem(QString());
        comboBox_Profile->addItem(QString());
        comboBox_Profile->setObjectName(QStringLiteral("comboBox_Profile"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(comboBox_Profile->sizePolicy().hasHeightForWidth());
        comboBox_Profile->setSizePolicy(sizePolicy1);

        gridLayout_3->addWidget(comboBox_Profile, 1, 0, 1, 1);

        pushButton_Edit = new QPushButton(tab);
        pushButton_Edit->setObjectName(QStringLiteral("pushButton_Edit"));

        gridLayout_3->addWidget(pushButton_Edit, 1, 1, 1, 1);


        verticalLayout_3->addLayout(gridLayout_3);

        gridLayout_4 = new QGridLayout();
        gridLayout_4->setObjectName(QStringLiteral("gridLayout_4"));
        checkBox_AutoLIghtColor = new QCheckBox(tab);
        checkBox_AutoLIghtColor->setObjectName(QStringLiteral("checkBox_AutoLIghtColor"));

        gridLayout_4->addWidget(checkBox_AutoLIghtColor, 0, 1, 1, 1);

        checkBox_UseProfileLIght = new QCheckBox(tab);
        checkBox_UseProfileLIght->setObjectName(QStringLiteral("checkBox_UseProfileLIght"));

        gridLayout_4->addWidget(checkBox_UseProfileLIght, 0, 0, 1, 1);


        verticalLayout_3->addLayout(gridLayout_4);

        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        horizontalSlider_Red = new QSlider(tab);
        horizontalSlider_Red->setObjectName(QStringLiteral("horizontalSlider_Red"));
        horizontalSlider_Red->setOrientation(Qt::Horizontal);

        gridLayout->addWidget(horizontalSlider_Red, 0, 1, 1, 1);

        horizontalSlider_Green = new QSlider(tab);
        horizontalSlider_Green->setObjectName(QStringLiteral("horizontalSlider_Green"));
        horizontalSlider_Green->setOrientation(Qt::Horizontal);

        gridLayout->addWidget(horizontalSlider_Green, 1, 1, 1, 1);

        horizontalSlider_Blue = new QSlider(tab);
        horizontalSlider_Blue->setObjectName(QStringLiteral("horizontalSlider_Blue"));
        horizontalSlider_Blue->setOrientation(Qt::Horizontal);

        gridLayout->addWidget(horizontalSlider_Blue, 2, 1, 1, 1);

        label_5 = new QLabel(tab);
        label_5->setObjectName(QStringLiteral("label_5"));

        gridLayout->addWidget(label_5, 0, 0, 1, 1);

        label_4 = new QLabel(tab);
        label_4->setObjectName(QStringLiteral("label_4"));

        gridLayout->addWidget(label_4, 1, 0, 1, 1);

        label_3 = new QLabel(tab);
        label_3->setObjectName(QStringLiteral("label_3"));

        gridLayout->addWidget(label_3, 2, 0, 1, 1);

        pushButton_CustomColor = new QPushButton(tab);
        pushButton_CustomColor->setObjectName(QStringLiteral("pushButton_CustomColor"));

        gridLayout->addWidget(pushButton_CustomColor, 3, 0, 1, 2);


        verticalLayout_3->addLayout(gridLayout);

        checkBox_UseProfileIdle = new QCheckBox(tab);
        checkBox_UseProfileIdle->setObjectName(QStringLiteral("checkBox_UseProfileIdle"));

        verticalLayout_3->addWidget(checkBox_UseProfileIdle);

        groupBox_IdleOptions = new QGroupBox(tab);
        groupBox_IdleOptions->setObjectName(QStringLiteral("groupBox_IdleOptions"));
        verticalLayout_2 = new QVBoxLayout(groupBox_IdleOptions);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        checkBox_IdleDisconnect = new QCheckBox(groupBox_IdleOptions);
        checkBox_IdleDisconnect->setObjectName(QStringLiteral("checkBox_IdleDisconnect"));

        verticalLayout_2->addWidget(checkBox_IdleDisconnect);

        gridLayout_5 = new QGridLayout();
        gridLayout_5->setObjectName(QStringLiteral("gridLayout_5"));
        doubleSpinBox = new QDoubleSpinBox(groupBox_IdleOptions);
        doubleSpinBox->setObjectName(QStringLiteral("doubleSpinBox"));

        gridLayout_5->addWidget(doubleSpinBox, 0, 0, 1, 1);

        comboBox_2 = new QComboBox(groupBox_IdleOptions);
        comboBox_2->setObjectName(QStringLiteral("comboBox_2"));

        gridLayout_5->addWidget(comboBox_2, 0, 1, 1, 1);


        verticalLayout_2->addLayout(gridLayout_5);


        verticalLayout_3->addWidget(groupBox_IdleOptions);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_3->addItem(verticalSpacer);

        tabWidget->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QStringLiteral("tab_2"));
        tabWidget->addTab(tab_2, QString());

        verticalLayout->addWidget(tabWidget);

        buttonBox = new QDialogButtonBox(DevicePropertiesDialog);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(DevicePropertiesDialog);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(DevicePropertiesDialog);
    } // setupUi

    void retranslateUi(QDialog *DevicePropertiesDialog)
    {
        DevicePropertiesDialog->setWindowTitle(QApplication::translate("DevicePropertiesDialog", "Dialog", nullptr));
        label->setText(QApplication::translate("DevicePropertiesDialog", "Device name:", nullptr));
        label_2->setText(QApplication::translate("DevicePropertiesDialog", "Profile", nullptr));
        comboBox_Profile->setItemText(0, QApplication::translate("DevicePropertiesDialog", "[Default]", nullptr));
        comboBox_Profile->setItemText(1, QApplication::translate("DevicePropertiesDialog", "Dummy", nullptr));
        comboBox_Profile->setItemText(2, QApplication::translate("DevicePropertiesDialog", "Dummy", nullptr));
        comboBox_Profile->setItemText(3, QApplication::translate("DevicePropertiesDialog", "Dummy", nullptr));
        comboBox_Profile->setItemText(4, QApplication::translate("DevicePropertiesDialog", "Dummy", nullptr));
        comboBox_Profile->setItemText(5, QApplication::translate("DevicePropertiesDialog", "Dummy", nullptr));
        comboBox_Profile->setItemText(6, QApplication::translate("DevicePropertiesDialog", "Dummy", nullptr));
        comboBox_Profile->setItemText(7, QApplication::translate("DevicePropertiesDialog", "Dummy", nullptr));

        comboBox_Profile->setCurrentText(QApplication::translate("DevicePropertiesDialog", "[Default]", nullptr));
        pushButton_Edit->setText(QApplication::translate("DevicePropertiesDialog", "Edit", nullptr));
        checkBox_AutoLIghtColor->setText(QApplication::translate("DevicePropertiesDialog", "Automatic light color", nullptr));
        checkBox_UseProfileLIght->setText(QApplication::translate("DevicePropertiesDialog", "Use profile light", nullptr));
        label_5->setText(QApplication::translate("DevicePropertiesDialog", "Red", nullptr));
        label_4->setText(QApplication::translate("DevicePropertiesDialog", "Green", nullptr));
        label_3->setText(QApplication::translate("DevicePropertiesDialog", "Blue", nullptr));
        pushButton_CustomColor->setText(QApplication::translate("DevicePropertiesDialog", "Custom", nullptr));
        checkBox_UseProfileIdle->setText(QApplication::translate("DevicePropertiesDialog", "Use profile idle settings", nullptr));
        groupBox_IdleOptions->setTitle(QApplication::translate("DevicePropertiesDialog", "Idle options", nullptr));
        checkBox_IdleDisconnect->setText(QApplication::translate("DevicePropertiesDialog", "Disconnect on idle", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab), QApplication::translate("DevicePropertiesDialog", "Properties", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QApplication::translate("DevicePropertiesDialog", "Readout", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DevicePropertiesDialog: public Ui_DevicePropertiesDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DEVICEPROPERTIESDIALOG_H
