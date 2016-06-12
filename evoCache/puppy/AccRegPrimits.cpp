#include "AccRegPrimits.h"
#include "bot.h"

bool
Puppy::Primitive::tryReplaceArgumentNode(unsigned int inN, std::string primName, Context &ioContext)
{
	DBG() << "tryReplaceArgumentNode " << inN << " " << primName;
	assert(inN < mNumberArguments);
	auto iterPrim = ioContext.mPrimitiveMap.find(primName);
	if(iterPrim == ioContext.mPrimitiveMap.end()) {
		ERR() << "Could not find the primitive " << primName.c_str();
		return false;
	}
	// make a tree made of only that primitive to later use the swap node function
	Tree newTree;
	std::vector<unsigned int> newStack;
	newStack.push_back(0);
	newTree.push_back(Node(iterPrim->second->giveReference(ioContext), 1));
	DBG() << "ini.newTree " << newTree.toStr();
	DBG() << "ini.mTree " << ioContext.mTree->toStr();

	// index in current tree
	unsigned int lIndex = ioContext.mCallStack.back() + 1;
	for(unsigned int i=0; i<inN; ++i)
		lIndex += (*ioContext.mTree)[lIndex].mSubTreeSize;
	ioContext.mCallStack.push_back(lIndex);

	exchangeSubTrees(*ioContext.mTree, lIndex, ioContext.mCallStack
					 , newTree, 0, newStack);
	DBG() << "new.newTree " << newTree.toStr();
	DBG() << "new.mTree " << ioContext.mTree->toStr();

	ioContext.mCallStack.pop_back();

//	// make all unused features to be the same
//	newStack.push_back(1);
//	unsigned int tempStack = ioContext.mCallStack.back();
//	ioContext.mCallStack.pop_back();
//	for (unsigned int iFeat = BotContext::MAX_NUM_FEATURES - 1; iFeat >= BotContext::LIMIT_NUM_FEATURES; --iFeat) {
//		Tree dupTree = Tree(*ioContext.mTree);
//		unsigned int indexFeat = ioContext.mCallStack.back() + 1;
//		for(unsigned int j=0; j<iFeat; ++j)
//			indexFeat += (*ioContext.mTree)[indexFeat].mSubTreeSize;
//		ioContext.mCallStack.push_back(indexFeat);
//		exchangeSubTrees(*ioContext.mTree, indexFeat, ioContext.mCallStack
//						 , dupTree, 1, newStack);
//		ioContext.mCallStack.pop_back();
//	}
//	ioContext.mCallStack.push_back(tempStack);
//	WARN() << "final.mTree " << ioContext.mTree->toStr();

	return true;
}



void AccountFeature::execute(void *outDatum, Puppy::Context &ioContext)
{
	double& output = *(double*)outDatum;
	localStaticArgs()->m_fitness = output = 0.0;
	if (cannotExecute(ioContext))
		return;

	onGeneration(ioContext.currentGeneration, ioContext.generationProgress, ioContext);

	getArgs(ioContext);
	cleanArgs();

	// will be ALL the transactions if m_filterHash < 0
//	TransactionBundle& allTrans = ioContext.m_pUser->transBundle(m_filterHash);
	int checkingInvolved = ~(Transaction::OtherToOther | Transaction::OtherExterior);
	int flag = ioContext.isPostTreatment ? checkingInvolved : localStaticArgs()->m_filterFlags;
	TransactionBundle& allTrans = ioContext.m_pUser->transFlagBundle(m_filterHash, flag);
	if (allTrans.count() == 0) {
		return;
	}
#ifdef QT_DEBUG
	for (int i = 0; i < allTrans.count(); ++i) {
		Q_ASSERT(allTrans.trans(i).flags == flag);
	}
#endif
	output = apply(allTrans, ioContext.isPostTreatment, ioContext.m_summaryJsonObj);
	localStaticArgs()->m_fitness = output;

	isolateBundledTransactions(ioContext.isPostTreatment);

	onJustApplied(allTrans, ioContext);
	output = localStaticArgs()->m_fitness;

	m_billProba = maxDailyProbability();

	if (ioContext.m_summaryJsonObj) {
		QJsonArray features = (*ioContext.m_summaryJsonObj)["features"].toArray();
		features.append(toJson(ioContext));
		ioContext.m_summaryJsonObj->insert("features", features);

		emitGraphics(ioContext);

	}
		// making a shared pointer that will take care of cleaning once the oracle is no longer referenced
		Oracle* pNewOr = makeNewOracle();
		pNewOr->isPostTreatment = ioContext.isPostTreatment;
		QSharedPointer<Oracle> newOracle(pNewOr);
		ioContext.m_pUser->oracle()->addSubOracle(newOracle);
}

void AccountFeature::cleanArgs() {
	unsigned int i = m_filterCheckingPovCase % 8;
	switch (i) {
	case 0:
		localStaticArgs()->m_filterFlags = Transaction::CheckingPOV::FromOtherAcc;
		break;
	case 1:
		localStaticArgs()->m_filterFlags = Transaction::CheckingPOV::FromExterior;
		break;
	case 2:
		localStaticArgs()->m_filterFlags = Transaction::CheckingPOV::FromDigitLike;
		break;
	case 3:
		localStaticArgs()->m_filterFlags = Transaction::CheckingPOV::ToOtherAcc;
		break;
	case 4:
		localStaticArgs()->m_filterFlags = Transaction::CheckingPOV::ToExterior;
		break;
	case 5:
		localStaticArgs()->m_filterFlags = Transaction::CheckingPOV::ToDigitLike;
		break;
	case 6:
		localStaticArgs()->m_filterFlags = Transaction::CheckingPOV::OtherToOther;
		break;
	case 7:
		localStaticArgs()->m_filterFlags = Transaction::CheckingPOV::OtherExterior;
		break;
	default:
		localStaticArgs()->m_filterFlags = m_filterCheckingPovCase;
		break;
	}
}

void AccountFeature::isolateBundledTransactions(bool isPostTreatment /*= false*/)
{
	Q_UNUSED(isPostTreatment);
	// isolate the transaction that were fitted to the target
	for (int i = 0; i < localStaticArgs()->m_bundle.count(); ++i) {
		localStaticArgs()->m_bundle.trans(i).setDimensionOfVoid();
	}
}






