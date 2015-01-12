#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "EvolutionSpinner.h"
#include "ACChart/acdata.h"

MainWindow::MainWindow(QString jsonFile, QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);


	// an account object that is going to be populated by the json file
	Account* account = new Account();
	account->load(jsonFile);

	ui->accountPlot->loadCompressedAmount(account);
//	ui->accountPlot->loadAmount(account);

	// needed to spin a new thread and run the evolution in it
	m_evoThread = new QThread();
	m_evoSpinner = new EvolutionSpinner(account);
	m_evoSpinner->moveToThread(m_evoThread);
	connect(m_evoThread, &QThread::finished, m_evoSpinner, &QObject::deleteLater);
	connect(m_evoSpinner, &EvolutionSpinner::resultReady, this, &MainWindow::handleResults);
	connect(ui->startButton, SIGNAL(clicked(bool)), m_evoSpinner, SLOT(startEvolution(bool)));
	connect(m_evoSpinner, &EvolutionSpinner::sendMask, this, &MainWindow::plotMask);
	m_evoThread->start();
}

MainWindow::~MainWindow()
{
	delete ui;
}

