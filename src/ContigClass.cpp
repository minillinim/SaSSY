/******************************************************************************
**
** File: ContigClass.cpp
**
*******************************************************************************
**
** Implements the methods needed to make Contigs. Can make and print contigs
** from any generic node graph. You need to choode which edge set to use (dual or 
** uni)
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
******************************************************************************/

// system includes
#include <iostream>

#include <queue>
//INC #include <queue>
//INC 

//INC #include "nodeblockdef.h"
//INC 
#include "nodeblockdef.h"

//INC #include "StatsManager.h"
//INC 
#include "StatsManager.h"

// local includes
#include "ContigMemWrapper.h"
#include "intdef.h"
#include "IdTypeDefs.h"
#include "ContigClass.h"

using namespace std;

//PV int mN50;
//PV 
//PV int mLongestContig;
//PV 
//PV int mTotalBases;
//PV 
/******************************************************************************
** #INCLUDES #DEFINES AND PRIVATE VARS
******************************************************************************/

/******************************************************************************
** INTIALISATION AND SAVING
******************************************************************************/

/*HV*/ void
ContigClass::
/*HV*/ extendedDestructor(void)
//HO ;
{
    //-----
    // Add anything you would like to happen when the destructor is called
    //
}
//HO 

/*HV*/ bool
ContigClass::
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
ContigClass::
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
ContigClass::
/*HV*/ extendedSave(std::string fileName)
//HO ;
{
    //-----
    // Add anything you would like to happen when saving (including saving private vars)
    //
    return true;
}
//HO 

    /*
    ** Inline wrapper for setting the external object pointers
    */
//HO inline void initialise2(GenericNodeClass * GNC, StatsManager * SM, NODE_CLASS nc) { mN50  = 0; mLongestContig = 0; mTotalBases = 0; mData->initialise2(GNC, SM, nc); }
//HO 

//HO inline void setCurrentContextId(ContextId CTXID) { mData->setCurrentContextId(CTXID); }
//HO 
//HO inline ContextId getCurrentContextId(void) { return mData->getCurrentContextId(); }
//HO 

/******************************************************************************
** EXTENSIONS
******************************************************************************/
    /*
    ** Inline wrapper for cutting contigs
    */
//HO inline ContigId splice(void) { mN50  = 0; mLongestContig = 0; mTotalBases = 0; return mData->startSplice(); }
//HO 
    /*
    ** Inline wrapper for cutting contigs restricted to a particular context
    */
//HO inline ContigId splice(ContextId CTXID) { mN50  = 0; mLongestContig = 0; mTotalBases = 0; return mData->startSplice(CTXID); }
//HO 
//HO inline void sortContigs(void) { mData->sortContigs(); }
//HO 

    /*
    ** Inline wrapper for cutting contigs restricted to a particular context
    */
//HO inline ContigId getNthLongestContig(int nth) { return mData->getNthLongestContig(nth); }
//HO 

//HO inline void reverseContig(ContigId CID) { mData->reverseContig(CID); }
//HO 
//HO inline GenericNodeId getStart(ContigId CID) { return mData->getTrueStartNode(CID); }
//HO 
//HO inline GenericNodeId getNextStart(ContigId CID) { return mData->getTrueNextStartNode(CID); }
//HO 
//HO inline GenericNodeId getEnd(ContigId CID) { return mData->getTrueEndNode(CID); }
//HO 
//HO inline GenericNodeId getNextEnd(ContigId CID) { return mData->getTrueNextEndNode(CID); }
//HO 

/******************************************************************************
** NAIVE++
******************************************************************************/
    /*
    ** Inline wrapper for making supplementry naive contigs
    */
//HO inline int makeSupplementaryNaiveContigs(std::string fileName) { return mData->makeSupplementaryNaiveContigs(fileName); }
//HO 

/******************************************************************************
** PRINTING AND OUTPUT
******************************************************************************/

    /*
    ** Inline wrapper for printing basic contig info
    */
//HO inline void printContigInfo(void) { mData->printAllContigInfo(); }
//HO 
//HO inline bool makeContigStats(void) { return mData->makeContigStats(&mN50, &mLongestContig, &mTotalBases); }
//HO 
//HO inline int getN50(void) { return mN50; }
//HO 
//HO inline int getLongestContigLength(void) { return mLongestContig; }
//HO 
//HO inline int getTotalBases(void) { return mTotalBases; }
//HO 
//HO inline int getNumContigs(void) { return mData->getNumContigs(); }
//HO 

    /*
    ** Inline wrappers for printing contigs
    */

//HO inline void printContigs(std::string fileName) { mData->printAllContigs(fileName); }
//HO 
//HO inline void printMappings(std::string fileName) { mData->printAllMappings(fileName); }
//HO 

//HO inline void printContig(std::string fileName, ContigId CID) { mData->printContig(fileName, CID); }
//HO 
//HO inline void getContigSequence(std::string * rawSeq, ContigId CID) { mData->getContigSequence(rawSeq, 0, 0, CID); }
//HO 
//HO inline void getContigSequence(std::string * rawSeq, bool doGC, ContigId CID) { mData->getContigSequence(rawSeq, doGC, CID); }
//HO 
//HO inline void getContigSequence(std::string * sequence, int trimFront, int trimEnd, bool doGC, GenericNodeId * startNode, bool doRC, ContigId CID) { mData->getContigSequence(sequence, trimFront, trimEnd, doGC, startNode, doRC, CID); }
//HO 
//HO inline int getContigSequenceLengthManual(ContigId CID) { return mData->getContigSequenceLengthManual(CID); }
//HO 
//HO inline std::string getContigEnd(uMDInt len, bool rc, GenericNodeId startNode, ContigId CID) { return mData->getContigEnd(len, rc, startNode, CID); }
//HO 
//HO inline void getNthLongestContigSequence(std::string * rawSeq, int nth) { mData->getContigSequence(rawSeq, 0, 0, mData->getNthLongestContig(nth)); }
//HO 
//HO inline void cutRawSeq(std::string * raw_seq, std::ostringstream * buffer) { mData->cutRawSeq(raw_seq, buffer); }
//HO 

    /*
    ** Inline wrapper for printing contig graphs
    */

//HO inline void printContigGraph(bool show_names) { mData->printContigGraph(show_names); }
//HO 
//HO inline void printContigGraph(bool show_names, ContextId CTXID) { mData->printContigGraph(show_names, CTXID); }
//HO 

    /*
    ** Inline wrapper for printing contig coverage info
    */
    
//HO inline void writeNaiveCoverageMap(std::string fileName) { mData->writeNaiveCoverageMap(fileName); }
//HO 
