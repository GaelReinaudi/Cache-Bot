#include "EvolutionSpinner.h"

#include "puppy/Puppy.hpp"
#include "AccRegPrimits.h"

const double THRESHOLD_PROBA_BILL = 0.1;

#define POP_SIZE_DEFAULT 100//75//0
#define PRE_GEN_DEFAULT 100
#define NBR_GEN_DEFAULT 1000
#define NBR_PART_TOURNAMENT_DEFAULT 3
#define MAX_DEPTH_DEFAULT 6
#define MIN_INIT_DEPTH_DEFAULT 3
#define MAX_INIT_DEPTH_DEFAULT 5
#define INIT_GROW_PROBA_DEFAULT 0.15f
#define CROSSOVER_PROBA_DEFAULT 0.18f
#define CROSSOVER_DISTRIB_PROBA_DEFAULT 0.9f
#define MUT_STD_PROBA_DEFAULT 0.5f
#define MUT_MAX_REGEN_DEPTH_DEFAULT 5
#define MUT_SWAP_PROBA_DEFAULT 0.5f
#define MUT_SWAP_DISTRIB_PROBA_DEFAULT 0.5f

using namespace Puppy;

EvolutionSpinner::EvolutionSpinner(QObject* parent)
	: QObject(parent)
{
	qRegisterMetaType<ZoneVector>("ZoneVector");
}

