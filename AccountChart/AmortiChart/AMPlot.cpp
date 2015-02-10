#include "AMPlot.h"
#include "ACChart/acdata.h"

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

	xAxis->setTickLabelType(QCPAxis::ltDateTime);
	xAxis->setDateTimeFormat("yyyy/MM/dd\nhh");
	setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
	axisRect(0)->setRangeZoomAxes(xAxis, 0);

	addGraph(xAxis, yAxis2);
	graph(0)->setPen(QPen(QBrush(QColor(255, 255, 0, 128)), 5.0));
	addGraph(xAxis, yAxis2);
	graph(1)->setPen(QPen(QBrush(QColor(255, 0, 0, 128)), 5.0));

	yAxis2->setVisible(true);
}

void AMPlot::loadAmount(Account* account)
{
	graph(0)->clearData();
	graph(1)->clearData();
	for (const auto& trans : account->transactions().list()) {
		if(m_lastDate.daysTo(trans.startDate().date()) > 0)
			m_lastDate = trans.startDate().date();
		if(m_firstDate.daysTo(trans.startDate().date()) < 0)
			m_firstDate = trans.startDate().date();
	}
	qDebug() << m_firstDate << m_lastDate << "days =" << m_firstDate.daysTo(m_lastDate);
	double firstTime = QDateTime(m_firstDate).toTime_t();
	double lastTime = QDateTime(m_lastDate).toTime_t();

	const int height = 256*16;
	const int midHeight = height / 2;
	colorMap->data()->setSize(m_firstDate.daysTo(m_lastDate), height);
	colorMap->data()->setRange(
				QCPRange(firstTime, lastTime)
				, QCPRange(-midHeight, -midHeight + colorMap->data()->valueSize()));

	int totalDays = colorMap->data()->keySize() + 60;
	QVector<double> integral(totalDays, 0.0);
	QVector<double> integralAmort(totalDays, 0.0);
	for (int x=0; x < totalDays; ++x) {
		for (int y=0; y < colorMap->data()->valueSize(); ++y) {
			colorMap->data()->setCell(x, y, -10.0);
		}
	}
	// first amortized transactions
	double color = 0;
	int amortDur = 0;
	int maxAmort = 999999999;
	qsrand(55);
	for (int minAmort = 2; minAmort > 0; --minAmort) {
		for (const Transaction& trans : account->transactions().list()) {
			amortDur = qRound(trans.numDays());
			//int perDay = qAbs(kindaLog((trans.amount() * 1.0) / amort));
			double perDay = trans.amount() / amortDur;
			if (amortDur >= minAmort && amortDur < maxAmort) {
				int iDay = m_firstDate.daysTo(trans.startDate().date());
				qDebug() << iDay << amortDur << perDay;

				int mul = perDay > 0.0 ? 1 : -1;
				int h = 0;
				// we look amort day in the future to set the color
				for (int a = 0; a < amortDur; ++a) {
					// the first empty cell of a day can be used
					h = 0;
					while (colorMap->data()->cell(iDay + a, midHeight + mul * h) > 0.0) {
						++h;
					}

					for (int c = 0; c < qAbs(perDay); ++c) {
						colorMap->data()->setCell(iDay + a, midHeight + mul * (h + c), color);
					}
					if ( iDay + a < totalDays) {
						integralAmort[iDay + a] += perDay;
						if (a == 0)
							integral[iDay + a] += trans.amount();
					}
				}
				color = qrand() % 128;
			}
		}
		maxAmort = minAmort;
	}
	for (int x=1; x < totalDays; ++x) {
		integral[x] += integral[x - 1];
		integralAmort[x] += integralAmort[x - 1];
		graph(0)->addData(firstTime + double(x) * 86400.0, integral[x]);
		graph(1)->addData(firstTime + double(x) * 86400.0, integralAmort[x]);
	}

	colorMap->rescaleDataRange(true);
	rescaleAxes();
	xAxis->setRange(xAxis->range().lower + 0*3600*24, xAxis->range().upper);// + amortDur*3600*24);
	yAxis->setRange(yAxis->range().lower - 0.5, yAxis->range().upper + 0.5);
	double maxY2 = qMax(qAbs(yAxis2->range().lower), qAbs(yAxis2->range().upper));
	yAxis2->setRange(-maxY2, maxY2);
	replot();
}
