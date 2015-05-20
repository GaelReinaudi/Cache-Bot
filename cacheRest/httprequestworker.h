#ifndef HTTPREQUESTWORKER_H
#define HTTPREQUESTWORKER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>

static const QString LoginRoute = "https://cache-heroku.herokuapp.com/login";
static const QString IdsRoute = "https://cache-heroku.herokuapp.com/cache-bot/user_ids";
static const QString UserDataRoute = "https://cache-heroku.herokuapp.com/cache-bot/data";
static const QString SendExtraCashRoute = "https://cache-heroku.herokuapp.com/cache-bot/extracash";

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


class HttpRequestWorker : public QObject {
	Q_OBJECT

public:
	QByteArray response;
	QNetworkReply::NetworkError error_type;
	QString error_str;

	explicit HttpRequestWorker(QObject *parent = 0);

	QString http_attribute_encode(QString attribute_name, QString input);
	QNetworkReply *execute(HttpRequestInput *input);

signals:
	void on_execution_finished(HttpRequestWorker *worker);
	void repliedLogin(QString);
	void repliedIds(QString);
	void repliedUserData(QString);

private:
	QNetworkAccessManager *manager;

public slots:
	void on_manager_finished(QNetworkReply *reply);

private:
	bool m_isLoggedIn = false;
};

#endif // HTTPREQUESTWORKER_H
