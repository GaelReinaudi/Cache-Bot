#include "cacheConnect.h"
#include "cacherest.h"

CacheConnect::CacheConnect()
	: CacheAccountConnector(QString(), QJsonObject())
{
	connect(CacheRest::Instance()->worker, &HttpRequestWorker::on_execution_finished, this, &CacheConnect::onFinished, Qt::QueuedConnection);
}

void CacheConnect::onLoggedIn(bool didLogin)
{
	QString url = "https://cache-heroku.herokuapp.com:443/api/admin/run_nightly_update/pull_plaid/true";
	if (!didLogin) {
		ERR() << "could not login";
		return;
	}
	QJsonObject args;
	CacheRest::Instance()->hitUrl(url, args);
}

void CacheConnect::onFinished(HttpRequestWorker*)
{
	NOTICE() << "onFinished";
	qApp->exit(0);
}

