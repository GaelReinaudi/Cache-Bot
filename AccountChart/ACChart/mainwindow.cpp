#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "../../core/acdata.h"

MainWindow::MainWindow(QString jsonFile, QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	// an account object that is going to be populated by the json file
	Account account;
	account.loadPlaidJson(jsonFile, 0, 0);

    ui->plot->loadCompressedAmount(&account);
//    ui->plot->loadAmount(&account);

	ui->sliderHash->setRange(-1, ui->plot->hashKeys().count() - 1);

	connect(ui->sliderHash, SIGNAL(valueChanged(int)), ui->plot, SLOT(showHash(int)));
	connect(ui->sliderHash, SIGNAL(valueChanged(int)), ui->spinHash, SLOT(setValue(int)));
	connect(ui->spinHash, SIGNAL(valueChanged(int)), ui->sliderHash, SLOT(setValue(int)));
	connect(ui->plot, SIGNAL(newLabel(QString)), ui->labelBundle, SLOT(setText(QString)));
	connect(ui->plot, SIGNAL(newSum(double)), ui->spinSum, SLOT(setValue(double)));
	connect(ui->plot, SIGNAL(newHashValue(int)), ui->spinHashVal, SLOT(setValue(int)));
}

MainWindow::~MainWindow()
{
	delete ui;
}
