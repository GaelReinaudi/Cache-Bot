#include "userLauncherwindow.h"
#include "ui_userLauncherwindow.h"
#include <QFile>
#include <QJsonDocument>
#include <QPushButton>
#include "flowWidget.h"

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

	for (const QString& k : m_UserIds.keys()) {
		QListWidgetItem* item = new QListWidgetItem();
		ui->listWidget->insertItem(0, item);
		item->setSizeHint(QSize(100, 25));
		ui->listWidget->setItemWidget(item, new FlowWidget(k, m_UserIds[k].toObject()));
	}
}

SpinnerWindow::~SpinnerWindow()
{
	delete ui;
}
