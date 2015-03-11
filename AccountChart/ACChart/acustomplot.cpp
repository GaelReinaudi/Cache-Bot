#include "acustomplot.h"
#include "acdata.h"

ACustomPlot::ACustomPlot(QWidget *parent) :
	QCustomPlot(parent)
{
	// setup the chart options
	addGraph();
    graph(0)->setLineStyle(QCPGraph::lsNone);
	graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 2.0));
    graph(0)->setPen(QPen(QColor(40,40,40,255)));
	xAxis->setTickLabelType(QCPAxis::ltDateTime);
	xAxis->setDateTimeFormat("yyyy/MM/dd hh");
	yAxis->setAutoTickCount(10);
	setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
	axisRect(0)->setRangeZoomAxes(xAxis, 0);

	addGraph();
	graph(1)->setLineStyle(QCPGraph::lsStepLeft);
	graph(1)->setPen(QPen(Qt::gray));
	yAxis2->setAutoTickCount(10);

}

void ACustomPlot::loadCompressedAmount(Account* account)
{
	m_mode = Mode::logKinda;
	yAxis->setLabel("log($)");
	// add the purchase points
	for (const auto& trans : account->transactions().list()) {
		graph(0)->addData(trans.time_t(), trans.compressedAmount());
		m_integral += trans.amountDbl();
		graph(1)->addData(trans.time_t(), kindaLog(m_integral));
	}
	rescaleAxes();
	//xAxis->setRange(xAxis->range().lower - 7*24*3600, xAxis->range().upper + 7*24*3600);
	yAxis->setRange(yAxis->range().lower - 0.5, yAxis->range().upper + 0.5);
}


void ACustomPlot::loadAmount(Account* account)
{
	m_mode = Mode::linear;
	yAxis->setLabel("$");
	// add the purchase points
	for (const auto& trans : account->transactions().list()) {
		graph(0)->addData(trans.time_t(), trans.amountDbl());
	}
	rescaleAxes();
	xAxis->setRange(xAxis->range().lower - 7*24*3600, xAxis->range().upper + 7*24*3600);
	yAxis->setRange(yAxis->range().lower - 500, yAxis->range().upper + 500);
}

