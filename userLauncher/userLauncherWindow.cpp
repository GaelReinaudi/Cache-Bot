#include "userLauncherwindow.h"
#include "ui_userLauncherwindow.h"
#include <QFile>
#include <QJsonDocument>
#include <QPushButton>
#include "flowWidget.h"
#include <QDebug>

SpinnerWindow::SpinnerWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::SpinnerWindow)
{
	ui->setupUi(this);

	QString userIdPath = "jsonAllUserIds.json";
	QFile file(userIdPath);
	file.open(QFile::ReadOnly);
	QString jsonStr = file.readAll();
	m_UserIds = QJsonDocument::fromJson(jsonStr.toUtf8()).object();

	QMap<QString, QString> mapFirstName_Id;
	for (const QString& k : m_UserIds.keys()) {
		mapFirstName_Id.insert(m_UserIds[k].toObject()["firstName"].toString(), k);
	}
	for (const QString& f : mapFirstName_Id.keys()) {
		QString k = mapFirstName_Id[f].trimmed();
		if (k == "55518f01574600030092a822")
			continue;
		qDebug() << k;
		QListWidgetItem* item = new QListWidgetItem();
		ui->listWidget->addItem(item);
		item->setSizeHint(QSize(100, 25));
		ui->listWidget->setItemWidget(item, new FlowWidget(k, m_UserIds[k].toObject()));
	}
}

SpinnerWindow::~SpinnerWindow()
{
	delete ui;
}
