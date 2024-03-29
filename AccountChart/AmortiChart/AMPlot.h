#ifndef AMPLOT_H
#define AMPLOT_H

#include "../qcustomplot/qcustomplot.h"
class User;

class AMPlot : public QCustomPlot
{
	Q_OBJECT
public:
	explicit AMPlot(QWidget *parent = 0);

	void loadAmount(User* pUser);

signals:

public slots:

private:
	QCPColorMap *colorMap = 0;
};

#endif // AMPLOT_H
