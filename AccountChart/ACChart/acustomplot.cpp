#include "acustomplot.h"

ACustomPlot::ACustomPlot(QWidget *parent) :
	QCustomPlot(parent)
{
	xAxis->setTickLabelType(QCPAxis::ltDateTime);
	xAxis->setDateTimeFormat("yyyy/MM/dd hh");
	yAxis->setAutoTickCount(10);
	setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
	axisRect(0)->setRangeZoomAxes(xAxis, 0);
}

void ACustomPlot::makeGraphs(HashedBundles& hashBundles) {
	clearGraphs();
	m_hashGraphs.clear();
	m_hashBund.clear();
	m_labels.clear();
	addGraph();
	graph(0)->setLineStyle(QCPGraph::lsStepLeft);
	graph(0)->setPen(QPen(Qt::gray));
	yAxis2->setAutoTickCount(10);
	for (const auto& h : hashBundles.keys()) {
		TransactionBundle* bundle = hashBundles[h];
		m_hashBund[h] = bundle;
		m_hashGraphs[h] = addGraph();
		m_hashGraphs[h]->setLineStyle(QCPGraph::lsNone);
		m_hashGraphs[h]->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 5.0));
		int r = h % 255;
		int g = (h >> 1) % 255;
		int b = (h >> 2) % 255;
		m_hashGraphs[h]->setPen(QPen(QColor(r,g,b,255)));
		m_labels.append(bundle->averageName() + "    last : " + bundle->trans(-1).name + "   uniques: " + bundle->uniqueNames().join(" | "));
	}
}

void ACustomPlot::showHash(int ithLayer)
{
	if (ithLayer < 0) {
		for (const auto& h : m_hashGraphs.keys()) {
			m_hashGraphs[h]->setVisible(true);
		}
		emit newLabel("");
		emit newSum(0.0);
	}
	else {
		for (uint h : m_hashGraphs.keys()) {
			m_hashGraphs[h]->setVisible(false);
		}
		uint h = m_hashGraphs.keys()[ithLayer];
		m_hashGraphs[h]->setVisible(true);
		emit newLabel(m_labels[ithLayer]);
		TransactionBundle* bundle = m_hashBund[h];
		emit newSum(bundle->sumDollar());
	}
	replot();
}

void ACustomPlot::loadCompressedAmount(Account* account)
{
	makeGraphs(account->hashBundles());

	yAxis->setLabel("log($)");
	// add the purchase points
	TransactionBundle& allTrans = account->allTrans();
	for (int i = 0; i < allTrans.count(); ++i) {
		double t = allTrans.trans(i).time_t();
		uint h = allTrans.trans(i).nameHash.hash;
		m_integral += allTrans.trans(i).amountDbl();
		graph(0)->addData(t, kindaLog(m_integral));
		m_hashGraphs[h]->addData(t, allTrans.trans(i).compressedAmount());
	}
	rescaleAxes();
	//xAxis->setRange(xAxis->range().lower - 7*24*3600, xAxis->range().upper + 7*24*3600);
	yAxis->setRange(yAxis->range().lower - 0.5, yAxis->range().upper + 0.5);
}


void ACustomPlot::loadAmount(Account* account)
{
	yAxis->setLabel("$");
	// add the purchase points
	TransactionBundle& allTrans = account->allTrans();
	for (int i = 0; i < allTrans.count(); ++i) {
		double t = allTrans.trans(i).time_t();
		graph(0)->addData(t, allTrans.trans(i).compressedAmount());
	}
	rescaleAxes();
	xAxis->setRange(xAxis->range().lower - 7*24*3600, xAxis->range().upper + 7*24*3600);
	yAxis->setRange(yAxis->range().lower - 500, yAxis->range().upper + 500);
}

