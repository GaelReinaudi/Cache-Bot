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
 *  \file   puppy/Puppy.hpp
 *  \brief  Inclusion of BEAGLE Puppy headers and functions declarations.
 *  \author Christian Gagne
 *  \author Marc Parizeau
 *  $Revision: 1.3 $
 *  $Date: 2004/02/11 15:46:20 $
 */

/*!
 *  \namespace Puppy
 *  \brief     Namespace of all BEAGLE Puppy GP framework classes and global functions.
 */

/*!
 *  \defgroup Puppy BEAGLE Puppy Library
 *  \brief Classes and functions related BEAGLE Puppy core mechanisms.
 */
 
/*!
 *  \mainpage BEAGLE Puppy Reference Manual
 *
 *  If you are new to BEAGLE Puppy, start by browsing the <A HREF="modules.html">modules</A>,
 *  especially the examples modules.
 *
 *  If you are looking for informations on specific element of the BEAGLE Puppy framework,
 *  you can either check in the <A HREF="annotated.html">compound list</A>
 *  for a specific class, or the <A HREF="functions.html">compound member index</A> for a
 *  specific method.
 *
 */
 
#ifndef Puppy_Puppy_hpp
#define Puppy_Puppy_hpp

#include "puppy/PrimitiveHandle.hpp"
#include "puppy/Primitive.hpp"
#include "puppy/PrimitiveInline.hpp"
#include "puppy/Randomizer.hpp"
#include "puppy/Context.hpp"
#include "puppy/Tree.hpp"
#include "puppy/TokenT.hpp"

#include "log.h"

namespace Puppy {

void calculateStats(const std::vector<Tree>& inPopulation,
                    unsigned int inGeneration,
                    std::ostream& ioOS=std::cout);

void applyCrossover(std::vector<Tree>& ioPopulation,
                    Context& ioContext,
					float inMatingProba,
					float inDistribProba,
					unsigned int inMaxTreeDepth);

void exchangeSubTrees(Tree& ioTree1,
                      unsigned int inNode1,
                      const std::vector<unsigned int>& inStack1,
                      Tree& ioTree2,
                      unsigned int inNode2,
                      const std::vector<unsigned int>& inStack2);

bool mateTrees(Tree& ioTree1,
               Tree& ioTree2,
               Context& ioContext,
			   float inDistribProba,
			   unsigned int inMaxTreeDepth);

void initializePopulation(std::vector<Tree>& ioPopulation,
                          Context& ioContext,
						  float inInitGrowProba,
						  unsigned int inMinDepth,
						  unsigned int inMaxDepth);

unsigned int initializeTreeFull(Tree& ioTree,
								Context& ioContext,
								unsigned int inDepth
								, int depthAtCall);

unsigned int initializeTree(Tree& ioTree,
							Puppy::Context& ioContext,
							QStringList &NodeNames,
							int depthAtCall);

unsigned int initializeTreeGrow(Tree& ioTree,
                                Context& ioContext,
                                unsigned int inMinDepth,
								unsigned int inMaxDepth
								, int depthAtCall);

void applyMutationStandard(std::vector<Tree>& ioPopulation,
                           Context& ioContext,
						   float inMutationProba,
						   unsigned int inMaxRegenDepth,
						   unsigned int inMaxDepth);

void mutateStandard(Tree& ioTree,
                    Context& ioContext,
                    unsigned int inMaxRegenDepth,
                    unsigned int inMaxDepth);

void applyMutationSwap(std::vector<Tree>& ioPopulation,
                       Context& ioContext,
					   float inMutationProba,
					   float inDistribProba);

void mutateSwap(Tree& ioTree,
                Context& ioContext,
                float inDistribProba);


void applySelectionTournament(std::vector<Tree>& ioPopulation,
                              Context& ioContext,
							  unsigned int inNumberParticipants);

void applySelectionRoulette(std::vector<Tree>& ioPopulation,
                            Context& ioContext);
                                                                                                                                                    
}


#endif // Puppy_Puppy_hpp
