#include "extraCashView.h"
#include "ui_extraCashView.h"
#include "cacherest.h"
#include "../extraCash/extraCache.h"
#include "userMetrics.h"
#include "oracle.h"
#include "bot.h"

static const int numRevelations = 64;
static int alpha = 32;
static bool breakDown = false;
static int IND_GR_REVEL = -1;
static int IND_GR_BALANCE = -1;
static int IND_GR_SLOPE = -1;
static int IND_GR_PERCENTILE = -1;
static int IND_GR_AVG = -1;

const int displayDayPast = 31;//80;//60;
const int displayDayFuture = 62;//180;

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

	++tempInd;
	ui->plot->addGraph();
	ui->plot->graph(tempInd)->setPen(QPen((QColor(0, 0, 0, 64)), 5.0));
	ui->plot->graph(tempInd)->setLineStyle(QCPGraph::lsStepLeft);
	IND_GR_AVG = tempInd;

	++tempInd;
	ui->plot->addGraph();
	ui->plot->graph(tempInd)->setPen(QPen((QColor(0, 255, 0, 196)), 5.0));
	ui->plot->graph(tempInd)->setLineStyle(QCPGraph::lsStepLeft);
	IND_GR_PERCENTILE = tempInd;

	pBars = new QCPBars(ui->plot->xAxis2, ui->plot->yAxis2);
	ui->plot->addPlottable(pBars);
	pBars->setName("Flow");
	pBars->setPen(QColor(255, 131, 0));
	pBars->setBrush(QColor(255, 131, 0, 50));

	connect(ui->plot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(onWheelEvent(QWheelEvent*)));
	connect(ui->spinHypotheTrans, SIGNAL(valueChanged(int)), this, SLOT(onHypotheTrans(int)));
	ui->spinDaysOld->setValue(Transaction::maxDaysOld());
	connect(ui->spinDaysOld, SIGNAL(valueChanged(int)), this, SLOT(onDaysOldSpin(int)));
	ui->spinAgo->setValue(Transaction::currentDay().daysTo(QDate::currentDate()));
	connect(ui->spinAgo, SIGNAL(valueChanged(int)), this, SLOT(onAgo()));

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
	m_pbDate = Transaction::currentDay();
	m_realBalance = m_pExtraCache->user()->balance(Account::Type::Checking);
	m_pbBalance = m_realBalance;

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
		if (real.trans(i).date > m_pbDate) {
			// incrementally finds out the balance at the playback date
			if (!real.trans(i).isInternal()) {
				m_pbBalance -= real.trans(i).amountDbl();
			}
		}
	}
	LOG() << "initial pb balance"<< m_pbBalance << " at" << m_pbDate.toString() << endl;

	ui->spinBox->setValue(m_pbBalance);
//	ui->plot->setFocus();
	updateChart();
}

void ExtraCashView::onHypotheTrans(int transAmount)
{
	m_pExtraCache->user()->setHypotheTrans(transAmount);
//	m_pbBalance = m_pExtraCache->user()->balance(Account::Type::Checking);
	ui->spinBox->setValue(m_pbBalance);
	updateChart();
}

void ExtraCashView::onDaysOldSpin(int val)
{
	Transaction::setMaxDaysOld(val);
	m_pExtraCache->user()->reInjectBot();
}

void ExtraCashView::onAgo()
{
	Transaction::setCurrentDay(QDate::currentDate().addDays(-ui->spinAgo->value()));
	m_pExtraCache->user()->reInjectBot();
}

void ExtraCashView::keyPressEvent(QKeyEvent *event)
{
	Q_UNUSED(event);
	if (event->key() == Qt::Key_Space)
		updateChart();
}

void ExtraCashView::onWheelEvent(QWheelEvent * wEv)
{

//	m_lastBal = ui->spinBox->value();
	int step = 1;
	if(wEv->delta() > 0) {
		Transaction::setCurrentDay(Transaction::currentDay().addDays(step));
	}
	else {
		Transaction::setCurrentDay(Transaction::currentDay().addDays(-step));
	}
	ui->spinAgo->setValue(Transaction::currentDay().daysTo(QDate::currentDate()));
//	m_pExtraCache->user()->reInjectBot();
}

