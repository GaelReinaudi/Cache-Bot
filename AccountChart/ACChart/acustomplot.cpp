#include "acustomplot.h"

ACustomPlot::ACustomPlot(QWidget *parent) :
	QCustomPlot(parent)
{
	xAxis->setTickLabelType(QCPAxis::ltDateTime);
	xAxis->setDateTimeFormat("yyyy/MM/dd hh");
//	yAxis->setAutoTickCount(18);
//	yAxis2->setAutoTickCount(10);
	setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
	axisRect(0)->setRangeZoomAxes(xAxis, 0);
	connect(yAxis, SIGNAL(rangeChanged(QCPRange)), yAxis2, SLOT(setRange(QCPRange)));
}

void ACustomPlot::makeGraphs(HashedBundles& hashBundles) {
	clearGraphs();
	m_hashGraphs.clear();
	m_hashBund.clear();
	m_labels.clear();
	addGraph(xAxis, yAxis2);
	graph(0)->setLineStyle(QCPGraph::lsStepLeft);
	graph(0)->setPen(QPen(Qt::gray, 3));
	graph(0)->setAdaptiveSampling(false);
	for (const auto& h : hashBundles.keys()) {
		TransactionBundle* bundle = hashBundles[h];
		m_hashBund[h] = bundle;
		m_labels.append(bundle->averageName() + "    last : " + bundle->trans(-1).name + "   uniques: " + bundle->uniqueNames().join(" | "));
		for (int iAccType = 0; iAccType <= 3; ++iAccType) {
			QCPGraph* pGraph = addGraph();
			m_hashGraphs[h].append(pGraph);
			pGraph->setLineStyle(QCPGraph::lsNone);
			pGraph->setAdaptiveSampling(false);
			int r = h % 225;
			int g = (h >> 1) % 225;
			int b = (h >> 2) % 225;
			pGraph->setPen(QPen(QColor(r,g,b,255)));
			switch (1 << iAccType) {
			case Account::Type::Checking:
				pGraph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 5.0));
				break;
			case Account::Type::Saving:
				pGraph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssSquare, 5.0));
				break;
			case Account::Type::Credit:
				pGraph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssPlus, 6.0));
				break;
			default:
				pGraph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssTriangle, 15.0));
			}
		}
	}
}

void ACustomPlot::showHash(int ithLayer)
{
	if (ithLayer < 0) {
		for (const auto& h : m_hashGraphs.keys()) {
			for (auto gr : m_hashGraphs[h])
				gr->setVisible(true);
		}
		emit newLabel("");
		emit newSum(0.0);
	}
	else {
		for (const auto& h : m_hashGraphs.keys()) {
			for (auto gr : m_hashGraphs[h])
				gr->setVisible(false);
		}
		const auto h = m_hashGraphs.keys()[ithLayer];
		for (auto gr : m_hashGraphs[h])
			gr->setVisible(true);
		emit newLabel(m_labels[ithLayer]);
		TransactionBundle* bundle = m_hashBund[h];
		emit newSum(bundle->sumDollar());
		emit newHashValue(h);
	}
	replot();
}

void ACustomPlot::loadCompressedAmount(User* pUser)
{
	makeGraphs(pUser->hashBundles());

	yAxis->setLabel("skblnrl($)");
	// add the purchase points
	auto& allTrans = pUser->allTrans();
	for (int i = 0; i < allTrans.count(); ++i) {
		Transaction& tr = allTrans.trans(i);
		double t = tr.time_t();
		uint h = tr.nameHash.hash();
		if (!tr.isInternal())
			m_integral += tr.amountDbl();
		graph(0)->addData(t, kindaLog(m_integral));
		QCPGraph* pGraph = 0;
		switch (tr.type()) {
		case Account::Type::Checking:
			pGraph = m_hashGraphs[h].at(0);
			break;
		case Account::Type::Saving:
			pGraph = m_hashGraphs[h].at(1);
			break;
		case Account::Type::Credit:
			pGraph = m_hashGraphs[h].at(2);
			break;
		default:
			pGraph = m_hashGraphs[h].at(3);
			qDebug() << "tr.type() =" << tr.type();
		}
		pGraph->addData(t, tr.compressedAmount());
	}
	graph(0)->clearData();
	// redo the integral to match the last point known.
	m_integral = pUser->balance(Account::Type::Checking) - m_integral;
	for (int i = 0; i < allTrans.count(); ++i) {
		Transaction& tr = allTrans.trans(i);
		double t = tr.time_t();
		if (!tr.isInternal())
			m_integral += tr.amountDbl();
		graph(0)->addData(t, kindaLog(m_integral));
	}
	rescaleAxes();
	//xAxis->setRange(xAxis->range().lower - 7*24*3600, xAxis->range().upper + 7*24*3600);
	yAxis->setRange(yAxis->range().lower - 0.5, yAxis->range().upper + 0.5);
}

AHashPlot::AHashPlot(QWidget *parent)  :
	ACustomPlot(parent) {
	xAxis->setTickLabelType(QCPAxis::ltNumber);
	yAxis2->setVisible(true);

	xAxis->setLabel("skblnrl($)");
	yAxis->setLabel("hash");
	yAxis2->setLabel("points");

	disconnect(yAxis, SIGNAL(rangeChanged(QCPRange)), yAxis2, SLOT(setRange(QCPRange)));
}

void AHashPlot::loadCompressedAmount(User *pUser)
{
	makeGraphs(pUser->hashBundles());

	// add the purchase points
	auto& allTrans = pUser->allTrans();
	for (int i = 0; i < allTrans.count(); ++i) {
		Transaction& tr = allTrans.trans(i);
		uint h = tr.nameHash.hash();
		uint d = tr.nameHash.manLength();
		if (!tr.isInternal())
			m_integral += 1.0;
		graph(0)->addData(tr.compressedAmount(), m_integral);
		QCPGraph* pGraph = 0;
		switch (tr.type()) {
		case Account::Type::Checking:
			pGraph = m_hashGraphs[h].at(0);
			break;
		case Account::Type::Saving:
			pGraph = m_hashGraphs[h].at(1);
			break;
		case Account::Type::Credit:
			pGraph = m_hashGraphs[h].at(2);
			break;
		default:
			pGraph = m_hashGraphs[h].at(3);
			qDebug() << "tr.type() =" << tr.type();
		}
		pGraph->addData(tr.compressedAmount(), d);
	}
	QList<double> orderedKeys = graph(0)->data()->keys();
	qDebug() << orderedKeys;
	graph(0)->clearData();
	double tot = 1.0;//m_integral;
	m_integral = 0.0;
	for (double dat : orderedKeys) {
		m_integral += 1.0 / tot;
		graph(0)->addData(dat, m_integral);
	}
	rescaleAxes();
	xAxis->setRange(xAxis->range().lower - 0.5, xAxis->range().upper + 0.5);
}
