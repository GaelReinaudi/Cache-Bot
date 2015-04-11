#include "mainwindow.h"
#include "ui_mainwindow.h"

const int dayPast = 10;
const int dayFuture = 30;
const int playBackStartAgo = 365;

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

	connect(ui->spinBox, SIGNAL(editingFinished()), this, SLOT(updateChart()));
	connect(ui->plot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(onWheelEvent(QWheelEvent*)));

	m_account.loadPlaidJson("../cacheLight/input.json", 0);

	// transaction at the starting date of the playback
	TransactionBundle& real = m_account.allTrans();
	m_date = real.trans(-1).date;//QDate::currentDate();
	m_d0 = m_date.toJulianDay();

	for (int i = 0; i < real.count(); ++i) {
		if (real.trans(i).jDay() >= m_d0 - playBackStartAgo) {
			m_ipb = i;
			break;
		}
	}

	ui->spinBox->setValue(600);
	ui->spinBox->editingFinished();
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::onWheelEvent(QWheelEvent * wEv)
{
	TransactionBundle& real = m_account.allTrans();

	m_lastBal = ui->spinBox->value();
	int step = 200 * qrand() / RAND_MAX;
	if(wEv->delta() > 0) {
		ui->spinBox->setValue(m_lastBal + 0);
		m_date = m_date.addDays(1);
	}
	else {
		int addDay = 1;
		if(m_ipb < real.count()) {
			Transaction& newTrans = real.trans(m_ipb);
			ui->spinBox->setValue(m_lastBal + newTrans.amountDbl());
			// if new date, move forward
			addDay = newTrans.jDay() - real.trans(m_ipb - 1).jDay();
			qDebug() << newTrans.amountDbl() << newTrans.name;
		}
		m_date = m_date.addDays(addDay);
		++m_ipb;
	}
	updateChart();
}

void MainWindow::updateChart()
{
	m_lastBal = ui->spinBox->value();
	double t = m_date.toJulianDay() - m_d0;
	qDebug() << t << m_lastBal;
	ui->plot->graph(1)->addData(t, m_lastBal);

	makePredictiPlot();

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
		int dayTo = m_date.daysTo(trans->date);
		if(dayTo == 0) {
			for (auto dat = ui->plot->graph(1)->data()->begin(); dat != ui->plot->graph(1)->data()->end(); ++dat) {
//				dat->value += trans->amountDbl();
			}
			minPredict = m_lastBal = ui->plot->graph(1)->data()->last().value;
			ui->spinBox->setValue(m_lastBal);
			//qDebug() << "today" << trans->amountDbl();
		}
		double smallInc = 1e-3;
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

