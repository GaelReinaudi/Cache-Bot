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
	m_evoThread->start();
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::plotMask(VectorRectF vecRect) {
	qDebug() << vecRect.size();
	if (!vecRect.empty()) {
		qDebug() << vecRect[0].left() << vecRect[0].right() << vecRect[0].top() << vecRect[0].bottom();
		ui->accountPlot->clearItems();
		for(const QRectF& rect : vecRect) {
			QRectF chartRect = ui->accountPlot->mapDayAgoToPlot(rect);
			chartRect = kindaLog(chartRect);
//			qDebug() << QDateTime::fromTime_t(int(chartRect.left())) << QDateTime::fromTime_t(int(chartRect.right())) << chartRect.top() << chartRect.bottom();
			QCPItemRect* itRect = new QCPItemRect(ui->accountPlot);
			itRect->topLeft->setCoords(chartRect.topLeft());
			itRect->bottomRight->setCoords(chartRect.bottomRight());
			itRect->setPen(QPen(QBrush(QColor(255, 0, 0, 128)), 0.0));
			itRect->setBrush(QBrush(QColor(255, 0, 0, 128)));
			ui->accountPlot->addItem(itRect);
		}
		ui->accountPlot->replot(QCustomPlot::rpQueued);
	}
}

