#ifndef BOT_H
#define BOT_H
#include "core_global.h"
#include "common.h"
#include "puppy/Puppy.hpp"
class BotContext;
class Oracle;
class User;

class CORESHARED_EXPORT Bot : public DBobj
{
//private:
//	Bot(BotContext* context, const Puppy::Tree& tree);
public:
	Bot(QJsonObject jsonBot, QObject* parent = 0);
	~Bot();
	//! builds the tree from the previously loaded json
	void init(BotContext* context);
	//! summaries the performances of the bot in a json
	QJsonObject summarize();
	QJsonObject postTreatment();
	User* user() const;

//public:
//	static Bot fromTree();

	QJsonObject lastStats() const {
		return m_lastStats;
	}

protected:
	double evaluate();

private:
	QDateTime m_created;
	Puppy::Tree m_puppyTree;
	QStringList m_botStrings;
	BotContext* m_context;
	QJsonObject m_lastStats;
};

#endif // BOT_H
