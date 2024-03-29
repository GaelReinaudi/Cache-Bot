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
	BotContext *context() const {
		return m_context;
	}

public slots:
	void startStopEvolution(bool doStart);
	void runEvolution();

protected:
	void makeSuperTreeMixtures(std::vector<Puppy::Tree> &ioPopulation, Puppy::Context &ioContext);
	void replaceFitness0WithSuperMixture(std::vector<Puppy::Tree>& ioPopulation, const QList<Puppy::Tree> &popFeatures, Puppy::Context &ioContext);
	double evaluateSymbReg(std::vector<Puppy::Tree> &ioPopulation, Puppy::Context &ioContext);
	QJsonObject summarize(Puppy::Tree &tree);

signals:
	void initialized(bool);
	void finishedEvolution(QJsonObject);

private:
	BotContext* m_context = 0;
	int m_gen = 0;
	volatile bool m_doSpin = false;
};

//Q_DECLARE_METATYPE(ZoneVector)

#endif // EVOLUTIONSPINNER_H
