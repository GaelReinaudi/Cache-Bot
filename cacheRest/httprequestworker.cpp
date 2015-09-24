#include "httprequestworker.h"
#include "log.h"
#include <QDateTime>
#include <QUrl>
#include <QFileInfo>
#include <QBuffer>
#include <QNetworkCookieJar>


HttpRequestInput::HttpRequestInput() {
	initialize();
}

HttpRequestInput::HttpRequestInput(QString v_url_str, QString v_http_method) {
	initialize();
	url_str = v_url_str;
	http_method = v_http_method;
}

void HttpRequestInput::initialize() {
	var_layout = NOT_SET;
	url_str = "";
	http_method = "GET";
}

void HttpRequestInput::add_var(QString key, QString value) {
	vars[key] = value;
}

void HttpRequestInput::add_file(QString variable_name, QString local_filename, QString request_filename, QString mime_type) {
	HttpRequestInputFileElement file;
	file.variable_name = variable_name;
	file.local_filename = local_filename;
	file.request_filename = request_filename;
	file.mime_type = mime_type;
	files.append(file);
}


HttpRequestWorker::HttpRequestWorker(QObject *parent)
	: QObject(parent), manager(NULL)
{
	qsrand(QDateTime::currentDateTime().toTime_t());

	// curl -ipv4 --insecure --cookie-jar jarfile -d "email=gael.reinaudi@gmail.com&password=wwwwwwww" -X POST https://cache-heroku.herokuapp.com/login
	// curl -ipv4 --insecure --cookie jarfile -H "Accept: application/json" -X GET https://cache-heroku.herokuapp.com:443/bank/f202f5004003ff51b7cc7e60523b7a43d541b38246c4abc0b765306e977126540f731d94478de121c44d5c214382d36cb3c1f3c4e117a532fc78a8b078c320bb24f671bbd0199ea599c15349d2b3d820
	manager = new QNetworkAccessManager(this);
	QNetworkCookieJar* cookieJar = new QNetworkCookieJar(0);
	manager->setCookieJar(cookieJar);
	connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(on_manager_finished(QNetworkReply*)));
}

QString HttpRequestWorker::http_attribute_encode(QString attribute_name, QString input) {
	// result structure follows RFC 5987
	bool need_utf_encoding = false;
	QString result = "";
	QByteArray input_c = input.toLocal8Bit();
	char c;
	for (int i = 0; i < input_c.length(); i++) {
		c = input_c.at(i);
		if (c == '\\' || c == '/' || c == '\0' || c < ' ' || c > '~') {
			// ignore and request utf-8 version
			need_utf_encoding = true;
		}
		else if (c == '"') {
			result += "\\\"";
		}
		else {
			result += c;
		}
	}

	if (result.length() == 0) {
		need_utf_encoding = true;
	}

	if (!need_utf_encoding) {
		// return simple version
		return QString("%1=\"%2\"").arg(attribute_name, result);
	}

	QString result_utf8 = "";
	for (int i = 0; i < input_c.length(); i++) {
		c = input_c.at(i);
		if (
			(c >= '0' && c <= '9')
			|| (c >= 'A' && c <= 'Z')
			|| (c >= 'a' && c <= 'z')
		) {
			result_utf8 += c;
		}
		else {
			result_utf8 += "%" + QString::number(static_cast<unsigned char>(input_c.at(i)), 16).toUpper();
		}
	}

	// return enhanced version with UTF-8 support
	return QString("%1=\"%2\"; %1*=utf-8''%3").arg(attribute_name, result, result_utf8);
}

