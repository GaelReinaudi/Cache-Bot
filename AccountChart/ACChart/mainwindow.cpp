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

	// setup the chart options
	ui->plot->addGraph();
	ui->plot->graph(0)->setLineStyle(QCPGraph::lsNone);
	ui->plot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc));
	ui->plot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
	ui->plot->xAxis->setDateTimeFormat("yyyy/MM/dd hh");
	ui->plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
	ui->plot->axisRect(0)->setRangeZoomAxes(ui->plot->xAxis, 0);

	// add the purchase points
	for (const auto& trans : account.transactions().list()) {
		ui->plot->graph(0)->addData(trans.time(), trans.amount());
	}
	ui->plot->rescaleAxes();
}

MainWindow::~MainWindow()
{
	delete ui;
}
