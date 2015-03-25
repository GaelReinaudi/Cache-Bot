#include "EvolutionSpinner.h"

#include "puppy/Puppy.hpp"
#include "AccRegPrimits.h"

#define POP_SIZE_DEFAULT 500
#define NBR_GEN_DEFAULT 20
#define NBR_PART_TOURNAMENT_DEFAULT 2
#define MAX_DEPTH_DEFAULT 5
#define MIN_INIT_DEPTH_DEFAULT 3
#define MAX_INIT_DEPTH_DEFAULT 5
#define INIT_GROW_PROBA_DEFAULT 0.15f
#define CROSSOVER_PROBA_DEFAULT 0.8f
#define CROSSOVER_DISTRIB_PROBA_DEFAULT 0.9f
#define MUT_STD_PROBA_DEFAULT 0.535f
#define MUT_MAX_REGEN_DEPTH_DEFAULT 5
#define MUT_SWAP_PROBA_DEFAULT 0.535f
#define MUT_SWAP_DISTRIB_PROBA_DEFAULT 0.5f
#define SEED_DEFAULT 0

using namespace Puppy;

EvolutionSpinner::EvolutionSpinner(Account *pAc, QObject* parent)
	: QObject(parent)
{
	qRegisterMetaType<ZoneVector>("ZoneVector");

	unsigned long lSeed                = SEED_DEFAULT;

	// Create evolution context add primitives used into it.
	LOG() << "Creating evolution context" << endl;
	m_context = new Context(pAc);
	m_context->mRandom.seed(lSeed);
	m_context->insert(new Add);
	m_context->insert(new Subtract);
	m_context->insert(new Multiply);
	m_context->insert(new Divide);
	m_context->insert(new Cosinus);
	m_context->insert(new TokenT<double>("0.01", 0.01));
	m_context->insert(new TokenT<double>("0.02", 0.02));
	m_context->insert(new TokenT<double>("0.03", 0.03));
	m_context->insert(new TokenT<double>("0.04", 0.04));
	m_context->insert(new TokenT<double>("0.05", 0.05));
	m_context->insert(new TokenT<double>("0.1", 0.1));
	m_context->insert(new TokenT<double>("0.2", 0.2));
	m_context->insert(new TokenT<double>("0.3", 0.3));
	m_context->insert(new TokenT<double>("0.4", 0.4));
	m_context->insert(new TokenT<double>("0.5", 0.5));
	m_context->insert(new TokenT<double>("0", 0.0));
	m_context->insert(new TokenT<double>("1", 1.0));
	m_context->insert(new TokenT<double>("2", 2.0));
	m_context->insert(new TokenT<double>("3", 3.0));
	m_context->insert(new TokenT<double>("4", 4.0));
	m_context->insert(new TokenT<double>("5", 5.0));
	m_context->insert(new TokenT<double>("10", 10.0));
	for (int i = 0; i < pAc->hashBundles().count(); ++i) {
		int h = pAc->hashBundles().keys()[i];
		if (pAc->hashBundles()[h]->count() > 10) {
			double avgKLA = pAc->hashBundles()[h]->averageKLA();
			m_context->insert(new TokenT<double>(QString("h%1").arg(i).toStdString(), i));
			m_context->insert(new TokenT<double>(QString("kla%1").arg(i).toStdString(), avgKLA));
		}
	}

	m_context->insert(new CacheBotRootPrimitive(this));
//	m_context->insert(new FeatureBiWeeklyAmount(this));
	m_context->insert(new FeatureMonthlyAmount(this));
//	m_context->insert(new DummyFeature(this));
}

void EvolutionSpinner::startStopEvolution(bool doStart) {
	if (!doStart) {
		m_doSpin = false;
		return;
	}
	m_doSpin = true;
	static bool once =false;
	if(!once) {
		once = true;
		QTimer::singleShot(0, this, SLOT(runEvolution()));
	}
}

