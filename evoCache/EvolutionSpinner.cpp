#include "EvolutionSpinner.h"

#include "puppy/Puppy.hpp"
#include "AccRegPrimits.h"

const double THRESHOLD_PROBA_BILL = 0.1;

#define POP_SIZE_DEFAULT 750
#define NBR_GEN_DEFAULT 30
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
	QVector<Tree> bestPreEvoTrees;
	QJsonObject finalBotObject;
	for (int j = 0; j < m_context->m_pUser->hashBundles().count(); ++j) {
		int h = m_context->m_pUser->hashBundles().keys()[j];
		if (m_context->m_pUser->hashBundles()[h]->count() < 2)
			continue;
		m_context->filterHashIndex = j;
		// Initialize population.
		std::vector<Tree> lPopulation(lPopSize);
		std::cout << "Initializing population for hash " << h << std::endl;
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
		}
		calculateStats(lPopulation, lNbrGen);
		LOG() << "End of evolution" << endl;

		// Outputting best individual
		std::vector<Tree>::iterator lBestIndividual =
				std::max_element(lPopulation.begin(), lPopulation.end());
		LOG() << "Best individual at generation " << lNbrGen << " is: ";
		LOG() << lBestIndividual->toStr() << endl;
//		std::vector<unsigned int> outCallStack = (*lBestIndividual).getFeatureStack(0, *m_context);
//		qDebug() << QVector<unsigned int>::fromStdVector(outCallStack);

		QJsonObject jsonBest = summarize(*lBestIndividual);
		double billProba = jsonBest["features"].toArray().first().toObject()["billProba"].toDouble();
		double fitness = jsonBest["features"].toArray().first().toObject()["fitness"].toDouble();
		output[billProba].append(jsonBest);
		qDebug() << "billProba" << billProba;
		if(fitness > THRESHOLD_PROBA_BILL || bestPreEvoTrees.isEmpty()) {
			(*lBestIndividual).mValid = false;
			bestPreEvoTrees.push_back(*lBestIndividual);
		}
	}

	for (int i = 0; i < output.count(); ++i) {
		double fit = output.keys()[i];
		LOG() << "-------------------------------- " << fit << " --------------------------------" << endl;
		for (const auto& jsonTree : output[fit]) {
			LOG() << QJsonDocument(jsonTree.toObject()).toJson() << endl;
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
	std::cout << "Initializing population" << std::endl;
	initializePopulation(lPopulation, *m_context, lInitGrowProba, lMinInitDepth, lMaxInitDepth);
	qDebug() << bestPreEvoTrees.count();
	if(bestPreEvoTrees.count()) {
		for(unsigned int i = 0; i < 5*lPopSize; ++i) {
			lPopulation.push_back(bestPreEvoTrees.at(i % bestPreEvoTrees.size()));
		}
		evaluateSymbReg(lPopulation, *m_context);
		calculateStats(lPopulation, 0);

		// Evolve population for the given number of generations
		LOG() << "Starting evolution" << endl;
		for(unsigned int i=1; i<=10*lNbrGen; ++i) {
			if(!m_doSpin)  {
				break;
			}
			LOG() << "Generation " << i << endl;
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
			calculateStats(lPopulation, i);
		}
		LOG() << "End of evolution" << endl;

//		QVector<Transaction> futureTransactions = predictTrans(veryBestTree, THRESHOLD_PROBA_BILL);
//		m_context->m_pAccount->toJson(futureTransactions, "predicted");

	}

	m_doSpin = false;
	qDebug() << "Exiting evolution. Features with positive fitness:" << finalBotObject["features"].toArray().count();
	emit finishedEvolution(finalBotObject);
//	qApp->exit();
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

	QString jsonStr = QJsonDocument(jsonObj).toJson(/*QJsonDocument::Compact*/);
	LOG() << "tree (" << fit << "): " << tree.toStr() << endl;
	LOG() << "    " << jsonStr << endl;
	emit m_context->needsReplot();
	emit m_context->newSummarizedTree(jsonObj);
	return jsonObj;
}







