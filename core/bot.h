#ifndef BOT_H
#define BOT_H
#include "core_global.h"
#include "common.h"
#include "puppy/Puppy.hpp"
class BotContext;

class CORESHARED_EXPORT Bot : public DBobj
{
public:
	Bot(QJsonObject jsonBot, QObject* parent = 0);
	//! builds the tree from the previously loaded json
	void init(BotContext* context);
	//! summariaes the performances of the bot in a json
	QJsonObject summarize();
	//! predicted transactions
	QVector<Transaction> predictTrans(double threshProba);

private:
	QDateTime m_created;
	Puppy::Tree m_puppyTree;
	QStringList m_botStrings;
	BotContext* m_context;
};

#endif // BOT_H
