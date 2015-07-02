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
	explicit EvoCacheView(QString userID, QVector<int> onlyLoadHashes = QVector<int>());
	~EvoCacheView();

public slots:
	void handleResults(const QString &) {

	}
	void plotMask(double x, double y, bool isTarget);
	void clearMasks();
	void replotCharts();
	void clearList();
	void onNewSummarizedTree(QJsonObject jsonObj);
	void onUserInjected(User* pUser);
	void plotTargetedTransaction(double x, double y) { plotMask(x, y, true); }
	void plotMatchedTransaction(double x, double y) { plotMask(x, y, false); }


signals:
	void operate(const QString &);

private:
	Ui::EvoCacheView *ui;
	QCustomPlot* m_plotFitness;
};

#endif // MAINWINDOW_EVOCACHEVIEW_H
