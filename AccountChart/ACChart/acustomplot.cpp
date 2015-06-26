#include "acustomplot.h"

ACustomPlot::ACustomPlot(QWidget *parent) :
	QCustomPlot(parent)
{
	xAxis->setTickLabelType(QCPAxis::ltDateTime);
	xAxis->setDateTimeFormat("yyyy/MM/dd hh");
	yAxis->setAutoTickCount(18);
	setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
	axisRect(0)->setRangeZoomAxes(xAxis, 0);
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
	yAxis2->setAutoTickCount(10);
	for (const auto& h : hashBundles.keys()) {
		TransactionBundle* bundle = hashBundles[h];
		m_hashBund[h] = bundle;
		m_hashGraphs[h] = addGraph();
		m_hashGraphs[h]->setLineStyle(QCPGraph::lsNone);
		qDebug() << bundle->trans(0).account->type();
		switch (bundle->trans(0).account->type()) {
		case Account::Type::Credit:
			m_hashGraphs[h]->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssPlus, 6.0));
			break;
		case Account::Type::Saving:
			m_hashGraphs[h]->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssSquare, 5.0));
			break;
		case Account::Type::Checking:
			m_hashGraphs[h]->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 5.0));
			break;
		default:
			m_hashGraphs[h]->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssTriangle, 15.0));
		}
		m_hashGraphs[h]->setAdaptiveSampling(false);
		int r = h % 225;
		int g = (h >> 1) % 225;
		int b = (h >> 2) % 225;
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
		double t = allTrans.trans(i).time_t();
		uint h = allTrans.trans(i).nameHash.hash();
		m_integral += allTrans.trans(i).amountDbl();
		graph(0)->addData(t, kindaLog(m_integral));
		m_hashGraphs[h]->addData(t, allTrans.trans(i).compressedAmount());
	}
	graph(0)->clearData();
	// redo the integral to match the last point known.
	m_integral = pUser->balance(Account::Type::Checking | Account::Type::Saving) - m_integral;
	for (int i = 0; i < allTrans.count(); ++i) {
		double t = allTrans.trans(i).time_t();
		m_integral += allTrans.trans(i).amountDbl();
		graph(0)->addData(t, kindaLog(m_integral));
	}
	rescaleAxes();
	//xAxis->setRange(xAxis->range().lower - 7*24*3600, xAxis->range().upper + 7*24*3600);
	yAxis->setRange(yAxis->range().lower - 0.5, yAxis->range().upper + 0.5);
}


void ACustomPlot::loadAmount(User* pUser)
{
	yAxis->setLabel("$");
	// add the purchase points
	auto& allTrans = pUser->allTrans();
	for (int i = 0; i < allTrans.count(); ++i) {
		double t = allTrans.trans(i).time_t();
		graph(0)->addData(t, allTrans.trans(i).compressedAmount());
	}
	rescaleAxes();
	xAxis->setRange(xAxis->range().lower - 7*24*3600, xAxis->range().upper + 7*24*3600);
	yAxis->setRange(yAxis->range().lower - 500, yAxis->range().upper + 500);
}



AHashPlot::AHashPlot(QWidget *parent)  :
	ACustomPlot(parent) {
	xAxis->setTickLabelType(QCPAxis::ltNumber);
	yAxis2->setVisible(true);

	xAxis->setLabel("skblnrl($)");
	yAxis->setLabel("hash");
	yAxis2->setLabel("points");
}

void AHashPlot::loadCompressedAmount(User *pUser)
{
	makeGraphs(pUser->hashBundles());

	// add the purchase points
	auto& allTrans = pUser->allTrans();
	for (int i = 0; i < allTrans.count(); ++i) {
		uint h = allTrans.trans(i).nameHash.hash();
		uint d = allTrans.trans(i).nameHash.manLength();
		m_integral += 1.0;
		graph(0)->addData(allTrans.trans(i).compressedAmount(), m_integral);
		m_hashGraphs[h]->addData(allTrans.trans(i).compressedAmount(), d);
	}
	QList<double> orderedKeys = graph(0)->data()->keys();
	qDebug() << orderedKeys;
	graph(0)->clearData();
	double tot = 1.0;//m_integral;
	m_integral = 0.0;//pUser->balance(Account::Type::Checking | Account::Type::Saving) - m_integral;
	for (double dat : orderedKeys) {
		m_integral += 1.0 / tot;
		graph(0)->addData(dat, m_integral);
	}
	rescaleAxes();
	xAxis->setRange(xAxis->range().lower - 0.5, xAxis->range().upper + 0.5);
}
