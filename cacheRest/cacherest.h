#ifndef CACHEREST_H
#define CACHEREST_H

#include "cacherest_global.h"
#include "httprequestworker.h"

// http://www.creativepulse.gr/en/blog/2014/restful-api-requests-using-qt-cpp-for-linux-mac-osx-ms-windows

class CACHERESTSHARED_EXPORT CacheRest : public QObject
{
public:
	CacheRest(QObject *parent = 0);

public:
	void login(QString username, QString password);
	void getUserIds();
	void getUserData(QString userId);

	void testGET() {
		QString url_str = "http://www.example.com/path/to/page.php";

		HttpRequestInput input(url_str, "GET");

		HttpRequestWorker *worker = new HttpRequestWorker(this);
		connect(worker, SIGNAL(on_execution_finished(HttpRequestWorker*)), this, SLOT(handle_result(HttpRequestWorker*)));
		worker->execute(&input);
	}
	void testPOST() {
		QString url_str = "http://www.example.com/path/to/page.php";

		HttpRequestInput input(url_str, "POST");

		input.add_var("key1", "value1");
		input.add_var("key2", "value2");

		HttpRequestWorker *worker = new HttpRequestWorker(this);
		connect(worker, SIGNAL(on_execution_finished(HttpRequestWorker*)), this, SLOT(handle_result(HttpRequestWorker*)));
		worker->execute(&input);
	}

	HttpRequestWorker *worker = 0;
};

#endif // CACHEREST_H
