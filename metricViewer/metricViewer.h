#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include "ACChart/acustomplot.h"

class User;
class CacheAccountConnector;

namespace Ui {
class MetricViewer;
}

class MetricViewer : public QMainWindow
{
	Q_OBJECT

public:
	explicit MetricViewer(QString userID);
	~MetricViewer();

	User* user() const;

public slots:
	void onUserInjected(User* pUser);
	void onBotInjected(Bot *bestBot);
	void onHoverListIndex(QModelIndex index);

private:
	Ui::MetricViewer *ui;
	CacheAccountConnector* m_pConnector = 0;
};

#endif // MAINWINDOW_H
