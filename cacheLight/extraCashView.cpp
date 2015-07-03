#include "extraCashView.h"
#include "ui_extraCashView.h"
#include "cacherest.h"
#include "../extraCash/extraCache.h"
#include "userMetrics.h"

const int dayPast = 60;
const int dayFuture = 60;
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

	ui->plot->addGraph();
	ui->plot->graph(0)->setPen(QPen(QBrush(Qt::magenta), 5.0));
	ui->plot->graph(0)->addData(-9999, 0);
	ui->plot->graph(0)->addData(9999, 0);

	ui->plot->addGraph();
	ui->plot->graph(1)->setLineStyle(QCPGraph::lsStepLeft);

	ui->plot->addGraph();
	ui->plot->graph(2)->setLineStyle(QCPGraph::lsStepLeft);
	ui->plot->graph(2)->setPen(QPen(QBrush(Qt::gray), 5.0));
	ui->plot->addGraph();
	ui->plot->graph(3)->setLineStyle(QCPGraph::lsStepLeft);
	ui->plot->graph(3)->setPen(QPen(QBrush(Qt::lightGray), 5.0));

	ui->plot->addGraph();
	ui->plot->graph(4)->setPen(QPen((QColor(255, 0, 0, 128)), 5.0));

	pBars = new QCPBars(ui->plot->xAxis, ui->plot->yAxis2);
	ui->plot->addPlottable(pBars);
	pBars->setName("Extra");
	pBars->setPen(QColor(255, 131, 0));
	pBars->setBrush(QColor(255, 131, 0, 50));

	connect(ui->plot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(onWheelEvent(QWheelEvent*)));

//	init();
	connect(m_pExtraCache, SIGNAL(botInjected(Bot*)), this, SLOT(init()));
}

ExtraCashView::~ExtraCashView()
{
	delete ui;
}

void ExtraCashView::init()
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
	m_pbDate = real.trans(m_ipb).date;
	m_d0 = m_pbDate.toJulianDay();
	LOG() << "initial pb balance"<< m_pbBalance << " at" << m_pbDate.toString() << endl;
	LOG() << "initial pb trans("<< m_ipb <<")" << real.trans(m_ipb).name << endl;

	ui->spinBox->setValue(m_pbBalance);
	ui->spinBox->editingFinished();
	ui->plot->setFocus();
	updateChart();
}

void ExtraCashView::keyPressEvent(QKeyEvent *event)
{
	int maxDayMove = 1;
	int minDayMove = 0;
	if(event && event->key() == Qt::Key_Up)
		minDayMove = 1;
	auto& real = m_pExtraCache->user()->allTrans();
	int addDay = 1;
	if(m_ipb < real.count()) {
		Transaction& newTrans = real.trans(m_ipb);
		// if new date, move forward
		addDay = newTrans.jDay() - m_pbDate.toJulianDay();
		qDebug() << newTrans.amountDbl() << newTrans.name << newTrans.date;
		if (addDay > maxDayMove) {
			// revert the soon to come increment so that we add a day and come back to this trans
			--m_ipb;
			addDay = maxDayMove;
		}
		else {
			double delta = newTrans.amountDbl();
			m_pbBalance += delta;
			ui->spinBox->setValue(m_pbBalance);
		}
		++m_ipb;
	}
	if (addDay < minDayMove) {
		return keyPressEvent(event);
	}
	m_pbDate = m_pbDate.addDays(addDay);
	double posSlope = qMax(0.0, m_pExtraCache->minSlope());
	double extraToday = posSlope * addDay / 2.0;
	if(m_extraToday < 0.0)
		m_extraToday = extraToday;
	if(extraToday > m_extraToday) {
		m_extraToday *= 0.95;
		m_extraToday += 0.05 * extraToday;
	}
	else {
		m_extraToday *= 0.9;
		m_extraToday += 0.1 * extraToday;
	}
//	m_slushThreshold += m_extraToday;
	ui->extraTodaySpinBox->setValue(m_extraToday);
	ui->spinSlushThresh->setValue(m_pExtraCache->slushBaseStart());
	pBars->addData(m_pbDate.toJulianDay() - m_d0, m_extraToday);
	ui->plot->yAxis2->rescale();

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
	double t = m_pbDate.toJulianDay() - m_d0;
	if(!ui->plot->graph(1)->data()->isEmpty()) {
		QCPData d1 = ui->plot->graph(1)->data()->last();
		if (t <= d1.key)
			t = d1.key + smallInc;
	}
	ui->plot->graph(1)->addData(t, m_pbBalance);
	ui->spinBox->setValue(m_pbBalance);

	makePredictiPlot(t);
	makePastiPlot();
	makeMinSlope();

	ui->plot->xAxis->setRange(t - dayPast, t + dayFuture + 1);
	ui->plot->yAxis->rescale();
	ui->plot->yAxis->setRange(-100, ui->plot->yAxis->range().upper + 100);
	ui->plot->replot();
}

