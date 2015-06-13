#include "extraCache.h"
#include "account.h"
#include "cacherest.h"
#include "fund.h"

ExtraCache::ExtraCache(QString userID)
	: CacheAccountConnector(userID)
{
}

void ExtraCache::onUserInjected(User* pUser)
{
	CacheAccountConnector::onUserInjected(pUser);
	CacheRest::Instance()->getBestBot(userID(), user());
}

void ExtraCache::onBotInjected()
{
	LOG() << "costLiving(50/75/90/95/99) "
		  << user()->costLiving(0.50)
		  << user()->costLiving(0.75)
		  << user()->costLiving(0.90)
		  << user()->costLiving(0.95)
		  << user()->costLiving(0.99)
		  << endl;

	// transaction at the starting date of the playback
	auto& real = user()->allTrans();
	m_date = real.lastTransactionDate();//.addDays(-playBackStartAgo);
	m_d0 = m_date.toJulianDay();
	qDebug() << "m_date" << m_date << "jD0" << m_d0;

	for (int i = 0; i < real.count(); ++i) {
		if (real.trans(i).date >= m_date) {
			//m_ipb = i;
			qDebug() << "initial trans("<<i<<")" << real.trans(i).date << real.trans(i).name;
			break;
		}
	}

	double m_lastBal = user()->balance(Account::Type::Checking);
	qDebug() << "m_lastBal" << m_lastBal << " (Un/Ch/Sa/Cr"
			 << user()->balance(Account::Type::Unknown)
			 << user()->balance(Account::Type::Checking)
			 << user()->balance(Account::Type::Saving)
			 << user()->balance(Account::Type::Credit)
			 << " )";

	// some arbitrary slush need to (try to) never go under of
	m_slushFundTypicalNeed = 0.5 * user()->balance(Account::Type::Checking);
//	m_slushFundTypicalNeed = 0.5 * user()->balance(Account::Type::Checking);
	m_slushFundTypicalNeed = 0.5 * user()->costLiving(0.75);

	// the amount of money on the extraCash fund already
	Fund* extraFund = user()->extraCacheFund();
	double extraTotal = 0.0;
	for (Cash& c : extraFund->cashes()) {
		extraTotal += c.amount;
	}
	m_slushFundStartsAt = extraTotal;
	qDebug() << "extraTotal" << m_slushFundStartsAt;

	double threshProba = 1.0;
	m_spark = user()->predictedSparkLine(threshProba);
	computeMinSlopeOver(60);

	int addDay = 1;
	double m_extraToday = -1.0;
	double posSlope = qMax(0.0, m_minSlope);
	double extraToday = posSlope * addDay / 2.0;
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

	CacheRest::Instance(0)->sendExtraCash(user()->id(), m_extraToday);
}

void ExtraCache::onRepliedExtraCache(QString strData)
{
	CacheAccountConnector::onRepliedExtraCache(strData);
	qApp->exit();
}

int ExtraCache::computeMinSlopeOver(int numDays)
{
	m_minSlope = 9999.9;
	int dayMin = -1;
	double tToday = m_date.toJulianDay() - m_d0;
	double yToday = m_slushFundStartsAt;
	SparkLine* pDat = &m_spark;
	SparkLine::iterator it = pDat->begin();
	// first value is the last known balance;
	double balanceNow = it.value();

	while(it != pDat->end() && it.key() < tToday + numDays + 1) {
		double futDay = it.key();
		double balanceThen = it.value();
		if (futDay > 0) {
			double effectiveSlushforDay = m_slushFundTypicalNeed * (0.5 + 1.0 * (futDay) / 30.0);
			double y = balanceThen - effectiveSlushforDay;
			double slope = (y - yToday) / qMax(1.0, futDay);
			if(slope < m_minSlope) {
				m_minSlope = slope;
				dayMin = futDay;
			}
		}
		++it;
	}
	if (m_minSlope == 9999.9) {
		double effectiveSlushforDay = m_slushFundTypicalNeed * (0.5 + 1.0 * (numDays) / 30.0);
		m_minSlope = (balanceNow - effectiveSlushforDay - m_slushFundStartsAt) / numDays;// / 2.0;
		dayMin = tToday + numDays;
	}
	LOG() << "computeMinSlopeOver(" << numDays << ") = [$/d]" << m_minSlope << "dayMin" << dayMin << endl;
	return dayMin;
}
