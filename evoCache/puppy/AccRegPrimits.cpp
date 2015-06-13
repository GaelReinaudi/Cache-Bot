#include "AccRegPrimits.h"

inline bool
Puppy::Primitive::tryReplaceArgumentNode(unsigned int inN, std::string primName, Context &ioContext)
{
	assert(inN < mNumberArguments);
	auto iterPrim = ioContext.mPrimitiveMap.find(primName);
	if(iterPrim == ioContext.mPrimitiveMap.end()) {
		LOG() << "Could not find the primitive " << primName.c_str() << endl;
		return false;
	}
	// make a tree made of only that primitive to later use the swap node function
	Tree newTree;
	std::vector<unsigned int> newStack;
	newStack.push_back(0);
	newTree.push_back(Node(iterPrim->second->giveReference(ioContext), 1));

	// index in current tree
	unsigned int lIndex = ioContext.mCallStack.back() + 1;
	for(unsigned int i=0; i<inN; ++i)
		lIndex += (*ioContext.mTree)[lIndex].mSubTreeSize;
	ioContext.mCallStack.push_back(lIndex);

	exchangeSubTrees(*ioContext.mTree, lIndex, ioContext.mCallStack
					 , newTree, 0, newStack);

	ioContext.mCallStack.pop_back();

	// make all unused features to be the same
	newStack.push_back(1);
	unsigned int tempStack = ioContext.mCallStack.back();
	ioContext.mCallStack.pop_back();
	for (unsigned int iFeat = MAX_NUM_FEATURES - 1; iFeat >= LIMIT_NUM_FEATURES; --iFeat) {
		Tree dupTree = Tree(*ioContext.mTree);
		unsigned int indexFeat = ioContext.mCallStack.back() + 1;
		for(unsigned int j=0; j<iFeat; ++j)
			indexFeat += (*ioContext.mTree)[indexFeat].mSubTreeSize;
		ioContext.mCallStack.push_back(indexFeat);
		exchangeSubTrees(*ioContext.mTree, indexFeat, ioContext.mCallStack
						 , dupTree, 1, newStack);
		ioContext.mCallStack.pop_back();
	}
	ioContext.mCallStack.push_back(tempStack);

	return true;
}

QVector<Transaction> FeatureMonthlyAmount::targetTransactions(QDate iniDate, QDate lastDate)
{
	QVector<Transaction> targetTrans;
	QDate currentDate = iniDate;
	while (currentDate < lastDate) {
		// the target day of this month. If neg, make it count from the end of the month.
		int targetDayThisMonth = qMin(m_dayOfMonth, currentDate.daysInMonth());
		targetDayThisMonth = qMax(targetDayThisMonth, 1 - currentDate.daysInMonth());
		while (targetDayThisMonth <= 0)
			targetDayThisMonth += currentDate.daysInMonth();

		// move current date to the correct target day that month
		currentDate = currentDate.addDays(targetDayThisMonth - currentDate.day());
		// assert that it is 30ish days after the previous date
		assert(targetTrans.count() == 0 || qAbs(targetTrans.last().date.daysTo(currentDate) - 30) <= 2);

		targetTrans.append(Transaction());
		targetTrans.last().date = currentDate;
		targetTrans.last().setKLA(m_kla);
		targetTrans.last().indexHash = 0;
		targetTrans.last().nameHash.hash = m_b[0];
		targetTrans.last().flags |= Transaction::Predicted;

		currentDate = currentDate.addMonths(1);
	}

	return targetTrans;
}

