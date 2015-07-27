#include "extraCashView.h"
#include "ui_extraCashView.h"
#include "cacherest.h"
#include "../extraCash/extraCache.h"
#include "userMetrics.h"
#include "oracle.h"
#include "bot.h"

static const int numRevelations = 10;
static int IND_GR_REVEL = -1;
static int IND_GR_BALANCE = -1;
static int IND_GR_SLOPE = -1;

const int displayDayPast = 60;
const int displayDayFuture = 60;

const int playBackStartAgo = 60;

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
		ui->plot->graph(tempInd)->setPen(QPen((QColor(255, 0, 0, 64)), 3.0));
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

	connect(m_pExtraCache, SIGNAL(botInjected(Bot*)), this, SLOT(onBotInjected(Bot*)));
}

ExtraCashView::~ExtraCashView()
{
	delete ui;
}

void ExtraCashView::onBotInjected(Bot* pBot)
{
	ui->costLive50SpinBox->setValue(CostRateMonthPercentileMetric<6, 50>::get(m_pExtraCache->user())->value(m_pbDate));
	ui->costLive75SpinBox->setValue(CostRateMonthPercentileMetric<6, 75>::get(m_pExtraCache->user())->value(m_pbDate));
	ui->costLive90SpinBox->setValue(CostRateMonthPercentileMetric<6, 90>::get(m_pExtraCache->user())->value(m_pbDate));
	ui->costLive95SpinBox->setValue(CostRateMonthPercentileMetric<6, 95>::get(m_pExtraCache->user())->value(m_pbDate));
	ui->costLive99SpinBox->setValue(CostRateMonthPercentileMetric<6, 99>::get(m_pExtraCache->user())->value(m_pbDate));

	m_pbDate = QDate::currentDate().addDays(-playBackStartAgo);
	m_pbBalance = m_pExtraCache->user()->balance(Account::Type::Checking);

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
	makeMinSlope();

	ui->plot->xAxis->setRange(x - displayDayPast, x + displayDayFuture + 1);
	ui->plot->yAxis->rescale();
	ui->plot->yAxis->setRange(-100, ui->plot->yAxis->range().upper + 100);
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
		pGr->clearData();
		double t = QDate::currentDate().daysTo(m_pbDate);
		pGr->addData(t, curBal);
		m_pExtraCache->user()->oracle()->resetDate(m_pbDate);
		QVector<Transaction> rev = m_pExtraCache->user()->oracle()->revelation(m_pbDate.addDays(displayDayFuture));
		LOG() << "makeRevelationPlot" << i << curBal;
		for (Transaction& tr : rev) {
			curBal += tr.amountDbl();
			t = QDate::currentDate().daysTo(tr.date);
			pGr->addData(t, curBal);
			LOG() << t << curBal;
		}
	}

	if(false) {//minPredict <= m_pExtraCache->slushBaseStart()) {
		ui->plot->setBackground(QBrush(Qt::red));
	}
	else {
		int greenVal = 255;//qMin(4*255.0, minPredict - m_pExtraCache->slushBaseStart()) / 4;
		ui->plot->setBackground(QBrush(QColor(255 - greenVal, greenVal, 0)));
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

