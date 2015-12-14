#include "spinnerwindow.h"
#include "ui_spinnerwindow.h"

SpinnerWindow::SpinnerWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::SpinnerWindow)
{
	ui->setupUi(this);
	CacheRest::Instance(this);
	CacheRest::Instance()->login();
	connect(CacheRest::Instance()->worker, SIGNAL(loggedIn(bool)), this, SLOT(onLoggedIn(bool)));
}

SpinnerWindow::~SpinnerWindow()
{
	delete ui;
}

void SpinnerWindow::onLoggedIn(bool didLogin)
{
	if(didLogin) {
		CacheRest::Instance()->getUserIds();
		connect(CacheRest::Instance()->worker, SIGNAL(repliedIds(QString)), this, SLOT(onRepliedIds(QString)));
	}
	else {
		qWarning() << "could not log into the server";
	}
}

void SpinnerWindow::onRepliedIds(QString strData)
{
	qDebug() << strData;
}
