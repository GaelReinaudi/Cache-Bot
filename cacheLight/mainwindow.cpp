#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "cacherest.h"
#include "../extraCash/extraCache.h"

const int dayPast = 60;
const int dayFuture = 60;
const int playBackStartAgo = 210;

double smallInc = 1e-3;

MainWindow::MainWindow(QString userID, QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	m_pExtraCache = new ExtraCache(userID);
	m_pExtraCache->sendExtraCash = false;

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
	connect(m_pExtraCache, SIGNAL(botInjected()), this, SLOT(init()));
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::init()
{
	ui->costLive50SpinBox->setValue(m_pExtraCache->user()->costLiving(0.50));
	ui->costLive75SpinBox->setValue(m_pExtraCache->user()->costLiving(0.75));
	ui->costLive90SpinBox->setValue(m_pExtraCache->user()->costLiving(0.90));
	ui->costLive95SpinBox->setValue(m_pExtraCache->user()->costLiving(0.95));
	ui->costLive99SpinBox->setValue(m_pExtraCache->user()->costLiving(0.99));

	// transaction at the starting date of the playback
	auto& real = m_pExtraCache->user()->allTrans();
	m_date = real.lastTransactionDate();//.addDays(-playBackStartAgo);
	m_d0 = m_date.toJulianDay();
	qDebug() << "m_date" << m_date;

	for (int i = 0; i < real.count(); ++i) {
		if (real.trans(i).date >= m_date) {
			m_ipb = i;
			qDebug() << "initial trans("<<i<<")" << real.trans(i).date << real.trans(i).name;
			break;
		}
	}

	m_lastBal = m_pExtraCache->user()->balance(Account::Type::Checking);
	ui->spinBox->setValue(m_lastBal);
	ui->spinBox->editingFinished();
	ui->plot->setFocus();
	updateChart();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
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
		addDay = newTrans.jDay() - m_date.toJulianDay();
		qDebug() << newTrans.amountDbl() << newTrans.name << newTrans.date;
		if (addDay > maxDayMove) {
			// revert the soon to come increment so that we add a day and come back to this trans
			--m_ipb;
			addDay = maxDayMove;
		}
		else {
			double delta = newTrans.amountDbl();
			m_lastBal += delta;
			ui->spinBox->setValue(m_lastBal);
		}
		++m_ipb;
	}
	if (addDay < minDayMove) {
		return keyPressEvent(event);
	}
	m_date = m_date.addDays(addDay);
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
	pBars->addData(m_date.toJulianDay() - m_d0, m_extraToday);
	ui->plot->yAxis2->rescale();

	updateChart();
}

void MainWindow::onWheelEvent(QWheelEvent * wEv)
{

//	m_lastBal = ui->spinBox->value();
	//int step = 200 * qrand() / RAND_MAX;
	if(wEv->delta() > 0) {
		ui->spinBox->setValue(m_lastBal);
		m_date = m_date.addDays(1);
	}
	else {
		keyPressEvent(0);
	}
	updateChart();
}

void MainWindow::updateChart()
{
	//m_lastBal = ui->spinBox->value();
	double t = m_date.toJulianDay() - m_d0;
	if(!ui->plot->graph(1)->data()->isEmpty()) {
		QCPData d1 = ui->plot->graph(1)->data()->last();
		if (t <= d1.key)
			t = d1.key + smallInc;
	}
	qDebug() << t << m_lastBal;
	ui->plot->graph(1)->addData(t, m_lastBal);

	makePredictiPlot();
	makePastiPlot();
	makeMinSlope();

	ui->plot->xAxis->setRange(t - dayPast, t + dayFuture + 1);
	ui->plot->yAxis->rescale();
	ui->plot->yAxis->setRange(-100, ui->plot->yAxis->range().upper + 100);
	ui->plot->replot();
}

void MainWindow::makePredictiPlot()
{
	ui->plot->graph(2)->clearData();
	double minPredict = m_lastBal;
	auto temp = m_pExtraCache->user()->predictedFutureTransactions(1.0);
	for(int i = 0; i < temp.count(); ++i) {
		Transaction* trans = &temp[i];
		// not do anything if it already came true
		if (trans->flags & Transaction::CameTrue) {
			qDebug() << "not charting prediction that came true";
			continue;
		}
		int dayTo = m_date.daysTo(trans->date);
		if(dayTo == 0) {
			for (auto dat = ui->plot->graph(1)->data()->begin(); dat != ui->plot->graph(1)->data()->end(); ++dat) {
				//dat->value += trans->amountDbl();
			}
			m_lastBal = ui->plot->graph(1)->data()->last().value;
			ui->spinBox->setValue(m_lastBal);
			minPredict = m_lastBal;
			qDebug() << "predicted today" << trans->amountDbl();
		}
		if(dayTo > 0 && dayTo < dayFuture) {
			// first point predicted
			if(ui->plot->graph(2)->data()->isEmpty()) {
				QCPData d = ui->plot->graph(1)->data()->last();
				ui->plot->graph(2)->addData(d.key, d.value);
			}
			QCPData d1 = ui->plot->graph(1)->data()->last();
			QCPData d2 = ui->plot->graph(2)->data()->last();
			double predVal = d2.value + trans->amountDbl();
			double t = d1.key + dayTo;
			if (t <= d2.key)
				t = d2.key + smallInc;
			ui->plot->graph(2)->addData(t, predVal);
			minPredict = qMin(minPredict, predVal);
			if(dayTo < 5) {
				//qDebug() << "fut" << dayTo << trans->amountDbl() << t;
			}
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

void MainWindow::makePastiPlot()
{
	QCPGraph* graph = ui->plot->graph(3);
	graph->clearData();
	double minPredict = m_lastBal;
	auto temp = m_pExtraCache->user()->predictedFutureTransactions(1.0);
	for(int i = 0; i < temp.count(); ++i) {
		Transaction* trans = &temp[i];
//		// not do anything if it already came true
//		if (trans->flags & Transaction::CameTrue) {
//			qDebug() << "not charting prediction that came true";
//			continue;
//		}
		int dayTo = m_date.daysTo(trans->date);
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

void MainWindow::makeMinSlope()
{
	QCPGraph* graph = ui->plot->graph(4);
	graph->clearData();
	double tToday = m_date.toJulianDay() - m_d0;
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

