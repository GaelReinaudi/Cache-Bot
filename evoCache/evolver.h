#ifndef EVOLVER_H
#define EVOLVER_H

#include "EvolutionSpinner.h"
#include "cacheAccountConnector.h"
#include <QThread>

class EvolutionSpinner;

class Evolver : public CacheAccountConnector
{
	Q_OBJECT

public:
	explicit Evolver(QString jsonFile, int afterJday, int beforeJday);
	explicit Evolver(QString userID);
	~Evolver();

	void init();

protected:
	void onRepliedUserData(QString strData) override;
	void onRepliedSendNewBot(QString strData) override;

protected slots:
	void onFinishedEvolution(QJsonObject finalBotObject);

signals:
	void initialized(bool);

private:
	QThread* m_evoThread;
	EvolutionSpinner* m_evoSpinner;
};

#endif // EVOLVER_H
