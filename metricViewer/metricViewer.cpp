#include "metricViewer.h"
#include "ui_metricViewer.h"
#include "user.h"
#include "botContext.h"
#include "bot.h"
#include "cacherest.h"
#include "cacheAccountConnector.h"
#include "userMetrics.h"

MetricViewer::MetricViewer(QString userID)
	: QMainWindow()
	, ui(new Ui::MetricViewer)
{
	ui->setupUi(this);

	m_pConnector = new CacheAccountConnector(userID);
	connect(m_pConnector, SIGNAL(injected(User*)), this, SLOT(onUserInjected(User*)));
	connect(m_pConnector, SIGNAL(botInjected(Bot*)), this, SLOT(onBotInjected(Bot*)));
}

MetricViewer::~MetricViewer()
{
	if (m_pConnector) {
		delete m_pConnector;
	}
	delete ui;
}

User *MetricViewer::user() const
{
	return m_pConnector->user();
}

void MetricViewer::onUserInjected(User* pUser)
{
	MetricSmoother<7>::get(CostRateMonthPercentileMetric<1, 90 >::get(pUser));
	MetricSmoother<7>::get(CostRateMonthPercentileMetric<1, 99 >::get(pUser));
	MetricSmoother<7>::get(MakeRateMonthPercentileMetric<1, 90 >::get(pUser));
	MetricSmoother<7>::get(MakeRateMonthPercentileMetric<1, 99 >::get(pUser));
	MetricSmoother<7>::get(CostRateMonthPercentileMetric<3, 90 >::get(pUser));
	MetricSmoother<7>::get(CostRateMonthPercentileMetric<3, 99 >::get(pUser));
	MetricSmoother<7>::get(MakeRateMonthPercentileMetric<3, 90 >::get(pUser));
	MetricSmoother<7>::get(MakeRateMonthPercentileMetric<3, 99 >::get(pUser));
	MetricSmoother<7>::get(Flow01<90, 90>::get(pUser));
	MetricSmoother<7>::get(Flow01<95, 90>::get(pUser));
	MetricSmoother<7>::get(Flow01<95, 95>::get(pUser));
	MetricSmoother<7>::get(Flow01<99, 95>::get(pUser));
//	MetricSmoother<7>::get(Flow01<100, 100>::get(pUser));

	for (const QString& str: HistoMetric::allNames()) {
		QColor color(qrand() % 192 + 64, qrand() % 192 + 64, qrand() % 192 + 64);
		QListWidgetItem* item = new QListWidgetItem(str, ui->metricList);
		item->setFlags(item->flags() | Qt::ItemIsUserCheckable); // set checkable flag
		item->setCheckState(Qt::Checked); // AND initialize check state
		item->setBackground(QBrush(color));
		ui->metricList->addItem(item);
		// adds the corresponding graph
		ui->acPlot->addGraph()->setPen(QPen(color));
	}

	QDate day  = QDate::currentDate();
	QDate downToDate = day.addMonths(-6);
	while (day >= downToDate) {
		qDebug() << day;
		double t = QDateTime(day).toTime_t();
		int i = 0;
		for (const QString& str: HistoMetric::allNames()) {
			if (HistoMetric::get(str)->isValid(day)) {
				double v = HistoMetric::get(str)->value(day);
				ui->acPlot->graph(i)->addData(t, v);
			}
			++i;
		}
		day = day.addDays(-1);
	}
	ui->acPlot->rescaleAxes();
	ui->acPlot->replot();

	CacheRest::Instance()->getBestBot(pUser->id(), pUser);
}

void MetricViewer::onBotInjected(Bot* bestBot)
{
	bestBot->summarize();
}

