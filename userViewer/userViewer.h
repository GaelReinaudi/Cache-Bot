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
	explicit UserViewer(QString userID, QVector<int> onlyLoadHashes = QVector<int>());
	~UserViewer();

	User* user() const;

public slots:
	void handleResults(const QString &) {

	}
	void plotMask(double x, double y, int flag = 0);
	void clearMasks();
	void replotCharts();
	void onNewSummarizedTree(QJsonObject jsonObj);
	void onUserInjected(User* pUser);
	void onBotInjected(Bot *bestBot);
	void onAgo();

signals:
	void operate(const QString &);

private:
	Ui::UserViewer *ui;
	CacheAccountConnector* m_pConnector = 0;
};

#endif // MAINWINDOW_H