void EvolutionSpinner::init(User* pUser)
{
//	// Create evolution context add primitives used into it.
//	m_context = pUser->makeBotContext();
	m_context = pUser->botContext();
	emit initialized(true);
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

	QMap<double, QJsonArray> output;
	QMap<double, Tree> bestPreEvoTrees;
	QMap<double, Tree> superBestPreEvoTrees;
	QJsonObject finalBotObject;
	for (int j = 0; j < m_context->m_pUser->hashBundles().count(); ++j) {
		int h = m_context->m_pUser->hashBundles().keys()[j];
		if (m_context->m_pUser->hashBundles()[h]->count() < 2)
			continue;
		m_context->filterHashIndex = j;
		m_context->currentGeneration = 0;
		// Initialize population.
		std::vector<Tree> lPopulation(5 * lPopSize);
		NOTICE() << "Initializing population for hash " << h;
		initializePopulation(lPopulation, *m_context, lInitGrowProba, lMinInitDepth, lMaxInitDepth);
		double bestFitness = evaluateSymbReg(lPopulation, *m_context);
		double newBestFitness = bestFitness;
		auto result = std::minmax_element(lPopulation.begin(), lPopulation.end());
		Tree bestTree = lPopulation[result.second - lPopulation.begin()];
		calculateStats(lPopulation, 0);

		if(!m_doSpin)  {
			break;
		}
		// Evolve population for the given number of generations
		INFO() << "Starting evolution " << newBestFitness;

		for(m_context->currentGeneration = 1; m_context->currentGeneration <= PRE_GEN_DEFAULT; ++m_context->currentGeneration ) {
//			while(!m_doSpin)  {
//				QThread::msleep(100);
//			}

			DBG() << "Generation " << m_context->currentGeneration ;
//			auto result = std::minmax_element(lPopulation.begin(), lPopulation.end());
//			bestTree = lPopulation[result.second - lPopulation.begin()];

//			summarize(bestTree);

			applySelectionTournament(lPopulation, *m_context, lNbrPartTournament);
			applyCrossover(lPopulation, *m_context, lCrossoverProba, lCrossDistribProba, lMaxDepth);
			applyMutationStandard(lPopulation, *m_context, lMutStdProba, lMutMaxRegenDepth, lMaxDepth);
			applyMutationSwap(lPopulation, *m_context, lMutSwapProba, lMutSwapDistribProba);

			//bestTree.mValid = false;
			//lPopulation.push_back(bestTree);

			newBestFitness = evaluateSymbReg(lPopulation, *m_context);
			if (newBestFitness > bestFitness * 1.01) {
				NOTICE() << "newBestFitness " << newBestFitness
						 << " > bestFitness " << bestFitness
						 << ". Reseting i=" << m_context->currentGeneration  << " to 0";
				m_context->currentGeneration  = 0;
				bestFitness = newBestFitness;
				auto result = std::minmax_element(lPopulation.begin(), lPopulation.end());
				bestTree = lPopulation[result.second - lPopulation.begin()];
			}
		}
		--m_context->currentGeneration;
		bestTree.mValid = false;
//		summarize(bestTree);
		lPopulation.push_back(bestTree);
		newBestFitness = evaluateSymbReg(lPopulation, *m_context);
		calculateStats(lPopulation, lNbrGen);
		INFO() << "End of evolution " << newBestFitness;

		// Outputting best individual
		std::vector<Tree>::iterator lBestIndividual =
				std::max_element(lPopulation.begin(), lPopulation.end());
		DBG() << "Best individual at generation " << lNbrGen << " is: "
			  << lBestIndividual->toStr();

		QJsonObject jsonBest = summarize(*lBestIndividual);
		double fitness = jsonBest["features"].toArray().first().toObject()["fitness"].toDouble();
		output[fitness].append(jsonBest);
		if(fitness > THRESHOLD_PROBA_BILL || bestPreEvoTrees.isEmpty()) {
			(*lBestIndividual).mValid = false;
			bestPreEvoTrees.insertMulti(fitness, *lBestIndividual);
		}
		if(fitness > 1.0) {
			(*lBestIndividual).mValid = false;
			superBestPreEvoTrees.insertMulti(fitness, Tree(*lBestIndividual));
		}
	}

	for (int i = 0; i < output.count(); ++i) {
		double fit = output.keys()[i];
		INFO() << "-------------------------------- " << fit << " --------------------------------";
		for (const auto& jsonTree : output[fit]) {
			INFO() << jsonTree.toObject();
		}
	}

	// run again with full features
	m_context->filterHashIndex = -1;
	if (Transaction::onlyLoadHashes.isEmpty())
		m_context->lim_NUM_FEATURE = BotContext::MAX_NUM_FEATURES;
	else
		m_context->lim_NUM_FEATURE = qMin(Transaction::onlyLoadHashes.size(), int(BotContext::MAX_NUM_FEATURES));

	Tree veryBestTree;
	// Initialize population.
	std::vector<Tree> lPopulation(0);
	initializePopulation(lPopulation, *m_context, lInitGrowProba, lMinInitDepth, lMaxInitDepth);
	NOTICE() << "bestPreEvoTrees.count " << bestPreEvoTrees.count();
	NOTICE() << "superBestPreEvoTrees.count " << superBestPreEvoTrees.count();
	const int NthReAddSuper = 4;
	if(bestPreEvoTrees.count()) {
		const auto bestbundltree = bestPreEvoTrees.values();
		for(unsigned int i = 0; i < lPopSize; ++i) {
			lPopulation.push_back(bestbundltree.at(i % bestbundltree.size()));
		}
		const auto superBestbundltree = superBestPreEvoTrees.values();
		for (int j = 0; j <= NthReAddSuper; ++j) {
			for(int i = j * superBestbundltree.size() / NthReAddSuper; i < superBestbundltree.size(); ++i) {
				lPopulation.push_back(superBestbundltree.at(superBestbundltree.size()-1-i));
			}
		}
		makeSuperTreeMixtures(lPopulation, *m_context);
		m_context->currentGeneration = 0;
		evaluateSymbReg(lPopulation, *m_context);
		calculateStats(lPopulation, 0);

		// Evolve population for the given number of generations
		NOTICE() << "Starting evolution";
		// 0 after a reinjection of features
		int afterReinjection = 0;
		std::vector<Tree> bestGenTree;
		for(m_context->currentGeneration = 1; m_context->currentGeneration <=10*lNbrGen; ++m_context->currentGeneration ) {
			if(!m_doSpin)  {
				break;
			}
			INFO() << "Generation " << m_context->currentGeneration  << " pop " << lPopulation.size();
			auto result = std::minmax_element(lPopulation.begin(), lPopulation.end());
			Tree bestTree = lPopulation[result.second - lPopulation.begin()];
			bestGenTree.push_back(bestTree);

			if (bestTree.mFitness > veryBestTree.mFitness) {
				veryBestTree = bestTree;
				finalBotObject = summarize(veryBestTree);
			}
			else {
				emit m_context->newSummarizedTree(finalBotObject);
			}

//			if (afterReinjection > NthReAddSuper)
				applySelectionTournament(lPopulation, *m_context, lNbrPartTournament);

			veryBestTree.mValid = false;
			if (m_context->currentGeneration % 32 == 0) {
				lPopulation.push_back(veryBestTree);
//				std::vector<Tree> superTreesFeatures;
//				for(int i = 0; i < superBestbundltree.size(); ++i) {
//					superTreesFeatures.push_back(superBestbundltree.at(i));
//				}
//				makeSuperTreeMixtures(superTreesFeatures, *m_context);
//				for (uint i = 0; i < superTreesFeatures.size(); ++i)
//					lPopulation.push_back(superTreesFeatures[i]);
//				for (uint i = 0; i < bestGenTree.size(); ++i)
//					lPopulation.push_back(bestGenTree[i]);
				replaceFitness0WithSuperMixture(lPopulation, superBestbundltree, *m_context);

				afterReinjection = 0;
			}

			applyCrossover(lPopulation, *m_context, lCrossoverProba, lCrossDistribProba, lMaxDepth);

//			if (afterReinjection > NthReAddSuper)
				applyMutationStandard(lPopulation, *m_context, lMutStdProba, lMutMaxRegenDepth, lMaxDepth);
//			if (afterReinjection > NthReAddSuper)
				applyMutationSwap(lPopulation, *m_context, lMutSwapProba, lMutSwapDistribProba);


			evaluateSymbReg(lPopulation, *m_context);
			calculateStats(lPopulation, m_context->currentGeneration );
			++afterReinjection;
		}
		NOTICE() << "End of evolution";
	}

	m_doSpin = false;

	finalBotObject["_git"] = QString(GIT_VERSION);
	qDebug() << "Exiting evolution. Features with positive fitness:" << finalBotObject["features"].toArray().count();
	emit finishedEvolution(finalBotObject);
}

