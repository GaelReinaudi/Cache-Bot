#ifndef MAINWINDOW_EVOCACHEVIEW_H
#define MAINWINDOW_EVOCACHEVIEW_H

#include <QMainWindow>
#include <QThread>
#include "ACChart/acustomplot.h"
#include "AmortiChart/AMPlot.h"
#include "EvolutionSpinner.h"

class EvolutionSpinner;

namespace Ui {
class EvoCacheView;
}

class EvoCacheView : public QMainWindow
{
	Q_OBJECT

public:
	explicit EvoCacheView(QString userID, QJsonObject jsonArgs);
	~EvoCacheView();

public slots:
	void handleResults(const QString &) {

	}
	void plotMask(double x, double y, int flag = 0);
	void clearMasks();
	void replotCharts();
	void clearList();
	void onNewSummarizedTree(QJsonObject jsonObj);
	void onUserInjected(User* pUser);

signals:
	void operate(const QString &);

private:
	Ui::EvoCacheView *ui;
	QCustomPlot* m_plotFitness;
};

#endif // MAINWINDOW_EVOCACHEVIEW_H
