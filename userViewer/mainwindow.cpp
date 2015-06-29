#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "user.h"
#include "botContext.h"
#include "bot.h"
#include "cacherest.h"
#include "cacheAccountConnector.h"

MainWindow::MainWindow(QString userID)
	: QMainWindow()
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	m_pConnector = new CacheAccountConnector(userID);
	connect(m_pConnector, SIGNAL(injected(User*)), this, SLOT(onUserInjected(User*)));
	connect(m_pConnector, SIGNAL(botInjected(Bot*)), this, SLOT(onBotInjected(Bot*)));
}

MainWindow::~MainWindow()
{
	if (m_pConnector) {
		delete m_pConnector;
	}
	delete ui;
}

User *MainWindow::user() const
{
	return m_pConnector->user();
}

void MainWindow::onUserInjected(User* pUser)
{
	ui->acPlot->loadCompressedAmount(pUser);
	ui->ahPlot->loadCompressedAmount(pUser);
	ui->sliderHash->setRange(-1, ui->acPlot->hashKeys().count() - 1);
	ui->sliderHash->setValue(-1);

	connect(ui->sliderHash, SIGNAL(valueChanged(int)), ui->acPlot, SLOT(showHash(int)));
	connect(ui->sliderHash, SIGNAL(valueChanged(int)), ui->ahPlot, SLOT(showHash(int)));
	connect(ui->sliderHash, SIGNAL(valueChanged(int)), ui->spinHash, SLOT(setValue(int)));
	connect(ui->spinHash, SIGNAL(valueChanged(int)), ui->sliderHash, SLOT(setValue(int)));
	connect(ui->acPlot, SIGNAL(newLabel(QString)), ui->labelBundle, SLOT(setText(QString)));
	connect(ui->acPlot, SIGNAL(newSum(double)), ui->spinSum, SLOT(setValue(double)));
	connect(ui->acPlot, SIGNAL(newHashValue(int)), ui->spinHashVal, SLOT(setValue(int)));

	CacheRest::Instance()->getBestBot(pUser->id(), pUser);
}

void MainWindow::onBotInjected(Bot* bestBot)
{
	connect(user()->botContext(), &BotContext::targetedTransaction, this, &MainWindow::plotTargetedTransaction);
	connect(user()->botContext(), &BotContext::matchedTransaction, this, &MainWindow::plotMatchedTransaction);
	connect(user()->botContext(), &BotContext::summarizingTree, this, &MainWindow::clearMasks);
	connect(user()->botContext(), &BotContext::needsReplot, this, &MainWindow::replotCharts);
	connect(user()->botContext(), &BotContext::newSummarizedTree, this, &MainWindow::onNewSummarizedTree);

	bestBot->summarize();
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
	QColor colZone = isTarget ? QColor(239, 64, 53, 32) : QColor(0, 64, 253, 32);
	itRect->setPen(QPen(QBrush(colZone), 3.0));
	itRect->setBrush(QBrush(colZone));
	itRect->setClipToAxisRect(false);
	ui->acPlot->addItem(itRect);
}

void MainWindow::replotCharts()
{
	ui->acPlot->replot(QCustomPlot::rpQueued);
	ui->ahPlot->replot(QCustomPlot::rpQueued);
}

void MainWindow::onNewSummarizedTree(QJsonObject jsonObj)
{
	ui->listBills->clear();
	for (const auto f : jsonObj["features"].toArray()) {
		ui->listBills->addItem(QString(QJsonDocument(f.toObject()).toJson()));
	}
}

