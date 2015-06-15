#ifndef BOTCONTEXT_H
#define BOTCONTEXT_H
#include "core_global.h"
#include "common.h"
#include "puppy/Puppy.hpp"

#define CONTEXT_SEED_DEFAULT 0

class CORESHARED_EXPORT BotContext : public DBobj, public Puppy::Context
{
	Q_OBJECT

public:
	BotContext (User* pUser);

public:
	static const unsigned int MAX_NUM_FEATURES = 20;
	static unsigned int LIMIT_NUM_FEATURES;
	static unsigned int TARGET_TRANS_FUTUR_DAYS;

signals:
	void targetedTransaction(double t, double amount);
	void matchedTransaction(double t, double amount);
	void summarizingTree();
	void needsReplot();
	void newSummarizedTree(QJsonObject);

private:
//	Puppy::Context m_context;
};


#endif // BOTCONTEXT_H
