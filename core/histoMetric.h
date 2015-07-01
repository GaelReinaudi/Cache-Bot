#ifndef HISTOMETRIC_H
#define HISTOMETRIC_H
#include "core_global.h"
//#include "common.h"
#include <QtCore>

class CORESHARED_EXPORT HistoMetric
{
public:
	HistoMetric(const QString& name)
		: m_name(name)
	{
		s_AllMetrics.insert(name, this);
	}
	virtual ~HistoMetric() {}
	//! static repo of all metrics made so far
	static HistoMetric* get(const QString& withName) {
		return s_AllMetrics.value(withName, 0);
	}

public:
	double value(const QDate& date) {
		if (m_values.contains(date)) {
			return m_values[date];
		}
		// else we compute it
		bool isValid = false;
		double val = computeFor(date, isValid);
		m_values[date] = val;
		m_valid[date] = isValid;
		return val;
	}
	bool valid(const QDate& date) {
		if (m_values.contains(date)) {
			return m_valid[date];
		}
		value(date);
		return m_valid[date];
	}

protected:
	virtual double computeFor(const QDate& date, bool& isValid) = 0;

private:
	QMap<QDate, double> m_values;
	QMap<QDate, bool> m_valid;
	QString m_name;

	static QMap<QString, HistoMetric*> s_AllMetrics;
};

#endif // HISTOMETRIC_H
