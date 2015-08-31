#ifndef HISTOMETRIC_H
#define HISTOMETRIC_H
#include "core_global.h"
#include "log.h"
//#include "common.h"
#include <QtCore>
#include <QDebug>

class CORESHARED_EXPORT HistoMetric : public QObject
{
protected:
	HistoMetric(const QString& name, QObject* parent = 0)
		: QObject(parent)
		, m_name(name)
	{
		s_AllMetrics.insert(name, this);
		qDebug() << "making HistoMetric:" << m_name;
		NOTICE() << "making HistoMetric:" << m_name;
		}
	virtual ~HistoMetric() {
		s_AllMetrics.remove(m_name);
	}
public:
	static QStringList allNames() {
		return s_AllMetrics.keys();
	}
	//! static repo of all metrics made so far
	static HistoMetric* get(const QString& withName) {
		if (s_AllMetrics.contains(withName)) {
			return s_AllMetrics.value(withName, 0);
		}
		qDebug() << "HistoMetric::get() couldn't find" << withName;
		NOTICE() << "HistoMetric::get() couldn't find" << withName;
		return 0;
	}
	QString name() const { return m_name; }

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
		DBG() << QString("value %1(%2): %3").arg(m_name).arg(date.toString()).arg(val);

		return val;
	}
	bool isValid(const QDate& date) {
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

template <int DayPast>
class MetricSmoother : public HistoMetric
{
public:
	MetricSmoother(HistoMetric* pMetric)
		: HistoMetric(Name(pMetric), pMetric)
		, m_pMetric(pMetric)
	{
	}
	static QString Name(HistoMetric* pMetric) {
		return QString("Smooth%1_%2").arg(DayPast).arg(pMetric->name());
	}
	static MetricSmoother<DayPast>* get(HistoMetric* pMetric) {
		auto pMet = HistoMetric::get(Name(pMetric));
		if (pMet)
			return reinterpret_cast<MetricSmoother<DayPast>*>(pMet);
		return new MetricSmoother<DayPast>(pMetric);
	}
protected:
	double computeFor(const QDate& date, bool& isValid) override {
		double avg = 0.0;
		isValid = true;
		for (int i = 0; i < DayPast; ++i) {
			QDate ad = date.addDays(-i);
			avg += m_pMetric->value(ad);
			isValid &= m_pMetric->isValid(ad);
			//qDebug() << i << avg;
			if (!isValid) {
				WARN() << "m_pMetric" << i << " not valid";
				break;
			}
		}
		avg /= DayPast;
		return avg;
	}

private:

	HistoMetric* m_pMetric = 0;
};

#endif // HISTOMETRIC_H
