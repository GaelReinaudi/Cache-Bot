#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "user.h"
#include "botContext.h"
#include "bot.h"
#include "cacherest.h"
#include "cacheAccountConnector.h"
#include "userMetrics.h"

MainWindow::MainWindow(QString userID)
	: QMainWindow()
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	m_pConnector = new CacheAccountConnector(userID);
	connect(m_pConnector, SIGNAL(injected(User*)), this, SLOT(onUserInjected(User*)));
	connect(m_pConnector, SIGNAL(botInjected(Bot*)), this, SLOT(onBotInjected(Bot*)));
}

MainWindow::~MainWindow()
{
	if (m_pConnector) {
		delete m_pConnector;
	}
	delete ui;
}

User *MainWindow::user() const
{
	return m_pConnector->user();
}

void MainWindow::onUserInjected(User* pUser)
{
	CostMonthPercentileMetric<2, 50>::get(pUser);
	CostMonthPercentileMetric<2, 75>::get(pUser);
	CostMonthPercentileMetric<2, 90>::get(pUser);
	CostMonthPercentileMetric<2, 99>::get(pUser);
	CostMonthPercentileMetric<2, 100>::get(pUser);

	for (const QString& str: HistoMetric::allNames()) {
		QListWidgetItem* item = new QListWidgetItem(str, ui->metricList);
		item->setFlags(item->flags() | Qt::ItemIsUserCheckable); // set checkable flag
		item->setCheckState(Qt::Checked); // AND initialize check state
		ui->metricList->addItem(item);
		// adds the corresponding graph
		ui->acPlot->addGraph();
	}

	QDate day  = QDate::currentDate();
	QDate downToDate = day.addMonths(-6);
	while (day >= downToDate) {
		qDebug() << day;
		double t = QDateTime(day).toTime_t();
		int i = 0;
		for (const QString& str: HistoMetric::allNames()) {
			if (HistoMetric::get(str)->valid(day)) {
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

void MainWindow::onBotInjected(Bot* bestBot)
{
	bestBot->summarize();
}

