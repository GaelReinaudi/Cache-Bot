#include "AMPlot.h"
#include "acdata.h"

AMPlot::AMPlot(QWidget *parent)
	: QCustomPlot(parent)
	, m_lastDate(QDate(1, 1, 1))
	, m_firstDate(QDate(11111, 1, 1))

{
	colorMap = new QCPColorMap(xAxis, yAxis);
	addPlottable(colorMap);

	colorMap->setGradient(QCPColorGradient::gpSpectrum);
	colorMap->rescaleDataRange(true);
	colorMap->setInterpolate(false);

//	// setup the chart options
//	addGraph();
//	graph(0)->setLineStyle(QCPGraph::lsNone);
//	graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 2.0));
	xAxis->setTickLabelType(QCPAxis::ltDateTime);
	xAxis->setDateTimeFormat("yyyy/MM/dd hh");
	setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
	axisRect(0)->setRangeZoomAxes(xAxis, 0);
}

void AMPlot::loadAmount(Account* account)
{
	for (const auto& trans : account->transactions().list()) {
		if(m_lastDate.daysTo(trans.startDate().date()) > 0)
			m_lastDate = trans.startDate().date();
		if(m_firstDate.daysTo(trans.startDate().date()) < 0)
			m_firstDate = trans.startDate().date();
//		graph(0)->addData(trans.time(), trans.amount());
	}
	qDebug() << m_firstDate << m_lastDate << "days =" << m_firstDate.daysTo(m_lastDate);

	colorMap->data()->setSize(m_firstDate.daysTo(m_lastDate), 32);
	colorMap->data()->setRange(
				QCPRange(QDateTime(m_firstDate).toTime_t(), QDateTime(m_lastDate).toTime_t())
				, QCPRange(0, colorMap->data()->valueSize()));

	for (int x=0; x < colorMap->data()->keySize(); ++x) {
		for (int y=0; y < colorMap->data()->valueSize(); ++y) {
			colorMap->data()->setCell(x, y, -10.0);
		}
	}
	// first amortized transactions
	double color = 0;
	int day = 0;
	for (const Transaction& trans : account->transactions().list()) {
		int amort = qRound(trans.numDays());
		if (amort > 1) {
			int iDay = m_firstDate.daysTo(trans.startDate().date());
			qDebug() << iDay << amort;

			int h = 0;
			while (colorMap->data()->cell(iDay, h) > 0.0) {
				++h;
			}
			// we look amort day in the future to set the color
			for (int a = 0; a < amort; ++a) {
				// the first empty cell of a day can be used
				h = 0;
				while (colorMap->data()->cell(iDay + a, h) > 0.0) {
					++h;
				}

				colorMap->data()->setCell(iDay + a, h, color);
			}
			color = qrand() % 128;
		}
	}
	colorMap->rescaleDataRange(true);
	rescaleAxes();
	yAxis->setRange(yAxis->range().lower - 0.5, yAxis->range().upper + 0.5);
	replot();
}