QNetworkReply* HttpRequestWorker::execute(HttpRequestInput *input) {

	// reset variables

	QByteArray request_content = "";
	response = "";
	error_type = QNetworkReply::NoError;
	error_str = "";


	// decide on the variable layout

	if (input->files.length() > 0) {
		input->var_layout = MULTIPART;
	}
	if (input->var_layout == NOT_SET) {
		input->var_layout = input->http_method == "GET" || input->http_method == "HEAD" ? ADDRESS : URL_ENCODED;
	}
	if (!input->jsonObject.isEmpty()) {
		input->var_layout = JSON;
	}


	// prepare request content

	QString boundary = "";

	if (input->var_layout == ADDRESS || input->var_layout == URL_ENCODED) {
		// variable layout is ADDRESS or URL_ENCODED

		if (input->vars.count() > 0) {
			bool first = true;
			foreach (QString key, input->vars.keys()) {
				if (!first) {
					request_content.append("&");
				}
				first = false;

				request_content.append(QUrl::toPercentEncoding(key));
				request_content.append("=");
				request_content.append(QUrl::toPercentEncoding(input->vars.value(key)));
			}

			if (input->var_layout == ADDRESS) {
				input->url_str += "?" + request_content;
				request_content = "";
			}
		}
	}
	else if (input->var_layout == MULTIPART) {
		boundary = "__-----------------------"
			+ QString::number(QDateTime::currentDateTime().toTime_t())
			+ QString::number(qrand());
		QString boundary_delimiter = "--";
		QString new_line = "\r\n";

		// add variables
		foreach (QString key, input->vars.keys()) {
			// add boundary
			request_content.append(boundary_delimiter);
			request_content.append(boundary);
			request_content.append(new_line);

			// add header
			request_content.append("Content-Disposition: form-data; ");
			request_content.append(http_attribute_encode("name", key));
			request_content.append(new_line);
			request_content.append("Content-Type: text/plain");
			request_content.append(new_line);

			// add header to body splitter
			request_content.append(new_line);

			// add variable content
			request_content.append(input->vars.value(key));
			request_content.append(new_line);
		}

		// add files
		for (QList<HttpRequestInputFileElement>::iterator file_info = input->files.begin(); file_info != input->files.end(); file_info++) {
			QFileInfo fi(file_info->local_filename);

			// ensure necessary variables are available
			if (
				file_info->local_filename == NULL || file_info->local_filename.isEmpty()
				|| file_info->variable_name == NULL || file_info->variable_name.isEmpty()
				|| !fi.exists() || !fi.isFile() || !fi.isReadable()
			) {
				// silent abort for the current file
				continue;
			}

			QFile file(file_info->local_filename);
			if (!file.open(QIODevice::ReadOnly)) {
				// silent abort for the current file
				continue;
			}

			// ensure filename for the request
			if (file_info->request_filename == NULL || file_info->request_filename.isEmpty()) {
				file_info->request_filename = fi.fileName();
				if (file_info->request_filename.isEmpty()) {
					file_info->request_filename = "file";
				}
			}

			// add boundary
			request_content.append(boundary_delimiter);
			request_content.append(boundary);
			request_content.append(new_line);

			// add header
			request_content.append(QString("Content-Disposition: form-data; %1; %2").arg(
				http_attribute_encode("name", file_info->variable_name),
				http_attribute_encode("filename", file_info->request_filename)
			));
			request_content.append(new_line);

			if (file_info->mime_type != NULL && !file_info->mime_type.isEmpty()) {
				request_content.append("Content-Type: ");
				request_content.append(file_info->mime_type);
				request_content.append(new_line);
			}

			request_content.append("Content-Transfer-Encoding: binary");
			request_content.append(new_line);

			// add header to body splitter
			request_content.append(new_line);

			// add file content
			request_content.append(file.readAll());
			request_content.append(new_line);

			file.close();
		}

		// add end of body
		request_content.append(boundary_delimiter);
		request_content.append(boundary);
		request_content.append(boundary_delimiter);
	}
	else if (input->var_layout == JSON) {
		QString new_line = "\n";
		QJsonDocument jsonDoc(input->jsonObject);
		m_jsonByte = QByteArray(jsonDoc.toJson(QJsonDocument::Compact));
		request_content.append(m_jsonByte);
	}


	// prepare connection

	QNetworkRequest request = QNetworkRequest(QUrl(input->url_str));
	request.setRawHeader("User-Agent", "Cache-Bot agent");

	if (input->var_layout == URL_ENCODED) {
		request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
	}
	else if (input->var_layout == MULTIPART) {
		request.setHeader(QNetworkRequest::ContentTypeHeader, "multipart/form-data; boundary=" + boundary);
	}
	else if (input->var_layout == JSON) {
		request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
		request.setHeader(QNetworkRequest::ContentLengthHeader, QString::number(m_jsonByte.size()));
	}

	NOTICE() << "Sending to " << input->url_str;
	qDebug() << "Sending to" << input->url_str;
	QNetworkReply* reply = 0;
	if (input->http_method == "GET") {
		reply = manager->get(request);
	}
	else if (input->http_method == "POST") {
		reply = manager->post(request, request_content);
		INFO() << QString(QJsonDocument(input->jsonObject).toJson());
		qDebug() << QJsonDocument(input->jsonObject).toJson();
	}
	else if (input->http_method == "PUT") {
		reply = manager->put(request, request_content);
	}
	else if (input->http_method == "HEAD") {
		reply = manager->head(request);
	}
	else if (input->http_method == "DELETE") {
		reply = manager->deleteResource(request);
	}
	else {
		QBuffer buff(&request_content);
		reply = manager->sendCustomRequest(request, input->http_method.toLatin1(), &buff);
	}
	// clean object to not have JSON next time
	input->jsonObject = QJsonObject();
	return reply;
}

