#include "AccRegPrimits.h"
#include "bot.h"

bool
Puppy::Primitive::tryReplaceArgumentNode(unsigned int inN, std::string primName, Context &ioContext)
{
	assert(inN < mNumberArguments);
	auto iterPrim = ioContext.mPrimitiveMap.find(primName);
	if(iterPrim == ioContext.mPrimitiveMap.end()) {
		ERROR() << "Could not find the primitive " << primName.c_str();
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
	for (unsigned int iFeat = BotContext::MAX_NUM_FEATURES - 1; iFeat >= BotContext::LIMIT_NUM_FEATURES; --iFeat) {
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

