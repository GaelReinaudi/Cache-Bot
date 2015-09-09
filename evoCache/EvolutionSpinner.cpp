#include "EvolutionSpinner.h"

#include "puppy/Puppy.hpp"
#include "AccRegPrimits.h"

const double THRESHOLD_PROBA_BILL = 0.1;

#define POP_SIZE_DEFAULT 15//75//0
#define NBR_GEN_DEFAULT 30
#define NBR_PART_TOURNAMENT_DEFAULT 4
#define MAX_DEPTH_DEFAULT 6
#define MIN_INIT_DEPTH_DEFAULT 3
#define MAX_INIT_DEPTH_DEFAULT 5
#define INIT_GROW_PROBA_DEFAULT 0.15f
#define CROSSOVER_PROBA_DEFAULT 0.8f
#define CROSSOVER_DISTRIB_PROBA_DEFAULT 0.9f
#define MUT_STD_PROBA_DEFAULT 0.75f
#define MUT_MAX_REGEN_DEPTH_DEFAULT 5
#define MUT_SWAP_PROBA_DEFAULT 0.535f
#define MUT_SWAP_DISTRIB_PROBA_DEFAULT 0.5f

using namespace Puppy;

EvolutionSpinner::EvolutionSpinner(QObject* parent)
	: QObject(parent)
{
	qRegisterMetaType<ZoneVector>("ZoneVector");
}

void EvolutionSpinner::init(User* pUser)
{
	// Create evolution context add primitives used into it.
	m_context = pUser->makeBotContext();
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
	QJsonObject finalBotObject;
	for (int j = 0; j < m_context->m_pUser->hashBundles().count(); ++j) {
		int h = m_context->m_pUser->hashBundles().keys()[j];
		if (m_context->m_pUser->hashBundles()[h]->count() < 2)
			continue;
		m_context->filterHashIndex = j;
		// Initialize population.
		std::vector<Tree> lPopulation(lPopSize);
		DBG() << "Initializing population for hash " << h;
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

		for(m_context->currentGeneration = 1; m_context->currentGeneration <=lNbrGen; ++m_context->currentGeneration ) {
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
		bestTree.mValid = false;
		summarize(bestTree);
		lPopulation.push_back(bestTree);
		newBestFitness = evaluateSymbReg(lPopulation, *m_context);
		calculateStats(lPopulation, lNbrGen);
		INFO() << "End of evolution " << newBestFitness;

		// Outputting best individual
		std::vector<Tree>::iterator lBestIndividual =
				std::max_element(lPopulation.begin(), lPopulation.end());
		DBG() << "Best individual at generation " << lNbrGen << " is: "
			  << lBestIndividual->toStr();
//		std::vector<unsigned int> outCallStack = (*lBestIndividual).getFeatureStack(0, *m_context);
//		qDebug() << QVector<unsigned int>::fromStdVector(outCallStack);

		QJsonObject jsonBest = summarize(*lBestIndividual);
//		double billProba = jsonBest["features"].toArray().first().toObject()["billProba"].toDouble();
		double fitness = jsonBest["features"].toArray().first().toObject()["fitness"].toDouble();
		output[fitness].append(jsonBest);
//		qDebug() << "billProba" << billProba;
		if(fitness > THRESHOLD_PROBA_BILL || bestPreEvoTrees.isEmpty()) {
			(*lBestIndividual).mValid = false;
			bestPreEvoTrees.insertMulti(fitness, *lBestIndividual);
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
	if (Transaction::onlyLoadHashes.isEmpty())
		BotContext::LIMIT_NUM_FEATURES = BotContext::MAX_NUM_FEATURES;
	else
		BotContext::LIMIT_NUM_FEATURES = qMin(Transaction::onlyLoadHashes.size(), int(BotContext::MAX_NUM_FEATURES));

	m_context->filterHashIndex = -1;
	Tree veryBestTree;
	// Initialize population.
	std::vector<Tree> lPopulation(0);
	initializePopulation(lPopulation, *m_context, lInitGrowProba, lMinInitDepth, lMaxInitDepth);
	NOTICE() << "bestPreEvoTrees.count " << bestPreEvoTrees.count();
	if(bestPreEvoTrees.count()) {
		auto bestbundltree = bestPreEvoTrees.values();
		for(unsigned int i = 0; i < lPopSize; ++i) {
			lPopulation.push_back(bestbundltree.at(bestbundltree.size() - 1 - i % bestbundltree.size()));
		}
		evaluateSymbReg(lPopulation, *m_context);
		calculateStats(lPopulation, 0);

		// Evolve population for the given number of generations
		NOTICE() << "Starting evolution";
		for(m_context->currentGeneration = 1; m_context->currentGeneration <=10*lNbrGen; ++m_context->currentGeneration ) {
			if(!m_doSpin)  {
				break;
			}
			INFO() << "Generation " << m_context->currentGeneration  << " pop " << lPopulation.size();
			auto result = std::minmax_element(lPopulation.begin(), lPopulation.end());
			Tree bestTree = lPopulation[result.second - lPopulation.begin()];

			if (bestTree.mFitness > veryBestTree.mFitness)
				veryBestTree = bestTree;
			finalBotObject = summarize(bestTree);

			applySelectionTournament(lPopulation, *m_context, lNbrPartTournament);
			applyCrossover(lPopulation, *m_context, lCrossoverProba, lCrossDistribProba, lMaxDepth);
			applyMutationStandard(lPopulation, *m_context, lMutStdProba, lMutMaxRegenDepth, lMaxDepth);
			applyMutationSwap(lPopulation, *m_context, lMutSwapProba, lMutSwapDistribProba);

			bestTree.mValid = false;
			lPopulation.push_back(bestTree);

			evaluateSymbReg(lPopulation, *m_context);
			calculateStats(lPopulation, m_context->currentGeneration );
		}
		NOTICE() << "End of evolution";
	}

	m_doSpin = false;
	qDebug() << "Exiting evolution. Features with positive fitness:" << finalBotObject["features"].toArray().count();
	emit finishedEvolution(finalBotObject);
}

double EvolutionSpinner::evaluateSymbReg(std::vector<Tree>& ioPopulation,
											   Context& ioContext)
{
	double bestFitness = -1e6;
	for(unsigned int i=0; i<ioPopulation.size(); ++i) {
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
	INFO() << "    " << jsonObj;
	emit m_context->needsReplot();
	emit m_context->newSummarizedTree(jsonObj);
	return jsonObj;
}







