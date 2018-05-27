/********************************************************************************
** Form generated from reading UI file 'deviceproperties.ui'
**
** Created by: Qt User Interface Compiler version 5.11.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DEVICEPROPERTIES_H
#define UI_DEVICEPROPERTIES_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DeviceProperties
{
public:
    QVBoxLayout *verticalLayout;
    QTabWidget *tabWidget;
    QWidget *tab;
    QVBoxLayout *verticalLayout_4;
    QGridLayout *gridLayout_2;
    QLabel *label;
    QLineEdit *lineEdit;
    QGridLayout *gridLayout_3;
    QLabel *label_2;
    QComboBox *comboBox;
    QPushButton *pushButton;
    QGridLayout *gridLayout;
    QSlider *horizontalSlider_3;
    QSlider *horizontalSlider_2;
    QSlider *horizontalSlider;
    QLabel *label_5;
    QLabel *label_4;
    QLabel *label_3;
    QPushButton *pushButton_2;
    QSpacerItem *verticalSpacer;
    QWidget *tab_2;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *DeviceProperties)
    {
        if (DeviceProperties->objectName().isEmpty())
            DeviceProperties->setObjectName(QStringLiteral("DeviceProperties"));
        DeviceProperties->resize(400, 480);
        verticalLayout = new QVBoxLayout(DeviceProperties);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        tabWidget = new QTabWidget(DeviceProperties);
        tabWidget->setObjectName(QStringLiteral("tabWidget"));
        tab = new QWidget();
        tab->setObjectName(QStringLiteral("tab"));
        verticalLayout_4 = new QVBoxLayout(tab);
        verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
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

        lineEdit = new QLineEdit(tab);
        lineEdit->setObjectName(QStringLiteral("lineEdit"));

        gridLayout_2->addWidget(lineEdit, 1, 0, 1, 1);


        verticalLayout_4->addLayout(gridLayout_2);

        gridLayout_3 = new QGridLayout();
        gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
        label_2 = new QLabel(tab);
        label_2->setObjectName(QStringLiteral("label_2"));

        gridLayout_3->addWidget(label_2, 0, 0, 1, 1);

        comboBox = new QComboBox(tab);
        comboBox->addItem(QString());
        comboBox->addItem(QString());
        comboBox->addItem(QString());
        comboBox->addItem(QString());
        comboBox->addItem(QString());
        comboBox->addItem(QString());
        comboBox->addItem(QString());
        comboBox->addItem(QString());
        comboBox->setObjectName(QStringLiteral("comboBox"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(comboBox->sizePolicy().hasHeightForWidth());
        comboBox->setSizePolicy(sizePolicy1);

        gridLayout_3->addWidget(comboBox, 1, 0, 1, 1);

        pushButton = new QPushButton(tab);
        pushButton->setObjectName(QStringLiteral("pushButton"));

        gridLayout_3->addWidget(pushButton, 1, 1, 1, 1);


        verticalLayout_4->addLayout(gridLayout_3);

        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        horizontalSlider_3 = new QSlider(tab);
        horizontalSlider_3->setObjectName(QStringLiteral("horizontalSlider_3"));
        horizontalSlider_3->setOrientation(Qt::Horizontal);

        gridLayout->addWidget(horizontalSlider_3, 0, 1, 1, 1);

        horizontalSlider_2 = new QSlider(tab);
        horizontalSlider_2->setObjectName(QStringLiteral("horizontalSlider_2"));
        horizontalSlider_2->setOrientation(Qt::Horizontal);

        gridLayout->addWidget(horizontalSlider_2, 1, 1, 1, 1);

        horizontalSlider = new QSlider(tab);
        horizontalSlider->setObjectName(QStringLiteral("horizontalSlider"));
        horizontalSlider->setOrientation(Qt::Horizontal);

        gridLayout->addWidget(horizontalSlider, 2, 1, 1, 1);

        label_5 = new QLabel(tab);
        label_5->setObjectName(QStringLiteral("label_5"));

        gridLayout->addWidget(label_5, 0, 0, 1, 1);

        label_4 = new QLabel(tab);
        label_4->setObjectName(QStringLiteral("label_4"));

        gridLayout->addWidget(label_4, 1, 0, 1, 1);

        label_3 = new QLabel(tab);
        label_3->setObjectName(QStringLiteral("label_3"));

        gridLayout->addWidget(label_3, 2, 0, 1, 1);

        pushButton_2 = new QPushButton(tab);
        pushButton_2->setObjectName(QStringLiteral("pushButton_2"));

        gridLayout->addWidget(pushButton_2, 3, 0, 1, 2);


        verticalLayout_4->addLayout(gridLayout);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_4->addItem(verticalSpacer);

        tabWidget->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QStringLiteral("tab_2"));
        tabWidget->addTab(tab_2, QString());

        verticalLayout->addWidget(tabWidget);

        buttonBox = new QDialogButtonBox(DeviceProperties);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(DeviceProperties);
        QObject::connect(buttonBox, SIGNAL(accepted()), DeviceProperties, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), DeviceProperties, SLOT(reject()));

        QMetaObject::connectSlotsByName(DeviceProperties);
    } // setupUi

    void retranslateUi(QDialog *DeviceProperties)
    {
        DeviceProperties->setWindowTitle(QApplication::translate("DeviceProperties", "Dialog", nullptr));
        label->setText(QApplication::translate("DeviceProperties", "Name", nullptr));
        label_2->setText(QApplication::translate("DeviceProperties", "Profile", nullptr));
        comboBox->setItemText(0, QApplication::translate("DeviceProperties", "[Default]", nullptr));
        comboBox->setItemText(1, QApplication::translate("DeviceProperties", "Dummy", nullptr));
        comboBox->setItemText(2, QApplication::translate("DeviceProperties", "Dummy", nullptr));
        comboBox->setItemText(3, QApplication::translate("DeviceProperties", "Dummy", nullptr));
        comboBox->setItemText(4, QApplication::translate("DeviceProperties", "Dummy", nullptr));
        comboBox->setItemText(5, QApplication::translate("DeviceProperties", "Dummy", nullptr));
        comboBox->setItemText(6, QApplication::translate("DeviceProperties", "Dummy", nullptr));
        comboBox->setItemText(7, QApplication::translate("DeviceProperties", "Dummy", nullptr));

        comboBox->setCurrentText(QApplication::translate("DeviceProperties", "[Default]", nullptr));
        pushButton->setText(QApplication::translate("DeviceProperties", "Edit", nullptr));
        label_5->setText(QApplication::translate("DeviceProperties", "Red", nullptr));
        label_4->setText(QApplication::translate("DeviceProperties", "Green", nullptr));
        label_3->setText(QApplication::translate("DeviceProperties", "Blue", nullptr));
        pushButton_2->setText(QApplication::translate("DeviceProperties", "Custom", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab), QApplication::translate("DeviceProperties", "Tab 1", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QApplication::translate("DeviceProperties", "Tab 2", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DeviceProperties: public Ui_DeviceProperties {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DEVICEPROPERTIES_H
