#ifndef EVOLVER_H
#define EVOLVER_H

#include <QThread>
#include "EvolutionSpinner.h"

class EvolutionSpinner;

class Evolver : public QObject
{
	Q_OBJECT

public:
	explicit Evolver(QString jsonFile, int afterJday, int beforeJday);
	explicit Evolver(QString userID);
	~Evolver() {}

	void init();
public slots:
	void onLoggedIn(bool didLogin);
	void onRepliedUserData(QString strData);

signals:
	void initialized(bool);

private:
	QThread* m_evoThread;
	EvolutionSpinner* m_evoSpinner;
	Account* m_account;
	QString m_userId;
};

#endif // EVOLVER_H
