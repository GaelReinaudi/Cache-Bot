#include "cacheConnect.h"
#include "cacherest.h"
#include <QTimer>

CacheConnect::CacheConnect(QString url)
	: CacheAccountConnector(QString(), QJsonObject())
	, m_url(url)
{
	connect(CacheRest::Instance()->worker, &HttpRequestWorker::on_execution_finished, this, &CacheConnect::onFinished, Qt::QueuedConnection);
	QTimer::singleShot(60000, qApp, SLOT(quit()));
}

void CacheConnect::onLoggedIn(bool didLogin)
{
//	QString url = "https://cache-heroku.herokuapp.com:443/api/admin/run_nightly_update/pull_plaid/true";
	if (!didLogin) {
		ERR() << "could not login";
		return;
	}
	QJsonObject args;
	CacheRest::Instance()->hitUrl(m_url, args);
}

void CacheConnect::onFinished(HttpRequestWorker*)
{
	NOTICE() << "onFinished";
//	qApp->exit(0);
}