void EvolutionSpinner::runEvolution() {
	while(!m_doSpin)  {
		QThread::msleep(100);
	}
	// Create parameter variables with default values.
	unsigned int  lPopSize             = POP_SIZE_DEFAULT;
	unsigned int  lNbrGen              = NBR_GEN_DEFAULT;
	unsigned int  lNbrPartTournament   = NBR_PART_TOURNAMENT_DEFAULT;
	unsigned int  lMaxDepth            = MAX_DEPTH_DEFAULT;
	unsigned int  lMinInitDepth        = MIN_INIT_DEPTH_DEFAULT;
	unsigned int  lMaxInitDepth        = MAX_INIT_DEPTH_DEFAULT;
	float         lInitGrowProba       = INIT_GROW_PROBA_DEFAULT;
	float         lCrossoverProba      = CROSSOVER_PROBA_DEFAULT;
	float         lCrossDistribProba   = CROSSOVER_DISTRIB_PROBA_DEFAULT;
	float         lMutStdProba         = MUT_STD_PROBA_DEFAULT;
	unsigned int  lMutMaxRegenDepth    = MUT_MAX_REGEN_DEPTH_DEFAULT;
	float         lMutSwapProba        = MUT_SWAP_PROBA_DEFAULT;
	float         lMutSwapDistribProba = MUT_SWAP_DISTRIB_PROBA_DEFAULT;

QMap<double, QStringList> output;
for (int j = 0; j < m_context->m_pAccount->hashBundles().count(); ++j) {
	int h = m_context->m_pAccount->hashBundles().keys()[j];
	if (m_context->m_pAccount->hashBundles()[h]->count() < 5)
		continue;
	m_context->filterHashIndex = j;
	// Initialize population.
	std::vector<Tree> lPopulation(lPopSize);
	std::cout << "Initializing population" << std::endl;
	initializePopulation(lPopulation, *m_context, lInitGrowProba, lMinInitDepth, lMaxInitDepth);
	evaluateSymbReg(lPopulation, *m_context);
	calculateStats(lPopulation, 0);

	// Evolve population for the given number of generations
	LOG() << "Starting evolution" << endl;
	for(unsigned int i=1; i<=lNbrGen; ++i) {
		while(!m_doSpin)  {
			QThread::msleep(100);
		}
		LOG() << "Generation " << i << endl;
		auto result = std::minmax_element(lPopulation.begin(), lPopulation.end());
		Tree bestTree = lPopulation[result.second - lPopulation.begin()];

		summarize(bestTree);

		applySelectionTournament(lPopulation, *m_context, lNbrPartTournament);
		applyCrossover(lPopulation, *m_context, lCrossoverProba, lCrossDistribProba, lMaxDepth);
		applyMutationStandard(lPopulation, *m_context, lMutStdProba, lMutMaxRegenDepth, lMaxDepth);
		applyMutationSwap(lPopulation, *m_context, lMutSwapProba, lMutSwapDistribProba);

		bestTree.mValid = false;
		//lPopulation.push_back(bestTree);

		evaluateSymbReg(lPopulation, *m_context);
		calculateStats(lPopulation, i);
	}
	LOG() << "End of evolution" << endl;

	// Outputting best individual
	std::vector<Tree>::iterator lBestIndividual =
			std::max_element(lPopulation.begin(), lPopulation.end());
	LOG() << "Best individual at generation " << lNbrGen << " is: ";
	LOG() << lBestIndividual->toStr() << endl;

	QString strBest = summarize(*lBestIndividual);
	QString strFit = strBest.mid(strBest.indexOf("fitness: ") + 9).mid(0, 5).trimmed();
	LOG() << "AAAAAAAAAAAAAAAAAA " << strBest << endl;
	double fitness = strFit.toDouble();
	output[fitness].append(strBest);
}

	for (int i = output.count() - 1; i >= 0; --i) {
		double fit = output.keys()[i];
		LOG() << "-------------------------------- " << fit << " --------------------------------" << endl;
		for (const auto& str : output[fit])
		LOG() << str << endl;
	}

	std::cout << "Exiting program " << output.count() << endl << std::flush;
}

unsigned int EvolutionSpinner::evaluateSymbReg(std::vector<Tree>& ioPopulation,
											   Context& ioContext)
{
	unsigned int lNbrEval = 0;
	for(unsigned int i=0; i<ioPopulation.size(); ++i) {
		if(ioPopulation[i].mValid)
			continue;
		double lResult = 0.0;
		ioPopulation[i].interpret(&lResult, ioContext);
		ioPopulation[i].mFitness = lResult;
		ioPopulation[i].mValid = true;
		//LOG() << "Eval tree ("<<lResult<<"): " << ioPopulation[i].toStr() << endl;
		++lNbrEval;
	}
	return lNbrEval;
}

QString EvolutionSpinner::summarize(Tree& tree)
{
	QStringList retList;
	emit sendClearMask();
	double fit = tree.summarize(&retList, *m_context);
	LOG() << "tree (" << fit << "): " << tree.toStr() << endl;
	for (const QString& str : retList) {
		LOG() << "    " << str << endl;
	}
	emit needsReplot();
	emit newList(retList);
	return retList.join('\n');
}








