#include "evoCacheView.h"
#include "ui_evoCacheView.h"
#include "user.h"
#include "evolver.h"
#include "botContext.h"

EvoCacheView::EvoCacheView(QString userID, QJsonObject jsonArgs)
	: QMainWindow()
	, ui(new Ui::EvoCacheView)
{
	ui->setupUi(this);
	ui->amPlot->hide();
	setWindowTitle(QString("..")+userID.right(5));

	Evolver* pEvolver = new Evolver(userID, jsonArgs);
	if (jsonArgs["notSendBot"].toString() != "")
		pEvolver->flags &= ~CacheAccountConnector::SendBot;

	connect(pEvolver, SIGNAL(injected(User*)), this, SLOT(onUserInjected(User*)));
	connect(ui->startButton, SIGNAL(clicked(bool)), pEvolver, SIGNAL(startStopEvolution(bool)), Qt::DirectConnection);

	m_plotFitness = new QCustomPlot(ui->leftWidget);
	m_plotFitness->axisRect()->setupFullAxesBox();
	m_plotFitness->addGraph();
	m_plotFitness->addGraph(m_plotFitness->xAxis, m_plotFitness->yAxis2);
	m_plotFitness->graph(1)->setPen(QPen(Qt::red));
	m_plotFitness->yAxis2->setTickLabels(true);
	ui->leftWidget->layout()->addWidget(m_plotFitness);
}

EvoCacheView::~EvoCacheView()
{
	delete ui;
}

void EvoCacheView::onUserInjected(User* pUser)
{
	NOTICE() << "EvoCacheView::onUserInjected";
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
	connect(pUser->botContext(), &BotContext::computedGeneration, ui->spinGen, &QSpinBox::setValue);
}

void EvoCacheView::clearMasks()
{
	ui->acPlot->clearItems();
}

void EvoCacheView::plotMask(double x, double y, int flag)
{
	Q_ASSERT(x > -1e9 && x < 10e9 && y > -1e9 && y < 1e9);
	QCPItemRect* itRect = new QCPItemRect(ui->acPlot);
	y = kindaLog(y);
	bool isTarget = flag == 0;
	itRect->topLeft->setCoords(QPointF(x - 3600*24, y + (10+6*isTarget)*0.01));
	itRect->bottomRight->setCoords(QPointF(x + 3600*24, y - (10+6*isTarget)*0.01));
	QColor colZone = flag == 0 ? QColor(239, 64, 53, 32) : QColor(0, 64, 253, 64);
	if (flag & 2)
		colZone = QColor(0, 253, 64, 32);
	if (flag & 4)
		colZone = QColor(255, 165, 0, 32);
	if (flag & 8)
		colZone = QColor(255, 69, 0, 64);
	if (flag & 16)
		colZone = QColor(0, 165, 0, 64);
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
	// sort per tot amount
	QList<QJsonObject> listObj;
	for (auto f : jsonObj["features"].toArray()) {
		listObj.append(f.toObject());
		listObj.last().remove("args");
	}
	std::sort(listObj.begin(), listObj.end(), [](const QJsonObject& a, const QJsonObject& b){
		return qAbs(a["_total"].toDouble()) > qAbs(b["_total"].toDouble());
	});
	for (const auto f : listObj) {
		ui->listBills->addItem(QString(QJsonDocument(f).toJson()));
	}
	static double i = 0;
	++i;
	m_plotFitness->graph(0)->addData(i, jsonObj["fit"].toDouble());
	m_plotFitness->graph(1)->addData(i, jsonObj["flow"].toDouble());
	m_plotFitness->rescaleAxes();
	m_plotFitness->replot();
}

