#ifndef AMPLOT_H
#define AMPLOT_H

#include "../qcustomplot/qcustomplot.h"
class Account;

class AMPlot : public QCustomPlot
{
	Q_OBJECT
public:
	explicit AMPlot(QWidget *parent = 0);

	void loadAmount(Account *account);

signals:

public slots:

private:
	QCPColorMap *colorMap = 0;
	QDate m_lastDate;
	QDate m_firstDate;
};

#endif // AMPLOT_H
