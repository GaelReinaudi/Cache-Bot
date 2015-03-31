#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	ui->plot->xAxis->setVisible(false);
	ui->plot->yAxis->setVisible(false);

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
	m_date = QDate::currentDate();
	m_d0 = m_date.toJulianDay();

	m_account.loadPlaidJson("../cacheLight/input.json");

	ui->spinBox->setValue(1600);
	ui->spinBox->editingFinished();
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::onWheelEvent(QWheelEvent * wEv)
{
	m_lastBal = ui->spinBox->value();
	int step = 100 * qrand() / RAND_MAX;
	if(wEv->delta() > 0)
		ui->spinBox->setValue(m_lastBal - 2 * step);
	else
		ui->spinBox->setValue(m_lastBal - step);
	updateChart();
}

void MainWindow::updateChart()
{
	m_lastBal = ui->spinBox->value();
	m_date = m_date.addDays(1);
	double t = m_date.toJulianDay() - m_d0;
	qDebug() << t << m_lastBal;
	ui->plot->graph(1)->addData(t, m_lastBal);

	makePredictiPlot();

	ui->plot->xAxis->setRange(t - 15, t + 15);
	ui->plot->yAxis->rescale();
	ui->plot->yAxis->setRange(-100, ui->plot->yAxis->range().upper + 100);
	ui->plot->replot();
}

void MainWindow::makePredictiPlot()
{
	ui->plot->graph(2)->clearData();
	double minPredict = 9999999;
	for(int i = 0; i < m_account.m_predicted.count(); ++i) {
		Transaction* trans = m_account.m_predicted.transArray() + i;
		int dayTo = m_date.daysTo(trans->date);
		if(dayTo == 0) {
			QCPData d = ui->plot->graph(1)->data()->last();
			double val = d.value + trans->amountDbl();
			ui->plot->graph(1)->addData(d.key+0.1, val);
			ui->spinBox->setValue(val);
			minPredict = val;
		}
		if(dayTo > 0 && dayTo < 16) {
			// first point predicted
			if(ui->plot->graph(2)->data()->isEmpty()) {
				QCPData d = ui->plot->graph(1)->data()->last();
				ui->plot->graph(2)->addData(d.key, d.value);
			}
			QCPData d1 = ui->plot->graph(1)->data()->last();
			QCPData d2 = ui->plot->graph(2)->data()->last();
			double predVal = d2.value + trans->amountDbl();
			ui->plot->graph(2)->addData(d1.key + dayTo, predVal);
			minPredict = qMin(minPredict, predVal);
		}
	}
	if(minPredict <= 0) {
		ui->plot->setBackground(QBrush(Qt::red));
	}
	else {
		int greenVal = qMin(255.0, minPredict);
		ui->plot->setBackground(QBrush(QColor(255 - greenVal, greenVal, 0, 64)));
	}
}

