#include "extraCache.h"
#include "account.h"
#include "bot.h"
#include "oracle.h"
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
	m_date = user()->currentDay();
	LOG() << "m_date" << m_date.toString() << endl;

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

	QJsonObject statObj = bestBot->postTreatment();

	double flow = user()->oracle()->avgCashFlow();
	QJsonObject flowObj;
	flowObj.insert("rate", flow);
	flowObj.insert("state", QString("kFlow"));
	statObj.insert("flow", flowObj);

	if (flags & SendExtraCash) {
		CacheRest::Instance()->sendExtraCash(user()->id(), 0.0, statObj);
	}
}

void ExtraCache::onRepliedSendExtraCache(QString strData)
{
	CacheAccountConnector::onRepliedSendExtraCache(strData);
	if(CacheAccountConnector::flags & CacheAccountConnector::AutoExit)
		qApp->exit();
}


