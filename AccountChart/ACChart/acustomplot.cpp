#include "acustomplot.h"
#include "acdata.h"

ACustomPlot::ACustomPlot(QWidget *parent) :
	QCustomPlot(parent)
{
	// setup the chart options
	addGraph();
	graph(0)->setLineStyle(QCPGraph::lsNone);
	graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 2.0));
	xAxis->setTickLabelType(QCPAxis::ltDateTime);
	xAxis->setDateTimeFormat("yyyy/MM/dd hh");
	setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
	axisRect(0)->setRangeZoomAxes(xAxis, 0);
}

void ACustomPlot::loadCompressedAmount(Account* account)
{
	m_mode = Mode::logKinda;
	yAxis->setLabel("log($)");
	// add the purchase points
	for (const auto& trans : account->transactions().list()) {
		graph(0)->addData(trans.time(), trans.compressedAmount());
	}
	rescaleAxes();
}

