#include "extraCache.h"
#include "account.h"
#include "cacherest.h"

extraCache::extraCache(QString userID)
	: CacheAccountConnector(userID)
{
}

void extraCache::onUserInjected()
{
	CacheAccountConnector::onUserInjected();
	CacheRest::Instance()->getBestBot(userID(), user());
}

void extraCache::onBotInjected()
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
	qDebug() << "m_date" << m_date;

	for (int i = 0; i < real.count(); ++i) {
		if (real.trans(i).date >= m_date) {
			//m_ipb = i;
			qDebug() << "initial trans("<<i<<")" << real.trans(i).date << real.trans(i).name;
			break;
		}
	}


	qApp->exit();
}
