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

	QRectF mapDayAgoToPlot(QRectF rectDayAgo) const;

signals:

public slots:

private:
	enum Mode{linear, logKinda} m_mode;
	QDate m_lastDate;
};

#endif // ACUSTOMPLOT_H