void ExtraCashView::updateChart()
{
	double daysAgo = Transaction::currentDay().daysTo(QDate::currentDate());

	makeBalancePlot();
	makeRevelationPlot();
	makePercentilePlot(0.5);
//	makeMinSlope();

	ui->plot->xAxis->setRange(-displayDayPast, displayDayFuture + 1);
	static double maxY = 0.0;
	ui->plot->yAxis->rescale();
	maxY = qMax(maxY, ui->plot->yAxis->range().upper + 100);
	ui->plot->yAxis->setRange(qMin(ui->plot->yAxis->range().lower, -100.0), maxY);

	double perCentFlow = 100.0 * m_pExtraCache->user()->oracle()->avgCashFlow();
	ui->spinAvgCashFlow->setValue(perCentFlow);
	pBars->addData(-daysAgo, perCentFlow);
	ui->plot->yAxis2->rescale();
	ui->plot->xAxis2->setRange(-daysAgo - displayDayPast, -daysAgo + displayDayFuture + 1);

	ui->plot->replot();
}

void ExtraCashView::makeBalancePlot()
{
	double x = Transaction::currentDay().daysTo(QDate::currentDate());
	ui->plot->graph(IND_GR_BALANCE)->clearData();
	ui->plot->graph(IND_GR_BALANCE)->addData(x, m_realBalance);
	ui->spinBox->setValue(m_pbBalance);
	double balanceThen = m_realBalance;
	for (int i = m_pExtraCache->user()->allTrans().count() - 1; i >= 0; --i) {
		const Transaction& tr = m_pExtraCache->user()->allTrans().trans(i);
		if (tr.isInternal())
			continue;
		double x = Transaction::currentDay().daysTo(tr.date);
		ui->plot->graph(IND_GR_BALANCE)->addData(x, balanceThen);
		balanceThen -= tr.amountDbl();
	}
	ui->plot->graph(IND_GR_BALANCE)->addData(-9999, balanceThen);
}

void ExtraCashView::makeRevelationPlot()
{
	for (int i = IND_GR_REVEL; i < IND_GR_REVEL + numRevelations; ++i) {
		QCPGraph* pGr = ui->plot->graph(i);
		double curBal = m_pbBalance;
		LOG() << "makeRevelationPlot balance = " << curBal << endl;
		pGr->clearData();
		double t = -0.01; // to be the first point, slightly on the left
		pGr->addData(t, curBal);
		m_pExtraCache->user()->oracle()->resetDate(m_pbDate);
		const QVector<Transaction> rev = m_pExtraCache->user()->oracle()->revelation(m_pbDate.addDays(displayDayFuture));
		double epsilon = 0.0000001;
		double manyEspilon = epsilon;
		for (const Transaction& tr : rev) {
			double amnt = tr.amountDbl();
			curBal += amnt;
			t = Transaction::currentDay().daysTo(tr.date) + manyEspilon;
			pGr->addData(t, curBal);
//			LOG() << amnt << " -> bal = " << curBal << endl;
			manyEspilon += epsilon;
		}
		if (!breakDown)
			continue;
		pGr = ui->plot->graph(i + numRevelations);
		curBal = m_pbBalance;
		pGr->clearData();
		t = -0.01; // to be the first point, slightly on the left
		pGr->addData(t, curBal);
		for (const Transaction& tr : rev) {
			double amnt = tr.amountDbl();
			if(amnt < 0)
				continue;
			curBal += amnt;
			t = Transaction::currentDay().daysTo(tr.date) + manyEspilon;
			pGr->addData(t, curBal);
			manyEspilon += epsilon;
		}
		pGr = ui->plot->graph(i + numRevelations + numRevelations);
		curBal = m_pbBalance;
		pGr->clearData();
		t = -0.01; // to be the first point, slightly on the left
		pGr->addData(t, curBal);
		for (const Transaction& tr : rev) {
			double amnt = tr.amountDbl();
			if(amnt > 0)
				continue;
			curBal += amnt;
			t = Transaction::currentDay().daysTo(tr.date) + manyEspilon;
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

void ExtraCashView::makePercentilePlot(double fracPerc)
{
	QCPGraph* pGrPerc = ui->plot->graph(IND_GR_PERCENTILE);
	QCPGraph* pGrAvg = ui->plot->graph(IND_GR_AVG);
	pGrPerc->clearData();
	pGrAvg->clearData();
	for (double d = 0; d < displayDayFuture; ++d) {
		QVector<double> allY;
		for (int i = IND_GR_REVEL; i < IND_GR_REVEL + numRevelations; ++i) {
			QCPDataMap* pDat = ui->plot->graph(i)->data();
			auto it = pDat->upperBound(d + 0.1) - 1;
			if (it+1 != pDat->begin() && it != pDat->end()) {
				double vi = it->value;
				allY.append(vi);
			}
			else
				allY.append(0.0);
		}
		qSort(allY);
		if (allY.count()) {
			double vAtPerc = allY[qRound((allY.count() - 1) * fracPerc)];
			pGrPerc->addData(d, vAtPerc);
			double sum = 0.0;
			for (auto v : allY)
				sum += v;
			pGrAvg->addData(d, sum / allY.count());
		}
	}
}


