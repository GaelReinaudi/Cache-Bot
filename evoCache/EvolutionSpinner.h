#ifndef EVOLUTIONSPINNER_H
#define EVOLUTIONSPINNER_H
#include <QObject>
#include <QRectF>

#include "puppy/Puppy.hpp"

class Zone : public QRectF
{

public:
	bool m_isFilled = false;
};

typedef QVector<Zone> ZoneVector;

class EvolutionSpinner : public QObject
{
	Q_OBJECT

public:
	EvolutionSpinner(QObject* parent = 0);

	void init(Account *pAc);
public slots:
	void startStopEvolution(bool doStart);
	void runEvolution();

protected:
	unsigned int evaluateSymbReg(std::vector<Puppy::Tree> &ioPopulation, Puppy::Context &ioContext);
	QString summarize(Puppy::Tree &tree);
	QVector<Transaction> predictTrans(Puppy::Tree &tree, double threshProba);

signals:
	void resultReady(const QString &result);
	void sendMask(double, double, bool);
	void sendClearMask();
	void needsReplot();
	void sendClearList();
	void newList(QStringList strList);

private:
	Puppy::Context* m_context = 0;
	int m_gen = 0;
	volatile bool m_doSpin = false;
};

//Q_DECLARE_METATYPE(ZoneVector)

#endif // EVOLUTIONSPINNER_H
