#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include "ACChart/acustomplot.h"
#include "AmortiChart/AMPlot.h"

class User;
class CacheAccountConnector;

namespace Ui {
class UserViewer;
}

class UserViewer : public QMainWindow
{
	Q_OBJECT

public:
	explicit UserViewer(QString userID);
	~UserViewer();

	User* user() const;

public slots:
	void handleResults(const QString &) {

	}
	void plotMask(double x, double y, bool isTarget);
	void clearMasks();
	void replotCharts();
	void onNewSummarizedTree(QJsonObject jsonObj);
	void onUserInjected(User* pUser);
	void onBotInjected(Bot *bestBot);
	void plotTargetedTransaction(double x, double y) { plotMask(x, y, true); }
	void plotMatchedTransaction(double x, double y) { plotMask(x, y, false); }


signals:
	void operate(const QString &);

private:
	Ui::UserViewer *ui;
	CacheAccountConnector* m_pConnector = 0;
};

#endif // MAINWINDOW_H