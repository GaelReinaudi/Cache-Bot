#ifndef BOT_H
#define BOT_H
#include "core_global.h"
#include "common.h"
#include "puppy/Puppy.hpp"
class BotContext;
//class Bot;
//
//class CORESHARED_EXPORT BotStats
//{
//public:
//	BotStats(Bot* parent) {};
//
//public:
//
//private:
//	Bot* m_bot;
//};

class CORESHARED_EXPORT Bot : public DBobj
{
//private:
//	Bot(BotContext* context, const Puppy::Tree& tree);
public:
	Bot(QJsonObject jsonBot, QObject* parent = 0);
	//! builds the tree from the previously loaded json
	void init(BotContext* context);
	//! summaries the performances of the bot in a json
	QJsonObject summarize();
	//! predicted transactions
	QVector<Transaction> predictTrans(double threshProba);

//public:
//	static Bot fromTree();

	QJsonObject lastStats() const {
		return m_lastStats;
	}

private:
	QDateTime m_created;
	Puppy::Tree m_puppyTree;
	QStringList m_botStrings;
	BotContext* m_context;
	QJsonObject m_lastStats;

private:
	void postTreatment(QJsonObject &sumObj, const QVector<Transaction> &predictedTrans);
};

#endif // BOT_H
