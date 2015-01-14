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
	m_lastDate = QDate(1, 1, 1);
}

void ACustomPlot::loadCompressedAmount(Account* account)
{
	m_mode = Mode::logKinda;
	yAxis->setLabel("log($)");
	// add the purchase points
	for (const auto& trans : account->transactions().list()) {
		if(m_lastDate.daysTo(trans.startDate().date()) > 0)
			m_lastDate = trans.startDate().date();
		graph(0)->addData(trans.time(), trans.compressedAmount());
	}
	qDebug() << m_lastDate;
	rescaleAxes();
}


void ACustomPlot::loadAmount(Account* account)
{
	m_mode = Mode::linear;
	yAxis->setLabel("$");
	// add the purchase points
	for (const auto& trans : account->transactions().list()) {
		if(m_lastDate.daysTo(trans.startDate().date()) > 0)
			m_lastDate = trans.startDate().date();
		graph(0)->addData(trans.time(), trans.amount());
	}
	qDebug() << m_lastDate;
	rescaleAxes();
}

QRectF ACustomPlot::mapDayAgoToPlot(QRectF rectDayAgo) const {
	QRectF chartRect(rectDayAgo);
	QDate timeLeft = m_lastDate.addDays(-chartRect.left());
	QDate timeRight = m_lastDate.addDays(-chartRect.right() + 1);
	chartRect.setLeft(QDateTime(timeLeft).toTime_t());
	chartRect.setRight(QDateTime(timeRight).toTime_t());
	return chartRect;
}

