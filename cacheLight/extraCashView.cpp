#include "extraCashView.h"
#include "ui_extraCashView.h"
#include "cacherest.h"
#include "../extraCash/extraCache.h"
#include "userMetrics.h"
#include "oracle.h"
#include "bot.h"

static const int numRevelations = 32;
static int alpha = 32;
static int IND_GR_REVEL = -1;
static int IND_GR_BALANCE = -1;
static int IND_GR_SLOPE = -1;

const int displayDayPast = 1;//60;
const int displayDayFuture = 62;//180;

const int playBackStartAgo = 0;

double smallInc = 1e-3;

ExtraCashView::ExtraCashView(QString userID, QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::ExtraCashView)
{
	ui->setupUi(this);

	m_pExtraCache = new ExtraCache(userID);
	m_pExtraCache->flags = CacheAccountConnector::None;

//	ui->plot->xAxis->setVisible(false);
//	ui->plot->yAxis->setVisible(false);
	ui->plot->yAxis2->setVisible(true);
	ui->plot->yAxis->setSubTickCount(10);

	int tempInd = -1;

	++tempInd;
	ui->plot->addGraph();
	ui->plot->graph(tempInd)->setLineStyle(QCPGraph::lsStepLeft);
	IND_GR_BALANCE = tempInd;

	++tempInd;
	ui->plot->addGraph();
	ui->plot->graph(tempInd)->setLineStyle(QCPGraph::lsStepLeft);
	IND_GR_SLOPE = tempInd;

	for (int i = 0; i < numRevelations; ++i) {
		++tempInd;
		ui->plot->addGraph();
		ui->plot->graph(tempInd)->setPen(QPen((QColor(255, 0, 255, alpha)), 3.0));
		ui->plot->graph(tempInd)->setLineStyle(QCPGraph::lsStepLeft);
		ui->plot->graph(tempInd)->addData(-9999, 0);
		ui->plot->graph(tempInd)->addData(9999, 0);
		if (IND_GR_REVEL < 0)
			IND_GR_REVEL = tempInd;
	}
	for (int i = 0; i < numRevelations; ++i) {
		++tempInd;
		ui->plot->addGraph();
		ui->plot->graph(tempInd)->setPen(QPen((QColor(0, 0, 255, alpha)), 3.0));
		ui->plot->graph(tempInd)->setLineStyle(QCPGraph::lsStepLeft);
		ui->plot->graph(tempInd)->addData(-9999, 0);
		ui->plot->graph(tempInd)->addData(9999, 0);
		if (IND_GR_REVEL < 0)
			IND_GR_REVEL = tempInd;
	}
	for (int i = 0; i < numRevelations; ++i) {
		++tempInd;
		ui->plot->addGraph();
		ui->plot->graph(tempInd)->setPen(QPen((QColor(255, 0, 0, alpha)), 3.0));
		ui->plot->graph(tempInd)->setLineStyle(QCPGraph::lsStepLeft);
		ui->plot->graph(tempInd)->addData(-9999, 0);
		ui->plot->graph(tempInd)->addData(9999, 0);
		if (IND_GR_REVEL < 0)
			IND_GR_REVEL = tempInd;
	}

	pBars = new QCPBars(ui->plot->xAxis, ui->plot->yAxis2);
	ui->plot->addPlottable(pBars);
	pBars->setName("Extra");
	pBars->setPen(QColor(255, 131, 0));
	pBars->setBrush(QColor(255, 131, 0, 50));

	connect(ui->plot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(onWheelEvent(QWheelEvent*)));
	connect(ui->spinHypotheTrans, SIGNAL(valueChanged(int)), this, SLOT(onHypotheTrans(int)));

	connect(m_pExtraCache, SIGNAL(botInjected(Bot*)), this, SLOT(onBotInjected(Bot*)));
}

ExtraCashView::~ExtraCashView()
{
	delete ui;
}