void EvolutionSpinner::makeSuperTreeMixtures(std::vector<Tree>& ioPopulation,
											   Context& ioContext)
{
	static int jF = 0;
	for(unsigned int i=0; i < ioPopulation.size(); ++i) {
		Tree& treeToComplete = ioPopulation[i];
		for (int f = treeToComplete.lim_NUM_FEATURE; f < ioContext.lim_NUM_FEATURE; ++f) {
			Tree copyRandTreeLimited = ioPopulation[jF % ioPopulation.size()];
			++jF;
			int limitedIndex = qrand() % copyRandTreeLimited.lim_NUM_FEATURE;
			uint fi = treeToComplete.getIndexOfFeature(f);
			uint li = copyRandTreeLimited.getIndexOfFeature(limitedIndex);
			std::vector<unsigned int> lStack1;
			treeToComplete.setStackToNode(fi, lStack1);
			std::vector<unsigned int> lStack2;
			copyRandTreeLimited.setStackToNode(li, lStack2);
			Puppy::exchangeSubTrees(treeToComplete, fi, lStack1, copyRandTreeLimited, li, lStack2);
		}
		treeToComplete.mValid = false;
	}
}

void EvolutionSpinner::replaceFitness0WithSuperMixture(std::vector<Tree>& ioPopulation, const QList<Tree>& popFeatures,
											   Context& ioContext)
{
	static int jF = 0;
	for(unsigned int i=0; i < ioPopulation.size(); ++i) {
		Tree& treeToComplete = ioPopulation[i];
		for (int f = 0; f < ioContext.lim_NUM_FEATURE; ++f) {
			if (treeToComplete.fitness.size() < uint(f) || treeToComplete.fitness[f] > 0)
				continue;
			Tree copyTreeLimited = popFeatures[jF % popFeatures.size()];
			++jF;
			int jFIndex = qrand() % copyTreeLimited.lim_NUM_FEATURE;
			uint fi = treeToComplete.getIndexOfFeature(f);
			uint li = copyTreeLimited.getIndexOfFeature(jFIndex);
			std::vector<unsigned int> lStack1;
			treeToComplete.setStackToNode(fi, lStack1);
			std::vector<unsigned int> lStack2;
			copyTreeLimited.setStackToNode(li, lStack2);
			Puppy::exchangeSubTrees(treeToComplete, fi, lStack1, copyTreeLimited, li, lStack2);
		}
		treeToComplete.mValid = false;
	}
}

double EvolutionSpinner::evaluateSymbReg(std::vector<Tree>& ioPopulation,
											   Context& ioContext)
{
	emit m_context->computedGeneration(ioContext.currentGeneration);
	double bestFitness = -1e6;
	double ratioPop = 1.0 / double(ioPopulation.size());
	for(unsigned int i=0; i<ioPopulation.size(); ++i) {
		ioContext.generationProgress = double(i+1) * ratioPop;
		if(ioPopulation[i].mValid)
			continue;
		double lResult = 0.0;
		ioPopulation[i].interpret(&lResult, ioContext);
		ioPopulation[i].mFitness = lResult;
		ioPopulation[i].mValid = true;
		//DBG() << "Eval tree ("<<lResult<<"): " << ioPopulation[i].toStr();
		bestFitness = qMax(bestFitness, lResult);
	}
	return bestFitness;
}

QJsonObject EvolutionSpinner::summarize(Tree& tree)
{
	QJsonObject jsonObj;
	jsonObj.insert("features", QJsonArray());
	emit m_context->summarizingTree();
	tree.mValid = false;
	m_context->m_summaryJsonObj = &jsonObj;
	double fit;
	tree.interpret(&fit, *m_context);
	m_context->m_summaryJsonObj = 0;

	jsonObj.insert("fit", fit);

	DBG() << "tree (" << fit << "): " << tree.toStr();
	INFO() << "----" << jsonObj;
	emit m_context->needsReplot();
	emit m_context->newSummarizedTree(jsonObj);
	return jsonObj;
}







