#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "acdata.h"

MainWindow::MainWindow(QString jsonFile, QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	// an account object that is going to be populated by the json file
	Account account;
	account.load(jsonFile);

	ui->plot->loadAmount(&account);
}

MainWindow::~MainWindow()
{
	delete ui;
}