void ExtraCashView::makePredictiPlot(double t)
{
	ui->plot->graph(2)->clearData();
	ui->plot->graph(2)->addData(t, m_pbBalance);
	// the lowest predicted balance in the future
	double minPredict = m_pbBalance;
	double predBal = m_pbBalance;

	auto temp = m_pExtraCache->user()->predictedFutureTransactions(0.5);
	for(int i = 0; i < temp.count(); ++i) {
		Transaction* trans = &temp[i];
		// not do anything if it already came true
		if (trans->flags & Transaction::CameTrue) {
			LOG() << "not charting prediction that came true" << trans->amountDbl() << trans->date.toString() << trans->name << endl;
			continue;
		}
		int dayTo = m_pbDate.daysTo(trans->date);
		predBal += trans->amountDbl();
		if(dayTo == 0) { // same day
			LOG() << "predicted today " << trans->amountDbl() << endl;
		}
		if(dayTo > 0 && dayTo < dayFuture) {
			double predT = t + dayTo;
			ui->plot->graph(2)->addData(predT, predBal);
			minPredict = qMin(minPredict, predBal);
		}
	}

	if(minPredict <= m_pExtraCache->slushBaseStart()) {
		ui->plot->setBackground(QBrush(Qt::red));
	}
	else {
		int greenVal = qMin(4*255.0, minPredict - m_pExtraCache->slushBaseStart()) / 4;
		ui->plot->setBackground(QBrush(QColor(255 - greenVal, greenVal, 0)));
	}
}

void ExtraCashView::makePastiPlot()
{
	QCPGraph* graph = ui->plot->graph(3);
	graph->clearData();
	double minPredict = m_pbBalance;
	auto temp = m_pExtraCache->user()->predictedFutureTransactions(1.0);
	for(int i = 0; i < temp.count(); ++i) {
		Transaction* trans = &temp[i];
//		// not do anything if it already came true
//		if (trans->flags & Transaction::CameTrue) {
//			qDebug() << "not charting prediction that came true";
//			continue;
//		}
		int dayTo = m_pbDate.daysTo(trans->date);
//		if(dayTo <== 0) {
//			for (auto dat = ui->plot->graph(1)->data()->begin(); dat != ui->plot->graph(1)->data()->end(); ++dat) {
//				//dat->value += trans->amountDbl();
//			}
//			m_lastBal = ui->plot->graph(1)->data()->last().value;
//			ui->spinBox->setValue(m_lastBal);
//			minPredict = m_lastBal;
//			//qDebug() << "predicted today" << trans->amountDbl();
//		}
		if(dayTo <= 0 && dayTo > -dayPast) {
			// first point predicted
			QCPData d1 = ui->plot->graph(1)->data()->last();
			if(graph->data()->isEmpty()) {
				graph->addData(d1.key, d1.value);
			}
			QCPData d2 = graph->data()->first();
			double predVal = d2.value - trans->amountDbl();
			double t = d1.key + dayTo;
			if (t >= d2.key)
				t = d2.key - smallInc;
			graph->addData(t, predVal);
			minPredict = qMin(minPredict, predVal);
			if(dayTo < 5) {
				//qDebug() << "fut" << dayTo << trans->amountDbl() << t;
			}
		}
	}
}

void ExtraCashView::makeMinSlope()
{
	QCPGraph* graph = ui->plot->graph(4);
	graph->clearData();
	double tToday = m_pbDate.toJulianDay() - m_d0;
	double slushBase = m_pExtraCache->slushBaseStart();
	if(graph->data()->isEmpty()) {
		graph->addData(tToday, slushBase);
	}
	double dayMin = m_pExtraCache->futDayMinSlope() - tToday;
	double minSlope = qMax(0.0, m_pExtraCache->minSlope());
	double effectiveSlushforDay = m_pExtraCache->slushNeed() * (0.5 + 1.0 * (dayMin) / 30.0);
	graph->addData(tToday + dayFuture, slushBase + minSlope * dayFuture);
	graph->addData(tToday + dayMin, slushBase + minSlope * dayMin);
	graph->addData(tToday + dayMin + 0.001, slushBase + minSlope * dayMin + effectiveSlushforDay);
	graph->addData(tToday + dayMin + 0.002, slushBase + minSlope * dayMin);
	graph->addData(tToday + dayFuture + 0.003, slushBase + minSlope * dayFuture);
	qDebug() << tToday << dayFuture << dayMin << minSlope;
}

