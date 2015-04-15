#include "mainwindow.h"
#include "ui_mainwindow.h"

const int dayPast = 60;
const int dayFuture = 60;
const int playBackStartAgo = 210;

double smallInc = 1e-3;

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

//	connect(ui->spinBox, SIGNAL(editingFinished()), this, SLOT(updateChart()));
	connect(ui->plot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(onWheelEvent(QWheelEvent*)));

	m_account.loadPlaidJson("../cacheLight/input.json", 0, 0);

	// transaction at the starting date of the playback
	TransactionBundle& real = m_account.allTrans();
	m_date = real.trans(-1).date.addDays(-playBackStartAgo);//QDate::currentDate();
	m_d0 = m_date.toJulianDay();
	qDebug() << m_date;

	for (int i = 0; i < real.count(); ++i) {
		if (real.trans(i).date >= m_date) {
			m_ipb = i;
			break;
		}
	}
//	// trick predictions at time of playback
//	for (int i = 0; i < m_account.m_predicted.count(); ++i) {
//		(m_account.m_predicted.transArray()+i)->date = (m_account.m_predicted.transArray()+i)->date.addDays(-playBackStartAgo);
//	}

	m_lastBal = 20000;
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
	TransactionBundle& real = m_account.allTrans();
	int addDay = 1;
	if(m_ipb < real.count()) {
		Transaction& newTrans = real.trans(m_ipb);
		// is this transaction predicted in a way?
		if (wasPredicted(newTrans)) {
			qDebug() << "prediction that came true";
			newTrans.flags = Transaction::CameTrue;
		}
		else {
			double delta = newTrans.amountDbl();
			m_lastBal += delta;
			ui->spinBox->setValue(m_lastBal);
		}
		// if new date, move forward
		addDay = newTrans.jDay() - real.trans(m_ipb - 1).jDay();
		qDebug() << newTrans.amountDbl() << newTrans.name;
	}
	m_date = m_date.addDays(addDay);
	++m_ipb;

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

	ui->plot->xAxis->setRange(t - dayPast, t + dayFuture);
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

	if(minPredict <= 0) {
		ui->plot->setBackground(QBrush(Qt::red));
	}
	else {
		int greenVal = qMin(4*255.0, minPredict) / 4;
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

