#ifndef EVOLUTIONSPINNER_H
#define EVOLUTIONSPINNER_H
#include <QObject>

class EvolutionSpinner : public QObject
{
	Q_OBJECT

public:
	EvolutionSpinner(QObject* parent = 0);
signals:
	void resultReady(const QString &result);

private:
	int m_gen = 0;
};

#endif // EVOLUTIONSPINNER_H