void ExtraCashView::onBotInjected(Bot* pBot)
{
	Q_UNUSED(pBot);
	LOG() << "ExtraCashView::onBotInjected" << endl;
	m_pbDate = QDate::currentDate().addDays(-playBackStartAgo);
	m_pbBalance = m_pExtraCache->user()->balance(Account::Type::Checking);

	ui->costLive50SpinBox->setValue(CostRateMonthPercentileMetric<6, 50>::get(m_pExtraCache->user())->value(m_pbDate));
	ui->costLive75SpinBox->setValue(CostRateMonthPercentileMetric<6, 75>::get(m_pExtraCache->user())->value(m_pbDate));
	ui->costLive90SpinBox->setValue(CostRateMonthPercentileMetric<6, 90>::get(m_pExtraCache->user())->value(m_pbDate));
	ui->costLive95SpinBox->setValue(CostRateMonthPercentileMetric<6, 95>::get(m_pExtraCache->user())->value(m_pbDate));
	ui->costLive99SpinBox->setValue(CostRateMonthPercentileMetric<6, 99>::get(m_pExtraCache->user())->value(m_pbDate));

	// transaction at the starting date of the playback
	auto& real = m_pExtraCache->user()->allTrans();
	qDebug() << "m_date" << m_pbDate;

	for (int i = 0; i < real.count(); ++i) {
		// finds the index of the last transaction within the playback date
		if (real.trans(i).date <= m_pbDate) {
			m_ipb = i;
		}
		else {
			// incrementally finds out the balance at the playback date
			if (!real.trans(i).isInternal()) {
				m_pbBalance -= real.trans(i).amountDbl();
			}
		}
	}
	LOG() << "initial pb balance"<< m_pbBalance << " at" << m_pbDate.toString() << endl;
	LOG() << "initial pb trans("<< m_ipb <<")" << real.trans(m_ipb).name << endl;

	ui->spinBox->setValue(m_pbBalance);
	ui->spinBox->editingFinished();
	ui->plot->setFocus();
	updateChart();
}

void ExtraCashView::onHypotheTrans(int transAmount)
{
	m_pExtraCache->user()->setHypotheTrans(transAmount);
//	m_pbBalance = m_pExtraCache->user()->balance(Account::Type::Checking);
	ui->spinBox->setValue(m_pbBalance);
	updateChart();
}

void ExtraCashView::keyPressEvent(QKeyEvent *event)
{
	Q_UNUSED(event);
	updateChart();
}

void ExtraCashView::onWheelEvent(QWheelEvent * wEv)
{

//	m_lastBal = ui->spinBox->value();
	//int step = 200 * qrand() / RAND_MAX;
	if(wEv->delta() > 0) {
		ui->spinBox->setValue(m_pbBalance);
		m_pbDate = m_pbDate.addDays(1);
	}
	else {
		keyPressEvent(0);
	}
	updateChart();
}

void ExtraCashView::updateChart()
{
	double x = QDate::currentDate().daysTo(m_pbDate);

	makeBalancePlot();
	makeRevelationPlot();
	makePastiPlot();
//	makeMinSlope();

	ui->plot->xAxis->setRange(x - displayDayPast, x + displayDayFuture + 1);
	static double maxY = 0.0;
	ui->plot->yAxis->rescale();
	maxY = qMax(maxY, ui->plot->yAxis->range().upper + 100);
	ui->plot->yAxis->setRange(qMin(ui->plot->yAxis->range().lower, -100.0), maxY);
	ui->plot->replot();
}

void ExtraCashView::makeBalancePlot()
{
	double x = QDate::currentDate().daysTo(m_pbDate);
	ui->plot->graph(IND_GR_BALANCE)->addData(x, m_pbBalance);
	ui->spinBox->setValue(m_pbBalance);
}

