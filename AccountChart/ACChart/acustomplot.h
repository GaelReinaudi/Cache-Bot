#ifndef ACUSTOMPLOT_H
#define ACUSTOMPLOT_H

#include "../qcustomplot/qcustomplot.h"
#include "user.h"

class ACustomPlot : public QCustomPlot
{
	Q_OBJECT
public:
	ACustomPlot(QWidget *parent = 0);

	virtual void loadCompressedAmount(User *pUser);
	QList<uint> hashKeys() {
		return m_hashGraphs.keys();
	}

protected:
	void makeGraphs(HashedBundles& hashBundles);

signals:
	void newLabel(QString);
	void newSum(double);
	void newHashValue(int);

public slots:
	void showHash(int ithLayer);

protected:
	double m_integral = 0.0;
<<<<<<< HEAD
	QMap<uint, QCPGraph*> m_hashGraphs;
    QCPGraph* m_integralLineGraph;
=======
	QMap<uint, QList<QCPGraph*> > m_hashGraphs;
>>>>>>> gr-dev
	QStringList m_labels;
	HashedBundles m_hashBund;
};

class AHashPlot : public ACustomPlot
{
public:
	AHashPlot(QWidget *parent = 0);

	void loadCompressedAmount(User *pUser) override;
		void histogramGraph(int indGr);
};

#endif // ACUSTOMPLOT_H
