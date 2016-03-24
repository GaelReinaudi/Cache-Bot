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

	connect(ui->userViewButton, &QPushButton::clicked, [this, id](){
		QString program = "userViewer.exe";
		QStringList arguments;
		arguments << QString("{\"user_id\":\"%1\"}").arg(id);

		QProcess* myProcess = new QProcess(this);
		myProcess->start(program, arguments);
		qDebug() << myProcess->errorString();
	});
}

FlowWidget::~FlowWidget()
{
	delete ui;
}
