#include "extraCache.h"
#include "account.h"
#include "bot.h"
#include "cacherest.h"
#include "fund.h"
#include "userMetrics.h"

static const double SLOPE_MULTIPLICATION_EXTRA_TODAY = 0.5;

ExtraCache::ExtraCache(QString userID)
	: CacheAccountConnector(userID)
{
}

void ExtraCache::onUserInjected(User* pUser)
{
	CacheAccountConnector::onUserInjected(pUser);

	// transaction at the starting date of the playback
	auto& real = user()->allTrans();
	m_date = QDate::currentDate();
	m_d0 = m_date.toJulianDay();
	LOG() << "m_date" << m_date.toString() << "jD0" << m_d0 << endl;

	for (int i = 0; i < real.count(); ++i) {
		if (real.trans(i).date >= m_date) {
			//m_ipb = i;
			LOG() << "initial trans("<<i<<")" << real.trans(i).date.toString() << real.trans(i).name << endl;
			break;
		}
	}

	// some arbitrary slush need to (try to) never go under of
	m_slushFundTypicalNeed = 0.0;
	m_slushFundTypicalNeed += 0.5 * user()->balance(Account::Type::Checking);
	m_slushFundTypicalNeed += 0.5 * -CostRateMonthPercentileMetric<6, 75>::get(user())->value(m_date) * 30;

	LOG() << "m_slushFundTypicalNeed" << m_slushFundTypicalNeed << endl;

	// the amount of money on the extraCash fund already
	Fund* extraFund = user()->extraCacheFund();
	double extraTotal = 0.0;
//	for (const Cash& c : extraFund->cashes()) {
//		extraTotal += c.amount;
//	}
	m_slushFundStartsAt = extraTotal;
	LOG() << "extraTotal" << m_slushFundStartsAt << endl;

	CacheRest::Instance()->getBestBot(userID(), user());
}

void ExtraCache::onBotInjected(Bot* bestBot)
{
	LOG() << "ExtraCache::onBotInjected" << endl;
	bestBot->summarize();

	computeMinSlopeOver(30);

	double m_extraToday = -1.0;
	double posSlope = m_minSlope;//qMax(0.0, m_minSlope);
	double extraToday = SLOPE_MULTIPLICATION_EXTRA_TODAY * posSlope;
	if(m_extraToday < 0.0)
		m_extraToday = extraToday;
	if(extraToday > m_extraToday) {
		m_extraToday *= 0.95;
		m_extraToday += 0.05 * extraToday;
	}
	else {
		m_extraToday *= 0.9;
		m_extraToday += 0.1 * extraToday;
	}
	m_slushFundStartsAt += m_extraToday;

	if (flags & SendExtraCash) {
		CacheRest::Instance()->sendExtraCash(user()->id(), m_extraToday, bestBot->lastStats());
	}
}

void ExtraCache::onRepliedSendExtraCache(QString strData)
{
	CacheAccountConnector::onRepliedSendExtraCache(strData);
	if(CacheAccountConnector::flags & CacheAccountConnector::AutoExit)
		qApp->exit();
}

void ExtraCache::computeMinSlopeOver(int numDays)
{
	m_minSlope = 9999.9;
	int dayMin = -1;
	double tToday = m_date.toJulianDay() - m_d0;
	SparkLine* pDat = &m_spark;
	SparkLine::iterator it = pDat->begin();
	// first value is the last known balance;
	double balanceNow = it.value();

	double addIncomePerDay = MakeRateMonthPercentileMetric<6, 75>::get(user())->value(m_date);
	double futDay = numDays;
	double balanceThen = balanceNow + futDay * addIncomePerDay;

	qDebug() << "balanceNow" << balanceNow << "addIncomePerDay" << addIncomePerDay << "m_slushFundTypicalNeed" << m_slushFundTypicalNeed << "m_slushFundStartsAt" << m_slushFundStartsAt;
	while(it != pDat->end() && it.key() < tToday + numDays + 1) {
		double futDay = it.key();
		double balanceThen = it.value() + futDay * addIncomePerDay;
		if (futDay > 0) {
			double effectiveSlushforDay = m_slushFundTypicalNeed * (0.5 + 1.0 * (futDay) / 30.0);
			double effectY = balanceThen - effectiveSlushforDay;
			double slope = (effectY - m_slushFundStartsAt) / qMax(1.0, futDay);
			qDebug() << "futDay" << futDay << "balanceThen" << balanceThen << "    effectiveSlushforDay" << effectiveSlushforDay << "effectY" << effectY << "(effectY - yToday)" << (effectY - m_slushFundStartsAt);
			if(slope < m_minSlope) {
				m_minSlope = slope;
				dayMin = futDay;
			}
			qDebug() << "    slope" << slope << "m_minSlope" << m_minSlope;
		}
		++it;
	}
	if (m_minSlope == 9999.9) {
		double effectiveSlushforDay = m_slushFundTypicalNeed * (0.5 + 1.0 * (futDay) / 30.0);
		double effectY = balanceThen - effectiveSlushforDay;
		m_minSlope = (effectY - m_slushFundStartsAt) / futDay;// / 2.0;
		dayMin = tToday + futDay;
		qDebug() << "futDay:=" << 30 << "balanceThen" << balanceThen << "    effectiveSlushforDay" << effectiveSlushforDay << "effectY" << effectY << "(effectY - yToday)" << (effectY - m_slushFundStartsAt);
		qDebug() << "    m_minSlope" << m_minSlope;
	}
	LOG() << "computeMinSlopeOver(" << numDays << ") = [$/d]" << m_minSlope << "dayMin" << dayMin << endl;
	m_futDayMinSlopeCollision = dayMin;
}


