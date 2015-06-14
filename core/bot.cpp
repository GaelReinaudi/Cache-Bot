#include "bot.h"
#include "AccRegPrimits.h"

unsigned int BotContext::LIMIT_NUM_FEATURES = 1;
unsigned int BotContext::TARGET_TRANS_FUTUR_DAYS = 0;

BotContext::BotContext(User *pUser)
	: QObject(pUser)
	, Puppy::Context(pUser)
{
	LOG() << "Creating evolution context" << endl;
	mRandom.seed(CONTEXT_SEED_DEFAULT);
	insert(new Add);
	insert(new Subtract);
	insert(new Multiply);
	insert(new Divide);
	insert(new Cosinus);
	insert(new Puppy::TokenT<double>("0.01", 0.01));
	insert(new Puppy::TokenT<double>("0.02", 0.02));
	insert(new Puppy::TokenT<double>("0.03", 0.03));
	insert(new Puppy::TokenT<double>("0.04", 0.04));
	insert(new Puppy::TokenT<double>("0.05", 0.05));
	insert(new Puppy::TokenT<double>("0.1", 0.1));
	insert(new Puppy::TokenT<double>("0.2", 0.2));
	insert(new Puppy::TokenT<double>("0.3", 0.3));
	insert(new Puppy::TokenT<double>("0.4", 0.4));
	insert(new Puppy::TokenT<double>("0.5", 0.5));
	insert(new Puppy::TokenT<double>("0", 0.0));
	insert(new Puppy::TokenT<double>("1", 1.0));
	insert(new Puppy::TokenT<double>("2", 2.0));
	insert(new Puppy::TokenT<double>("3", 3.0));
	insert(new Puppy::TokenT<double>("4", 4.0));
	insert(new Puppy::TokenT<double>("5", 5.0));
	insert(new Puppy::TokenT<double>("10", 10.0));
	for (int i = 0; i < pUser->hashBundles().count(); ++i) {
		int h = pUser->hashBundles().keys()[i];
		if (pUser->hashBundles()[h]->count() > 1)
		{
			int avgKLA = pUser->hashBundles()[h]->averageKLA();
			insert(new Puppy::TokenT<double>(QString("h%1").arg(h).toStdString(), h));
			insertIfNotThere(new Puppy::TokenT<double>(QString("kla%1").arg(avgKLA).toStdString(), avgKLA));
		}
	}
	insert(new CacheBotRootPrimitive());
	insert(new FeatureBiWeeklyAmount());
	insert(new FeatureMonthlyAmount());
	//		insert(new DummyFeature());
}


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
