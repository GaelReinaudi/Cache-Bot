#ifndef CACHEREST_H
#define CACHEREST_H

#include "cacherest_global.h"
#include "httprequestworker.h"

class CACHERESTSHARED_EXPORT CacheRest : public QObject
{
public:
	CacheRest(QObject *parent = 0);

public:
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
};

#endif // CACHEREST_H
