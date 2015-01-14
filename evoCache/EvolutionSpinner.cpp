#include "EvolutionSpinner.h"

#include "puppy/Puppy.hpp"
#include "AccRegPrimits.h"

#define POP_SIZE_DEFAULT 10000
#define NBR_GEN_DEFAULT 10000
#define NBR_PART_TOURNAMENT_DEFAULT 2
#define MAX_DEPTH_DEFAULT 5
#define MIN_INIT_DEPTH_DEFAULT 2
#define MAX_INIT_DEPTH_DEFAULT 5
#define INIT_GROW_PROBA_DEFAULT 0.5f
#define CROSSOVER_PROBA_DEFAULT 0.9f
#define CROSSOVER_DISTRIB_PROBA_DEFAULT 0.9f
#define MUT_STD_PROBA_DEFAULT 0.45f
#define MUT_MAX_REGEN_DEPTH_DEFAULT 5
#define MUT_SWAP_PROBA_DEFAULT 0.35f
#define MUT_SWAP_DISTRIB_PROBA_DEFAULT 0.5f
#define SEED_DEFAULT 0

using namespace Puppy;

EvolutionSpinner::EvolutionSpinner(Account *pAc, QObject* parent)
	: QObject(parent)
{
	qRegisterMetaType<VectorRectF>("VectorRectF");

	unsigned long lSeed                = SEED_DEFAULT;

	// Create evolution context add primitives used into it.
	std::cout << "Creating evolution context" << std::endl;
	m_context = new Context(pAc);
	m_context->mRandom.seed(lSeed);
	m_context->insert(new Add);
	m_context->insert(new Subtract);
	m_context->insert(new Multiply);
	m_context->insert(new Divide);
	m_context->insert(new Cosinus);
	m_context->insert(new TokenT<double>("0", 0.0));
	m_context->insert(new TokenT<double>("1", 1.0));
	m_context->insert(new TokenT<double>("2", 2.0));
	m_context->insert(new TokenT<double>("5", 5.0));
	m_context->insert(new TokenT<double>("10", 10.0));
	m_context->insert(new TokenT<double>("20", 20.0));
	m_context->insert(new TokenT<double>("50", 50.0));
	m_context->insert(new TokenT<double>("100", 100.0));
	m_context->insert(new TokenT<double>("200", 200.0));
	m_context->insert(new TokenT<double>("500", 500.0));
	m_context->insert(new TokenT<double>("1000", 1000.0));
	m_context->insert(new TokenT<double>("2000", 2000.0));
	m_context->insert(new TokenT<double>("5000", 5000.0));

	m_context->insert(new FeatureSalary(this));
}

void EvolutionSpinner::startEvolution(bool doStart) {
	if (!doStart)
		return;
	m_doSpin = true;

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

	// Initialize population.
	std::vector<Tree> lPopulation(lPopSize);
	std::cout << "Initializing population" << std::endl;
	initializePopulation(lPopulation, *m_context, lInitGrowProba, lMinInitDepth, lMaxInitDepth);
	evaluateSymbReg(lPopulation, *m_context);
	calculateStats(lPopulation, 0);

	// Evolve population for the given number of generations
	std::cout << "Starting evolution" << std::endl;
	for(unsigned int i=1; i<=lNbrGen; ++i) {
//		m_context->bestResult = 0.0;
		applySelectionTournament(lPopulation, *m_context, lNbrPartTournament);
		applyCrossover(lPopulation, *m_context, lCrossoverProba, lCrossDistribProba, lMaxDepth);
		applyMutationStandard(lPopulation, *m_context, lMutStdProba, lMutMaxRegenDepth, lMaxDepth);
		applyMutationSwap(lPopulation, *m_context, lMutSwapProba, lMutSwapDistribProba);
		evaluateSymbReg(lPopulation, *m_context);
		calculateStats(lPopulation, i);
	}
	std::cout << "End of evolution" << std::endl;

	// Outputting best individual
	std::vector<Tree>::const_iterator lBestIndividual =
			std::max_element(lPopulation.begin(), lPopulation.end());
	std::cout << "Best individual at generation " << lNbrGen << " is: ";
	std::cout << *lBestIndividual << std:: endl;

	std::cout << "Exiting program" << std::endl << std::flush;
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
		++lNbrEval;
	}
	return lNbrEval;
}
