#include "extraCashView.h"
#include "ui_extraCashView.h"
#include "cacherest.h"
#include "../extraCash/extraCache.h"
#include "userMetrics.h"
#include "oracle.h"
#include "bot.h"

static const int numRevelations = 128;
static int alpha = 32;
static bool breakDown = false;
static int IND_GR_REVEL = -1;
static int IND_GR_BALANCE = -1;
static int IND_GR_SLOPE = -1;
static int IND_GR_PERCENTILE = -1;
static int IND_GR_AVG = -1;

double smallInc = 1e-3;

ExtraCashView::ExtraCashView(QString userID, QJsonObject jsonArgs) :
	QMainWindow(),
	ui(new Ui::ExtraCashView)
{
	ui->setupUi(this);
	setWindowTitle(QString("..")+userID.right(5));

	m_pExtraCache = new ExtraCache(userID, jsonArgs);
	m_pExtraCache->flags = CacheAccountConnector::None;
	if (jsonArgs["send2Bot"].toString() != "")
		m_pExtraCache->flags = CacheAccountConnector::SendExtraCash;
	m_pExtraCache->flags &= ~CacheAccountConnector::AutoExit;

	ui->plot->yAxis2->setVisible(true);
	ui->plot->yAxis->setSubTickCount(10);
	ui->plot->yAxis2->setRange(-100.0, 100);

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
	ui->spinDaysOld->setValue(Transaction::maxDaysOld());
	connect(ui->spinDaysOld, SIGNAL(valueChanged(int)), this, SLOT(onDaysOldSpin(int)));
	ui->spinAgo->setValue(Transaction::currentDay().daysTo(Transaction::actualCurrentDay()));
	connect(ui->spinAgo, SIGNAL(valueChanged(int)), this, SLOT(onAgo()));

	connect(m_pExtraCache, SIGNAL(injected(User*)), this, SLOT(onUserInjected(User*)));
	connect(m_pExtraCache, SIGNAL(botInjected(Bot*)), this, SLOT(onBotInjected(Bot*)));
}

ExtraCashView::~ExtraCashView()
{
	delete ui;
}

void ExtraCashView::onUserInjected(User* pUser)
{
	connect(ui->spinHypotheTrans, SIGNAL(valueChanged(int)), pUser, SLOT(setHypotheTrans(int)));
	connect(ui->spinHypotheTrans, SIGNAL(valueChanged(int)), pUser, SLOT(reInjectBot()));
}

void ExtraCashView::onBotInjected(Bot* pBot)
{
	Q_UNUSED(pBot);
	NOTICE() << "ExtraCashView::onBotInjected";
	m_pbDate = Transaction::currentDay();
	m_realBalance = BalanceMetric::get(m_pExtraCache->user())->value(Transaction::actualCurrentDay());
	m_pbBalance = BalanceMetric::get(m_pExtraCache->user())->value(m_pbDate);

	HistoMetric::clearAll();
	double d2z50 = Montecarlo<128>::get(m_pExtraCache->user())->value(Transaction::currentDay());
	ui->spinT2z50->setValue(d2z50);
	double d2z80 = Montecarlo<128>::get(m_pExtraCache->user())->d2zPerc(Transaction::currentDay(), 0.80);
	ui->spinT2z80->setValue(d2z80);
	double d2z20 = Montecarlo<128>::get(m_pExtraCache->user())->d2zPerc(Transaction::currentDay(), 0.20);
	ui->spinT2z20->setValue(d2z20);

	double valMin50 = 0.0;
	double valMin20 = 0.0;
	double valMin80 = 0.0;
	double d2M50 = Montecarlo<128>::get(m_pExtraCache->user())->d2MinPerc(Transaction::currentDay(), 0.50, &valMin50);
	double d2M20 = Montecarlo<128>::get(m_pExtraCache->user())->d2MinPerc(Transaction::currentDay(), 0.20, &valMin20);
	double d2M80 = Montecarlo<128>::get(m_pExtraCache->user())->d2MinPerc(Transaction::currentDay(), 0.80, &valMin80);
	ui->spinT2min50->setValue(d2M50);
	ui->spinvalMin50->setValue(valMin50);
	ui->spinT2min20->setValue(d2M20);
	ui->spinvalMin20->setValue(valMin20);
	ui->spinT2min80->setValue(d2M80);
	ui->spinvalMin80->setValue(valMin80);

	ui->costLive50SpinBox->setValue(CostRateMonthPercentileMetric<6, 50>::get(m_pExtraCache->user())->value(m_pbDate));
	ui->costLive75SpinBox->setValue(CostRateMonthPercentileMetric<6, 75>::get(m_pExtraCache->user())->value(m_pbDate));
	ui->costLive90SpinBox->setValue(CostRateMonthPercentileMetric<6, 90>::get(m_pExtraCache->user())->value(m_pbDate));
	ui->costLive95SpinBox->setValue(CostRateMonthPercentileMetric<6, 95>::get(m_pExtraCache->user())->value(m_pbDate));
	ui->costLive99SpinBox->setValue(CostRateMonthPercentileMetric<6, 99>::get(m_pExtraCache->user())->value(m_pbDate));

	NOTICE() << "initial pb balance"<< m_pbBalance << " at" << m_pbDate.toString();

	updateChart();
}

