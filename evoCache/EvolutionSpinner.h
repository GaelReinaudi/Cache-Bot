#ifndef EVOLUTIONSPINNER_H
#define EVOLUTIONSPINNER_H
#include <QObject>
#include <QRectF>

#include "bot.h"

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

	void init(User *pUser);
public slots:
	void startStopEvolution(bool doStart);
	void runEvolution();

protected:
	unsigned int evaluateSymbReg(std::vector<Puppy::Tree> &ioPopulation, Puppy::Context &ioContext);
	QJsonObject summarize(Puppy::Tree &tree);
	QVector<Transaction> predictTrans(Puppy::Tree &tree, double threshProba);

signals:
	void initialized(bool);
	void finishedEvolution(QJsonObject);
	void needsReplot();
	void summarizingTree();
	void newSummarizedTree(QJsonObject);

private:
	BotContext* m_context = 0;
	int m_gen = 0;
	volatile bool m_doSpin = false;
};

//Q_DECLARE_METATYPE(ZoneVector)

#endif // EVOLUTIONSPINNER_H
