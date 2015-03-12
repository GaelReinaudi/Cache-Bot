#include "AMPlot.h"
#include "ACChart/acdata.h"

AMPlot::AMPlot(QWidget *parent)
	: QCustomPlot(parent)
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

	const int height = 256*16;
	const int midHeight = height / 2;
	auto firstTrans = account->transactions().list().front();
	auto lastTrans = account->transactions().list().back();
	colorMap->data()->setSize(1 + firstTrans.date.daysTo(lastTrans.date), height);
	colorMap->data()->setRange(
				QCPRange(firstTrans.time_t(), lastTrans.time_t())
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
	int colorInc = 0;
	for (int minAmort = 2; minAmort > 0; --minAmort) {
		for (const Transaction& trans : account->transactions().list()) {
			amortDur = qRound(trans.numDays());
			//int perDay = qAbs(kindaLog((trans.amountDbl() * 1.0) / amort));
			double perDay = trans.amountDbl() / amortDur;
			if (amortDur >= minAmort && amortDur < maxAmort) {
				int iDay = firstTrans.date.daysTo(trans.date);
				//qDebug() << iDay << amortDur << perDay;

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
							integral[iDay] += trans.amountDbl();
					}
				}
				colorInc += 11111;
				color = colorInc % 128;
			}
		}
		maxAmort = minAmort;
	}
	for (int x=1; x < totalDays; ++x) {
		integral[x] += integral[x - 1];
		integralAmort[x] += integralAmort[x - 1];
		graph(0)->addData(firstTrans.time_t() + double(x) * 86400.0, integral[x]);
		graph(1)->addData(firstTrans.time_t() + double(x) * 86400.0, integralAmort[x]);
	}

	colorMap->rescaleDataRange(true);
	rescaleAxes();
	xAxis->setRange(xAxis->range().lower + 0*3600*24, xAxis->range().upper);// + amortDur*3600*24);
	yAxis->setRange(yAxis->range().lower - 0.5, yAxis->range().upper + 0.5);
	double maxY2 = qMax(qAbs(yAxis2->range().lower), qAbs(yAxis2->range().upper));
	yAxis2->setRange(-maxY2, maxY2);
	replot();
}
