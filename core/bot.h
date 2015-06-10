#ifndef BOT_H
#define BOT_H
#include "core_global.h"
#include "common.h"
#include "puppy/Puppy.hpp"
#include "AccRegPrimits.h"

using namespace Puppy;

class BotContext
{
public:
	BotContext (User* pUser)
		: puppyContext(pUser)
	{
		puppyContext.insert(new Add);
	}

private:
	Context puppyContext;
};

class CORESHARED_EXPORT Bot : public DBobj
{
public:
	Bot(QJsonObject jsonBot, QObject* parent = 0)
		:DBobj(jsonBot["_id"].toString(), parent)
	{
		Q_ASSERT(!jsonBot["_id"].toString().isEmpty());
		m_created = QDateTime::fromString(jsonBot["createdAt"].toString(), Qt::ISODate);
	}

private:
	QDateTime m_created;
	Tree m_puppyTree;
};

#endif // BOT_H