void ExtraCashView::onDaysOldSpin(int val)
{
	Transaction::setMaxDaysOld(val);
	m_pExtraCache->user()->reInjectBot();
}

void ExtraCashView::onAgo()
{
	Transaction::setCurrentDay(Transaction::actualCurrentDay().addDays(-ui->spinAgo->value()));
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
	ui->spinAgo->setValue(Transaction::currentDay().daysTo(Transaction::actualCurrentDay()));
//	m_pExtraCache->user()->reInjectBot();
}

void ExtraCashView::updateChart()
{
	double daysAgo = Transaction::currentDay().daysTo(Transaction::actualCurrentDay());

	makeBalancePlot();
	makeRevelationPlot();
	makePercentilePlot(0.5);
//	makeMinSlope();

	ui->plot->xAxis->setRange(-displayDayPast, displayDayFuture + 1);
	static double maxY = 0.0;
	static bool once = true;
	if (once)
		ui->plot->yAxis->rescale();
	once = false;
	maxY = qMax(maxY, ui->plot->yAxis->range().upper);
	ui->plot->yAxis->setRange(qMin(ui->plot->yAxis->range().lower, -100.0), maxY);

	SuperOracle::Summary summary = m_pExtraCache->user()->oracle()->computeAvgCashFlow();
	double perCentFlow = summary.flow() * 100.0;
	ui->spinAvgCashFlow->setValue(perCentFlow);
	ui->dailyBillSpin->setValue(summary.bill);
	ui->dailyNegSpin->setValue(summary.negSum);
	ui->dailyPosSpin->setValue(summary.posSum);
	ui->dailySalSpin->setValue(summary.salary);

	ui->freq->setValue(summary.weekDetails["_dailyFrequent"].toDouble());
	ui->infreq->setValue(summary.weekDetails["_dailyInfrequent"].toDouble());
	ui->thresh->setValue(summary.weekDetails["_threshAmountFrequent"].toDouble());

	pBars->addData(-daysAgo, perCentFlow);
//	ui->plot->yAxis2->rescale();
//	double minY1 = ui->plot->yAxis->range().lower;
//	double maxY1 = ui->plot->yAxis->range().upper;
//	ui->plot->yAxis2->setRange(100.0*qMin(-1.0, minY1/maxY1), 100.0*qMax(1.0, -maxY1/minY1));
	ui->plot->xAxis2->setRange(-daysAgo - displayDayPast, -daysAgo + displayDayFuture + 1);

	ui->plot->replot();
}

void ExtraCashView::makeBalancePlot()
{
	double x = Transaction::currentDay().daysTo(Transaction::actualCurrentDay());
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
		NOTICE() << "makeRevelationPlot balance = " << curBal;
		pGr->clearData();
		double t = -0.01; // to be the first point, slightly on the left
		pGr->addData(t, curBal);
		m_pExtraCache->user()->oracle()->resetDate(m_pbDate);
		const QVector<Transaction>& rev = m_pExtraCache->user()->oracle()->revelation(m_pbDate.addDays(displayDayFuture));
		double epsilon = 0.0000001;
		double manyEspilon = epsilon;
		for (const Transaction& tr : rev) {
			double amnt = tr.amountDbl();
			curBal += amnt;
			t = Transaction::currentDay().daysTo(tr.date) + manyEspilon;
			pGr->addData(t, curBal);
			NOTICE() << "t " << t << " " << amnt << " -> bal = " << curBal;
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
	double curBal = m_pbBalance;
	double t = -0.01; // to be the first point, slightly on the left
	pGrPerc->addData(t, curBal);
	pGrAvg->addData(t, curBal);
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