void ExtraCashView::makeRevelationPlot()
{
	for (int i = IND_GR_REVEL; i < IND_GR_REVEL + numRevelations; ++i) {
		QCPGraph* pGr = ui->plot->graph(i);
		double curBal = m_pbBalance;
		LOG() << "makeRevelationPlot balance = " << curBal << endl;
		pGr->clearData();
		double t = QDate::currentDate().daysTo(m_pbDate) - 0.01; // to be the first point, slightly on the left
		pGr->addData(t, curBal);
		m_pExtraCache->user()->oracle()->resetDate(m_pbDate);
		const QVector<Transaction> rev = m_pExtraCache->user()->oracle()->revelation(m_pbDate.addDays(displayDayFuture));
		double epsilon = 0.0000001;
		double manyEspilon = epsilon;
		for (const Transaction& tr : rev) {
			double amnt = tr.amountDbl();
			curBal += amnt;
			t = QDate::currentDate().daysTo(tr.date) + manyEspilon;
			pGr->addData(t, curBal);
			LOG() << "-> bal = " << curBal << endl;
			manyEspilon += epsilon;
		}
		pGr = ui->plot->graph(i + numRevelations);
		curBal = m_pbBalance;
		pGr->clearData();
		t = QDate::currentDate().daysTo(m_pbDate) - 0.01; // to be the first point, slightly on the left
		pGr->addData(t, curBal);
		for (const Transaction& tr : rev) {
			double amnt = tr.amountDbl();
			if(amnt < 0)
				continue;
			curBal += amnt;
			t = QDate::currentDate().daysTo(tr.date) + manyEspilon;
			pGr->addData(t, curBal);
			manyEspilon += epsilon;
		}
		pGr = ui->plot->graph(i + numRevelations + numRevelations);
		curBal = m_pbBalance;
		pGr->clearData();
		t = QDate::currentDate().daysTo(m_pbDate) - 0.01; // to be the first point, slightly on the left
		pGr->addData(t, curBal);
		for (const Transaction& tr : rev) {
			double amnt = tr.amountDbl();
			if(amnt > 0)
				continue;
			curBal += amnt;
			t = QDate::currentDate().daysTo(tr.date) + manyEspilon;
			pGr->addData(t, curBal);
			manyEspilon += epsilon;
		}
	}

	if(false) {//minPredict <= m_pExtraCache->slushBaseStart()) {
		ui->plot->setBackground(QBrush(Qt::red));
	}
	else {
		int greenVal = 32;//qMin(4*255.0, minPredict - m_pExtraCache->slushBaseStart()) / 4;
		ui->plot->setBackground(QBrush(QColor(255 - greenVal, 255-32+greenVal, 255-32)));
	}
}

void ExtraCashView::makePastiPlot()
{
}

void ExtraCashView::makeMinSlope()
{
	QCPGraph* graph = ui->plot->graph(IND_GR_SLOPE);
	graph->clearData();
	double tToday = QDate::currentDate().daysTo(m_pbDate);
	double slushBase = m_pExtraCache->slushBaseStart();
	if(graph->data()->isEmpty()) {
		graph->addData(tToday, slushBase);
	}
	double dayMin = m_pExtraCache->futDayMinSlope() - tToday;
	double minSlope = qMax(0.0, m_pExtraCache->minSlope());
	double effectiveSlushforDay = m_pExtraCache->slushNeed() * (0.5 + 1.0 * (dayMin) / 30.0);
	graph->addData(tToday + displayDayFuture, slushBase + minSlope * displayDayFuture);
	graph->addData(tToday + dayMin, slushBase + minSlope * dayMin);
	graph->addData(tToday + dayMin + 0.001, slushBase + minSlope * dayMin + effectiveSlushforDay);
	graph->addData(tToday + dayMin + 0.002, slushBase + minSlope * dayMin);
	graph->addData(tToday + displayDayFuture + 0.003, slushBase + minSlope * displayDayFuture);
	qDebug() << tToday << displayDayFuture << dayMin << minSlope;
}

