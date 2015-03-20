#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "core/acdata.h"

MainWindow::MainWindow(QString jsonFile, QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);


	// an account object that is going to be populated by the json file
	account = new Account();
	account->loadPlaidJson(jsonFile);

	ui->acPlot->loadCompressedAmount(account);
	ui->amPlot->loadAmount(account);

	connect(ui->acPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->amPlot->xAxis, SLOT(setRange(QCPRange)));
	connect(ui->acPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->amPlot, SLOT(replot()));

//	ui->acPlot->setPlottingHints(QCP::phFastPolylines | QCP::phCacheLabels);

	// needed to spin a new thread and run the evolution in it
	m_evoThread = new QThread();
	m_evoSpinner = new EvolutionSpinner(account);
	m_evoSpinner->moveToThread(m_evoThread);
	connect(m_evoThread, &QThread::finished, m_evoSpinner, &QObject::deleteLater);
	connect(m_evoSpinner, &EvolutionSpinner::resultReady, this, &MainWindow::handleResults);
	connect(ui->startButton, SIGNAL(clicked(bool)), m_evoSpinner, SLOT(startStopEvolution(bool)), Qt::DirectConnection);
	connect(m_evoSpinner, &EvolutionSpinner::sendMask, this, &MainWindow::plotMask, Qt::BlockingQueuedConnection);
	connect(m_evoSpinner, &EvolutionSpinner::sendClearMask, this, &MainWindow::clearMasks, Qt::BlockingQueuedConnection);
	connect(m_evoSpinner, &EvolutionSpinner::needsReplot, this, &MainWindow::replotCharts, Qt::BlockingQueuedConnection);
	connect(m_evoSpinner, &EvolutionSpinner::sendClearList, this, &MainWindow::clearList);
	connect(m_evoSpinner, &EvolutionSpinner::newList, this, &MainWindow::newList, Qt::BlockingQueuedConnection);
	m_evoThread->start();
//	ui->startButton->click();
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::clearMasks()
{
	ui->acPlot->clearItems();
}

void MainWindow::plotMask(double x, double y)
{
	QCPItemRect* itRect = new QCPItemRect(ui->acPlot);
	y = kindaLog(y);
	itRect->topLeft->setCoords(QPointF(x - 4*3600*24, y + 0.1));
	itRect->bottomRight->setCoords(QPointF(x + 4*3600*24, y - 0.1));
	QColor colZone = QColor(11, 96, 254, 128);// : QColor(239, 64, 53, 128);
	itRect->setPen(QPen(QBrush(colZone), 3.0));
	itRect->setBrush(QBrush(colZone));
	itRect->setClipToAxisRect(false);
	ui->acPlot->addItem(itRect);
}

void MainWindow::replotCharts()
{
	ui->acPlot->replot(QCustomPlot::rpQueued);
	ui->amPlot->loadAmount(account);
	//	ui->amPlot->replot(QCustomPlot::rpQueued);
}

void MainWindow::clearList()
{
	ui->listBills->clear();
}

void MainWindow::newList(QStringList strList)
{
	ui->listBills->clear();
	for (const QString& str : strList) {
		ui->listBills->addItem(str);
	}
}

