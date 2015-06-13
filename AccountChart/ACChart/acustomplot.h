#ifndef ACUSTOMPLOT_H
#define ACUSTOMPLOT_H

#include "../qcustomplot/qcustomplot.h"
#include "user.h"

class ACustomPlot : public QCustomPlot
{
	Q_OBJECT
public:
	explicit ACustomPlot(QWidget *parent = 0);

	void loadCompressedAmount(User *pUser);
	void loadAmount(User* pUser);
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

private:
	double m_integral = 0.0;
	QMap<uint, QCPGraph*> m_hashGraphs;
	QStringList m_labels;
	HashedBundles m_hashBund;
};

#endif // ACUSTOMPLOT_H
