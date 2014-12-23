#include "EvolutionSpinner.h"

#include "puppy/Puppy.hpp"
#include "AccRegPrimits.h"

#define POP_SIZE_DEFAULT 100
#define NBR_GEN_DEFAULT 50
#define NBR_PART_TOURNAMENT_DEFAULT 2
#define MAX_DEPTH_DEFAULT 17
#define MIN_INIT_DEPTH_DEFAULT 2
#define MAX_INIT_DEPTH_DEFAULT 5
#define INIT_GROW_PROBA_DEFAULT 0.5f
#define CROSSOVER_PROBA_DEFAULT 0.9f
#define CROSSOVER_DISTRIB_PROBA_DEFAULT 0.9f
#define MUT_STD_PROBA_DEFAULT 0.05f
#define MUT_MAX_REGEN_DEPTH_DEFAULT 5
#define MUT_SWAP_PROBA_DEFAULT 0.05f
#define MUT_SWAP_DISTRIB_PROBA_DEFAULT 0.5f
#define SEED_DEFAULT 0

using namespace Puppy;

EvolutionSpinner::EvolutionSpinner(QObject* parent)
	: QObject(parent)
{
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
	unsigned long lSeed                = SEED_DEFAULT;

	// Create evolution context add primitives used into it.
	std::cout << "Creating evolution context" << std::endl;
	Context lContext;
	lContext.mRandom.seed(lSeed);
	lContext.insert(new Add);
	lContext.insert(new Subtract);
	lContext.insert(new Multiply);
	lContext.insert(new Divide);
	lContext.insert(new Cosinus);
	lContext.insert(new TokenT<double>("X", 0.0));

	// Sample equation on 20 random points in [-1.0, 1.0].
	std::cout << "Sampling equation to regress" << std::endl;
	std::vector<double> lX(20);
	std::vector<double> lF(20);
	for(unsigned int i=0; i<lX.size(); ++i) {
		lX[i] = lContext.mRandom.rollUniform(-1.0, 1.0);
		lF[i] = lX[i]*(lX[i]*(lX[i]*(lX[i]+1.0)+1.0)+1.0);
	}

	// Initialize population.
	std::vector<Tree> lPopulation(lPopSize);
	std::cout << "Initializing population" << std::endl;
	initializePopulation(lPopulation, lContext, lInitGrowProba, lMinInitDepth, lMaxInitDepth);
	evaluateSymbReg(lPopulation, lContext, lX, lF);
	calculateStats(lPopulation, 0);

	// Evolve population for the given number of generations
	std::cout << "Starting evolution" << std::endl;
	for(unsigned int i=1; i<=lNbrGen; ++i) {
		applySelectionTournament(lPopulation, lContext, lNbrPartTournament);
		applyCrossover(lPopulation, lContext, lCrossoverProba, lCrossDistribProba, lMaxDepth);
		applyMutationStandard(lPopulation, lContext, lMutStdProba, lMutMaxRegenDepth, lMaxDepth);
		applyMutationSwap(lPopulation, lContext, lMutSwapProba, lMutSwapDistribProba);
		evaluateSymbReg(lPopulation, lContext, lX, lF);
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
											   Context& ioContext,
											   const std::vector<double>& inX,
											   const std::vector<double>& inF)
{
	assert(inX.size() == inF.size());
	unsigned int lNbrEval = 0;
	for(unsigned int i=0; i<ioPopulation.size(); ++i) {
		if(ioPopulation[i].mValid) continue;
		double lQuadErr = 0.0;
		for(unsigned int j=0; j<inX.size(); ++j) {
			ioContext.mPrimitiveMap["X"]->setValue(&inX[j]);
			double lResult = 0.0;
			ioPopulation[i].interpret(&lResult, ioContext);
			double lErr = lResult - inF[j];
			lQuadErr += (lErr * lErr);
		}
		double lRMS = std::sqrt(lQuadErr / inX.size());
		ioPopulation[i].mFitness = 1. / (1. + lRMS);
		ioPopulation[i].mValid = true;
		++lNbrEval;
	}
	return lNbrEval;
}
