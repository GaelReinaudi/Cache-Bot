#ifndef HTTPREQUESTWORKER_H
#define HTTPREQUESTWORKER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>

//static const QString server = "http://127.0.0.1:8080";
static const QString server = "https://cache-heroku.herokuapp.com";
static const QString amazonServer = "http://ec2-52-24-203-78.us-west-2.compute.amazonaws.com:8081";
static const QString LoginRoute = server + "/login";
static const QString IdsRoute = server + "/cache-bot/user_ids";
static const QString UserDataRoute = server + "/cache-bot/data";
static const QString SendExtraCashRoute = server + "/api/cache-bot/user/%1/flow";
static const QString SendNewBotRoute = server + "/cache-bot/newBot";
static const QString BestBotRoute = server + "/cache-bot/bestBot";

static const QString ExtraCashEC2Compute = "https://qvg5wdby95.execute-api.us-west-2.amazonaws.com/prod/extraCash001";

static const QString StringLoggedInReplySuccess = "Moved Temporarily. Redirecting to /loginSuccess";
static const QString StringLoggedInReplyFailure = "Moved Temporarily. Redirecting to /loginFailure";

enum HttpRequestVarLayout {NOT_SET, ADDRESS, URL_ENCODED, MULTIPART, JSON};


class HttpRequestInputFileElement {

public:
	QString variable_name;
	QString local_filename;
	QString request_filename;
	QString mime_type;

};


class HttpRequestInput {

public:
	QString url_str;
	QString http_method;
	HttpRequestVarLayout var_layout;
	QMap<QString, QString> vars;
	QList<HttpRequestInputFileElement> files;
	QJsonObject jsonObject;

	HttpRequestInput();
	HttpRequestInput(QString v_url_str, QString v_http_method);
	void initialize();
	void add_var(QString key, QString value);
	void add_file(QString variable_name, QString local_filename, QString request_filename, QString mime_type);
	void add_json(QJsonObject json) {
		jsonObject = json;
	}

};


class HttpRequestWorker : public QObject
{
	Q_OBJECT

public:
	explicit HttpRequestWorker(QObject *parent = 0);

	QByteArray response;
	QNetworkReply::NetworkError error_type;
	QString error_str;

	QString http_attribute_encode(QString attribute_name, QString input);
	QNetworkReply *execute(HttpRequestInput *input);

signals:
	void on_execution_finished(HttpRequestWorker *worker);
	void repliedLogin(QString);
	void loggedIn(bool);
	void repliedIds(QString);
	void repliedUserData(QString);
	void repliedSendExtraCache(QString);
	void repliedSendNewBot(QString);
	void repliedBestBot(QString);

	void repliedExtraCashEC2Computation(QString);

private:
	QNetworkAccessManager *manager;
	QByteArray m_jsonByte;

public slots:
	virtual void on_manager_finished(QNetworkReply *reply);

private:
	bool m_isLoggedIn = false;
};

class OfflineHttpRequestWorker : public HttpRequestWorker
{
public:
	explicit OfflineHttpRequestWorker(QObject *parent = 0)
		: HttpRequestWorker(parent)
	{}
	void on_manager_finished(QNetworkReply *reply) override;
};

#endif // HTTPREQUESTWORKER_H
