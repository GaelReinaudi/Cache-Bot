#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "core/acdata.h"

MainWindow::MainWindow(QString jsonFile, QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	// an account object that is going to be populated by the json file
	Account account;
	account.loadPlaidJson(jsonFile);

	ui->plot->loadAmount(&account);
}

MainWindow::~MainWindow()
{
	delete ui;
}
