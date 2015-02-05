#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ACChart/acdata.h"

MainWindow::MainWindow(QString jsonFile, QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);


	// an account object that is going to be populated by the json file
	Account* account = new Account();
	account->load(jsonFile);

	ui->accountPlot->loadCompressedAmount(account);
//	ui->accountPlot->loadAmount(account);

//	ui->accountPlot->setPlottingHints(QCP::phFastPolylines | QCP::phCacheLabels);

	// needed to spin a new thread and run the evolution in it
	m_evoThread = new QThread();
	m_evoSpinner = new EvolutionSpinner(account);
	m_evoSpinner->moveToThread(m_evoThread);
	connect(m_evoThread, &QThread::finished, m_evoSpinner, &QObject::deleteLater);
	connect(m_evoSpinner, &EvolutionSpinner::resultReady, this, &MainWindow::handleResults);
	connect(ui->startButton, SIGNAL(clicked(bool)), m_evoSpinner, SLOT(startEvolution(bool)));
	connect(m_evoSpinner, &EvolutionSpinner::sendMask, this, &MainWindow::plotMask);
	connect(m_evoSpinner, &EvolutionSpinner::sendClearMask, this, &MainWindow::clearMasks);
	m_evoThread->start();
//	ui->startButton->click();
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::clearMasks() {
	ui->accountPlot->clearItems();
}

void MainWindow::plotMask(ZoneVector vecZone) {
//	qDebug() << vecZone.size();
	if (!vecZone.empty()) {
//		qDebug() << vecZone[0].left() << vecZone[0].right() << vecZone[0].top() << vecZone[0].bottom();
		for(const auto& zone : vecZone) {
			QRectF chartRect = ui->accountPlot->mapDayAgoToPlot(zone);
			chartRect = kindaLog(chartRect);
//			qDebug() << QDateTime::fromTime_t(int(chartRect.left())) << QDateTime::fromTime_t(int(chartRect.right())) << chartRect.top() << chartRect.bottom();
			QCPItemRect* itRect = new QCPItemRect(ui->accountPlot);
			itRect->topLeft->setCoords(chartRect.topLeft());
			itRect->bottomRight->setCoords(chartRect.bottomRight());
            QColor colZone = zone.m_isFilled ? QColor(11, 96, 254, 128) : QColor(239, 64, 53, 128);
			itRect->setPen(QPen(QBrush(colZone), 3.0));
			itRect->setBrush(QBrush(colZone));
			itRect->setClipToAxisRect(false);
			ui->accountPlot->addItem(itRect);
		}
		ui->accountPlot->replot(QCustomPlot::rpQueued);
	}
}

