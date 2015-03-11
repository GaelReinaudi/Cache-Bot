#ifndef ACUSTOMPLOT_H
#define ACUSTOMPLOT_H

#include "../qcustomplot/qcustomplot.h"
class Account;

class ACustomPlot : public QCustomPlot
{
	Q_OBJECT
public:
	explicit ACustomPlot(QWidget *parent = 0);

	void loadCompressedAmount(Account *account);
	void loadAmount(Account *account);

signals:

public slots:

private:
	enum Mode{linear, logKinda} m_mode;
	double m_integral = 0.0;
};

#endif // ACUSTOMPLOT_H
