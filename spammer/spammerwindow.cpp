#include "spammerwindow.h"
#include "ui_spammerwindow.h"
#include <QPushButton>
#include "cacheAccountConnector.h"
#include "cacherest.h"

SpammerWindow::SpammerWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::SpammerWindow)
{
	ui->setupUi(this);
	connect(ui->buttonSpam, &QPushButton::toggled, this, &SpammerWindow::onStartStop);
	connect(ui->spinTrig, SIGNAL(valueChanged(int)), this, SLOT(sendSignup()), Qt::QueuedConnection);

	connect(CacheRest::Instance()->worker, &HttpRequestWorker::repliedFakeSignup, this, &SpammerWindow::onRepliedFakeSignup);


	QString userID = "...";
	QJsonObject jsonArgs;
	jsonArgs["aaa"] = 1;
	m_pConnector = new CacheAccountConnector(userID, jsonArgs);
}

SpammerWindow::~SpammerWindow()
{
	delete ui;
}

void SpammerWindow::onStartStop(bool doStart)
{
	qDebug() << "onStartStop " << doStart;
	sendSignup();
}

void SpammerWindow::sendSignup()
{
	CacheRest::Instance()->fakeSignup("");
	ui->spinSent->setValue(ui->spinSent->value() + 1);
}

void SpammerWindow::onRepliedFakeSignup(QString response)
{
	qDebug() << response;
	ui->spinReply->setValue(ui->spinReply->value() + 1);
}
