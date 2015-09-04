#include "evoCacheView.h"
#include "ui_evoCacheView.h"
#include "user.h"
#include "evolver.h"
#include "botContext.h"

EvoCacheView::EvoCacheView(QString userID, QVector<int> onlyLoadHashes)
	: QMainWindow()
	, ui(new Ui::EvoCacheView)
{
	ui->setupUi(this);
	ui->amPlot->hide();

	Evolver* pEvolver = new Evolver(userID);
//	pEvolver->flags &= ~CacheAccountConnector::SendBot;
	Transaction::onlyLoadHashes = onlyLoadHashes;

	connect(pEvolver, SIGNAL(injected(User*)), this, SLOT(onUserInjected(User*)));
	connect(ui->startButton, SIGNAL(clicked(bool)), pEvolver, SIGNAL(startStopEvolution(bool)), Qt::DirectConnection);

	m_plotFitness = new QCustomPlot(ui->leftWidget);
	m_plotFitness->addGraph();
	ui->leftWidget->layout()->addWidget(m_plotFitness);
}

EvoCacheView::~EvoCacheView()
{
	delete ui;
}

void EvoCacheView::onUserInjected(User* pUser)
{
	ui->acPlot->loadCompressedAmount(pUser);
	ui->amPlot->loadAmount(pUser);

	connect(ui->acPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->amPlot->xAxis, SLOT(setRange(QCPRange)));
	connect(ui->acPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->amPlot, SLOT(replot()));

//	ui->acPlot->setPlottingHints(QCP::phFastPolylines | QCP::phCacheLabels);

	connect(pUser->botContext(), &BotContext::matchedTransaction, this, &EvoCacheView::plotMask);
	connect(pUser->botContext(), &BotContext::summarizingTree, this, &EvoCacheView::clearMasks, Qt::BlockingQueuedConnection);
	connect(pUser->botContext(), &BotContext::needsReplot, this, &EvoCacheView::replotCharts, Qt::BlockingQueuedConnection);
	connect(pUser->botContext(), &BotContext::needsReplot, this, &EvoCacheView::clearList);
	connect(pUser->botContext(), &BotContext::newSummarizedTree, this, &EvoCacheView::onNewSummarizedTree, Qt::BlockingQueuedConnection);
}

void EvoCacheView::clearMasks()
{
	ui->acPlot->clearItems();
}

void EvoCacheView::plotMask(double x, double y, int flag)
{
	QCPItemRect* itRect = new QCPItemRect(ui->acPlot);
	y = kindaLog(y);
	bool isTarget = flag == 0;
	itRect->topLeft->setCoords(QPointF(x - 4*3600*24, y + (10+6*isTarget)*0.01));
	itRect->bottomRight->setCoords(QPointF(x + 4*3600*24, y - (10+6*isTarget)*0.01));
	QColor colZone = flag == 0 ? QColor(239, 64, 53, 32) : QColor(0, 64, 253, 64);
	if (flag & 2)
		colZone = QColor(0, 253, 64, 32);
	if (flag & 4)
		colZone = QColor(255, 165, 0, 32);
	itRect->setPen(QPen(QBrush(colZone), 3.0));
	itRect->setBrush(QBrush(colZone));
	itRect->setClipToAxisRect(false);
	ui->acPlot->addItem(itRect);
}

void EvoCacheView::replotCharts()
{
	ui->acPlot->replot(QCustomPlot::rpQueued);
	//ui->amPlot->loadAmount(account);
	//	ui->amPlot->replot(QCustomPlot::rpQueued);
}

void EvoCacheView::clearList()
{
	ui->listBills->clear();
}

void EvoCacheView::onNewSummarizedTree(QJsonObject jsonObj)
{
	ui->listBills->clear();
	for (const auto f : jsonObj["features"].toArray()) {
		QJsonObject fobj = f.toObject();
		fobj.remove("args");
		ui->listBills->addItem(QString(QJsonDocument(fobj).toJson()));
	}
	static double i = 0;
	++i;
	m_plotFitness->graph(0)->addData(i, jsonObj["fit"].toDouble());
	m_plotFitness->rescaleAxes();
	m_plotFitness->replot();
}

