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
	explicit Evolver(QString userID, QString onlyAccount = "");
	~Evolver();

	void init();

protected:
	void onUserInjected(User* pUser) override;
	void onRepliedSendNewBot(QString strData) override;

protected slots:
	void onFinishedEvolution(QJsonObject finalBotObject = QJsonObject());

signals:
	void initialized(bool);
	void startStopEvolution(bool doStart);
	void sendMask(double, double, bool);
	void summarizingTree();
	void sendClearList();
	void newSummarizedTree(QJsonObject);

private:
	QThread* m_evoThread;
	EvolutionSpinner* m_evoSpinner;
};

#endif // EVOLVER_H
