#include "flowWidget.h"
#include "ui_flowWidget.h"
#include <QProcess>
#include <QDebug>

FlowWidget::FlowWidget(QString id, QJsonObject userObj, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::FlowWidget),
	m_obj(userObj)
{
	ui->setupUi(this);

	ui->firstName->setText(m_obj["firstName"].toString());
	ui->lastName->setText(m_obj["lastName"].toString());

	QStringList args;
	args << QString("{\"user_id\":\"%1\"}").arg(id);

	connect(ui->userViewButton, &QPushButton::clicked, [this, id, args](){
		QProcess* myProcess = new QProcess(this);
		myProcess->start("userViewer.exe", args);
		qDebug() << myProcess->errorString();
	});
	connect(ui->evoButton, &QPushButton::clicked, [this, id, args](){
		QProcess* myProcess = new QProcess(this);
		myProcess->start("evoCacheView.exe", args);
		qDebug() << myProcess->errorString();
	});
	connect(ui->flowButton, &QPushButton::clicked, [this, id, args](){
		QProcess* myProcess = new QProcess(this);
		myProcess->start("extraCash.exe", args);
		qDebug() << myProcess->errorString();
	});
	connect(ui->cacheLightButton, &QPushButton::clicked, [this, id, args](){
		QProcess* myProcess = new QProcess(this);
		myProcess->start("cacheLight.exe", args);
		qDebug() << myProcess->errorString();
	});
}

FlowWidget::~FlowWidget()
{
	delete ui;
}
