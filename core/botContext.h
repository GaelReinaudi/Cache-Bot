#ifndef BOTCONTEXT_H
#define BOTCONTEXT_H
#include "core_global.h"
#include "common.h"
#include "puppy/Puppy.hpp"

#define CONTEXT_SEED_DEFAULT 1

class CORESHARED_EXPORT BotContext : public DBobj, public Puppy::Context
{
	Q_OBJECT

public:
	BotContext (User* pUser);

public:
	static const unsigned int MAX_NUM_FEATURES = 32;
	static unsigned int LIMIT_NUM_FEATURES;
	static unsigned int TARGET_TRANS_FUTUR_DAYS;

signals:
	void matchedTransaction(double t, double amount, int flag = 0);
	void summarizingTree();
	void needsReplot();
	void newSummarizedTree(QJsonObject);
	void computedGeneration(int gen);

private:
//	Puppy::Context m_context;
};


#endif // BOTCONTEXT_H