void FeatureMonthlyAmount::execute(void *outDatum, Puppy::Context &ioContext)
{
	AccountFeature::execute(outDatum, ioContext);
	double& output = *(double*)outDatum;

	getArgs(ioContext);
	cleanArgs();

	m_fitness = 0.0;

	// will be ALL the transactions if m_filterHash < 0
	auto& allTrans = ioContext.m_pUser->transBundle(m_filterHash);
	QDate lastDate = ioContext.m_pUser->allTrans().lastTransactionDate();//.addDays(-4);
	QDate iniDate = ioContext.m_pUser->allTrans().firstTransactionDate();

	QVector<Transaction> targetTrans = targetTransactions(iniDate, lastDate.addDays(TARGET_TRANS_FUTUR_DAYS));
	if (targetTrans.count() == 0) {
		LOG() << "MonthlyAmount(0 TARGET): day "<<m_dayOfMonth<<" kla "<< m_kla << endl;
	}
	else {
//		LOG() << "MonthlyAmount("<<targetTrans.count()<<" TARGET): day "<<m_dayOfMonth<<" kla"<< m_kla <<"="<<targetTrans.first().amountDbl() << " h=" <<targetTrans.first().nameHash.hash << endl;
	}
	if (double(m_kla) < -6 * KLA_MULTIPLICATOR || double(m_kla) > 6 * KLA_MULTIPLICATOR) {
//		double fitness = -1;
	}

	double totalOneOverExpDist = 0.0;
	quint64 localDist = 18446744073709551615ULL;
	Transaction* localTrans = 0;
	// the current target to compare to
	Transaction* iTarg = &targetTrans[0];
	m_bundle.clear();

	m_consecMonth = 0;
	m_consecMonthBeforeMissed = 0;
	m_consecMissed = 0;
	for (int i = 0; i < allTrans.count(); ++i) {
		Transaction& trans = allTrans.trans(i);
		quint64 dist = iTarg->dist(trans);
		if (dist < localDist) {
			localDist = dist;
			localTrans = &trans;
		}
		Q_ASSERT(localDist < 18446744073709551615ULL);
		static const int LIMIT_DIST_TRANS = 64 * 8;
		// if we get further away by approxSpacingPayment() / 2 days, we take the next target, or if last trans
		if (trans.jDay() > approxSpacingPayment() / 2 + iTarg->jDay() || i == allTrans.count() - 1) {
			if (localDist < LIMIT_DIST_TRANS) {
				m_bundle.append(localTrans);
				// isolate the transaction that were fitted to the target
				Q_ASSERT(localTrans->dimensionOfVoid == 0);
				localTrans->dimensionOfVoid++;
				iTarg->flags |= Transaction::CameTrue;
				if(m_consecMonth == 0) {
					m_consecMonthBeforeMissed = 0;
				}
				++m_consecMonthBeforeMissed;
				++m_consecMonth;
				m_consecMissed = 0;
			}
			else {
				m_consecMonth = 0;
				++m_consecMissed;
			}
//				totalOneOverExpDist += expoInt<64>(-localDist);
				totalOneOverExpDist += 4.0 / (4 + localDist);
			if (iTarg == &targetTrans.last() || (iTarg + 1)->date >= lastDate)
				break;
			++iTarg;
			localTrans = 0;
			localDist = quint64(-1);
			// keep this last trans in the pool if it was not just added
			if (&trans != localTrans)
				--i;
		}
	}
	// only sum that add up to > $N
	if (qAbs(m_bundle.sumDollar()) > 1) {
		m_fitness += totalOneOverExpDist;
		//m_fitness += qAbs(kindaLog(m_bundle.sumDollar())) * totalOneOverExpDist / m_bundle.count();
		//m_fitness *= m_consecMonthBeforeMissed;
		m_fitness *= 1.0 + (1.0 / (1.0 + m_consecMissed));
	}
	output = m_fitness;

//	// isolate the transaction that were fitted to the target
//	for (int i = 0; i < m_bundle.count(); ++i) {
//		Transaction& t = m_bundle.trans(i);
//		Q_ASSERT(t.dimensionOfVoid == 0);
//		++t.dimensionOfVoid;
//	}

	// summary if the json object exists
	if (ioContext.m_summaryJsonObj) {
		m_billProba = billProbability();
		if(ioContext.m_mapPredicted) {
			(*(ioContext.m_mapPredicted))[m_billProba] += targetTrans;
			for (auto& t : targetTrans) {
				Q_ASSERT(t.flags & Transaction::Predicted);
			}
		}

		if(m_billProba > 1.0) {
			QJsonArray features = (*ioContext.m_summaryJsonObj)["features"].toArray();
			features.append(toJson(ioContext));
			ioContext.m_summaryJsonObj->insert("features", features);
		}

		for (int i = 0; i < targetTrans.count(); ++i) {
			Transaction* iTarg = &targetTrans[i];
//			emit evoSpinner()->sendMask(iTarg->time_t(), iTarg->amountDbl(), true);
		}
		for (int i = 0; i < m_bundle.count(); ++i) {
			Transaction& t = m_bundle.trans(i);
//			emit evoSpinner()->sendMask(t.time_t(), t.amountDbl(), false);
		}
		qDebug() << targetTrans.count() << m_bundle.count();
	}
}

QVector<Transaction> FeatureBiWeeklyAmount::targetTransactions(QDate iniDate, QDate lastDate)
{
	QVector<Transaction> targetTrans;
	QDate currentDate = iniDate;
	while (currentDate < lastDate) {
		// the target day of this month. If neg, make it count from the end of the month.
		int targetDayThisMonth = qMin(m_dayOfMonth2, currentDate.daysInMonth());
		targetDayThisMonth = qMax(targetDayThisMonth, 1 - currentDate.daysInMonth());
		while (targetDayThisMonth <= 0)
			targetDayThisMonth += currentDate.daysInMonth();

		// move current date to the correct target day that month
		currentDate = currentDate.addDays(targetDayThisMonth - currentDate.day());
		// assert that it is 30ish days after the previous date
		assert(targetTrans.count() == 0 || qAbs(targetTrans.last().date.daysTo(currentDate) - 30) <= 2);

		targetTrans.append(Transaction());
		targetTrans.last().date = currentDate;
		targetTrans.last().setKLA(m_kla);
		targetTrans.last().indexHash = 0;
		targetTrans.last().nameHash.hash = m_b[0];
		targetTrans.last().flags |= Transaction::Predicted;

		currentDate = currentDate.addMonths(1);
	}

	targetTrans += (FeatureMonthlyAmount::targetTransactions(iniDate, lastDate));
	qSort(targetTrans.begin(), targetTrans.end(), Transaction::earlierThan);
	return targetTrans;
}