void HttpRequestWorker::on_manager_finished(QNetworkReply *reply) {
	error_type = reply->error();
	if (error_type == QNetworkReply::NoError) {
		response = reply->readAll();
	}
	else {
		error_str = reply->errorString();
	}

	reply->deleteLater();

	emit on_execution_finished(this);

	QString strResponseJsonDoc = QString(QJsonDocument::fromJson(response).toJson());
	qDebug() << "response is" << strResponseJsonDoc.left(256);
	DBG() << "response is" << strResponseJsonDoc;
	if(reply->request().url() == QUrl(LoginRoute)) {
		emit repliedLogin(response);
		if(QString(response).contains(StringLoggedInReplySuccess)) {
			emit loggedIn(true);
		}
		else if(QString(response).contains(StringLoggedInReplyFailure)) {
			emit loggedIn(false);
		}
		else {
			qWarning() << "Warning: Not connecting normally !!!!!!!!!!";
			ERR() << "Warning: Not connecting normally !!!!!!!!!!";
			qApp->exit(1);
		}
	}
	else if(reply->request().url() == QUrl(IdsRoute)) {
		emit repliedIds(response);
	}
	else if(reply->request().url().toString().startsWith(UserDataRoute)) {
		emit repliedUserData(response);
	}
	else if(reply->request().url().toString().endsWith(SendExtraCashRoute.split("/").last())) {
		emit repliedSendExtraCache(response);
	}
	else if(reply->request().url().toString().startsWith(SendNewBotRoute)) {
		emit repliedSendNewBot(response);
	}
	else if(reply->request().url().toString().startsWith(BestBotRoute)) {
		emit repliedBestBot(response);
	}
	else if(reply->request().url().toString().startsWith(ExtraCashEC2Compute)) {
		emit repliedExtraCashEC2Computation(response);
	}
}



void OfflineHttpRequestWorker::on_manager_finished(QNetworkReply *reply)
{
	if(reply->request().url() == QUrl(LoginRoute)) {
		emit loggedIn(true);
	}
	else if(reply->request().url().toString().startsWith(UserDataRoute)) {
		QString userDataPath = reply->request().url().toString().split("/").last();
		userDataPath += "Data.json";
		userDataPath.prepend("../../data/");
		WARN() << "Loading user data from file " << userDataPath;
		QFile fileReply(userDataPath);
		fileReply.open(QFile::ReadOnly);
		response = fileReply.readAll();
		emit repliedUserData(response);
	}
	else if(reply->request().url().toString().endsWith(SendExtraCashRoute.split("/").last())) {
		emit repliedSendExtraCache("");
	}
	else if(reply->request().url().toString().startsWith(SendNewBotRoute)) {
		emit repliedSendNewBot("");
	}
	else if(reply->request().url().toString().startsWith(BestBotRoute)) {
		QString userBotPath = reply->request().url().toString().split("/").last();
		userBotPath += "Bot.json";
		userBotPath.prepend("../../data/");
		WARN() << "Loading user bota from file " << userBotPath;
		QFile fileReply(userBotPath);
		fileReply.open(QFile::ReadOnly);
		response = fileReply.readAll();
		emit repliedBestBot(response);
	}
	else if(reply->request().url().toString().startsWith(ExtraCashEC2Compute)) {
		emit repliedExtraCashEC2Computation("");
	}
}
