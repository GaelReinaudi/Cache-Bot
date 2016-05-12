#include "bot.h"
#include "AccRegPrimits.h"
#include "botContext.h"
#include "userMetrics.h"
#include "oracle.h"
#include "featurePriceWindow.h"

Puppy::Tree* Bot::s_postTreatmentBot = 0;

Bot::Bot(QJsonObject jsonBot, QObject *parent)
	:DBobj(jsonBot["_id"].toString(), parent)
{
//	Q_ASSERT(!jsonBot["_id"].toString().isEmpty());
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
			QString strVal = QString::number(val, 'f', 20);
			m_botStrings += strVal;
		}
	}
	NOTICE() << "Bot::Bot from JsonObject";
	for (auto& s : m_botStrings) {
		INFO() << s;
	}
}

Bot::~Bot()
{
}

void Bot::init(BotContext *context)
{
	NOTICE() << "Bot::init" << m_botStrings.join(" ");
	m_context = context;
	m_context->lim_NUM_FEATURE = BotContext::MAX_NUM_FEATURES;
	Puppy::initializeTree(m_puppyTree, *m_context, m_botStrings);
	NOTICE() << "-->" << m_puppyTree.toStr();
}

double Bot::evaluate()
{
	double fit;
	m_puppyTree.interpret(&fit, *m_context);

	return fit;
}

QJsonObject Bot::summarize()
{
	QJsonObject jsonObj;
	jsonObj.insert("features", QJsonArray());
	m_puppyTree.mValid = false;
	emit m_context->summarizingTree();
	m_context->m_summaryJsonObj = &jsonObj;
	double fit = evaluate();
	m_context->m_summaryJsonObj = 0;

	jsonObj.insert("fit", fit);

	emit m_context->needsReplot();
	emit m_context->newSummarizedTree(jsonObj);

	QString jsonStr = QJsonDocument(jsonObj).toJson(/*QJsonDocument::Compact*/);
	DBG() << "Bot::summarize() tree (" << fit << "): " << m_puppyTree.toStr();
	DBG() << "    " << jsonStr;
	return jsonObj;
}

QJsonObject Bot::postTreatment()
{
	QDate lastDate = Transaction::currentDay();

	double avgDayIn095  = MakeRateMonthPercentileMetric<2, 95>::get(m_context->m_pUser)->value(lastDate);
	double avgDayOut099 = CostRateMonthPercentileMetric<2, 99>::get(m_context->m_pUser)->value(lastDate);

	QJsonObject statObj;

	double flowrate = avgDayIn095 + avgDayOut099;
	flowrate /= avgDayIn095;

	statObj.insert("Flow00", flowrate);

	flowrate = 0.01 * Flow01<95, 99>::get(m_context->m_pUser)->value(lastDate);
	statObj.insert("Flow01", flowrate);

	QString jsonStr = QJsonDocument(statObj).toJson();
	NOTICE() << "Bot::postTreatment    statObj  " << jsonStr;

	return statObj;
}

User* Bot::user() const
{
	return m_context->m_pUser;
}

Puppy::Tree* Bot::instancePostTreatmentBot(Puppy::Context& ioContext)
{
	if (Bot::s_postTreatmentBot)
		return Bot::s_postTreatmentBot;

	QStringList treeNodeList;
	treeNodeList += "ROOT";
//	double klaArg = -6;
//	double inc = 0.2 + 0.2;
	QVector<double> div = FeaturePriceWindow::s_priceWindowDivisionKLA;
	for (uint i = 1; i < div.count(); ++i) {
		treeNodeList += "PriceWindow";
		treeNodeList += "0";
//		double amntBill2 = toBillDigits_2(unKindaLog(klaArg));
//		QString klaStr = QString::number(kindaLog(amntBill2));
		double amntBill2 = 0.5 * (div[i - 1] + div[i]);
		QString klaStr = QString::number(amntBill2);
		ioContext.getPrimitiveByName(klaStr);
		treeNodeList += klaStr;
		treeNodeList += "0";
//		klaArg += inc;

//		if (klaArg <= inc/2 && inc < 0)
//			klaArg = inc = 0;
//		if (klaArg >= -inc/2 && inc > 0) {
//			klaArg = 8;
//			inc = -(0.4 + 0.4);
//		}
	}

	WARN() << "Making postTreatmentBot: " << treeNodeList.join(' ');

	Bot::s_postTreatmentBot = new Puppy::Tree;
	Puppy::initializeTree(*Bot::s_postTreatmentBot, ioContext, treeNodeList);
	return Bot::s_postTreatmentBot;
}
