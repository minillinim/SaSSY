/******************************************************************************
**
** File: ContextClass.cpp
**
*******************************************************************************
**
** Layer 3 Implementation file for use with projects including MemManager.cpp
**
** This file implements the methods needed to form contexts from seed sequences
** and DualNodes.
**
*******************************************************************************
**
**                        A
**                       A B
**                      A B R
**                     A B R A
**                    A B R A C
**                   A B R A C A
**                  A B R A C A D
**                 A B R A C A D A
**                A B R A C A D A B
**               A B R A C A D A B R
**              A B R A C A D A B R A
**
*******************************************************************************
**
** Copyright (C) 2009 2010 2011 Michael Imelfort and Dominic Eales
**
** This file is part of the Sassy Assembler Project.
**
** Sassy is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** Sassy is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Sassy.  If not, see <http://www.gnu.org/licenses/>.
** 
*******************************************************************************/

/******************************************************************************
** #INCLUDES #DEFINES AND PRIVATE VARS
******************************************************************************/

// system includes
#include <iostream>

// local includes
#include "intdef.h"
#include "IdTypeDefs.h"
#include "IdTypeStructs.h"

#include "ContextMemWrapper.h"
#include "ContextClass.h"

#include "ContigClass.h"
//INC #include "ContigClass.h"
//INC 

#include "StatsManager.h"
//INC #include "StatsManager.h"
//INC 

#include "GenericNodeClass.h"
//INC #include "GenericNodeClass.h"
//INC 

#include "nodeblockdef.h"
//INC #include "nodeblockdef.h"
//INC 

//PV StatsManager * mStatsManager;
//PV 
//PV GenericNodeClass * mNodes;
//PV 
/******************************************************************************
** INTIALISATION AND SAVING
******************************************************************************/

/*HV*/ void
ContextClass::
/*HV*/ extendedDestructor(void)
//HO ;
{
    //-----
    // Add anything you would like to happen when the destructor is called
    //
}
//HO 

/*HV*/ bool
ContextClass::
/*HV*/ extendedInitialise(uMDInt originalNumberOfBlocks )
//HO ;
{
    //-----
    // Add anything you would like to happen on a regular intialisation call
    //
    return true;
}
//HO 

/*HV*/ bool
ContextClass::
/*HV*/ extendedInitialise(std::string fileName)
//HO ;
{
    //-----
    // Add anything you would like to happen when reloading a saved instance
    //
    return true;
}
//HO 

/*HV*/ bool
ContextClass::
/*HV*/ extendedSave(std::string fileName)
//HO ;
{
    //-----
    // Add anything you would like to happen when saving (including saving private vars)
    //
    return true;
}
//HO 

/******************************************************************************
** HOUSEKEEPING
******************************************************************************/
//HO inline void setObjectPointers(GenericNodeClass * GNC, StatsManager * SM) { mData->setObjectPointers(GNC, SM); mNodes = GNC; mStatsManager = SM; }
//HO 
//HO inline void setStrict(bool strict_val) { mData->setStrict(strict_val); } 
//HO 
//HO inline void setOffsets(uMDInt mo, uMDInt no) { mData->setOffsets(mo,no); } 
//HO 
//HO inline bool validateContext(ContextId CTXID) { return mData->validateContext(CTXID); }
//HO 
//HO inline void deleteContext(ContextId CTXID) { mData->deleteContext(CTXID); }
//HO 
//HO inline void readNaiveCoverageMap(double cov_cut, std::string covFileName) { mData->readNaiveCoverageMap(cov_cut, covFileName); }
//HO 

/******************************************************************************
** ACCESS
******************************************************************************/
//HO inline std::map<ContextId, GenericNodeId> * getAllContexts(void) { return mData->getAllContexts(); }
//HO 

/******************************************************************************
** CONTEXT FORMATION AND MANIPULATION
******************************************************************************/
//HO inline int makeDualContexts(std::string fileName, int numNaiveContigs, bool clearContexts, bool supp) { return mData->makeDualContexts(fileName, numNaiveContigs, clearContexts, supp); }
//HO
//HO inline bool makeSingContexts(std::string fileName, int startContig, int numNaiveContigs) { return mData->makeSingContexts(fileName, startContig, numNaiveContigs); }
//HO

/******************************************************************************
** CONTEXT FINISHING
******************************************************************************/
//HO inline bool overlaperate(sMDInt minOlap, sMDInt maxWastage) { return mData->overlaperateAll(minOlap, maxWastage); }
//HO 
//HO inline void renumber(bool setStartEndNodes) { mData->renumberAll(setStartEndNodes); }
//HO 
//HO inline std::string getContigEnd(uMDInt len, bool isStart, bool rc, ContextId CTXID) { return mData->getContigEnd(len, isStart, rc, CTXID); }
//HO 

/******************************************************************************
** PRINTING AND OUTPUT
******************************************************************************/
//HO inline void getContigSequence(std::string * sequence, int trimFront, int trimEnd, bool doGC, GenericNodeId * startNode, bool doRC, ContextId CTXID) { mData->getContigSequence(sequence, trimFront, trimEnd, doGC, startNode, doRC, CTXID); }
//HO 
//HO inline void printContextGraph(ContextId CTXID) { mData->printContextGraph(CTXID); }
//HO 
//HO inline void printAllEdges(ContextId CTXID, bool showBanished) { mData->printAllEdges(CTXID, showBanished); }
//HO 
//HO inline void makeAllContigs(void) { mData->makeAllContigs(); }
//HO 
//HO inline void printAllContigs(std::string fileName, int * n50, int * totalBases, int * longestContig, int * numContigs) { mData->printAllContigs(fileName, n50, totalBases, longestContig, numContigs); }
//HO
//HO inline void cutRawSeq(std::string * raw_seq, std::ostringstream * buffer) { mData->cutRawSeq(raw_seq, buffer); }
//HO 
