/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.3.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>
#include "qcustomplot.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QHBoxLayout *mainLayout;
    QHBoxLayout *horizontalLayout;
    QFormLayout *formLayout;
    QLabel *label;
    QSpinBox *spinBox;
    QLabel *label_2;
    QSpinBox *spinSlushThresh;
    QLabel *costLive50Label;
    QDoubleSpinBox *costLive50SpinBox;
    QLabel *costLive75Label;
    QDoubleSpinBox *costLive75SpinBox;
    QLabel *costLive90Label;
    QDoubleSpinBox *costLive90SpinBox;
    QLabel *costLive95Label;
    QDoubleSpinBox *costLive95SpinBox;
    QLabel *costLive99Label;
    QDoubleSpinBox *costLive99SpinBox;
    QDoubleSpinBox *extraTodaySpinBox;
    QLabel *costLive99Label_2;
    QCustomPlot *plot;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(785, 596);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        mainLayout = new QHBoxLayout(centralWidget);
        mainLayout->setSpacing(1);
        mainLayout->setContentsMargins(11, 11, 11, 11);
        mainLayout->setObjectName(QStringLiteral("mainLayout"));
        mainLayout->setContentsMargins(1, 1, 1, 1);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(0);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        formLayout = new QFormLayout();
        formLayout->setSpacing(6);
        formLayout->setObjectName(QStringLiteral("formLayout"));
        formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
        label = new QLabel(centralWidget);
        label->setObjectName(QStringLiteral("label"));
        label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        formLayout->setWidget(0, QFormLayout::LabelRole, label);

        spinBox = new QSpinBox(centralWidget);
        spinBox->setObjectName(QStringLiteral("spinBox"));
        spinBox->setMinimum(-100000);
        spinBox->setMaximum(1000000);
        spinBox->setSingleStep(10);
        spinBox->setValue(1500);

        formLayout->setWidget(0, QFormLayout::FieldRole, spinBox);

        label_2 = new QLabel(centralWidget);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        formLayout->setWidget(1, QFormLayout::LabelRole, label_2);

        spinSlushThresh = new QSpinBox(centralWidget);
        spinSlushThresh->setObjectName(QStringLiteral("spinSlushThresh"));
        spinSlushThresh->setMinimum(-100000);
        spinSlushThresh->setMaximum(1000000);
        spinSlushThresh->setSingleStep(10);
        spinSlushThresh->setValue(1500);

        formLayout->setWidget(1, QFormLayout::FieldRole, spinSlushThresh);

        costLive50Label = new QLabel(centralWidget);
        costLive50Label->setObjectName(QStringLiteral("costLive50Label"));

        formLayout->setWidget(2, QFormLayout::LabelRole, costLive50Label);

        costLive50SpinBox = new QDoubleSpinBox(centralWidget);
        costLive50SpinBox->setObjectName(QStringLiteral("costLive50SpinBox"));
        costLive50SpinBox->setReadOnly(false);
        costLive50SpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
        costLive50SpinBox->setDecimals(1);
        costLive50SpinBox->setMinimum(-9999);
        costLive50SpinBox->setMaximum(9999);

        formLayout->setWidget(2, QFormLayout::FieldRole, costLive50SpinBox);

        costLive75Label = new QLabel(centralWidget);
        costLive75Label->setObjectName(QStringLiteral("costLive75Label"));

        formLayout->setWidget(3, QFormLayout::LabelRole, costLive75Label);

        costLive75SpinBox = new QDoubleSpinBox(centralWidget);
        costLive75SpinBox->setObjectName(QStringLiteral("costLive75SpinBox"));
        costLive75SpinBox->setReadOnly(false);
        costLive75SpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
        costLive75SpinBox->setDecimals(1);
        costLive75SpinBox->setMinimum(-9999);
        costLive75SpinBox->setMaximum(9999);

        formLayout->setWidget(3, QFormLayout::FieldRole, costLive75SpinBox);

        costLive90Label = new QLabel(centralWidget);
        costLive90Label->setObjectName(QStringLiteral("costLive90Label"));

        formLayout->setWidget(4, QFormLayout::LabelRole, costLive90Label);

        costLive90SpinBox = new QDoubleSpinBox(centralWidget);
        costLive90SpinBox->setObjectName(QStringLiteral("costLive90SpinBox"));
        costLive90SpinBox->setReadOnly(false);
        costLive90SpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
        costLive90SpinBox->setDecimals(1);
        costLive90SpinBox->setMinimum(-9999);
        costLive90SpinBox->setMaximum(9999);

        formLayout->setWidget(4, QFormLayout::FieldRole, costLive90SpinBox);

        costLive95Label = new QLabel(centralWidget);
        costLive95Label->setObjectName(QStringLiteral("costLive95Label"));

        formLayout->setWidget(5, QFormLayout::LabelRole, costLive95Label);

        costLive95SpinBox = new QDoubleSpinBox(centralWidget);
        costLive95SpinBox->setObjectName(QStringLiteral("costLive95SpinBox"));
        costLive95SpinBox->setReadOnly(false);
        costLive95SpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
        costLive95SpinBox->setDecimals(1);
        costLive95SpinBox->setMinimum(-9999);
        costLive95SpinBox->setMaximum(9999);

        formLayout->setWidget(5, QFormLayout::FieldRole, costLive95SpinBox);

        costLive99Label = new QLabel(centralWidget);
        costLive99Label->setObjectName(QStringLiteral("costLive99Label"));

        formLayout->setWidget(6, QFormLayout::LabelRole, costLive99Label);

        costLive99SpinBox = new QDoubleSpinBox(centralWidget);
        costLive99SpinBox->setObjectName(QStringLiteral("costLive99SpinBox"));
        costLive99SpinBox->setReadOnly(false);
        costLive99SpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
        costLive99SpinBox->setDecimals(1);
        costLive99SpinBox->setMinimum(-9999);
        costLive99SpinBox->setMaximum(9999);

        formLayout->setWidget(6, QFormLayout::FieldRole, costLive99SpinBox);

        extraTodaySpinBox = new QDoubleSpinBox(centralWidget);
        extraTodaySpinBox->setObjectName(QStringLiteral("extraTodaySpinBox"));
        extraTodaySpinBox->setReadOnly(false);
        extraTodaySpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
        extraTodaySpinBox->setDecimals(1);
        extraTodaySpinBox->setMinimum(-9999);
        extraTodaySpinBox->setMaximum(9999);

        formLayout->setWidget(7, QFormLayout::FieldRole, extraTodaySpinBox);

        costLive99Label_2 = new QLabel(centralWidget);
        costLive99Label_2->setObjectName(QStringLiteral("costLive99Label_2"));

        formLayout->setWidget(7, QFormLayout::LabelRole, costLive99Label_2);


        horizontalLayout->addLayout(formLayout);


        mainLayout->addLayout(horizontalLayout);

        plot = new QCustomPlot(centralWidget);
        plot->setObjectName(QStringLiteral("plot"));
        plot->setFocusPolicy(Qt::StrongFocus);

        mainLayout->addWidget(plot);

        mainLayout->setStretch(1, 1);
        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 785, 21));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        MainWindow->setStatusBar(statusBar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));
        label->setText(QApplication::translate("MainWindow", "balance", 0));
        spinBox->setPrefix(QApplication::translate("MainWindow", "$", 0));
        label_2->setText(QApplication::translate("MainWindow", "slush start", 0));
        spinSlushThresh->setPrefix(QApplication::translate("MainWindow", "$", 0));
        costLive50Label->setText(QApplication::translate("MainWindow", "cost live 50%", 0));
        costLive50SpinBox->setPrefix(QString());
        costLive50SpinBox->setSuffix(QApplication::translate("MainWindow", "$/d", 0));
        costLive75Label->setText(QApplication::translate("MainWindow", "cost live 75%", 0));
        costLive75SpinBox->setPrefix(QString());
        costLive75SpinBox->setSuffix(QApplication::translate("MainWindow", "$/d", 0));
        costLive90Label->setText(QApplication::translate("MainWindow", "cost live 90%", 0));
        costLive90SpinBox->setPrefix(QString());
        costLive90SpinBox->setSuffix(QApplication::translate("MainWindow", "$/d", 0));
        costLive95Label->setText(QApplication::translate("MainWindow", "cost live 95%", 0));
        costLive95SpinBox->setPrefix(QString());
        costLive95SpinBox->setSuffix(QApplication::translate("MainWindow", "$/d", 0));
        costLive99Label->setText(QApplication::translate("MainWindow", "cost live 99%", 0));
        costLive99SpinBox->setPrefix(QString());
        costLive99SpinBox->setSuffix(QApplication::translate("MainWindow", "$/d", 0));
        extraTodaySpinBox->setPrefix(QString());
        extraTodaySpinBox->setSuffix(QApplication::translate("MainWindow", "$", 0));
        costLive99Label_2->setText(QApplication::translate("MainWindow", "extra Today", 0));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
