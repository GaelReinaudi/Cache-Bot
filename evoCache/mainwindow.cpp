#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "EvolutionSpinner.h"

MainWindow::MainWindow(QString jsonFile, QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	connect(ui->startButton, SIGNAL(clicked(bool)), this, SLOT(startEvolution(bool)));

	// needed to spin a new thread and run the evolution in it
	m_evoThread = new QThread();
	m_evoSpinner = new EvolutionSpinner();
	m_evoSpinner->moveToThread(m_evoThread);
	connect(m_evoThread, &QThread::finished, m_evoSpinner, &QObject::deleteLater);
	connect(m_evoSpinner, &EvolutionSpinner::resultReady, this, &MainWindow::handleResults);
	m_evoThread->start();
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::startEvolution(bool doStart) {
	if (!doStart)
		return;

}
