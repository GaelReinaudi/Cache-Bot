#include "bot.h"
#include "AccRegPrimits.h"
#include "botContext.h"

Bot::Bot(QJsonObject jsonBot, QObject *parent)
	:DBobj(jsonBot["_id"].toString(), parent)
{
	Q_ASSERT(!jsonBot["_id"].toString().isEmpty());
	m_created = QDateTime::fromString(jsonBot["createdAt"].toString(), Qt::ISODate);

	m_botStrings.clear();
	m_botStrings += CacheBotRootPrimitive::rootName();
	QJsonArray jsonFeatArray = jsonBot["features"].toArray();
	for (int iF = 0; iF < jsonFeatArray.size(); ++iF) {
		QJsonObject jsonFeat = jsonFeatArray[iF].toObject();
		QString featName = jsonFeat["name"].toString();
		m_botStrings += featName;
		int numArgs = jsonFeat["numArgs"].toInt();
		QJsonArray jsonValArray = jsonFeat["args"].toArray();
		for (int iV = 0; iV < numArgs; ++iV) {
			double val = jsonValArray.at(iV).toDouble();
			QString strVal = QString::number(val);
			m_botStrings += strVal;
		}
	}
	qDebug() << m_botStrings;
}

void Bot::init(BotContext *context) {
	m_context = context;
	BotContext::LIMIT_NUM_FEATURES = BotContext::MAX_NUM_FEATURES;
	Puppy::initializeTree(m_puppyTree, *m_context, m_botStrings);
	qDebug() << m_puppyTree.toStr();
}

QJsonObject Bot::summarize()
{
	QJsonObject jsonObj;
	jsonObj.insert("features", QJsonArray());
	m_puppyTree.mValid = false;
	m_context->m_summaryJsonObj = &jsonObj;
	double fit;
	m_puppyTree.interpret(&fit, *m_context);
	m_context->m_summaryJsonObj = 0;

	emit m_context->needsReplot();
	emit m_context->newSummarizedTree(jsonObj);

	QString jsonStr = QJsonDocument(jsonObj).toJson(/*QJsonDocument::Compact*/);
	LOG() << "tree (" << fit << "): " << m_puppyTree.toStr() << endl;
	LOG() << "    " << jsonStr << endl;
	return jsonObj;
}

QVector<Transaction> Bot::predictTrans(double threshProba)
{
	QVector<Transaction> ret;
	QMap<double, QVector<Transaction> > mapFitPredicted;
	// makes the summary to compute predictions
	m_context->m_mapPredicted = &mapFitPredicted;
	summarize();
	m_context->m_mapPredicted = 0;

	for (int i = 0; i < mapFitPredicted.count(); ++i) {
		double proBill = mapFitPredicted.keys()[i];
		if (proBill > threshProba)
			ret += mapFitPredicted[proBill];
	}
	qSort(ret.begin(), ret.end(), Transaction::earlierThan);
	return ret;
}
