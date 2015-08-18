#include "botContext.h"
#include "bot.h"
#include "featureAllOthers.h"
#include "featureStatDistrib.h"
#include "featureOutlier.h"
#include "featurePeriodicAmount.h"
#include "featurePriceWindow.h"

unsigned int BotContext::LIMIT_NUM_FEATURES = 1;
unsigned int BotContext::TARGET_TRANS_FUTUR_DAYS = 100;

BotContext::BotContext(User *pUser)
	: DBobj(pUser)
	, Puppy::Context(pUser)
{
	LOG() << "Creating evolution context" << endl;
	mRandom.seed(CONTEXT_SEED_DEFAULT);
	insert(new Add);
	insert(new Avg);
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
//	insert(new FeatureBiWeeklyAmount());
//	insert(new FeatureMonthlyAmount());
//	insert(new FeatureStatDistrib());
//	insert(new FeatureOutlier());
//	insert(new FeatureAllOthers());
	insert(new FeaturePriceWindow());
}

