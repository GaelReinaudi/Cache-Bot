#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "user.h"
#include "evolver.h"

MainWindow::MainWindow(QString userID, int afterJday, int beforeJday)
	: QMainWindow()
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	Evolver* pEvolver = new Evolver(userID);

	connect(pEvolver, SIGNAL(injected(User*)), this, SLOT(onUserInjected(User*)));

//	connect(ui->startButton, SIGNAL(clicked(bool)), m_evoSpinner, SLOT(startStopEvolution(bool)), Qt::DirectConnection);
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::onUserInjected(User* pUser)
{
	ui->acPlot->loadCompressedAmount(pUser);
	ui->amPlot->loadAmount(pUser);
	ui->amPlot->hide();

	connect(ui->acPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->amPlot->xAxis, SLOT(setRange(QCPRange)));
	connect(ui->acPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->amPlot, SLOT(replot()));

//	ui->acPlot->setPlottingHints(QCP::phFastPolylines | QCP::phCacheLabels);

	connect(pUser->botContext(), &BotContext::targetedTransaction, this, &MainWindow::plotTargetedTransaction);
	connect(pUser->botContext(), &BotContext::matchedTransaction, this, &MainWindow::plotMatchedTransaction);
	connect(pUser->botContext(), &BotContext::summarizingTree, this, &MainWindow::clearMasks, Qt::BlockingQueuedConnection);
	connect(pUser->botContext(), &BotContext::needsReplot, this, &MainWindow::replotCharts, Qt::BlockingQueuedConnection);
	connect(pUser->botContext(), &BotContext::needsReplot, this, &MainWindow::clearList);
	connect(pUser->botContext(), &BotContext::newSummarizedTree, this, &MainWindow::onNewSummarizedTree, Qt::BlockingQueuedConnection);
}

void MainWindow::clearMasks()
{
	ui->acPlot->clearItems();
}

void MainWindow::plotMask(double x, double y, bool isTarget)
{
	QCPItemRect* itRect = new QCPItemRect(ui->acPlot);
	y = kindaLog(y);
	itRect->topLeft->setCoords(QPointF(x - 4*3600*24, y + (10+6*isTarget)*0.01));
	itRect->bottomRight->setCoords(QPointF(x + 4*3600*24, y - (10+6*isTarget)*0.01));
	QColor colZone = isTarget ? QColor(239, 64, 53, 128) : QColor(0, 64, 253, 128);
	itRect->setPen(QPen(QBrush(colZone), 3.0));
	itRect->setBrush(QBrush(colZone));
	itRect->setClipToAxisRect(false);
	ui->acPlot->addItem(itRect);
}

void MainWindow::replotCharts()
{
	ui->acPlot->replot(QCustomPlot::rpQueued);
	//ui->amPlot->loadAmount(account);
	//	ui->amPlot->replot(QCustomPlot::rpQueued);
}

void MainWindow::clearList()
{
	ui->listBills->clear();
}

void MainWindow::onNewSummarizedTree(QJsonObject jsonObj)
{
	ui->listBills->clear();
	for (const auto f : jsonObj["features"].toArray()) {
		ui->listBills->addItem(QString(QJsonDocument(f.toObject()).toJson()));
	}
}

