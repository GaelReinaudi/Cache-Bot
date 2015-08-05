/*
 *  BEAGLE Puppy
 *  Copyright (C) 2001-2004 by Christian Gagne and Marc Parizeau
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  Contact:
 *  Laboratoire de Vision et Systemes Numeriques
 *  Departement de genie electrique et de genie informatique
 *  Universite Laval, Quebec, Canada, G1V 4J7
 *  http://vision.gel.ulaval.ca
 *
 */

/*!
 *  \file   puppy/Context.hpp
 *  \brief  Definition of the class Context.
 *  \author Christian Gagne
 *  \author Marc Parizeau
 *  $Revision: 1.2 $
 *  $Date: 2004/02/11 15:46:20 $
 */

#ifndef Puppy_Context_hpp
#define Puppy_Context_hpp

#include <map>
#include <string>
#include <vector>

#include "puppy/PrimitiveHandle.hpp"
#include "puppy/Primitive.hpp"
#include "puppy/Randomizer.hpp"
#include "puppy/TokenT.hpp"
#include "user.h"

namespace Puppy {

class Tree;  // Forward declaration


/*!
 *  \class Context puppy/Context.hpp "puppy/Context.hpp"
 *  \brief Evolutionary context.
 *  \ingroup Puppy
 *
 *  The evolutionary context includes the execution context used when interpreting the trees
 *  along with the problem set-up defined with the function and terminal set, and the randomizer.
 */
class Context {

public:

	/*!
   *  \brief Build an evolutionary context.
   */
	inline Context(User* pUser) :
		mTree(NULL)
	  , m_pUser(pUser)
	{
	}

	/*!
   *  \brief Add a new primitive in the sets of primitive.
   *  \param inPrimitive Primitive added.
   */
	inline void insert(PrimitiveHandle inPrimitive)
	{
		assert(mPrimitiveMap.find(inPrimitive->getName()) == mPrimitiveMap.end());
		mPrimitiveMap[inPrimitive->getName()] = inPrimitive;
		if(inPrimitive->getNumberArguments() == 0 && !inPrimitive->isFeature())
			mTerminalSet.push_back(inPrimitive);
		else if(!inPrimitive->isFeature())
			mFunctionSet.push_back(inPrimitive);
		else if(!inPrimitive->isRoot())
			mAccountFeatureSet.push_back(inPrimitive);
		else
			mAccountRoot.push_back(inPrimitive);
	}
	inline void insertIfNotThere(PrimitiveHandle inPrimitive)
	{
		if(mPrimitiveMap.find(inPrimitive->getName()) == mPrimitiveMap.end())
			insert(inPrimitive);
	}
	inline bool hasPrimitive(QString name)
	{
		if(mPrimitiveMap.find(name.toStdString()) == mPrimitiveMap.end())
			return false;
		return true;
	}
	inline PrimitiveHandle getPrimitiveByName(QString name)
	{
		std::string stdname = name.toStdString();
		if (hasPrimitive(name)) {
			return mPrimitiveMap[stdname];
		}
		// else, is this a number ?
		bool yes = false;
		double val = name.toDouble(&yes);
		if (yes) {
			PrimitiveHandle newTerminal = new TokenT<double>(stdname, val);
			insert(newTerminal);
			return newTerminal;
		}
		return new TokenT<double>(std::string("ERROR_NAME_NOT_FOUND_AND_COULDNT_BE_CONVERTED"), 3.141592);
	}

	Randomizer                            mRandom;        //!< Random number generator.
	std::vector<PrimitiveHandle>          mFunctionSet;   //!< Set of functions usable to build trees.
	std::vector<PrimitiveHandle>          mAccountFeatureSet;   //!< Set of account features usable to build trees.
	std::vector<PrimitiveHandle>          mAccountRoot;   //!< Set of account features usable to build trees.
	std::vector<PrimitiveHandle>          mTerminalSet;   //!< Set of terminals usable to build trees.
	std::map<std::string,PrimitiveHandle> mPrimitiveMap;  //!< Name-primitive map.
	std::vector<unsigned int>             mCallStack;     //!< Execution call stack.
	Tree*                                 mTree;          //!< Actual tree evaluated.
	bool m_isInFeature = false;
	bool m_hasRecursiveFeature = false;
	QJsonObject* m_summaryJsonObj = 0;
	QMap<double, QVector<Transaction> >* m_mapPredicted = 0;
	User* m_pUser;
	int filterHashIndex = -1;
	enum {None = 0x0, AllOthers = 0x1};
	int flags = 0;
};

}


#endif // Puppy_Context_hpp
