#include "mainwindow.h"
#include "ui_mainwindow.h"

const int dayPast = 60;
const int dayFuture = 60;
const int playBackStartAgo = 210;

double smallInc = 1e-3;
double iniBalance = 3000.0;
double slushAmmount = 500.0;
QString jsonFile = "../cacheLight/chrisPurchases.json";
//QString jsonFile = "../cacheLight/input.json";

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
//	ui->plot->xAxis->setVisible(false);
//	ui->plot->yAxis->setVisible(false);
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

//	connect(ui->spinBox, SIGNAL(editingFinished()), this, SLOT(updateChart()));
	connect(ui->plot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(onWheelEvent(QWheelEvent*)));

	m_account.loadPlaidJson(jsonFile, 0, 0);
	ui->costLive50SpinBox->setValue(m_account.costLiving(0.50));
	ui->costLive75SpinBox->setValue(m_account.costLiving(0.75));
	ui->costLive90SpinBox->setValue(m_account.costLiving(0.90));
	ui->costLive95SpinBox->setValue(m_account.costLiving(0.95));
	ui->costLive99SpinBox->setValue(m_account.costLiving(0.99));

	// transaction at the starting date of the playback
	TransactionBundle& real = m_account.allTrans();
	m_date = real.trans(-1).date.addDays(-playBackStartAgo);//QDate::currentDate();
	m_d0 = m_date.toJulianDay();
	qDebug() << "m_date" << m_date;

	for (int i = 0; i < real.count(); ++i) {
		if (real.trans(i).date >= m_date) {
			m_ipb = i;
			qDebug() << "initial trans("<<i<<")" << real.trans(i).date << real.trans(i).name;
			break;
		}
	}
//	// trick predictions at time of playback
//	for (int i = 0; i < m_account.m_predicted.count(); ++i) {
//		(m_account.m_predicted.transArray()+i)->date = (m_account.m_predicted.transArray()+i)->date.addDays(-playBackStartAgo);
//	}

	m_lastBal = iniBalance;
	ui->spinBox->setValue(m_lastBal);
	ui->spinBox->editingFinished();
	ui->plot->setFocus();
	updateChart();
}

MainWindow::~MainWindow()
{
	delete ui;
}

bool MainWindow::wasPredicted(Transaction &trans)
{
	for(int i = 0; i < m_account.m_predicted.count(); ++i) {
		Transaction* pred = m_account.m_predicted.transArray() + i;
		//qDebug() << pred->jDay() << trans.jDay();
		if(pred->dist(trans) < 128) {
			qDebug() << "observing predicted transaction" << pred->dist(trans) << trans.name;
			return true;
		}
	}
	return false;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
	int maxDayMove = 1;
	int minDayMove = 0;
	if(event->key() == Qt::Key_Up)
		minDayMove = 1;
	TransactionBundle& real = m_account.allTrans();
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
		// is this transaction predicted in a way?
		else if (wasPredicted(newTrans)) {
			qDebug() << "prediction that came true";
			newTrans.flags = Transaction::CameTrue;
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
	double posSlope = qMax(0.0, m_minSlope);
	double extraToday = posSlope * addDay / 2.0;
	if(m_extraToday < 0.0)
		m_extraToday = extraToday;
	m_extraToday *= 0.8;
	m_extraToday += 0.2 * extraToday;
	m_slushThreshold += m_extraToday;
	ui->extraTodaySpinBox->setValue(m_extraToday);
	ui->spinSlushThresh->setValue(m_slushThreshold);

	updateChart();
}

int MainWindow::computeMinSlopeOver(int numDays)
{
	m_minSlope = 9999.9;
	int dayMin = -1;
	double tToday = m_date.toJulianDay() - m_d0;
	double yToday = m_slushThreshold;
	QCPDataMap *pDat = ui->plot->graph(2)->data();
	QMap<double, QCPData>::iterator it = pDat->begin();
	while(it != pDat->end() && it->key < tToday + numDays + 1) {
		if (it->key > tToday) {
			double y = it->value - slushAmmount;
			double slope = (y - yToday) / qMax(1.0, it->key - tToday);
			if(slope < m_minSlope) {
				m_minSlope = slope;
				dayMin = it->key;
			}
		}
		++it;
	}
	if (m_minSlope == 9999.9) {
		m_minSlope = (m_lastBal - slushAmmount - m_slushThreshold) / numDays;// / 2.0;
		dayMin = tToday + numDays;
	}
	return dayMin;
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
	for(int i = 0; i < m_account.m_predicted.count(); ++i) {
		Transaction* trans = m_account.m_predicted.transArray() + i;
		// not do anything if it already came true
		if (trans->flags == Transaction::CameTrue) {
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
			//qDebug() << "predicted today" << trans->amountDbl();
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

	if(minPredict <= m_slushThreshold) {
		ui->plot->setBackground(QBrush(Qt::red));
	}
	else {
		int greenVal = qMin(4*255.0, minPredict - m_slushThreshold) / 4;
		ui->plot->setBackground(QBrush(QColor(255 - greenVal, greenVal, 0)));
	}
}

void MainWindow::makePastiPlot()
{
	QCPGraph* graph = ui->plot->graph(3);
	graph->clearData();
	double minPredict = m_lastBal;
	for(int i = m_account.m_predicted.count() - 1; i >= 0; --i) {
		Transaction* trans = m_account.m_predicted.transArray() + i;
//		// not do anything if it already came true
//		if (trans->flags == Transaction::CameTrue) {
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
	if(graph->data()->isEmpty()) {
		graph->addData(tToday, m_slushThreshold);
	}
	double dayMin = computeMinSlopeOver(dayFuture) - tToday;
	double minSlope = qMax(0.0, m_minSlope);
	graph->addData(tToday + dayFuture, m_slushThreshold + minSlope * dayFuture);
	graph->addData(tToday + dayMin, m_slushThreshold + minSlope * dayMin);
	graph->addData(tToday + dayMin + 0.001, m_slushThreshold + minSlope * dayMin + slushAmmount);
	graph->addData(tToday + dayMin + 0.002, m_slushThreshold + minSlope * dayMin);
	graph->addData(tToday + dayFuture + 0.003, m_slushThreshold + minSlope * dayFuture);
	qDebug() << tToday << dayFuture << dayMin << minSlope;
}

