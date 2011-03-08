/******************************************************************************
**
** File: ContextMemWrapper.cpp
**
*******************************************************************************
** 
** This is the layer 2 implementation files for making and managing Contexts
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

/******************************************************************************
** #INCLUDES #DEFINES AND PRIVATE VARS
******************************************************************************/
// system includes
#include <vector>
//INC #include <vector>
//INC 

#include <map>
//INC #include <map>
//INC 

#include <queue>
//INC #include <queue>
//INC 

#include <deque>
//INC #include <deque>
//INC 

#include <list>
//INC #include <list>
//INC 

#include <iostream>
#include <fstream>
#include <sstream>
#include <limits.h>
#include <cmath>

// local includes
#include "ContextMemWrapper.h"
//INC #include "ContextMemWrapper.h"
//INC 

#include "StatsManager.h"
//INC #include "StatsManager.h"
//INC 

#include "GenericNodeClass.h"
//INC #include "GenericNodeClass.h"
//INC 

#include "NodePositionMapClass.h"
//INC #include "NodePositionMapClass.h"
//INC 

#include "TreeHistory.h"
//INC #include "TreeHistory.h"
//INC 

#include "ContigClass.h"
//INC #include "ContigClass.h"
//INC 

#include "Dataset.h"
//INC #include "Dataset.h"
//INC 

#include "SortedTreeBranchManager.h"
//INC #include "SortedTreeBranchManager.h"
//INC 

#include "NodeCheckClass.h"
//INC #include "NodeCheckClass.h"
//INC 

#include "nodeblockdef.h"
//INC #include "nodeblockdef.h"
//INC 

#include "intdef.h"
#include "IdTypeDefs.h"
#include "paranoid.h"
#include "Utils.h"

    // we use this class when we are trying to overlaperate contexts
//INC class CTX_OLAP_CONTENDER {
//INC 
//INC     public:
//INC 
//INC         CTX_OLAP_CONTENDER(GenericNodeClass * GNC, bool valid) { COC_GNC = GNC; COC_Valid = valid; COC_NodeLength = 0; COC_OlapLength = 0; COC_LongAscending = false; COC_LongEndPos = 0; COC_Contained = false;}
//INC 
//INC         CTX_OLAP_CONTENDER(GenericNodeClass * GNC, bool valid, GenericNodeId startShort, GenericNodeId startLong, ContextId shortCtx, ContextId longCtx, uMDInt longEndPos, uMDInt readLength) { COC_GNC = GNC; COC_Valid = valid; COC_NodeLength = 1; COC_OlapLength = readLength; COC_StartShort = startShort; COC_StartLong = startLong; COC_ShortContext = shortCtx; COC_LongContext = longCtx; COC_LongAscending = false; COC_LongEndPos = longEndPos; COC_Contained = false;}
//INC 
//INC         void printContents(uMDInt startPos_S, uMDInt startPos_L)
//INC 
//INC         {
//INC               std::string val_str = "Valid ";
//INC 
//INC               if(!COC_Valid) { val_str = "Invalid "; }
//INC 
//INC               if(COC_NodeLength > 3) { std::cout << val_str << "Olap between (S):" << COC_ShortContext << " {" << (COC_GNC->getCntxPos(COC_StartShort) - startPos_S) << ", " << (COC_GNC->getCntxPos(COC_EndShort) - startPos_S) << "} and (L) " << COC_LongContext << " {" << (COC_GNC->getCntxPos(COC_StartLong) - startPos_L) << ", " << (COC_GNC->getCntxPos(COC_EndLong) - startPos_L) << "} Nodes: " << COC_NodeLength << " Overlap: " << COC_OlapLength << std::endl; }
//INC 
//INC               else { std::cout << val_str << "Olap between (S):" << COC_ShortContext << " and (L) " << COC_LongContext << " NL: " << COC_NodeLength << " OL: " << COC_OlapLength << std::endl; }
//INC 
//INC               std::cout << "Short Ends: " << COC_StartShort << " : " << COC_NextStartShort << " : " << COC_NextEndShort << " : " << COC_EndShort << std::endl;
//INC 
//INC               std::cout << "Long Ends: " << COC_StartLong << " : " << COC_NextStartLong << " : " << COC_NextEndLong << " : " << COC_EndLong << std::endl;
//INC 
//INC               std::cout << "LEP: " << COC_LongEndPos << " ASC: " << COC_LongAscending << " Cont: " << COC_Contained << std::endl;
//INC 
//INC         }
//INC 
//INC         GenericNodeId COC_StartShort;
//INC 
//INC         GenericNodeId COC_NextStartShort;
//INC 
//INC         GenericNodeId COC_EndShort;
//INC 
//INC         GenericNodeId COC_NextEndShort;
//INC 
//INC         GenericNodeId COC_StartLong;
//INC 
//INC         GenericNodeId COC_NextStartLong;
//INC 
//INC         GenericNodeId COC_EndLong;
//INC 
//INC         GenericNodeId COC_NextEndLong;
//INC 
//INC         ContextId COC_ShortContext;
//INC 
//INC         ContextId COC_LongContext;
//INC 
//INC         uMDInt COC_OlapLength;
//INC 
//INC         uMDInt COC_NodeLength;
//INC 
//INC         uMDInt COC_LongEndPos;
//INC 
//INC         bool COC_LongAscending;
//INC 
//INC         bool COC_Contained;
//INC 
//INC         bool COC_Valid;
//INC 
//INC         GenericNodeClass * COC_GNC;
//INC 
//INC };
//INC 

    // used to store information about how contexts are to be merged
//INC class C_OLAP {
//INC 
//INC   public:
//INC 
//INC       C_OLAP(GenericNodeId initID) { CO_AStart = initID; CO_AEnd = initID; CO_BStart = initID; CO_BEnd = initID; CO_ANextStart = initID; CO_ANextEnd = initID; CO_BNextStart = initID; CO_BNextEnd = initID; CO_Contained = false; CO_Length = 0; }
//INC 
//INC       void printContents()
//INC 
//INC       {
//INC 
//INC           std::cout << "(L) " << CO_ConA << " S: " << CO_AStart << " NS: " << CO_ANextStart << " NE: " << CO_ANextEnd << " E: " << CO_AEnd << std::endl;
//INC 
//INC           std::cout << "(S) " << CO_ConB << " S: " << CO_BStart << " NS: " << CO_BNextStart << " NE: " << CO_BNextEnd << " E: " << CO_BEnd << std::endl;
//INC 
//INC           std::cout << "C: " << CO_Contained << " L: " << CO_Length << std::endl;
//INC
//INC       }
//INC 
//INC       ContextId CO_ConA;
//INC
//INC       ContextId CO_ConB;
//INC
//INC       GenericNodeId CO_AStart;
//INC 
//INC       GenericNodeId CO_AEnd;
//INC 
//INC       GenericNodeId CO_ANextStart;
//INC 
//INC       GenericNodeId CO_ANextEnd;
//INC 
//INC       GenericNodeId CO_BStart;
//INC 
//INC       GenericNodeId CO_BEnd;
//INC 
//INC       GenericNodeId CO_BNextStart;
//INC 
//INC       GenericNodeId CO_BNextEnd;
//INC 
//INC       bool CO_Contained;                  // if CO_Contained is true then Context B is completely contained within Context A
//INC 
//INC       uMDInt CO_Length;                   // the effective length of this overlap (if !contained)
//INC 
//INC };

    // used during overlaperation to sort Contexts by length
/*INC*/ typedef std::pair<ContextId, uMDInt> conLengthPair;
/*INC*/ 
/*INC*/ typedef std::deque<conLengthPair> conLengthQueue;
/*INC*/ 
/*INC*/ typedef std::deque<conLengthPair>::iterator conLengthQueueIterator;
/*INC*/ 

    // for quick retrieval of node position maps
/*INC*/ typedef map<ContextId, NodePositionMapClass *> contextNPMLinker;
/*INC*/ 
/*INC*/ typedef map<ContextId, NodePositionMapClass *>::iterator contextNPMLinkerIterator;
/*INC*/ 
    
    // used during extension to store fresh directions
/*INC*/ typedef std::pair<NodeCheckId, sMDInt> contextShoot;
/*INC*/ 
/*INC*/ typedef deque< contextShoot > shootQueue;
/*INC*/ 
/*INC*/ typedef deque< contextShoot >::iterator shootQueueIterator;
/*INC*/ 

    // custom classes we'll need to store our data
//PV GenericNodeClass * mNodes;                         // stores nodes
//PV 
//PV StatsManager * mStatsManager;                      // stores stats
//PV 
//PV ContigClass * mContigs;                            // stores contigs and their sequence properties
//PV 
//PV DataSet * mDataSet;                                // access to file stats
//PV 
//PV TreeHistory mHistories;                            // helps manage branches in each context
//PV 

    // command line options
//PV uMDInt mMaxOffset;
//PV 
//PV uMDInt mNaiveOffset;
//PV 
//PV uMDInt mMinOlap;
//PV 
//PV uMDInt mMaxWastage;
//PV 
//PV bool mStrict;
//PV 

    // all of our contexts
//PV std::map<ContextId, GenericNodeId> mAllContexts;
//PV 
// mapping positions of masters
//PV contextNPMLinker mHeadNPMs;
//PV 
//PV contextNPMLinker mMasterNPMs;
//PV 

    // header of the fasta sequence we're currently extending
//PV std::string mFastaHeader;
//PV 

    // use these guys alot, so why not just store them
//PV ContigId CTG_NULL_ID;
//PV 
//PV GenericNodeId GN_NULL_ID;
//PV 
//PV NodeCheckId NC_NULL_ID;
//PV 

    // naive coverage -> used during stretching to help avoid making chimeras
//PV std::map<ContigId, double> mCoverageMap;
//PV 
//PV double mLowerStretchCoverageCutoff;
//PV 
//PV double mUpperStretchCoverageCutoff;
//PV 
//PV double mCovCutPercent;
//PV 

    // safely store nodes...
//PV NodeCheckClass * mNodeCheck;
//PV 
//PV NodeCheckClass * mShootCheck;
//PV 

    // used during joining
//PV vector<GenericNodeId> mMappedNodes;
//PV 
//PV vector<uMDInt> mMappedPositions;
//PV 
//******************************************************************************
// INITIALISATION AND SAVING
//******************************************************************************

/*HV*/ void
ContextMemWrapper::
/*HV*/ extendedDestructor(void)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // Add anything you would like to happen when the destructor is called
    //
    std::map<ContextId, GenericNodeId>::iterator all_contexts_iter = mAllContexts.begin();
    while(all_contexts_iter != mAllContexts.end())
    {
        ContextId curr_CTXID = all_contexts_iter->first;
        if(isValidAddress(curr_CTXID))
            deleteContext(curr_CTXID);
        else
        {
            logError(curr_CTXID << " is not a valid address");
            return;
        }
        all_contexts_iter++;
    }

    if(mHeadNPMs.size() > 0)
    {
        contextNPMLinkerIterator npm_iter = mHeadNPMs.begin();
        contextNPMLinkerIterator npm_last = mHeadNPMs.end();
        while(npm_iter != npm_last)
        {
            if(npm_iter->second != NULL)
                delete npm_iter->second;
            npm_iter++;
        }
        mHeadNPMs.clear();
    }
    if(mMasterNPMs.size() > 0)
    {
        contextNPMLinkerIterator npm_iter = mMasterNPMs.begin();
        contextNPMLinkerIterator npm_last = mMasterNPMs.end();
        while(npm_iter != npm_last)
        {
            if(npm_iter->second != NULL)
                delete npm_iter->second;
            npm_iter++;
        }
        mMasterNPMs.clear();
    }
    if(mContigs != NULL)
    {
        delete mContigs;
        mContigs = NULL;
    }
    
    if(mNodeCheck != NULL)
    {
        delete mNodeCheck;
        mNodeCheck = NULL;
    }
    
    if(mShootCheck != NULL)
    {
        delete mShootCheck;
        mShootCheck = NULL;
    }
}
//HO 

/*HV*/ bool
ContextMemWrapper::
/*HV*/ extendedInitialise(uMDInt originalNumberOfBlocks )
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // Add anything you would like to happen on a regular initialisation call
    //
    mContigs = NULL;
    mNodes = NULL;
    mStatsManager = NULL;
    mDataSet = NULL;
    mNodeCheck = NULL;
    mShootCheck = NULL;
    
    mStrict = true;
    mCovCutPercent = -1;
    
    // set these defaults
    mMaxOffset = SAS_DEF_UN_OFFSET_DEF_MAX;
    mNaiveOffset = SAS_DEF_UN_OFFSET_DEF_MIN;
    
    return true;
}
//HO 

/*HV*/ bool
ContextMemWrapper::
/*HV*/ extendedInitialise(std::string fileName)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // Add anything you would like to happen when reloading a saved instance
    //
    mContigs = NULL;
    mNodes = NULL;
    mStatsManager = NULL;
    mDataSet = NULL;
    mNodeCheck = NULL;
    mShootCheck = NULL;

    mStrict = true;
    mCovCutPercent = -1;
    
    return true;
}
//HO 

/*HV*/ bool
ContextMemWrapper::
/*HV*/ extendedSave(std::string fileName)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // Add anything you would like to happen when saving (including saving private vars)
    //
    return true;
}
//HO 

//******************************************************************************
// HOUSEKEEPING
//******************************************************************************

/*HV*/ void
ContextMemWrapper::
/*HV*/ setObjectPointers(GenericNodeClass * GNC, StatsManager * SM)
//HO ;
{ // logInfo(__LINE__,1);
    mNodes = GNC;
    mStatsManager = SM;
    mDataSet = mStatsManager->getDS();

    // we need to have the null contig ID so we can be a bit dodgy here...
    if(mContigs != NULL)
    {
        delete mContigs;
        mContigs = NULL;
    }
    mContigs = new ContigClass();
    mContigs->initialise(2);
    mContigs->initialise2(mNodes, mStatsManager, DUALNODE);
    CTG_NULL_ID = mContigs->getNullContigId();
    delete mContigs;
    mContigs = NULL;

    mFastaHeader = "UNSET";
    
    GN_NULL_ID = mNodes->getNullGenericNodeId();
    
    // set the object pointers for the nodecheckers...
    if(mNodeCheck != NULL)
        delete mNodeCheck;
    mNodeCheck = new NodeCheckClass();
    PARANOID_ASSERT_L4(mNodeCheck != NULL);
    
    if(mShootCheck != NULL)
        delete mShootCheck;
    mShootCheck = new NodeCheckClass();
    PARANOID_ASSERT_L4(mShootCheck != NULL);
        
    mNodeCheck->initialise((int)SAS_DEF_CTX_NC_NODE_SIZE);
    mNodeCheck->setObjectPointers(mNodes, &mMasterNPMs, "Nodes");
    
    mShootCheck->initialise((int)SAS_DEF_CTX_NC_SHOOT_SIZE);
    mShootCheck->setObjectPointers(mNodes, &mMasterNPMs, "Shoots");
    
    NC_NULL_ID = mNodeCheck->getNullNodeCheckId();
    
}
//HO 

//HO inline void setStrict(bool strict_val) { mStrict = strict_val; } 
//HO 

//HO inline void setOffsets(uMDInt mo, uMDInt no) { mMaxOffset = mo; mNaiveOffset = no; } 
//HO 

//******************************************************************************
// ACCESS
//******************************************************************************

//HO inline std::map<ContextId, GenericNodeId> * getAllContexts(void) { return &mAllContexts; }
//HO 

//******************************************************************************
// CONTEXT FORMATION
//******************************************************************************

/*HV*/ int
ContextMemWrapper::
/*HV*/ makeDualContexts(std::string fileName, int numNaiveContigs, bool clearContexts, bool supp)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // Make paired end contexts and return how far through the file we got
    //
    int current_naive_entry = 1;
    int num_dual_contexts = 0;
    
    // clear any old contexts we make
    if(clearContexts)
        mAllContexts.clear();
    ContextId current_context;

    // go through all the naive contigs in the file
    // and make contexts
    logInfo("Starting context extension", 2);
    std::cout << "Starting seed extension" << std::endl;
    
    bool reached_limit = false;
    while(current_naive_entry <= numNaiveContigs)
    {
        logInfo("Extending sequence: " << current_naive_entry, 2);
        current_context = extendOn(&reached_limit, fileName, supp, current_naive_entry, 0, SAS_DEF_TOLL_MIN, SAS_DEF_TOLL_MAX, SAS_DEF_TOLL_STEPS);
        if(current_context != getNullContextId())
        {
            mAllContexts[current_context] = GN_NULL_ID;
            num_dual_contexts++;
        }
        if(reached_limit)
            break;
        current_naive_entry++;
    }

    logInfo("Made: " << num_dual_contexts << " doubly mapped contexts" ,2);
    std::cout << "Made: " << num_dual_contexts << " doubly mapped contexts" << std::endl;
    return current_naive_entry;
}
//HO 

/*HV*/ bool
ContextMemWrapper::
/*HV*/ makeSingContexts(std::string fileName, int startContig, int numNaiveContigs)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // Make all the single (unpaired contexts)
    //
    int num_sing_contexts = 0;
    int save_start = startContig;
    ContextId current_context;

    logInfo("Begin singular context creation at entry: " << startContig, 2);
    int tmp_counter = 0;
    int break_counter = 0;
    while(startContig <= numNaiveContigs)
    {
        current_context = extendOnSingular(fileName, startContig);
        if(current_context != getNullContextId())
        {
            mAllContexts[current_context] = GN_NULL_ID;
            num_sing_contexts++;
        }
        startContig++;
        tmp_counter++;
        if(2000 == tmp_counter)
        {
            logInfo(" ...Processed: " << (startContig - save_start) << " retained: " << num_sing_contexts, 2);
            tmp_counter = 0;
            break_counter++;
            if(break_counter == 20)
                break;
        }
    }
    logInfo("Made: " << num_sing_contexts << " singular contexts" ,2);
    std::cout << "Made: " << num_sing_contexts << " singular contexts" << std::endl;
    return true;
}
//HO 

    static shootQueue CMWG_EO_savePoint;
   
/*HV*/ ContextId
ContextMemWrapper::
/*HV*/ extendOn(bool * reachedLimit, std::string fileName, bool supp, int whichEntry, int longestContigLength, double tollMin, double tollMax, double tollSteps)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // Given a seed contig, the goal here is to create a overlap-pair graph and then extend as far as possible by double mapping 
    // read pairs. Double mapping must be done within the tolerance limits set.
    // On load, mDataSet has been updated using the longest seed contig and has been written to string.
    // Insert libs have been ranked accordingly.
    //
    std::string read_file_name;
    if(supp)
        read_file_name = fileName + SAS_DEF_SUPP_CONTIG_EXT;
    else
        read_file_name = fileName + SAS_DEF_NAIVE_CONTIG_EXT;

    logInfo("Start try making Context about sequence: " << whichEntry << " in: " << read_file_name, 5);

    // first lets check to see if the sequence is long enough
    std::string raw_sequence = "";

    if(!getPerfectFasta(&raw_sequence, read_file_name, whichEntry))
    {
        logError("Problem loading sequence");
        *reachedLimit = true;
        return CTX_NULL_ID;
    }

    int original_length = raw_sequence.length();
    logInfo("Seed length: " << original_length, 5);

    uMDInt read_length = mStatsManager->getReadLength();

    // trim the sequence
    uMDInt initial_trim_length = mMaxOffset + 1;
    uMDInt trim_length = read_length;

    // the ends of the sequences may contain errors
    // trim off just the very ends, (If they're right we'll get them back)
    if(!supp)
    {
        if(raw_sequence.length() > 2 * read_length)
        {
            raw_sequence = raw_sequence.substr(initial_trim_length, raw_sequence.length() - 2 * initial_trim_length);
        }
    }

    // check to see that the sequence is long enough
    uMDInt LUC = mDataSet->getLowestUpperCut();
    if((raw_sequence.length() < LUC) || ((uMDInt)(raw_sequence.length()) <= (uMDInt)(2 * read_length)))
    {
        logInfo("Sequence: " << whichEntry << "(" << raw_sequence.length() << ") in file: " << fileName << " is too short to extend on (<"<< LUC <<"). Exiting extension phase.", 4);
        *reachedLimit = true;
        return CTX_NULL_ID;
    }

    // we need to know if we made an extension
    int last_length = 0;
    int current_length = raw_sequence.length();
    bool made_shorter = false;

    // take the first and last rl bases for comparison in the case of same length strings
    std::string old_front = raw_sequence.substr(0, read_length);
    std::string old_end = raw_sequence.substr(raw_sequence.length()-read_length, read_length);
    std::string new_front = "";
    std::string new_end = "";

    // make us a new context
    ContextId CTXID = newContextId();
    mNodes->setCurrentContextId(CTXID);
    mNodes->addContextBlocks(CTXID);

    // determine tolerances to test along different assemblies
    double tolerance, toll_step;
    toll_step = (tollMax - tollMin)/(tollSteps);
    tolerance = tollMin;

    // out node position maps
    NodePositionMapClass * master_npm = NULL;
    NodePositionMapClass * head_npm = NULL;

    // for when we fail
    double backup_tolerance = 0;
    std::string backup_sequence = "";

    std::string backup_sequence_joining = "";
    double backup_tolerance_joining = 0;               // if joining was successful then we can save the seq and tolerance which worked,
                                                       // however it would be unwise to overwrite the true backup until after extension is proven
    unsigned int itteration_number = 1;

    // after we fail we need to force a break
    bool force_break = false;
    int num_loops = 0;

    while(current_length >= last_length)
    {
        // clear what's left of the last round - if it's not the first round...
        if( 1 < itteration_number )
        {
            // we had a success with extension.
            backup_sequence = backup_sequence_joining;
            backup_tolerance = backup_tolerance_joining;
        }
        
        // should check to see if we have got the same sequence
        // gauranteed to enter this part in the first round
        // last_length = 0.
        if(raw_sequence.length() >= 2 * read_length)
        {
            new_front = raw_sequence.substr(0, read_length);
            new_end = raw_sequence.substr(raw_sequence.length()-read_length, read_length);
        }
        else
        {
            // this sequence is too short!
            if( 1 < itteration_number )
                made_shorter = true;
            break;
        }
        if(current_length == last_length)
        {
            logInfo(new_front << " : " << new_end, 8);
            logInfo(old_front << " : " << old_end, 8);
            // check to see that even though the length is the same, that the sequence is different.
            // if it is the same then break out of this loop. This can not happen on the first round so
            // backup seeds/tolerance should be OK. We can get stuck in long loops if we're not careful...
            num_loops++;
            if(num_loops > 2)
                break;

            if(new_front == old_front)
            {
                if(new_end == old_end)
                {
                    made_shorter = true;
                    break;
                }
            }
            else
            {
                mNodes->revCmpSequence(&new_front);
                if(new_front == old_end)
                {
                    mNodes->revCmpSequence(&new_end);
                    if(new_end == old_front)
                    {
                        made_shorter = true;
                        break;
                    }
                }
            }
        }
        old_front = new_front;
        old_end = new_end;

        if( 1 < itteration_number )
        {
            // the ends of the sequences may contain errors
            // trim off just the very ends, (If they're right we'll get them back)
            if(raw_sequence.length() > 2 * trim_length)
            {
                raw_sequence = raw_sequence.substr(trim_length, raw_sequence.length() - 2 * trim_length);
            }

            // reset the master edges and npms if not the first round..
            // reset heads here too.
            mNodes->resetDNEdges(true, CTXID);
            if(!clearNPMs(CTXID))
                logError("NO NPM");
        }

        // make new npms
        head_npm = new NodePositionMapClass();
        PARANOID_ASSERT_L2(head_npm != NULL);
        head_npm->initialise(SAS_DEF_CTX_NPM_SIZE);
        master_npm = new NodePositionMapClass();
        PARANOID_ASSERT_L2(master_npm != NULL);
        master_npm->initialise(SAS_DEF_CTX_NPM_SIZE);

        // insert the npms.
        mHeadNPMs[CTXID] = head_npm;
        mMasterNPMs[CTXID] = master_npm;

        // map the nodes to the sequence
        mapPerfectFasta(head_npm, &raw_sequence);

// TRY CLOSE THE CONTEXT
        // try to join the context
        // keep trying until it works
        bool joined_ok = false;
        while(tolerance <= tollMax)//toll_high)
        {
            if(1 == itteration_number)
            {
                // first round, we need to join strictly
                mStatsManager->makeCutOffs(tolerance);
                joined_ok = join(CTXID);
            }
            else
            {
                // no need to make cut offs becuase joinRelaxed will set the cutoff to SAS_DEF_TOLL_MAX anyway
                joined_ok = joinRelaxed(CTXID);
            }

            if(joined_ok)
            {
                // make backups. we use the "joining" vars because even though this joined OK it may not extend.
                backup_tolerance_joining = tolerance;
                backup_sequence_joining = raw_sequence;
                break;
            }
            else
            {
                // we couldn't join.
                // clear the mapping to this context
                // but retain the mapped heads
                mNodes->resetDNEdges(false, CTXID);
        
                // clear the master npm
                // and then make a new one
                contextNPMLinkerIterator finder = mMasterNPMs.find(CTXID);
                if(finder != mMasterNPMs.end())
                {
                    if(finder->second != NULL)
                    {
                        delete finder->second;
                    }
                    mMasterNPMs.erase(finder);
                }
                else
                    logError("Cannot find a master NPM for: " << CTXID << " LINE: " << __LINE__ << " : " << __FILE__ );

                master_npm = new NodePositionMapClass();
                PARANOID_ASSERT_L2(master_npm != NULL);
                master_npm->initialise(SAS_DEF_CTX_NPM_SIZE);
                mMasterNPMs[CTXID] = master_npm;

                // try the next largest tolerance
                tolerance += toll_step;
            }
        }

        if(!joined_ok)
        {
            // all tolerances failed. We could not join
            if(1 == itteration_number)
            {
                // this was a failure on the first attempt to join
                // we should try to join_relaxed now just in case...
                // this baby should be ready to rock and roll
                logInfo("Semi-relaxed join on first run", 6);
                if(!joinSemiRelaxed(CTXID))
                {
                    // this seed is fubar
                    std::cout << "Failed extending sequence: " << whichEntry << " in file " << fileName << SAS_DEF_NAIVE_CONTIG_EXT << ". This sequence may be lost!" << std::endl;
                    // reset and this time do the heads too...
                    mNodes->resetDNEdges(true, CTXID);
                    clearNPMs(CTXID);
                    deleteContext(CTXID);
                    return CTX_NULL_ID;
                }
                else
                {
                    backup_tolerance_joining = SAS_DEF_TOLL_MAX;
                    backup_sequence_joining = raw_sequence;
                }
            }
            else
            {
                // we must have already worked out a longer extension which joined...
                mNodes->resetDNEdges(true, CTXID);
                if(!clearNPMs(CTXID))
                    logError("NO NPM");

                // load the last good raw_sequence and tolerance
                raw_sequence = backup_sequence;
                tolerance = backup_tolerance;

                // make new npms
                head_npm = new NodePositionMapClass();
                PARANOID_ASSERT_L2(head_npm != NULL);
                head_npm->initialise(SAS_DEF_CTX_NPM_SIZE);
                master_npm = new NodePositionMapClass();
                PARANOID_ASSERT_L2(master_npm != NULL);
                master_npm->initialise(SAS_DEF_CTX_NPM_SIZE);

                // insert the npms.
                mHeadNPMs[CTXID] = head_npm;
                mMasterNPMs[CTXID] = master_npm;

                // remap with the old head, reset stats and then rejoin.
                // this is gauranteed to work
                mapPerfectFasta(head_npm, &raw_sequence);
                
                mStatsManager->makeCutOffs(tolerance);
                
                if(!joinRelaxed(CTXID))
                {
                    logError("Couldn't rejoin from backup");
                    mNodes->resetDNEdges(true, CTXID);
                    if(!clearNPMs(CTXID))
                        logError("NO NPM");
                    logInfo("   -- " << whichEntry << ": could not be extended", 1);
                    deleteContext(CTXID);
                    return CTX_NULL_ID;
                }

                // after we re-build the context we will exit this while loop
                force_break = true;
                logInfo("Previous extension could not be re-joined - reverting to previous longest extension", 6);
            }
        }

        // we need some kind of save point. We can use a shootQueue for this
        // this shoot queue must not be passed through to a function which
        // will attach temporary shoots. pass a copy instead
        initialiseShoots(&CMWG_EO_savePoint, CTXID);

// CORRECT FOR INSTANCES WHERE JOINING ONLY RECOVERED A PARTIAL SEED
        // in the case of non-sassy seeds, we may not be able to join on the entire sequence
        // We only allow this to happen in the first round
        if(1 == itteration_number)
        {
            // the shoot queue size will always contain exactly two ends
            // we just need to make sure that they are the right
            // distance apart.
            uMDInt least_pos, greatest_pos, tmp_pos_front, tmp_pos_back;
            GenericNodeId shoot_front_GID = mShootCheck->checkOutNode(false, CTXID, (CMWG_EO_savePoint.front()).first);
            GenericNodeId shoot_back_GID = mShootCheck->checkOutNode(false, CTXID, (CMWG_EO_savePoint.back()).first);
            tmp_pos_front = mNodes->getCntxPos(shoot_front_GID);
            tmp_pos_back = mNodes->getCntxPos(shoot_back_GID);
            if(tmp_pos_back > tmp_pos_front)
            {
                least_pos = tmp_pos_front;
                greatest_pos = tmp_pos_back;
            }
            else
            {
                least_pos = tmp_pos_back;
                greatest_pos = tmp_pos_front;
            }

            uMDInt extender_length = greatest_pos - least_pos + read_length;

            if(raw_sequence.length() != extender_length)
            {
                // the extender must be shorter
                logInfo("Raw sequence at: " << raw_sequence.length() << " bases but shoots give: " << extender_length, 4);
                raw_sequence = raw_sequence.substr((least_pos - SAS_DEF_CONTEXT_START), extender_length);

                // there will be many nodes floating in limbo.
                // we know we can join at the backup tolerance so let's reset everything and redo it again....
                mNodes->resetDNEdges(true, CTXID);
                if(!clearNPMs(CTXID))
                    logError("NO NPM");

                // load the tolerance which brought us here in the first place
                tolerance = backup_tolerance_joining;

                // make new npms
                head_npm = new NodePositionMapClass();
                PARANOID_ASSERT_L2(head_npm != NULL);
                head_npm->initialise(SAS_DEF_CTX_NPM_SIZE);
                master_npm = new NodePositionMapClass();
                PARANOID_ASSERT_L2(master_npm != NULL);
                master_npm->initialise(SAS_DEF_CTX_NPM_SIZE);

                // insert the npms.
                mHeadNPMs[CTXID] = head_npm;
                mMasterNPMs[CTXID] = master_npm;

                // remap with the old head, reset stats and then rejoin.
                // this is gauranteed to work
                mapPerfectFasta(head_npm, &raw_sequence);
                
                mStatsManager->makeCutOffs(tolerance);
                
                // gauranteed first round. No need to join relaxed
                if(!join(CTXID))
                {
                    // something is not right here. Better just to bail.
                    if(!clearNPMs(CTXID))
                        logError("NO NPM");
                    deleteContext(CTXID);
                    return CTX_NULL_ID;
                }
                
                // this worked, save the sequence as the temporary backup
                backup_sequence_joining = raw_sequence;

                logInfo("Updating seed length to: " << extender_length, 1);

                if(raw_sequence.length() > read_length)
                {
                    new_front = raw_sequence.substr(0, read_length);
                    new_end = raw_sequence.substr(raw_sequence.length()-read_length, read_length);
                }
                else
                {
                    // this sequence is too short!
                    made_shorter = true;
                    break;
                }

                // just to keep everything kosher...
                current_length = raw_sequence.length();
                original_length = raw_sequence.length();
                old_front = new_front;
                old_end = new_end;

               // remake the shoots with the new closure...
               initialiseShoots(&CMWG_EO_savePoint, CTXID);
            }
        }

// START TRUE EXTENSION ALG...

        // keep these for stats
        int start_ex_length = raw_sequence.length();

        // start the growing engine on this savepoint.
        // the engine will keep growing until it runs out of steam
        doItterativeOptmalExtensions(&CMWG_EO_savePoint, tollMin, tollMax, tollSteps, CTXID);

        // now we need to know how long this sequence is...
        if(mContigs != NULL)
        {
            delete mContigs;
            mContigs = NULL;
        }
        mContigs = new ContigClass();
        PARANOID_ASSERT_L2(mContigs != NULL);
        mContigs->initialise(2);
        mContigs->initialise2(mNodes, mStatsManager, DUALNODE);
        mContigs->setCurrentContextId(CTXID);
        mContigs->splice(CTXID);

        // get the sequence for the longest contig and update the seed
        mContigs->getContigSequence(&raw_sequence, mContigs->getNthLongestContig(1));
        last_length = current_length;
        current_length = raw_sequence.length();

        // we're done with these contigs
        mNodes->resetDNContigs(CTXID);

        // if we are recovering from a failure to join and it is not the first round then we should 
        // have recovered the longest context, time to skoot!
        if(force_break)
            break;

        logInfo("   -- " << whichEntry << ": " << (current_length - start_ex_length) << " : [" << start_ex_length << " -> " << current_length << "]", 2);

        if(current_length < last_length)
        {
            // we must have somehow made this mo-fo shorter...
            made_shorter = true;
            break;
        }
        
        // increment and start again
        itteration_number++;
    }

    if(made_shorter)
    {
        // either way we need to clear this out
        mNodes->resetDNEdges(true, CTXID);
        if(!clearNPMs(CTXID))
        logError("NO NPM");

        if(1 == itteration_number)
        {
            // the extension failed on the first go.
            logInfo("   -- " << whichEntry << ": could not be extended", 1);
            deleteContext(CTXID);
            return CTX_NULL_ID;
        }
        else
        {
            // the final extension shortened the context!
            // we know we can build a longer sequence, we just need to remake it from the seed which originally gave us it
            // we have already worked out a longer extension which joined...

            // make new npms
            head_npm = new NodePositionMapClass();
            PARANOID_ASSERT_L2(head_npm != NULL);
            head_npm->initialise(SAS_DEF_CTX_NPM_SIZE);
            master_npm = new NodePositionMapClass();
            PARANOID_ASSERT_L2(master_npm != NULL);
            master_npm->initialise(SAS_DEF_CTX_NPM_SIZE);

            // insert the npms.
            mHeadNPMs[CTXID] = head_npm;
            mMasterNPMs[CTXID] = master_npm;

            // remap with the old head
            mapPerfectFasta(head_npm, &backup_sequence);

            // definitely NOT the first round, but maybe the first round is the only one which worked
            bool backup_join_ok = false;
            if(2 == itteration_number)
            {
                mStatsManager->makeCutOffs(backup_tolerance);
                backup_join_ok = join(CTXID);
            }
            else 
            {
                backup_join_ok = joinRelaxed(CTXID);
            }
            
            if(!backup_join_ok)
            {
                logError("Couldn't rejoin from backup");
                if(!clearNPMs(CTXID))
                    logError("NO NPM");
                logInfo("   -- " << whichEntry << ": could not be extended", 1);
                deleteContext(CTXID);
                return CTX_NULL_ID;
            }
            
            initialiseShoots(&CMWG_EO_savePoint, CTXID);
            doItterativeOptmalExtensions(&CMWG_EO_savePoint, tollMin, tollMax, tollSteps, CTXID);
            
            // now we need to know how long this sequence is...
            if(mContigs != NULL)
            {
                delete mContigs;
                mContigs = NULL;
            }
            mContigs = new ContigClass();
            PARANOID_ASSERT_L2(mContigs != NULL);
            mContigs->initialise(2);
            mContigs->initialise2(mNodes, mStatsManager, DUALNODE);
            mContigs->setCurrentContextId(CTXID);
            mContigs->splice(CTXID);
            
            mContigs->getContigSequence(&raw_sequence, mContigs->getNthLongestContig(1));
            current_length = raw_sequence.length();

            // we're done with these contigs
            mNodes->resetDNContigs(CTXID);
        }
    }

    // we may have extended the original naive contig or not
    // either way, return the context we're done for this sequence
    logInfo("   -- " << whichEntry << ". Final extension: " << (current_length-original_length) << " : [" << original_length << " -> " << current_length << "]", 1);
    setLongestContig((uMDInt)current_length, CTXID);
    if(!clearNPMs(CTXID))
        logError("NO NPM");

    return CTXID;
}
//HO 

/*HV*/ ContextId
ContextMemWrapper::
/*HV*/ extendOnSingular(std::string fileName, int whichEntry)
//HO ;
{ // logInfo(__LINE__,1);
    std::string read_file_name = fileName + SAS_DEF_NAIVE_CONTIG_EXT;
    logInfo("Making singular context about sequence: " << whichEntry << " in: " << read_file_name, 5);

    // first lets check to see if the sequence is long enough
    std::string raw_sequence = "";

    if(!getPerfectFasta(&raw_sequence, read_file_name, whichEntry))
    {
        logError("Problem loading sequence");
        return CTX_NULL_ID;
    }

    // make us a new context
    ContextId CTXID = newContextId();
    mNodes->setCurrentContextId(CTXID);
    mNodes->addContextBlocks(CTXID);

    // out node position maps
    NodePositionMapClass * head_npm = new NodePositionMapClass();
    PARANOID_ASSERT_L2(head_npm != NULL);
    head_npm->initialise(SAS_DEF_CTX_NPM_SML_SIZE);
    
    mHeadNPMs[CTXID] = head_npm;

    // map the nodes to the sequence
    mapPerfectFasta(head_npm, &raw_sequence);

    if(mMappedNodes.size() > 3)
    {
        // we need at least 4 nodes to be able to overlaperate them
        // "join" them
        if(singularJoin(CTXID))
        {
            return CTXID;
        }
        else
        {
            std::cout << "Failed joining singular sequence: " << whichEntry << " in file " << fileName << ". This sequence may be lost!" << std::endl;
        }
    }

    contextNPMLinkerIterator finder = mHeadNPMs.find(CTXID);
    if(finder != mHeadNPMs.end())
    {
        if(finder->second != NULL)
        {
            delete finder->second;
        }
        mHeadNPMs.erase(finder);
    }
    else
    {
        logError("Cannot find a head NPM for: " << CTXID);
    }
    
    deleteContext(CTXID);
    return CTX_NULL_ID;
}
//HO 

//******************************************************************************
// MAPPING and CONTEXT INITIALISATION
//******************************************************************************

/*HV*/ bool
ContextMemWrapper::
/*HV*/ getPerfectFasta(std::string * sequence, std::string fileName, int whichEntry)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // load a sequence from the fasta file into the string
    //
    // first we need to get to the correct entry in the fasta file (file is indexed from 1)
    fstream fasta_file;
    int f_counter = whichEntry;
    bool in_fasta = false;
    *sequence = "";
    fasta_file.open(fileName.c_str(), ios::in);
    if(fasta_file.is_open())
    {
        std::string line = "";
        stringstream buffer;
        
        while(getline(fasta_file , line))
        {
            if(line.substr(0, 1) == ">")
            {
                // this is a fasta identifier
                if(in_fasta)    // we are done!
                {
                    *sequence = buffer.str();
                    logInfo("Current: " << mFastaHeader, 6);
                    break;
                }
                f_counter--;
                if(0 == f_counter)
                {
                    // this is the right one!
                    in_fasta = true;
                    mFastaHeader = line;
                    // skip to the next line
                    if(!getline(fasta_file , line)) { fasta_file.close(); return false; }
                }
            }
            if(in_fasta)
                buffer << line;
        }
        // perhaps there is only one entry in the file
        if(in_fasta && sequence->length() == 0)
        {
            logInfo("Current: " << mFastaHeader, 6);
            *sequence = buffer.str();
        }
    }
    else
    {
        logError("Could not open file: " << fileName);
        return false;
    }
    
    fasta_file.close();
    
    if(!in_fasta)
    {
        logError("Sequence: " << whichEntry << " not found");
        return false;
    }
    
    // all must be OK
    return true;
}
//HO 

/*HV*/ void
ContextMemWrapper::
/*HV*/ mapPerfectFasta(NodePositionMapClass * npm, std::string * sequence)
//HO ;
{ // logInfo(__LINE__,1);
    //----
    // Map the head Generic Nodes to the sequence
    //
    // we cut kmers of length readLength and see what their genericNodeId is...
    mMappedNodes.clear();
    mMappedPositions.clear();
    int read_length = mStatsManager->getReadLength();
    int seq_length = sequence->length() - read_length + 1;
    int seq_used = 0;
    uMDInt pos = (uMDInt)SAS_DEF_CONTEXT_START;
    std::string kmer;
    GenericNodeId GID;
    while(seq_used < seq_length)
    {
        kmer = sequence->substr(seq_used, read_length);
        if(mNodes->getGn(&GID, &kmer))
        {
            // make sure this guy is kosher!
            uMDInt nt = mNodes->getNodeType(UNINODE, GID);
            if(UN_NODE_TYPE_BANISHED != nt && UN_NODE_TYPE_DETACHED != nt)
            {
                NPMHeadElem tmp_npm_elem;
                tmp_npm_elem.NPMHE_GID = GID;
                tmp_npm_elem.NPMHE_Position = pos;
                if(mNodes->getSequence(GID) != kmer)
                    tmp_npm_elem.NPMHE_Reversed = true;
                else
                    tmp_npm_elem.NPMHE_Reversed = false;
                npm->addElem(&tmp_npm_elem, GID);
                
                // push the node onto the list
                mMappedNodes.push_back(GID);
                mMappedPositions.push_back(pos);
            }
        }
        seq_used++;
        pos++;
    }
}
//HO 

static vector<GenericNodeId>::iterator CMWG_JF_mapped_nodes_iter, CMWG_JF_mapped_nodes_last, CMWG_JF_dummy_masters_iter, CMWG_JF_dummy_masters_last;
static vector<uMDInt>::iterator CMWG_JF_pos_iter;
static vector<GenericNodeId> CMWG_JF_dummy_masters;

/*HV*/ bool
ContextMemWrapper::
/*HV*/ singularJoin(ContextId CTXID)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // take a freshly mapped context and map the dualNode Pairs to it
    // however these mapped contexts are TOO SHORT to be doubly mapped...
    // Also we don;t worry about a NPM here
    //
    CMWG_JF_mapped_nodes_iter = mMappedNodes.begin();
    CMWG_JF_mapped_nodes_last = mMappedNodes.end();
    CMWG_JF_pos_iter = mMappedPositions.begin();
    CMWG_JF_dummy_masters.clear();
    
    NPMHeadElem near_head;
    NodePositionMapClass * npm = mHeadNPMs[CTXID];
    
    uMDInt max_pos = 0;
    GenericNodeElem gn_elem;
    
    // all we need to do is make a dummy for each read and set the correct reversed flags
    // then we can join them together
    while(CMWG_JF_mapped_nodes_iter != CMWG_JF_mapped_nodes_last)
    {
        // set the head here to the contextID, This is just a temporary labeling
        // as it will/can/may be overwritten later. But we can trust it just at this instant!
        mNodes->setCntxId(CTXID, *CMWG_JF_mapped_nodes_iter);
        if(mNodes->getElem(&gn_elem, *CMWG_JF_mapped_nodes_iter))
        {
            // gn_elem now holds only the head node
            // we need to get the correct head element
            max_pos = *CMWG_JF_pos_iter;
            if(npm->getHead(&near_head, *CMWG_JF_mapped_nodes_iter))
            {
                while(near_head.NPMHE_Position != max_pos)
                {
                    npm->getNextHead(&near_head);
                }
            }
            else
            {
                logError("No head stored for: " << *CMWG_JF_mapped_nodes_iter << " : " << *CMWG_JF_pos_iter);
            }
            if(near_head.NPMHE_Position != max_pos)
            {
                logError("could not locate near head: " << *CMWG_JF_mapped_nodes_iter << " : " << *CMWG_JF_pos_iter);
                return false;
            }
        }
        
        GenericNodeId new_dummy = createNewDummyNoNpm(near_head.NPMHE_Position, near_head.NPMHE_Reversed, near_head.NPMHE_GID, 0, CTXID);
        CMWG_JF_dummy_masters.push_back(new_dummy);
        CMWG_JF_mapped_nodes_iter++;
        CMWG_JF_pos_iter++;
    }
    
    // now run through all the masters and join them together
    CMWG_JF_dummy_masters_iter = CMWG_JF_dummy_masters.begin();
    CMWG_JF_dummy_masters_last = CMWG_JF_dummy_masters.end();
    
    GenericNodeId prev_GID = *CMWG_JF_dummy_masters_iter;
    
    while(CMWG_JF_dummy_masters_iter != CMWG_JF_dummy_masters_last)
    {
        if(!makeEdge(prev_GID, *CMWG_JF_dummy_masters_iter)) { logError("Make edge returned false"); }
        prev_GID = *CMWG_JF_dummy_masters_iter;
        CMWG_JF_dummy_masters_iter++;
    }
    return true;
}
//HO 

//HV inline bool joinRelaxed(ContextId CTXID) { return joinFunct(false, false, CTXID); }
//HV 
//HV inline bool joinSemiRelaxed(ContextId CTXID) { return joinFunct(false, true, CTXID); }
//HV 
//HV inline bool join(ContextId CTXID) { return joinFunct(true, false, CTXID); }
//HV 

/*HV*/ bool
ContextMemWrapper::
/*HV*/ joinFunct(bool strict, bool breakDummy, ContextId CTXID)
//HO ;
{ // logInfo(__LINE__,1);   
    //-----
    // take a freshly mapped context and map the dualNode Pairs to it
    //
    
    // set the cut offs to max tolerance if not in strict mode
    if(!strict)
        mStatsManager->makeCutOffs(SAS_DEF_TOLL_MAX);
    
    CMWG_JF_mapped_nodes_iter = mMappedNodes.begin();
    CMWG_JF_mapped_nodes_last = mMappedNodes.end();
    CMWG_JF_pos_iter = mMappedPositions.begin();
    
    logInfo("required to join " << (mMappedNodes.size()) << " nodes", 6);
    GenericNodeElem gn_elem;
    
    // get the max and min positions for this closure
    uMDInt max_pos = 0;
    
    // the positions of nodes in the context
    NPMHeadElem near_head, far_head;
    NodePositionMapClass * npm = mHeadNPMs[CTXID];
    NodePositionMapClass * master_npm = mMasterNPMs[CTXID];
    
    GenericNodeId far_head_GID, near_GID, far_GID;
    sMDInt tdist;
    bool double_mapped = false;
    bool any_double_mapped = false;
    
    // we can't allow too long a run of dummies
    bool in_dummy_run = false;
    uMDInt length_dummy_run = 0;
    uMDInt last_dummy_pos = 0;
    uMDInt max_dummy_run = (uMDInt)((mStatsManager->getReadLength())*2);
    
    // get the history for this guy and set it up ready
    HistoryId current_HID;
    mHistories.initialiseHistory(&current_HID);
    
    while(CMWG_JF_mapped_nodes_iter != CMWG_JF_mapped_nodes_last)
    {
        // set the head here to the contextID, This is just a temporary labeling
        // as it will/can/may be overwritten later. But we can trust it just at this instant!
        mNodes->setCntxId(CTXID, *CMWG_JF_mapped_nodes_iter);
        // go through all the Dualnodes at this position
        if(mNodes->getElem(&gn_elem, *CMWG_JF_mapped_nodes_iter))
        {
            // this guy wasn't double mapped yet
            double_mapped = false;
            
            // gn_elem now holds only the head node
            // we need to get the correct head element
            max_pos = *CMWG_JF_pos_iter;
            
            if(npm->getHead(&near_head, *CMWG_JF_mapped_nodes_iter))
            {
                while(near_head.NPMHE_Position != max_pos)
                {
                    if(!npm->getNextHead(&near_head))
                        break;
                }
            }
            else
            {
                logError("No head stored for: " << *CMWG_JF_mapped_nodes_iter << " : " << *CMWG_JF_pos_iter);
            }
            if(near_head.NPMHE_Position != max_pos)
            {
                logError("could not locate near head: " << *CMWG_JF_mapped_nodes_iter << " : " << *CMWG_JF_pos_iter);
                return false;
            }
            
            while(mNodes->getNextElem(&gn_elem))
            {
                // gn_elem now holds a non-head node
                // get the paired read
                near_GID = gn_elem.GNE_GID;
                if(CTX_NULL_ID == mNodes->getCntxId(near_GID))
                {
                    far_GID = mNodes->getPair(near_GID);
                    far_head_GID = mNodes->getHead(far_GID);
                    // check to see if this far head node is in the context
                    if(npm->getHead(&far_head, far_head_GID))
                    {
                        // and then the insert size and orientation
                        do {
                            // the pair is in the context
                            // get the insert size...
                            tdist = (sMDInt)far_head.NPMHE_Position - (sMDInt)near_head.NPMHE_Position;
                            // Reversed flags indicate the orientation of the read in the context as being read from start to finish (wrt the distance between the nodes +/-)
                            // Are they reversed from how they are saved? ie. are the reads as read in the contexts current direction in lowest lexigographically orientated
                            if(mStatsManager->isMappable(near_GID, far_GID, (mNodes->isFileReversed(near_GID) ^ near_head.NPMHE_Reversed), (mNodes->isFileReversed(far_GID) ^ far_head.NPMHE_Reversed), tdist))
                            {
                                // This is a double mapped paired end read.
                                // It satisfies all statistical requirements. We can lock it in eddie
                                double_mapped = true;
                                any_double_mapped = true;
                                NPMMasterElem tmp_elem;
                                bool found_master = false;
                                
                                // check if there is a master at the near position
                                if(master_npm->getMaster(&tmp_elem, near_head.NPMHE_Position))
                                {
                                    // there is a master here... ... but is is the right one?
                                    // check all masters for if the heads match
                                    do {
                                        if(mNodes->getHead(tmp_elem.NPMME_GID) == near_head.NPMHE_GID)
                                        {
                                            // bingo!
                                            found_master = true;
                                            break;
                                        }
                                    } while(master_npm->getNextMaster(&tmp_elem));
                                }
                                if(found_master)
                                {
                                    // make this guy a slave.
                                    makeSlave(near_GID, tmp_elem.NPMME_GID);
                                }
                                else
                                {
                                    // set this guy as master
                                    makeMaster(near_head.NPMHE_Position, near_head.NPMHE_Reversed, near_GID, current_HID, CTXID);
                                }
                                
                                // then check the far position
                                found_master = false;
                                if(master_npm->getMaster(&tmp_elem, far_head.NPMHE_Position))
                                {
                                    // there is a master here... ... but is is the right one?
                                    // check all masters for if the heads match
                                    do {
                                        if(mNodes->getHead(tmp_elem.NPMME_GID) == far_head.NPMHE_GID)
                                        {
                                            // bingo!
                                            found_master = true;
                                            break;
                                        }
                                    } while(master_npm->getNextMaster(&tmp_elem));
                                }
                                if(found_master)
                                {
                                    // make this guy a slave.
                                    makeSlave(far_GID, tmp_elem.NPMME_GID);
                                }
                                else
                                {
                                    // set this guy as master
                                    makeMaster(far_head.NPMHE_Position, far_head.NPMHE_Reversed, far_GID, current_HID, CTXID);
                                }
                                
                                // exit this while loop. We can only map a paired end read once...
                                break;
                            }
                        } while(npm->getNextHead(&far_head));
                    }
                }
                else if(CTXID == mNodes->getCntxId(near_GID) && mNodes->getCntxPos(near_GID) == max_pos)
                {
                    // if the near guy has a CTXID then he is double mapped
                    double_mapped = true;
                }
            }
            if(!double_mapped)
            {
                // we couldn't find a pair to map here. Perhaps later?
                if(breakDummy)
                {
                    // check to see if the dummy run is too long before we make a new one
                    if(in_dummy_run)
                    {
                        length_dummy_run += (uMDInt)fAbs((sMDInt)last_dummy_pos - (sMDInt)(near_head.NPMHE_Position));
                        if(length_dummy_run <= max_dummy_run)
                        {
                            // ok to make a dummy here. We can add it in too..
                            GenericNodeId new_dummy = createNewDummy(near_head.NPMHE_Position, near_head.NPMHE_Reversed, near_head.NPMHE_GID, current_HID, CTXID);
                        } // else do nothing, this will break the context into smaller parts and stop over assembly...
                    }
                    else
                    {
                        in_dummy_run = true;
                        GenericNodeId new_dummy = createNewDummy(near_head.NPMHE_Position, near_head.NPMHE_Reversed, near_head.NPMHE_GID, current_HID, CTXID);
                    }
                }
                else
                {
                    // if Not-semi relaxed then create a new dummy node and set it as master for this position
                    GenericNodeId new_dummy = createNewDummy(near_head.NPMHE_Position, near_head.NPMHE_Reversed, near_head.NPMHE_GID, current_HID, CTXID);
                    //std::cout << "ND: " << new_dummy << " : " << mNodes->getSequence(new_dummy) << " : " << near_head.NPMHE_GID << " : " << near_head.NPMHE_Position << std::endl;
                    if(strict)
                    {
                        if(in_dummy_run)
                        {
                            length_dummy_run += (uMDInt)fAbs((sMDInt)last_dummy_pos - (sMDInt)(near_head.NPMHE_Position));
                            if(length_dummy_run > max_dummy_run)
                            {
                                logInfo("Dummy run too long", 5);
                                return false;
                            }
                        }
                        else
                        {
                            in_dummy_run = true;
                        }
                    }
                }
                last_dummy_pos = near_head.NPMHE_Position;
            }
            else
            {
                length_dummy_run = 0;
                in_dummy_run = false;
                last_dummy_pos = 0;
            }
        }
        else
            logError("Could not get elem");
        CMWG_JF_mapped_nodes_iter++;
        CMWG_JF_pos_iter++;
    }
    
    // now we can actually join them together
    // there should only be one master at most at each position at this point in time.
    uMDInt pos = (uMDInt)SAS_DEF_CONTEXT_START;
    NPMMasterElem tmp_elem;
    
    GenericNodeId prev_GID = GN_NULL_ID;
    while(pos <= max_pos)
    {
        if(master_npm->getMaster(&tmp_elem, pos))
        {
            prev_GID = tmp_elem.NPMME_GID;
            pos++;
            break;
        }
        pos++;
    }
    while(pos <= max_pos)
    {
        // so get the master at this position
        if(master_npm->getMaster(&tmp_elem, pos))
        {
            if((mNodes->getCntxPos(tmp_elem.NPMME_GID) - mNodes->getCntxPos(prev_GID)) <= mMaxOffset)
            {
                // we expect this guy to break on a semi-relative join.
                makeEdge(prev_GID, tmp_elem.NPMME_GID, true);
            }
            prev_GID = tmp_elem.NPMME_GID;
        }
        pos++;
    }
    
    return any_double_mapped;
}
//HO 

/*HV*/ bool                               
ContextMemWrapper::
/*HV*/ initialiseShoots(shootQueue * shoots, ContextId CTXID)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // Before we can start growing contexts we need to prep the caps
    // so they are ready for extension. We are simply going to add the cap but we need to know
    // about which direction we want to be facing. Find the head node which
    // joins onto the cap and then take the opposite offset
    //
    // clear out any old shoots...
    shoots->clear();
    clearShootTokens();
        
    NodeBlockElem nbe;
    mNodes->getCapBlock_DN(&nbe, CTXID);
    while(mNodes->nextNBElem(&nbe))
    {
        GenericNodeEdgeElem gnee(DUALNODE);
        if(mNodes->getEdges(&gnee, GN_EDGE_STATE_NTRANS, 0, nbe.NBE_CurrentNode))
        {
            do {
                // only one of these guys should be in the context
                if(mNodes->getCntxId(mNodes->getOlapNode(gnee)) == CTXID)
                {
                    // this is the guy next to the cap
                    shoots->push_back(contextShoot(mShootCheck->checkInNode(nbe.NBE_CurrentNode), -1 * mNodes->getOffset(gnee)));
                    break;
                }
            } while(mNodes->getNextEdge(&gnee));
        }
    }
    
    if(0 == shoots->size())
    {
        logError("No shoots could be made for context: " << CTXID);
        return false;
    }   
    
    if(2 != shoots->size())    
    {
        // there are multiple shoots here...
        // probably caused by joining on a non-sassy made naive contig.
        // we need to get the longest of these sections and make it into a contig in it's own right
        // positions are unique!
        logInfo("SHOOT SIZE: " <<  shoots->size(), 10);
        
        // it would be handy to make a list of nodes and
        // chhose that way but because I am a moron we need to store
        // something immutable, hence the position vector...
        std::vector<uMDInt> shoot_pos_vec;
        std::vector<uMDInt>::iterator shoot_pos_vec_iter;
        std::vector<uMDInt>::iterator shoot_pos_vec_last;
        std::vector<uMDInt>::iterator next_iter;
        
        // and an npm to manage them..
        NodePositionMapClass * master_npm = mMasterNPMs[CTXID];
        NPMMasterElem master_npm_elem;
        
        uMDInt winning_position = 0;
        uMDInt winning_distance = 0;
        
        GenericNodeId disp_GID = GN_NULL_ID;
        
        // sort all the shoots
        mNodes->getCapBlock_DN(&nbe, CTXID);
        while(mNodes->nextNBElem(&nbe))
        {
            // push each of these guys onto the list in
            // ascending order of position
            uMDInt curr_pos = mNodes->getCntxPos(nbe.NBE_CurrentNode);
            bool added = false;
            shoot_pos_vec_iter = shoot_pos_vec.begin();
            shoot_pos_vec_last = shoot_pos_vec.end();
            while(shoot_pos_vec_iter != shoot_pos_vec_last)
            {
                if(*shoot_pos_vec_iter >= curr_pos)
                {
                    shoot_pos_vec.insert(shoot_pos_vec_iter, curr_pos);
                    added = true;
                    break;
                }
                shoot_pos_vec_iter++;
            }
            if(!added)
            {
                shoot_pos_vec.push_back(curr_pos);
            }
            
        }
        
        // now choose a winner
        shoot_pos_vec_iter = shoot_pos_vec.begin();
        shoot_pos_vec_last = shoot_pos_vec.end();
        while(shoot_pos_vec_iter != shoot_pos_vec_last)
        {
            next_iter = shoot_pos_vec_iter + 1;
            uMDInt this_dist = *next_iter - *shoot_pos_vec_iter;
            logInfo("N:" << *next_iter << " S: " << *shoot_pos_vec_iter << " : " << this_dist, 10);
            if(this_dist > winning_distance)
            {
                winning_distance = this_dist;
                winning_position = *shoot_pos_vec_iter;
            }
            shoot_pos_vec_iter += 2;
        }
        
        logInfo("W: " <<  winning_distance << " P: " <<   winning_position, 10);
        
        // now kill the losers
        shoot_pos_vec_iter = shoot_pos_vec.begin();
        shoot_pos_vec_last = shoot_pos_vec.end();
        int remmed = 1;
        while(shoot_pos_vec_iter != shoot_pos_vec_last)
        {
            if(winning_position != *shoot_pos_vec_iter)
            {
                // this is not the winner
                GenericNodeId dead_GID = GN_NULL_ID;
                remmed++;
                
                // linear graph at this stage, so we can
                // just take everything by position...
                if(master_npm->getMaster(&master_npm_elem, *shoot_pos_vec_iter))
                {
                    dead_GID = master_npm_elem.NPMME_GID;
                }
                
                PARANOID_ASSERT_L2(dead_GID != GN_NULL_ID);
                
                GenericNodeEdgeElem gnee(DUALNODE);
                if(mNodes->getEdges(&gnee, GN_EDGE_STATE_NTRANS, 0, dead_GID))
                {
                    do {
                        // only one of these guys should be in the context
                        if(mNodes->getCntxId(mNodes->getOlapNode(gnee)) == CTXID)
                        {
                            // this is the guy next to the cap
                            detachBranch(&dead_GID, mNodes->getOlapNode(gnee));
                            safeDetachNode(&disp_GID, dead_GID);
                            break;
                        }
                    } while(mNodes->getNextEdge(&gnee));
                }
            }
            
            // else do nothing...
            shoot_pos_vec_iter += 2;
        }
        
        // run this mo-fo again!
        return initialiseShoots(shoots, CTXID);
    }
    
    return true;
}
//HO 

//******************************************************************************
// EXTENSION
//******************************************************************************

/*HV*/ void
ContextMemWrapper::
/*HV*/ doItterativeOptmalExtensions(shootQueue * savePoint_new, double tollMin, double tollMax, double tollSteps, ContextId CTXID)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // Takes as input a joined (or pre-extended) context and a shootqueue of 2 shoots.
    // Oscillates between the two ends searching for the optimal tolerance within the given range
    // and then growing at that tolerance. Stretches the shoot once growing has completed, updates
    // the shoot at the growng face and then adds the shoot to the end of the queue.
    //
    //uMDInt saved_pos_sp = 0;
    while(savePoint_new->size() > 0)
    {
        // get the next save point
        contextShoot current_shoot = savePoint_new->front();
        savePoint_new->pop_front();
        
        GenericNodeId shoot_GID = mShootCheck->checkOutNode(false, CTXID, current_shoot.first);
        
        // holders for toll cutoffs
        double local_toll_max = tollMax;
        double local_toll_min = tollMin;
        double local_toll_step = (local_toll_max - local_toll_min)/(tollSteps + 2);
        double best_tolerance = 0;
        double best_range_tolerance = 0;
        uMDInt greatest_extension = 0;
        bool increased_length = true;
        bool ascending = false;

        GenericNodeEdgeElem gnee_asc(DUALNODE);
        if(mNodes->getEdges(&gnee_asc, GN_EDGE_STATE_NTRANS, 0, shoot_GID))
        {
            // there should only be one edge here...
            if(mNodes->getCntxPos(mNodes->getOlapNode(gnee_asc)) < mNodes->getCntxPos(shoot_GID))
            {
                ascending = true;
            }
        }
        else
        {
            logError("No edges for shoot");
        }

        // start the range grower
        // this while loop is a tolerance fine tuner. If there is a good growth, 
        // then it will further refine the tolerance and try to re-grow.
        int current_loop = 0;
        while(increased_length)
        {
            current_loop++;
            increased_length = false;

            // this will try to grow using a number of tolerances with the given range
            // it will return the best tolerance from the range and the amount that that tolerance
            // extended by
            uMDInt extension = findOptimalToleranceWithinRange(&current_shoot, &best_range_tolerance, local_toll_min, local_toll_max, tollSteps, ascending, CTXID);

            if(0 != extension)
            {
                // we got somewhere, but was it any REAL improvement?
                if(extension > greatest_extension)
                {
                    //update the tolerance boundaries
                    increased_length = true;
                    best_tolerance = best_range_tolerance;
                    local_toll_min = best_tolerance - local_toll_step;
                    if(local_toll_min < 0) { local_toll_min = 0; }
                    local_toll_max = best_tolerance + local_toll_step;
                    greatest_extension = extension;
                    if(current_loop > 2)
                        break;
                }
            }
        }

        // now we have tried to find the best extension for this side of the context
        // if we could extend it then we should have a value for greatest_extension > 0
        if(greatest_extension > 0)
        {
            // extension worked!
            // re-make the best extension

            // reset the tmp history (just in case...)
            mHistories.initialiseTmpHistory();

            // grow at the best tolerance and stretch
            growAtToleranceLevel(&current_shoot, best_tolerance, ascending, true, CTXID);

            // we need a new save point
            savePoint_new->push_back(current_shoot);
            
            // now merge the histories to lock in the changes
            mHistories.merge();
        }
    }
}

/*HV*/ uMDInt
ContextMemWrapper::
/*HV*/ findOptimalToleranceWithinRange(contextShoot * startShoot, double * bestTolerance, double tollMin, double tollMax, double tollSteps, bool ascending, ContextId CTXID)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // Starting from the savePoint try to grow out, take note of the best tolerance.
    // Leaves the context in the same state it was in when it was passed. This functions
    // only job is to try growing at a range of tolerances and report the tolerance for the longest extension.
    // Works on a single shoot only.
    // 
    // If two identical tolerances return the same extension then it will keep the highest
    //

    // work out the tolerance ranges
    double tolerance, toll_step;
    toll_step = (tollMax - tollMin)/(tollSteps);
    tolerance = tollMin;

    int grow_dir = startShoot->second;
    uMDInt furthest_extension = 0;                               // the furthest we can extend

    // extend across the range
    while(tolerance <= tollMax)//toll_high)
    {
        // copy the start shoot
        // dont forget to make a deep copy...
        contextShoot start_shoot_copy = deepCopyShoot(startShoot, CTXID);
        
        // try to grow this guy at the currnet tolerance.
        uMDInt amount_extended = growAtToleranceLevel(&start_shoot_copy, tolerance, ascending, true, CTXID);

        if(0 != amount_extended)
        {
            if(amount_extended > furthest_extension)
            {
                // this grow step worked the best so far
                // save this information
                *bestTolerance = tolerance;
                furthest_extension = amount_extended;
            }
        }
        tolerance += toll_step;

        // check out the copied node
        mShootCheck->checkOutNode(true, CTXID, start_shoot_copy.first);
        
        // now we dismantle the growing we just did
        // find the first node we attached...
        // take note of where we are
        GenericNodeId start_node = mShootCheck->checkOutNode(false, CTXID, startShoot->first);
        PARANOID_ASSERT_PRINT_L2(mNodes->isValidAddress(start_node), "Invalid start node: " << start_node);
        GenericNodeEdgeElem gnee(DUALNODE);
        if(mNodes->getEdges(&gnee, GN_EDGE_STATE_NTRANS, grow_dir, start_node))
        {
            // and detach
            detachBranch(&start_node, mNodes->getOlapNode(gnee));
        }

        // reset the tmp history
        mHistories.initialiseTmpHistory();

        // everything should be as it was when we started...
    }

    return furthest_extension;
}
//HO 

    static shootQueue CMWG_disposable_queue;

/*HV*/ uMDInt
ContextMemWrapper::
/*HV*/ growAtToleranceLevel(contextShoot * startShoot, double tolerance, bool ascending, bool stretch, ContextId CTXID)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // grow from the shoot with the given tolerance
    // return the total amount of extension (including stretching if appropriate)
    // update the shoot for future grow steps...
    //
    
    // set the cut offs
    mStatsManager->makeCutOffs(tolerance);

    // we will need to know the start point (these guys will get changed around a bit)
    GenericNodeId fork_node = mShootCheck->checkOutNode(false, CTXID, startShoot->first);
    uMDInt fork_point = mNodes->getCntxPos(fork_node);
    uMDInt start_point = fork_point;

    // clear any rogue (non-shoot) tokens now...
    clearNodeTokens();

    // we'll need some shooty-type vars
    contextShoot new_shoot = deepCopyShoot(startShoot, CTXID);
    contextShoot final_shoot = deepCopyShoot(startShoot, CTXID);
    
    CMWG_disposable_queue.clear();
    CMWG_disposable_queue.push_front(*startShoot);
    
    // try to map the shoot
    bool some_mapped = false;
    int round = 1;
    while(mapAllShoots(&CMWG_disposable_queue, &fork_node, &fork_point, ascending, CTXID))
    {
        some_mapped = true;
        
        // now we can try to set forces to each affected node
        if(profilePrune(&new_shoot, &fork_node, fork_point, ascending, CTXID))
        {
            // we will need to know the start point
            fork_node = mShootCheck->checkOutNode(false, CTXID, new_shoot.first);
            fork_point = mNodes->getCntxPos(fork_node);

            // clear any rogue tokens now...
            clearNodeTokens();
            CMWG_disposable_queue.push_front(new_shoot);
            final_shoot = new_shoot;
        }
        else
        {
            // Profile prune returned false, new_shoot cannot be trusted!
            return 0;
        }
        round++;
        // the new_shoot may fail at this tolerance still
    }
    
    // did we extend at all?
    if(some_mapped)
    {
        // the new_shoot failed at the current tolerance
        // still we can try it again at a different tolerance
        *startShoot = final_shoot;
        // get the total extension
        uMDInt last_fork_point = mNodes->getCntxPos(mShootCheck->checkOutNode(false, CTXID, final_shoot.first));
        uMDInt extension = 0;
        if(last_fork_point > start_point)
            extension = last_fork_point - start_point;
        else
            extension = start_point - last_fork_point;

        // stretch the shoot if necessary
        if(stretch)
        {
            extension += stretchShoot(startShoot, ascending, CTXID);
        }
        
        return extension;
    }
    return 0;
}
//HO 

/*HV*/ uMDInt
ContextMemWrapper::
/*HV*/ stretchShoot(contextShoot * startShoot, bool ascending, ContextId CTXID)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // after a grow step it would be nice if we could use the naive contig information
    // to grow a shoot out so it coincides with the end of a naive contig. We would need to 
    // do no harsh joining or look at dummy runs but we could get the maimum trusted extension
    // at each round. This is the function which does that!  YAY!
    //
    GenericNodeId joining_GID = mShootCheck->checkOutNode(false, CTXID, startShoot->first);
    
    ContigId CID = mNodes->getContig(UNINODE, joining_GID);
    if(CID == CTG_NULL_ID)
    {
        // nothing to do as we landed on the boundary of a null contig
        // IE. a uninode which was a cross in a previous life...
        return 0;
    }

    // check to see if it's within limits
    if(-1 != mCovCutPercent)
    {
        if(mCoverageMap[CID] > mUpperStretchCoverageCutoff)
            return 0;
    }
    
    // always stretch at the greatest tolerance
    mStatsManager->makeCutOffs(SAS_DEF_TOLL_MAX);
    
    uMDInt start_stretch_pos = mNodes->getCntxPos(joining_GID);
    uMDInt end_stretch_pos = start_stretch_pos;
    
    // now we make these guys...
    GenericNodeId next_walk_GID = GN_NULL_ID;
    GenericNodeId shoot_head_GID = mNodes->getHead(joining_GID);
    GenericNodeEdgeElem gnee(UNINODE);

    // get all the non-transitive forward UN edges
    // pick out the one which has a matching contig ID
    if(mNodes->getEdges(&gnee, GN_EDGE_STATE_NTRANS, startShoot->second, shoot_head_GID))
    {
        do {
            // we need to check that this guy is both in the same uninode contig
            // AND that it would have been so when the contig was made
            if((mNodes->getContig(UNINODE, mNodes->getOlapNode(gnee)) == CID) && ((uMDInt)(fAbs(mNodes->getOffset(gnee))) <= mNaiveOffset))
            {
                // this is our guy!
                next_walk_GID = mNodes->getOlapNode(gnee);
                break;
            }
        } while(mNodes->getNextEdge(&gnee));
    }

    if(next_walk_GID != GN_NULL_ID)
    {
        // for walking along
        GenericNodeWalkingElem gnwe(UNINODE);
        if(mNodes->startWalk(&gnwe, shoot_head_GID, next_walk_GID))
        {
            //
            // these vars are all disposable but they are need to make the function 
            // header work.
            // LIVE WITH IT!
            //
            shootQueue sq_disp;
            
            uMDInt nhp_disp;
            bool nhr_disp;

            // these guys are needed for the call to mapDualNodesAtHead
            GenericNodeId join_GID = joining_GID;
            uMDInt fork_point = mNodes->getCntxPos(join_GID);
            HistoryId HID = mNodes->getCntxHistory(join_GID);

            // we will need these to repair the shoot at the end
            GenericNodeId last_trusted_master = join_GID;
            uMDInt last_trusted_offset = 0;
            uMDInt last_dummy_pos = 0;
            
            // we need the node position map for this context so we can grab the master at ease...
            NPMMasterElem master_npm_elem;
            NodePositionMapClass * master_npm = mMasterNPMs[CTXID];

            unsigned int consecutive_dummy_run = 0;
            unsigned int max_dummy_run = mStatsManager->getReadLength();
            
            bool added_some = false;
            bool added_non_dummy = false;
            
            int steps = 0;
            
            do {
                steps++;
                if(mNodes->getContig(UNINODE, mNodes->getCurrentNode(gnwe)) != CID)
                {
                    break;
                }
                
                PARANOID_ASSERT_PRINT_L2(mNodes->getContig(UNINODE, mNodes->getCurrentNode(gnwe)) == CID, "No match on the contigs: " << mNodes->getContig(UNINODE, mNodes->getCurrentNode(gnwe)) << " != " << CID);
                GenericNodeId for_new_head = GN_NULL_ID;
                if(mapDualNodesAtHead(&sq_disp, &nhp_disp, &nhr_disp, (int)mNodes->getOffset(gnwe), (int)mNodes->getReturnOffset(gnwe), true, &join_GID, mNodes->getCurrentNode(gnwe), HID, fork_point, ascending, CTXID))
                {
                    // we mapped the head. nhp_disp holds the position it was mapped to 
                    // so we can use this to get hold of the master...

                    // IF there are forks here of equal length (or an existing fork which is longer than this one)
                    // then this call could fail, however, stretchShoot is only called after prune
                    // so the whole thing SHOULD be linear. hence no forks! :)
                    if(master_npm->getMaster(&master_npm_elem, nhp_disp))
                    {
                        join_GID = master_npm_elem.NPMME_GID;
                    }
                    else
                        logError("No master at position: " << nhp_disp);

                    // reset this guy
                    consecutive_dummy_run = 0;

                    // set the flag so we know the extension was not 100% dummy.
                    added_non_dummy = true;
                    added_some = true;
                    
                    // we only want to finish on a "real" master...
                    last_trusted_offset = mNodes->getReturnOffset(gnwe);
                    last_trusted_master = join_GID;
                    for_new_head = join_GID;
                }
                else
                {
                    // too many dummies?
                    if(0 != consecutive_dummy_run)
                    {
                        if(ascending)
                        {
                            consecutive_dummy_run += (nhp_disp - last_dummy_pos);
                        }
                        else
                        {
                            consecutive_dummy_run += (last_dummy_pos - nhp_disp);
                        }
                        if(consecutive_dummy_run > max_dummy_run)
                        {
                            break;
                        }
                    }
                    else
                    {
                        consecutive_dummy_run++ ;
                    }
                    
                    GenericNodeId new_dummy = createNewDummy(nhp_disp, nhr_disp, mNodes->getCurrentNode(gnwe), HID, CTXID);
                    if(!makeEdge(new_dummy, join_GID)) { logError("Make edge returned false"); }
                    join_GID = new_dummy;
                    for_new_head = join_GID;
                    
                    last_dummy_pos = mNodes->getCntxPos(join_GID);
                }
                if(GN_NULL_ID != for_new_head)
                {
                    // we will need to add the head of this node to the npm
                    NPMHeadElem tmp_npm_elem;
                    NodePositionMapClass * tmp_head_npm = mHeadNPMs[CTXID];
                    
                    for_new_head = mNodes->getHead(for_new_head);
                    
                    tmp_npm_elem.NPMHE_GID = for_new_head;
                    tmp_npm_elem.NPMHE_Position = mNodes->getCntxPos(join_GID);
                    tmp_npm_elem.NPMHE_Reversed = mNodes->isCntxReversed(join_GID);
                    tmp_head_npm->addElem(&tmp_npm_elem, for_new_head);
                }

                // fix the fork point and potential shoot info
                fork_point = (uMDInt)nhp_disp;
                added_some = true;

                // these guys just contain muck...
                sq_disp.clear();
                
            } while(mNodes->contigStep(&gnwe));
            
            if(added_some)
            {
                // we extended (at least a bit)
                // don't want to end on a dummy run.
                if(0 != consecutive_dummy_run)
                {
                    // there is a chain of dummies at the end of this guy of length at least 1
                    GenericNodeEdgeElem gnee_kill(DUALNODE);
                    if(!added_non_dummy)
                    {
                        // 100% of the stretch is dummy. This means that the incomming shoot is valid
                        // so we need only to detach the string of dummies
                        GenericNodeId tmp_GID = mShootCheck->checkOutNode(false, CTXID, startShoot->first);
                        if(mNodes->getEdges(&gnee_kill, GN_EDGE_STATE_NTRANS, startShoot->second, tmp_GID))
                        {
                            GenericNodeId next_to_kill = mNodes->getOlapNode(gnee_kill);
                            detachBranch(&tmp_GID, next_to_kill);
                        }
                        return 0;
                    }
                    else
                    {
                        // there was at least 1 paired master added...
                        // everything past the last trusted master is a dummy 
                        // so this means we can trust these values and use them as the next shoot
                        if(mNodes->getEdges(&gnee_kill, GN_EDGE_STATE_NTRANS, -1 * last_trusted_offset, last_trusted_master))
                        {
                            GenericNodeId next_to_kill = mNodes->getOlapNode(gnee_kill);
                            detachBranch(&last_trusted_master, next_to_kill);
                        }
                    }
                }
                // lets fix up the shoot we were given
                updateShoot(startShoot, -1 * last_trusted_offset, last_trusted_master, CTXID);
                
                end_stretch_pos = mNodes->getCntxPos(last_trusted_master);
            }
        }
    }
    if(end_stretch_pos > start_stretch_pos)
    {
        return  end_stretch_pos -  start_stretch_pos;
    }
    return start_stretch_pos - end_stretch_pos;
}
//HO 

//******************************************************************************
// EXTENSION WORKHORSES
//******************************************************************************

/*HV*/ bool
ContextMemWrapper::
/*HV*/ mapAllShoots(shootQueue * shoots, GenericNodeId * forkNode, uMDInt * forkPoint, bool ascending, ContextId CTXID)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // Go through the queue and try map the dualnodes at the shoots
    //
    bool any_mapped = false;
    bool ok_to_move_fork_point = true;
    if(shoots->size() > 1)
    {
        logError("Must begin on exactly one shoot, you supplied: " << shoots->size());
        return false;
    }
    int round = 0;
    while(shoots->size() > 0)
    {
        bool forked = false;
        if(mapShoot(shoots, &forked, forkNode, forkPoint, ok_to_move_fork_point, SAS_DEF_CONTEXT_MAX_TREE, ascending, CTXID, round))
        {
            any_mapped = true;
            if(forked)
                ok_to_move_fork_point = false;
        }
        round++;
    }
    return any_mapped;
}
//HO 

/*HV*/ bool
ContextMemWrapper::
/*HV*/ mapShoot(shootQueue * shoots, bool * forked, GenericNodeId * forkNode, uMDInt * forkPoint, bool okToMoveForkPoint, int level, bool ascending, ContextId CTXID, int round)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // Go through the queue and try map the dualnodes at the shoots
    //
    // get the head of the queue, this is the current shoot.
    // pop it off now
    contextShoot current_shoot = shoots->front();
    shoots->pop_front();
    
    // we need to know if a recurrsive call modifies the fork values
    bool mapped_in_recurrsion = false;
    bool forked_in_recurrsion = false;
    bool any_mapped = false;
    
    GenericNodeId joining_GID = mShootCheck->checkOutNode(false, CTXID, current_shoot.first);
    PARANOID_ASSERT_PRINT_L3(mNodes->isValidAddress(joining_GID), "START: " << joining_GID << " is fubar");
    GenericNodeId shoot_head_GID = mNodes->getHead(joining_GID);
    
    GenericNodeEdgeElem gnee(UNINODE);

    // for very short insert sizes we can invalidate this node
    // then we'll have to try get it back
    NodeCheckId join_token = mNodeCheck->checkInNode(joining_GID);
    HistoryId current_HID = mNodes->getCntxHistory(joining_GID);
    
#ifdef MAKE_PARANOID
      if(0 == current_HID)
      {
          std::cout << "0 HID: " << std::endl;
          mNodes->printEdges(DUALNODE, joining_GID);
      }
#endif

    // work out the history stuff
    int num_forks = 0;                                              // the number of forks here
    int current_num_forks = 0;                                      // so we can see if things change
    // get all the non-transitive forward UN edges
    if(mNodes->getEdges(&gnee, GN_EDGE_STATE_NTRANS, current_shoot.second, shoot_head_GID))
    {
        do {
            // get the next potential head
            GenericNodeId potential_head_GID = mNodes->getOlapNode(gnee);
            
            sMDInt offset_J_PH = mNodes->getOffset(gnee);
            sMDInt offset_PH_J = mNodes->getReturnOffset(gnee);
            sMDInt shootDirection =  -1 * offset_PH_J;
            
            bool mapped_potential_head = false;
            uMDInt PH_position;     // these are set in the call to mapDualNodesAtHead below
            bool PH_reversed;
            
            if(current_num_forks != num_forks)
            {
                // we must have mapped something on the last edge
                // we'll need to fork the history
                current_HID = mHistories.forkHistory(num_forks, current_HID);

#ifdef MAKE_PARANOID
                if(0 == current_HID)
                {
                    std::cout << "0 HID (FORK): " << num_forks << std::endl;
                    mNodes->printEdges(DUALNODE, joining_GID);
                }
#endif
                current_num_forks = num_forks;
            }

            // try to map the dualnodes about the potential head
            if(mapDualNodesAtHead(shoots, &PH_position, &PH_reversed, offset_J_PH, offset_PH_J, true, &joining_GID, potential_head_GID, current_HID, *forkPoint, ascending, CTXID))
            {                    
                // we mapped some dualnodes, we should have fixed the masters and the new shoots in the preceding call
                // we just need to set the head node in the context and the nodeposition map
                // put the head into the context
                mapped_potential_head = true;
                any_mapped = true;
                PARANOID_ASSERT_PRINT_L3(mNodes->isValidAddress(joining_GID), "MDNAH: " << joining_GID << " is fubar");
            }
            else if(level > 0) // check if recurrsion is acceptable
            {                    
            
                PARANOID_ASSERT_PRINT_L3(mNodes->isValidAddress(joining_GID), "PREMAP: " << joining_GID << " is fubar");
                // make a dummy of the potential head.
                bool ok_to_keep_dummy = false;
                GenericNodeId new_dummy = createNewDummy(PH_position, PH_reversed, potential_head_GID, current_HID, CTXID);
                
                // make a shoot of the dummy and push it onto the front of the queue
                // this will tee it up for the recurrsive call
                NodeCheckId new_dummy_token = mShootCheck->checkInNode(new_dummy);
                shoots->push_front(contextShoot(new_dummy_token, shootDirection));
                
                // call this function recurrsively
                if(mapShoot(shoots, &forked_in_recurrsion, forkNode, forkPoint, okToMoveForkPoint, --level, ascending, CTXID, round))
                {                    
                    // the dummy is mapped, there is no need to shoost it.
                    // we just need to set the potential head node into the context
                    any_mapped = true;
                    mapped_in_recurrsion = true;
                    ok_to_keep_dummy = true;
                    mapped_potential_head = true;
                }
                else if(!mStrict)
                {
                    // all the forward non-transitive edges should be checked now.
                    // we just want to see if any of the forward transitive edges look like they may be in the context
                    // this is the last chance for this dummy...

                    // if the dummy has many many edges then this can get really out of hand
                    // pick some arbitrary cutoff and limit it at that...
                    GenericNodeEdgeElem gnee_trans(UNINODE);
                    if(mNodes->getTRank(UNINODE, potential_head_GID) < 50)
                    {
                        if(mNodes->getEdges(&gnee_trans, GN_EDGE_STATE_TRANS, shootDirection, potential_head_GID))
                        {
                            do {
                                // we won't map the transitive edge but we'll se if he would map
                                GenericNodeId non_map_head_GID = mNodes->getOlapNode(gnee_trans);
                                sMDInt offset_PH_NMH = mNodes->getOffset(gnee_trans);
                                sMDInt offset_NMH_PH = mNodes->getReturnOffset(gnee_trans);
                            
                                uMDInt NMH_position;
                                bool NMH_reversed;
                              
                                if(mapDualNodesAtHead(shoots, &NMH_position, &NMH_reversed, offset_PH_NMH, offset_NMH_PH, false, &new_dummy, non_map_head_GID, current_HID, *forkPoint, ascending, CTXID))
                                {
                                    // we need to shoost this dummy and set the potential head node into the context
                                    ok_to_keep_dummy = true;
                                    any_mapped = true;
                                    mapped_potential_head = true;

                                    // make a new shoot and put it on the end of the queue
                                    // we will try the dummy again later
                                    shoots->push_back(contextShoot(new_dummy_token, shootDirection));
                                    break;
                                }
                            } while(mNodes->getNextEdge(&gnee_trans));
                        }
                    }
                }

                // re-constitute the joining node
                joining_GID = mNodeCheck->checkOutNode(false, CTXID, join_token); 

                if(ok_to_keep_dummy)
                {
                    // join the dummy to the original capnode and we're done
                    new_dummy = mShootCheck->checkOutNode(false, CTXID, new_dummy_token);
                    joining_GID = mNodeCheck->checkOutNode(false, CTXID, join_token); 
                    if(!makeEdge(new_dummy, joining_GID)) { logError("Make edge returned false"); }
                }
                else
                {
                    // clear from the shoot token maps
                    mShootCheck->checkOutNode(true, CTXID, new_dummy_token);
                    // kill the dummy
                    deleteDummy(new_dummy);
                }
            } // else we're at the last level of recurrsion, no dice!
            
            if(mapped_potential_head)
            {
                // somewhere we mapped a master from this potential head
                // we need to set it into the context
                mNodes->setCntxId(CTXID, potential_head_GID);
                NodePositionMapClass * head_npm = mHeadNPMs[CTXID];
                NPMHeadElem tmp_npm_elem;
                tmp_npm_elem.NPMHE_GID = potential_head_GID;
                tmp_npm_elem.NPMHE_Position = PH_position;
                tmp_npm_elem.NPMHE_Reversed = PH_reversed;
                head_npm->addElem(&tmp_npm_elem, potential_head_GID);
                num_forks++;
            }
            
        } while(mNodes->getNextEdge(&gnee));
        
    }
    
	// re-constitute the joining node    
	joining_GID = mNodeCheck->checkOutNode(true, CTXID, join_token); 

    // tell the world if we forked
    if(num_forks > 1)
    {
        // we forked here so overwrite the fork values
        *forked = true;
        if(okToMoveForkPoint)
        {
            *forkNode = joining_GID;
            *forkPoint = mNodes->getCntxPos(joining_GID);
        }
        return true;
    }
    else if(forked_in_recurrsion)
    {
        // we forked in a recurrsive call
        // fork point and node have been handled in the recurrsive call
        *forked = true;
        return true;
    }
    
    // we didn't fork, if we mapped some stuff then we can move the fork point along
    if(okToMoveForkPoint)
    {
		
        // no fork so if we mapped something in recurrsion then we don't want to overwrite it here
        if(!mapped_in_recurrsion && any_mapped)
        {
			

            *forkNode = joining_GID;
            *forkPoint = mNodes->getCntxPos(*forkNode);
            
        }
        
    }
    
    return any_mapped;
}
//HO 

/*HV*/ bool
ContextMemWrapper::
/*HV*/ mapDualNodesAtHead(shootQueue * shoots, uMDInt * NH_position, bool * NH_reversed, int offset_J_NH, int offset_NH_J, bool map_perm, GenericNodeId * joining_GID, GenericNodeId near_head_GID, HistoryId HID, uMDInt forkPoint, bool ascending, ContextId CTXID)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // given a head generic node we try to double map all dualnodes
    // in the ring.
    //
    
    PARANOID_ASSERT_PRINT_L2(mNodes->isValidAddress(near_head_GID), "Invalid near head: " << near_head_GID);

    // work out the orientation and position of the near head in the context
    // the calling function will use these values too

    // get the absolute value of the offset
    uMDInt abs_offset = (uMDInt)(fAbs(offset_J_NH));

    uMDInt join_pos = mNodes->getCntxPos(*joining_GID);

    // the joining_GID may get assimilated in this call
    // use these guys to get it back
    bool join_is_dummy = mNodes->isDummy(*joining_GID);
    NodeCheckId join_token = mNodeCheck->checkInNode(*joining_GID);    

    if(ascending)
    {
        *NH_position = join_pos + abs_offset;
    }
    else
    {
        *NH_position = join_pos - abs_offset;
    }
    *NH_reversed = (0 < (offset_J_NH * offset_NH_J)) ^ (mNodes->isCntxReversed(*joining_GID));

    // the positions of the head nodes in the context
    NPMHeadElem far_head;
    GenericNodeId near_GID, far_GID, far_head_GID, far_master_GID, near_master_GID;
    GenericNodeElem gn_elem;
    bool near_master_mapped = false;
    
    // get the node position map of the masters
    NodePositionMapClass * head_npm = mHeadNPMs[CTXID];
    
    // go through all the Dualnodes at this position
    if(mNodes->getElem(&gn_elem, near_head_GID))
    {
        // the gn_elem holds the head we were passed.
        // so we jump onto the first in the loop
        while(mNodes->getNextElem(&gn_elem))
        {
            // gn_elem now holds a non-head node
            // make sure this dualnode is not mapped
            near_GID = gn_elem.GNE_GID;
            if(mNodes->getCntxId(near_GID) == CTX_NULL_ID)
            {
                // get the paired read
                far_GID = mNodes->getPair(near_GID);
                PARANOID_ASSERT_PRINT_L2(mNodes->isValidAddress(far_GID), "This is not a valid address: " << far_GID << " pair of " << near_GID << " with Fork: " << *joining_GID);

                // get the head node. It could be mapped to  many positions so we check each
                // one to see if it's position an orientation match
                far_head_GID = mNodes->getHead(far_GID);
                if(head_npm->getHead(&far_head, far_head_GID))
                {
                    // the head is in the context, we need to find the head on the same branch
                    // once we've mapped the paired read we can break
                    bool mapped_elem = false;
                    
                    // for each of the heads mappings...
                    do {
                        NodePositionMapClass * master_npm = mMasterNPMs[CTXID];
                        NPMMasterElem tmp_master_elem;
                        
                        PARANOID_ASSERT_PRINT_L2(mNodes->isValidAddress(far_head.NPMHE_GID), "Invalid head: " << far_head.NPMHE_GID);

                        // get all the masters at the mapped heads position
                        if(master_npm->getMaster(&tmp_master_elem, far_head.NPMHE_Position))
                        {
                            // there is a master here... ... but is is the right one?
                            // check all masters for if the heads match
                            do {
                                PARANOID_ASSERT_PRINT_L2(mNodes->isValidAddress(tmp_master_elem.NPMME_GID), "Invalid (SOW) " << tmp_master_elem.NPMME_GID);
                                // explicity check if the heads match
                                if(mNodes->getHead(tmp_master_elem.NPMME_GID) == far_head.NPMHE_GID)
                                {
                                    // heads match, check the history
                                    if(mHistories.onSameBranch(mNodes->getCntxHistory(tmp_master_elem.NPMME_GID), HID))
                                    {
                                        // bingo!
                                        far_master_GID = tmp_master_elem.NPMME_GID;

                                        // lets see if the reads lie on either side of the fork point
                                        if(((far_head.NPMHE_Position > forkPoint) ^ (*NH_position > forkPoint)) && (far_head.NPMHE_Position != forkPoint))
                                        {
                                            // now we need to check and see if the insert sizes are OK
                                            sMDInt tdist = (sMDInt)far_head.NPMHE_Position - (sMDInt)*NH_position;
                        
                                            // Reversed flags indicate the orientation of the read in the context as being read from start to finish (wrt the distance between the nodes +/-)
                                            // Are they reversed from how they are saved? ie. are the reads as read in the contexts current direction in lowest lexigographically orientated
                                            if(mStatsManager->isMappable(near_GID, far_GID, (mNodes->isFileReversed(near_GID) ^ *NH_reversed), (mNodes->isFileReversed(far_GID) ^ far_head.NPMHE_Reversed), tdist))
                                            {
                                                // This is a double mapped paired end read.
                                                // It satisfies all statistical requirements. We can lock it in eddie
                                                mapped_elem = true;

                                                // we need to see if we needed to map this guy or not
                                                if(map_perm)
                                                {
                                                    if(near_master_mapped)
                                                    {
                                                        // make this guy a slave.
                                                        makeSlave(near_GID, near_master_GID);
                                                    }
                                                    else
                                                    {
                                                        // set this guy as master
                                                        makeMaster(*NH_position, *NH_reversed, near_GID, HID, CTXID);

                                                        // join this guy onto the end of the context
                                                        *joining_GID = mNodeCheck->checkOutNode(false, CTXID, join_token); 
                                                        if(!makeEdge(*joining_GID, near_GID)) { logError("Make edge returned false"); }

                                                        // note that we've made a master here
                                                        near_master_mapped = true;
                                                        near_master_GID = near_GID;

                                                        // make a new shoot and put it on the end of the queue
                                                        shoots->push_back(contextShoot(mShootCheck->checkInNode(near_GID), -1 * offset_NH_J));
                                                    }

                                                    // now map the far slave
                                                    // there is a chance that the joining_GID
                                                    // could be the far master and also that
                                                    // it could be a slave. In which case it will get assimilated
                                                    // take care of that here...
                                                    if(far_master_GID == *joining_GID && join_is_dummy)
                                                    {
                                                        makeSlave(far_GID, far_master_GID);
                                                        *joining_GID = far_GID;
                                                        join_is_dummy = false;
                                                    }
                                                    else
                                                        makeSlave(far_GID, far_master_GID);
                                                }
                                                else
                                                {
                                                    // we were calling this on transitive edges so we can exit now
                                                    return true;
                                                }
                                                // exit this while loop. We can only map a paired end read once...
                                                break;
                                            }
                                        }
                                    }
                                }
                            } while(master_npm->getNextMaster(&tmp_master_elem));
                        }
                        
                        // break out of trying to find the far head master
                        if(mapped_elem)
                            break;
                    } while(head_npm->getNextHead(&far_head));
                }
            }
        }
    }

    return near_master_mapped;
}
//HO 

    static std::vector<GenericNodeId> CMWG_SS_to_detach_holder;                         // nodes which are on DEPs
    static std::vector<GenericNodeId> CMWG_SS_dummy_tip_holder;                         // tips which are all dummies
    static std::vector<GenericNodeId>::iterator CMWG_SS_detach_iter, CMWG_SS_detach_last;

/*HV*/ bool
ContextMemWrapper::
/*HV*/ smartShave(GenericNodeId * forkNode, uMDInt forkPoint, bool ascending)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // This function does some very light shaving of tips and also removes
    // any ends which are completely made out of dummies
    // we also make the lists of cap nodes from the working node set
    //
    NodeCheckId fork_token = mNodeCheck->checkInNode(*forkNode);
    ContextId CTXID = mNodes->getCntxId(*forkNode);
    
    GenericNodeEdgeElem gnee(DUALNODE);                             // we'll need these to do some walking
    GenericNodeWalkingElem gnwe(DUALNODE);
    
    // when we do some very very light shaving
    // we also will trim back all the dummies at the end
    // use these guys to store the nodes to detach
    GenericNodeId detach_delay, dum_detach_delay, query_node;
    CMWG_SS_to_detach_holder.clear();                         // nodes which are on DEPs
    CMWG_SS_dummy_tip_holder.clear();                         // tips which are all dummies

    // we have a list of caps now
    // do iterative shaving
    bool cleaned_up_some = true;
    while(cleaned_up_some)
    {
        // reset this variable so we'll know when we've done nothing
        cleaned_up_some = false;
        
        int num_detached = 0;

        // go through all the caps
        // because we are not using detachnode with an NBE we need to keep hold of the previous guy
        // NOTE:
        // This code, kinda relies on the fact that the algorithm adds to the cap queue in a specific way
        // When this was written it was always the case that the last cap in the cap block 
        // is the cap at the far end. If you change this behaviour, it will fuck things up - big time.
        // 25-02-11
        NodeBlockElem nbe;
        mNodes->getCapBlock_DN(&nbe, CTXID);
        mNodes->nextNBElem(&nbe);
        GenericNodeId prev_cap = nbe.NBE_CurrentNode;
        while(mNodes->nextNBElem(&nbe))
        {
            if(ascending ^ (mNodes->getCntxPos(prev_cap) < forkPoint))
            {
                // this is a cap so we're guarunteed only 1 edge
                if(mNodes->getEdges(&gnee, GN_EDGE_STATE_NTRANS, 0, prev_cap))
                {
                    // clear these guys
                    CMWG_SS_to_detach_holder.clear();
                    CMWG_SS_dummy_tip_holder.clear();
                    bool straight_dummy_run = true;
                    int count = 0;
                    if(mNodes->startWalk(&gnwe, prev_cap, mNodes->getOlapNode(gnee)))
                    {
                        query_node = mNodes->getPrevNode(gnwe);
                        
                        // put the cap on the detach list
                        detach_delay = query_node;
                        
                        // check to see if the cap is a dummy and do accordingly...
                        if(mNodes->isDummy(query_node))
                            dum_detach_delay = query_node;
                        else
                            straight_dummy_run = false;
                        
                        // now do the rest
                        do {
                            query_node = mNodes->getCurrentNode(gnwe);
                            
                            // it's only worth adding to this list while 
                            // the count is less than the prescribed depth
                            if(count <= SAS_DEF_CTX_SHAVE_DEPTH)
                            {
                                CMWG_SS_to_detach_holder.push_back(detach_delay);
                                detach_delay = query_node;
                                count++;
                            }
                            
                            // as long as we're on a run we can keep adding them on
                            if(straight_dummy_run)
                            {
                                CMWG_SS_dummy_tip_holder.push_back(dum_detach_delay);
                                if(mNodes->isDummy(query_node))
                                    dum_detach_delay = query_node;
                                else
                                    straight_dummy_run = false;
                            }
                            
                            // we can break out if the count is greater than the prescribed shave depth
                            // but if we're on a straight run of dummies then we should keep going unless 
                            // we get all the way back to the forkNode
                            if(!straight_dummy_run)
                            {
                                if(count > SAS_DEF_CTX_SHAVE_DEPTH)
                                    break;
                            }
                        } while(mNodes->rankStep(&gnwe));
                    }
                    else
                    {
                        logError("could not walk from cap");
                        *forkNode = mNodeCheck->checkOutNode(true, CTXID, fork_token); 
                        return false;
                    }
                    
                    bool need_to_shave = false;
                    if(count <= SAS_DEF_CTX_SHAVE_DEPTH)
                    {
                        // we know we're gonna have to shave something but which list?
                        need_to_shave = true;
                        if(CMWG_SS_dummy_tip_holder.size() > CMWG_SS_to_detach_holder.size())
                        {
                            CMWG_SS_detach_iter = CMWG_SS_dummy_tip_holder.begin();
                            CMWG_SS_detach_last = CMWG_SS_dummy_tip_holder.end();
                        }
                        else
                        {
                            CMWG_SS_detach_iter = CMWG_SS_to_detach_holder.begin();
                            CMWG_SS_detach_last = CMWG_SS_to_detach_holder.end();
                        }
                    }
                    else if(0 != CMWG_SS_dummy_tip_holder.size())
                    {
                        need_to_shave = true;
                        CMWG_SS_detach_iter = CMWG_SS_dummy_tip_holder.begin();
                        CMWG_SS_detach_last = CMWG_SS_dummy_tip_holder.end();
                    }
                    if(need_to_shave)
                    {
                        // note that we did some cleaning
                        cleaned_up_some = true;
                        
                        // we need an incorruptible list of nodes to detach,
                        // this is the best we can do...
                        
                        std::vector<NodeCheckId> safe_detach_tokens;
                        while(CMWG_SS_detach_iter != CMWG_SS_detach_last)
                        {
                            safe_detach_tokens.push_back(mNodeCheck->checkInNode(*CMWG_SS_detach_iter));
                            
                            // fix the counters
                            num_detached++;
                            CMWG_SS_detach_iter++;
                        }     
                        
                        std::vector<NodeCheckId>::iterator sdt_iter = safe_detach_tokens.begin();
                        std::vector<NodeCheckId>::iterator sdt_last = safe_detach_tokens.end();
                        while(sdt_iter != sdt_last)
                        {
                            GenericNodeId det_node = mNodeCheck->checkOutNode(true, CTXID, *sdt_iter);
                            
                            if(GN_NULL_ID != det_node)
                            {
                                if(det_node != *forkNode)
                                {
                                    // if forkNode is wrecked, then it will be fixed before
                                    // safeDetachNode exits!
                                    safeDetachNode(forkNode, det_node);
                                    
                                }
                                else
                                {
                                    // made if all the way back to the fork node. the growing 
                                    // was preeeety useless...
                                    *forkNode = mNodeCheck->checkOutNode(true, CTXID, fork_token); 
                                    
                                    return false;
                                }
                                
                            }
                            else
                            {
                                logError("1. NO MASTER ");
                            }
                            
                            sdt_iter++;
                        }
                        
                        // for very short insert sizes the query node (the old fork)
                        // may have been detached as a pair and may now be a dummy
                        // in this case it will have already been curated so there's no need to
                        // do anything.
                        if(mNodes->isValidAddress(query_node))
                        {
                            if(GN_NODE_TYPE_DETACHED != mNodes->getNodeType(DUALNODE, query_node))
                                mNodes->curateMasterState(query_node);
                        }
                        
                    }
                }
                else
                {
                    logError("No edges at cap: " << prev_cap );
                    *forkNode = mNodeCheck->checkOutNode(true, CTXID, fork_token); 
                    return false;
                }
            }
            prev_cap = nbe.NBE_CurrentNode;
        }
    }
    *forkNode = mNodeCheck->checkOutNode(true, CTXID, fork_token); 
    return true;
}
//HO 

    // for storing the rank of the inserts used to calculate forces
    static std::map<GenericNodeId, int> CMWG_PP_rank_map;

    // fuse these guys to store the probability distributions 
    // for each branch arm
    static std::map<GenericNodeId, double> CMWG_PP_per_arm_ave_over_map;
    static std::map<GenericNodeId, double> CMWG_PP_per_arm_ave_under_map;

    // these four maps are used to calculate the cumulative average stdev for 
    // each map, both under and over shoots...
    static std::map<GenericNodeId, double> CMWG_PP_per_arm_num_over_map;
    static std::map<GenericNodeId, double> CMWG_PP_per_arm_num_under_map;
    static std::map<GenericNodeId, double> CMWG_PP_per_arm_cum_over_map;
    static std::map<GenericNodeId, double> CMWG_PP_per_arm_cum_under_map;
    
    // we need to store the length of the arms
    static std::map<GenericNodeId, uMDInt> CMWG_PP_per_arm_length_map;
    
/*HV*/ bool
ContextMemWrapper::
/*HV*/ profilePrune(contextShoot * newShoot, GenericNodeId * stopNode, uMDInt outerForkPoint, bool ascending, ContextId CTXID)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // we need to work out the best path through this branch set
    // to do this we assign force values to nodes, based on the idea of springs
    // easy to do. see elsewhere for why. This function does the assignment
    // of power values
    //
    // The output of this function is the production of the crossQueue and the cross neighbours map
    // These are:
    // crossQueue - all the cross nodes which have been assigned a force value, that is, all cross nodes in the branch
    // crossNeighbours - all the forward nodes from all of the cross nodes. These guys have been assigned
    // the "average" force of all the forward branches.
    //
    // Sometimes we find that there is no crosses, this is OK, we make the new forward shoot here and 
    // return 0
    //
    // We start with current rank == 1 so we are only using edges from
    // the longest insert library. As soon as we can make a decision about
    // the best route we have "decided" and we break out. If we are still undecided
    // then we up the rank and try to decide using a shorter insert library
    // Note that this only makes sense when there is some "significant" difference
    // in the sizes of the insert libraries. (say PE vs MP) if the libraries are all
    // with mean of say 243, 248, 239 then it makes no sense to bias it this way
    // The way we decide whether to use rankings is this:
    // 1) if there is only one insert library then don't worry.
    // 2) if all the insert libraries lie within 1 sd of the mean for the shortest
    //    insert library then we consider them to all be the same.
    // This is all done automagically in the StatsManager and the Dataset
    //
    // clear all the static helper maps
    CMWG_PP_per_arm_ave_over_map.clear();
    CMWG_PP_per_arm_ave_under_map.clear();
    CMWG_PP_per_arm_num_over_map.clear();
    CMWG_PP_per_arm_num_under_map.clear();
    CMWG_PP_per_arm_cum_over_map.clear();
    CMWG_PP_per_arm_cum_under_map.clear();
    CMWG_PP_per_arm_length_map.clear();
    CMWG_PP_rank_map.clear();
    
    // we use this guy to manage the complex sorted list of cap nodes
    // this is a convenience object only.
    SortedTreeBranchManager stbm(ascending);
    
    // do some very simple shaving first. this also gives us a ist of all the cap nodes for this puppy
    if(!smartShave(stopNode, outerForkPoint, ascending))
    {
        return false;
    }
    
    NodeBlockElem nbe;
    mNodes->getCapBlock_DN(&nbe, CTXID);
    while(mNodes->nextNBElem(&nbe))
    {
        if(ascending ^ (mNodes->getCntxPos(nbe.NBE_CurrentNode) < outerForkPoint))
        {
            int ret_val = primeArm(&stbm, newShoot, ascending, outerForkPoint, true, nbe.NBE_CurrentNode, CTXID);
            if(0 == ret_val)
            { return true; }
            else if( 0 > ret_val )
            { return false;  }
        }
    }
    
    if(stbm.getNumBranches() == 0)
    { return false; }

    // now we ca go through and detach one arm at a time. All the arms 
    // have been sorted in the stbm so we just go through this datastructure until it tells us to quit...
    std::vector<GenericNodeId> * inner_vec;
    std::vector<GenericNodeId>::iterator inner_vec_iter;
    std::vector<GenericNodeId>::iterator inner_vec_last;
    while(stbm.getNext(&inner_vec))
    {
        // sanity check.
        if(inner_vec == NULL)
        {
            logError("corrupt inner vector");
            return false;
        }
        
        bool mult_groups = false;
        
        if(inner_vec->size() > 2)
        {
            // in cases where the list of contestants is greater than 2
            // we may have a situation where multiple separate groups oF CAPS
            // just happen to have fork points at the same position. so we need
            // to check for this explcitly...
            
            // there may be multiple groups all collapsed on top of one another
            // There is no pretty way around this so we need to just go through
            // and see how many groups there are
            GenericNodeId group_cross = GN_NULL_ID;
            GenericNodeEdgeElem group_gnee(DUALNODE);
            std::map<GenericNodeId, std::vector<GenericNodeId> * > group_map;                       // indexes the cross node to the vector of nodes attached to it
                                                                                                    // these are all key nodes
                                                                                                    // WE NEED this becuase there may be multiple cross ndoes
                                                                                                    // with the same position. so they need to be separated into groups
            std::map<GenericNodeId, std::vector<GenericNodeId> * >::iterator group_map_iter;
            
            GenericNodeId current_arm_key_GID;
            inner_vec_iter = inner_vec->begin();
            inner_vec_last = inner_vec->end();
            
            while(inner_vec_iter != inner_vec_last)
            {
                if(!stbm.getKey(&current_arm_key_GID, *inner_vec_iter))
                {
                    logError("no key for GID: " << *inner_vec_iter);
                }
                if(mNodes->getEdges(&group_gnee, GN_EDGE_STATE_NTRANS, 0, current_arm_key_GID))
                {
                    do {
                        if(mNodes->getNtRank(DUALNODE, mNodes->getOlapNode(group_gnee)) > 2)
                        {
                            // this is the cross we're looking for...
                            group_map_iter = group_map.find(mNodes->getOlapNode(group_gnee));
                            if(group_map_iter == group_map.end())
                            {
                                // new group here!
                                // so we need a new vector
                                std::vector<GenericNodeId> * new_vec = new std::vector<GenericNodeId>();
                                new_vec->push_back(*inner_vec_iter);
                                group_map[mNodes->getOlapNode(group_gnee)] = new_vec;
                            }
                            else
                            {
                                (group_map_iter->second)->push_back(*inner_vec_iter);
                            }
                            break;
                        }
                    } while(mNodes->getNextEdge(&group_gnee));
                }
                inner_vec_iter++;
            }
            if(group_map.size() > 1)
            {
                // we know we have got multiple groups
                // so we need to be a bit tricky here
                mult_groups = true;
                
                // release the current stbm list from memory
                stbm.release();
                
                // now we go through for each group
                group_map_iter = group_map.begin();
                std::map<GenericNodeId, std::vector<GenericNodeId> * >::iterator group_map_last = group_map.end();
               
                while(group_map_iter != group_map_last)
                {
                    // choose a winner
                    GenericNodeId winning_GID = GN_NULL_ID;
                    chooseBranchWinner(&stbm, group_map_iter->second, &winning_GID);
                    
                    // add the winning arm back into the list at the new position
                    // we know there are still multiple branches to go through here
                    // so we should never allow the formation of new shoots...
                    int ret_val = primeArm(&stbm, newShoot, ascending, outerForkPoint, false, winning_GID, CTXID);
                    if( 0 > ret_val )
                        return false;
                    
                    // free the memory...
                    if(group_map_iter->second != NULL)
                    {
                        delete group_map_iter->second;
                        group_map_iter->second = NULL;
                    }
                    group_map_iter++;
                }
            }
            else
            {
                // false alarm!
                // though we still need to free the memory used...
                group_map_iter = group_map.begin();
                std::map<GenericNodeId, std::vector<GenericNodeId> * >::iterator group_map_last = group_map.end();
                while(group_map_iter != group_map_last)
                {
                    if(group_map_iter->second != NULL)
                    {
                        delete group_map_iter->second;
                        group_map_iter->second = NULL;
                    }
                    group_map_iter++;
                }
            }
        }
        
        // we don't use an "else" here cause the above if may be a false alarm...
        if(!mult_groups)
        {
            // singular group guarunteed...
            // choose a winner
            GenericNodeId winning_GID = GN_NULL_ID;
            chooseBranchWinner(&stbm, inner_vec, &winning_GID);
            
            // release the current list from memory
            stbm.release();
            
            // add the winning arm back into the list at the new position
            int ret_val = primeArm(&stbm, newShoot, ascending, outerForkPoint, true, winning_GID, CTXID);
            
            if(0 == ret_val)
                return true;
            else if( 0 > ret_val )
                return false;            
        }
        // next!
    }
    
    logError("How did I get here?");
    return false;
}
//HO 

/*HV*/ int
ContextMemWrapper::
/*HV*/ primeArm(SortedTreeBranchManager * stbm, contextShoot * newShoot, bool ascending, uMDInt stopPoint, bool allowFirstBreak, GenericNodeId GID, ContextId CTXID)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // work out the distribution for a given arm
    // take care of managing all the various lists...
    // start at the longest insert size.
    // keep going through until we can assign a
    // power value to the node just before the cross node
    // these two will be passed to the stbm
    uMDInt fork_point = 0;
    GenericNodeId arm_key_GID = GN_NULL_ID;
    GenericNodeWalkingElem gnwe(DUALNODE);
    GenericNodeEdgeElem gnee(DUALNODE);
    GenericNodeId near_GID, ring_near_GID, far_GID;                 // IDs for near and far nodes
    bool near_rev = false;                                          // reverse flags too
    bool far_rev = false;
    sMDInt tdist = 0;
    bool in_range = false;
    int current_rank = 1;

    // these two will be passed to the stbm
    fork_point = 0;
    arm_key_GID = GN_NULL_ID;
            
    while(!in_range)
    {
        near_GID = GID;
        // find the next edge from the cap
        if(mNodes->getEdges(&gnee, GN_EDGE_STATE_NTRANS, 0, near_GID))
        {
            // there should only be one edge here.
            if(mNodes->startWalk(&gnwe, near_GID, mNodes->getOlapNode(gnee)))
            {
                // we need to know the position of the cross node
                // back behind the cap node we start from.
                // This is because we are only interested in reads which span this point.
                //
                // We also need to find out the ID of the last node before the cross because this
                // node will act as the "ID" for the entire arm
                uMDInt arm_end_pos = mNodes->getCntxPos(near_GID);
                do { //mNodes->printContentsOfWE(gnwe);
                    if(stopPoint == mNodes->getCntxPos(mNodes->getCurrentNode(gnwe)))
                    {
                        // there are a number of reasons why we could be at the stop node 
                        // but not all of them mean we can make new shoots and get on with it
                        // check to see if we're allowed o make new shoots here
                        if(allowFirstBreak && 0 == stbm->getNumBranches() && (mNodes->getNtRank(DUALNODE, mNodes->getCurrentNode(gnwe)) == 2))
                        {
                            updateShoot(newShoot, -1 * mNodes->getOffset(gnee), near_GID, CTXID);
                            return 0;
                        }
                        else
                        {
                            break;
                        }
                    }
                } while(mNodes->rankStep(&gnwe));
                
                fork_point = mNodes->getCntxPos(mNodes->getCurrentNode(gnwe));
                arm_key_GID = mNodes->getPrevNode(gnwe);

                // how long is this arm?
                if(ascending)
                    CMWG_PP_per_arm_length_map[near_GID] =  arm_end_pos - fork_point;
                else
                    CMWG_PP_per_arm_length_map[near_GID] =  fork_point - arm_end_pos;

                // initialise the per_arm maps
                // set these two to unrealistic values so that if we do nothing
                // to these guys then we will never choose them...
                CMWG_PP_per_arm_ave_over_map[arm_key_GID] = 1000000;
                CMWG_PP_per_arm_ave_under_map[arm_key_GID] = 1000000;
                CMWG_PP_per_arm_num_over_map[arm_key_GID] = 0;
                CMWG_PP_per_arm_num_under_map[arm_key_GID] = 0;
                CMWG_PP_per_arm_cum_over_map[arm_key_GID] = 0;
                CMWG_PP_per_arm_cum_under_map[arm_key_GID] = 0;

                // now re-start the walk
                mNodes->startWalk(&gnwe, near_GID, mNodes->getOlapNode(gnee));
                do {
                    // no point in calculating forces AT the stop point
                    if(stopPoint == mNodes->getCntxPos(mNodes->getCurrentNode(gnwe)))
                        break;
                    near_GID = mNodes->getPrevNode(gnwe);
                    // we need to go through the entire ring here.
                    // updating the per_arms map as we go
                    GN_ELEM data;
                    if(mNodes->getElem(&data, mNodes->getUniNode(near_GID)))
                    {
                        while(mNodes->getNextElem(&data))
                        {
                            ring_near_GID = data.GNE_GID;
                            // we cannot use a dummy node to get the insert distribution
                            // as dummies have no pairs.
                            if(!mNodes->isDummy(ring_near_GID))
                            {
                                far_GID = mNodes->getPair(ring_near_GID);
                                // make sure these guys are on opposite
                                // sides of the fork_point
                                if((mNodes->getCntxId(ring_near_GID) == CTXID) && (mNodes->getCntxPos(far_GID) > fork_point) ^ (mNodes->getCntxPos(near_GID) > fork_point) )
                                {
                                    near_rev = mNodes->isFileReversed(ring_near_GID) ^ mNodes->isCntxReversed(near_GID);
                                    far_rev = mNodes->isFileReversed(far_GID) ^ mNodes->isCntxReversed(far_GID);
                                    tdist = (sMDInt)(mNodes->getCntxPos(far_GID)) - (sMDInt)(mNodes->getCntxPos(near_GID));
                                    mStatsManager->updatePerArmMaps(ring_near_GID, far_GID, near_rev, far_rev, tdist, current_rank, arm_key_GID, &CMWG_PP_per_arm_num_over_map, &CMWG_PP_per_arm_cum_over_map, &CMWG_PP_per_arm_num_under_map, &CMWG_PP_per_arm_cum_under_map);
                                }
                            }
                        }
                    }
                } while(mNodes->rankStep(&gnwe));

                // if we had any hits then we should update the averages...
                if(CMWG_PP_per_arm_num_over_map[arm_key_GID] > 0)
                {
                    in_range = true;
                    CMWG_PP_per_arm_ave_over_map[arm_key_GID] = CMWG_PP_per_arm_cum_over_map[arm_key_GID] /  CMWG_PP_per_arm_num_over_map[arm_key_GID];
                }

                if(CMWG_PP_per_arm_num_under_map[arm_key_GID] > 0)
                {
                    in_range = true;
                    CMWG_PP_per_arm_ave_under_map[arm_key_GID] = CMWG_PP_per_arm_cum_under_map[arm_key_GID] /  CMWG_PP_per_arm_num_under_map[arm_key_GID];
                }

                // we should be at a cross now
                if(mNodes->getNtRank(DUALNODE, mNodes->getCurrentNode(gnwe)) > 2)
                {
                    // only add here if we know we have updated the force maps at least once
                    // for this rank
                    if(in_range)
                    {
                        // add the cap to the sortedTreeBranchManger
                        stbm->addNew(fork_point, arm_key_GID, GID);
                        CMWG_PP_rank_map[arm_key_GID] = current_rank;
                        break;
                    }
                }
                else
                {
                    // the rank here is less than 3. It should be just 2
                    // we can only be here when there was a fork just at the stopNode
                    // which has been since removed via shaving. Thus there is no branch
                    // to prune, just a single path. Our job is done!
                    // We should make a shoot and then return true so that extension can
                    // continue without writing to file
                    updateShoot(newShoot, -1 * mNodes->getOffset(gnee), GID, CTXID);
                    return 0;
                }
            }
            else
            {
                logError("could not walk from branch end!");
                return -1;
            }
        }
        else
        {
            logError("No edges for cap");
            return -1;
        }

        // search on a smaller insert library (if possible)
        current_rank++;
        if(current_rank > mStatsManager->getNumRankBands())
        {
            // nothing mapped for this guy.
            // something random is happening here.
            // add the node to the manager as we've already set the arm averages
            // to some hideous value above
            stbm->addNew(fork_point, arm_key_GID, GID);
            // just to be sure, set an impossible rank band...
            CMWG_PP_rank_map[arm_key_GID] = mStatsManager->getNumRankBands() + 1;
            break;
        }
    }
    if(!in_range)
    {
        logError("Couldn't define a profile for branch");
        return -1;
    }
    return 1;
}
//HO 

/*HV*/ void
ContextMemWrapper::
/*HV*/ chooseBranchWinner(SortedTreeBranchManager * stbm, std::vector<GenericNodeId> * contestants, GenericNodeId * winning_GID)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // for a given list of contestants, choose the winner and detach the branches of the losers
    // thus trimmimg down the extension
    //
    int best_rank = mStatsManager->getNumRankBands() + 1;       // set this guy to something impossible
    double best_profile_distance = 0;
    GenericNodeId current_arm_key_GID;
    *winning_GID = GN_NULL_ID;
    
    std::vector<GenericNodeId>::iterator inner_vec_iter = contestants->begin();
    std::vector<GenericNodeId>::iterator inner_vec_last = contestants->end();
    while(inner_vec_iter != inner_vec_last)
    {
        bool new_guy_wins = false;
        // get the key for this guy
        if(!stbm->getKey(&current_arm_key_GID, *inner_vec_iter))
        {
            logError("no key for GID: " << *inner_vec_iter);
        }
        if(CMWG_PP_rank_map[current_arm_key_GID] < best_rank)
        {
            // this guy is the new leader regardless of comparing profile distances
            // this always happens on the first run
            new_guy_wins = true;
        }
        else if (CMWG_PP_rank_map[current_arm_key_GID] == best_rank)
        {
            // equal rank comparisons so we must compare forces
            double current_profile_distance = mStatsManager->calculateProfileDistance(best_rank, CMWG_PP_per_arm_ave_over_map[current_arm_key_GID], CMWG_PP_per_arm_ave_under_map[current_arm_key_GID]);
            double percent_variance = 0.1;
            double bp_upper = best_profile_distance * (1 + percent_variance);
            double bp_lower = best_profile_distance * (1 - percent_variance);
            if((current_profile_distance <= bp_upper) && (current_profile_distance >= bp_lower))
            {
                // these guys are more or less the same
                // choose based on length
                if(CMWG_PP_per_arm_length_map[*inner_vec_iter] > CMWG_PP_per_arm_length_map[*winning_GID])
                    new_guy_wins = true;
            }
            else if(current_profile_distance < best_profile_distance)
            {
                // if he's not within the range, perhaps he's much lower
                // we have a new winner.
                new_guy_wins = true;
            }
            // else the new guy is rubbish (or... ... IDENTICAL!!!)
        }
        if(new_guy_wins)
        {
            // the current guy is the best so we'll need to add him in
            // first we need to check and see if there was already a guy
            // in first place...
            if(*winning_GID != GN_NULL_ID)
            {
                // there was a previous winner here, so we need to detach him
                GenericNodeWalkingElem tmp_gnwe(DUALNODE);
                GenericNodeEdgeElem tmp_gnee(DUALNODE);
                if(mNodes->getEdges(&tmp_gnee, GN_EDGE_STATE_NTRANS, 0, *winning_GID))
                {
                    // there should only be one edge here.
                    if(mNodes->startWalk(&tmp_gnwe, *winning_GID, mNodes->getOlapNode(tmp_gnee)))
                    {
                        while(mNodes->rankStep(&tmp_gnwe));
                    }
                    else
                        logError("Something went wrong here");
                }
                else
                    logError("Something went wrong here too");
                GenericNodeId last_cross = mNodes->getCurrentNode(tmp_gnwe);
                detachBranch(&last_cross, mNodes->getPrevNode(tmp_gnwe));
            }
            // now we are free to update
            best_rank = CMWG_PP_rank_map[current_arm_key_GID];
            *winning_GID = *inner_vec_iter;
            best_profile_distance = mStatsManager->calculateProfileDistance(best_rank, CMWG_PP_per_arm_ave_over_map[current_arm_key_GID], CMWG_PP_per_arm_ave_under_map[current_arm_key_GID]);
        }
        else
        {
            // the current guy is rubbish DETACH!!!
            GenericNodeWalkingElem tmp_gnwe(DUALNODE);
            GenericNodeEdgeElem tmp_gnee(DUALNODE);
            if(mNodes->getEdges(&tmp_gnee, GN_EDGE_STATE_NTRANS, 0, *inner_vec_iter))
            {
                // there should only be one edge here.
                if(mNodes->startWalk(&tmp_gnwe, *inner_vec_iter, mNodes->getOlapNode(tmp_gnee)))
                {
                    while(mNodes->rankStep(&tmp_gnwe));
                }
                else
                    logError("Something went wrong here too too");
            }
            else
                logError("Something went wrong here too too too");
            GenericNodeId last_cross = mNodes->getCurrentNode(tmp_gnwe);
            detachBranch(&last_cross, mNodes->getPrevNode(tmp_gnwe));
        }
        inner_vec_iter++;
    }
}
//HO 

//******************************************************************************
// CONTEXT MERGING / OVERLAPERATING
//******************************************************************************

//HO static bool sort_contexts_by_length_descending(const conLengthPair & left, const conLengthPair & right) { return right.second < left.second; }
//HO 
//HO static bool sort_contexts_by_length_ascending(const conLengthPair & left, const conLengthPair & right) { return left.second < right.second; }
//HO 

/*HV*/ void
ContextMemWrapper::
/*HV*/ renumberAll(bool setStartEndNodes)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // Go through all the Contexts and re-number them from 0 - length...
    // make sure that their caps are also stored in the correct order
    //
    std::map<ContextId, GenericNodeId>::iterator all_contexts_iter = mAllContexts.begin();
    std::map<ContextId, GenericNodeId>::iterator all_contexts_last = mAllContexts.end();
    while(all_contexts_iter != all_contexts_last)
    {
        renumberContext(setStartEndNodes, all_contexts_iter->first);
        all_contexts_iter++;
    }
    
}
//HO 

/*HV*/ void
ContextMemWrapper::
/*HV*/ renumberContext(bool setStartEndNodes, ContextId CTXID)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // Re-number context positions from 0 - length...
    // make sure that the caps are also stored in the correct order in the node block
    //
    // first fix up the cap block
    // Fix the3 start and end positions, preliminary...
    if(getStartPos(CTXID) > getEndPos(CTXID))
    {
        // switch these guys around
        uMDInt tmp_pos = getStartPos(CTXID);
        setStartPos(getEndPos(CTXID), CTXID);
        setEndPos(tmp_pos, CTXID);
    }
    
    NB_ELEM nbe;
    mNodes->getCapBlock_DN(&nbe, CTXID);
    GenericNodeId last_GID = GN_NULL_ID;
    while(mNodes->nextNBElem(&nbe))
    {
        last_GID = nbe.NBE_CurrentNode;
    }
    if(mNodes->getCntxPos(last_GID) != getEndPos(CTXID))
        mNodes->shuffleCapForward_DN(last_GID, CTXID);
    
    // now fix the ACTUAL context positions...
    GenericNodeWalkingElem gnwe(DUALNODE);
    GenericNodeId start_GID = GN_NULL_ID;
    GenericNodeId next_start_GID = GN_NULL_ID;
    mNodes->getCapBlock_DN(&nbe, CTXID);
    while(mNodes->nextNBElem(&nbe))
    {
        start_GID = nbe.NBE_CurrentNode;
        break;
    }
    GenericNodeEdgeElem gnee(DUALNODE);
    if(mNodes->getEdges(&gnee, GN_EDGE_STATE_NTRANS, 0,start_GID))
    {
        next_start_GID = mNodes->getOlapNode(gnee);
        
        uMDInt position = 0;
        setStartPos(position, CTXID);
        uMDInt last_position = 0;
        if(mNodes->startWalk(&gnwe, start_GID, next_start_GID))
        {
            if(setStartEndNodes)
            {
                setStartGn(start_GID, CTXID);
            }
            // process prev_node
            remapPos(&position, &last_position, mNodes->getPrevNode(gnwe), CTXID);
            do {
                //process current node
                remapPos(&position, &last_position, mNodes->getCurrentNode(gnwe), CTXID);
            } while(mNodes->rankStep(&gnwe));
        }
        setEndPos(position, CTXID);
        if(setStartEndNodes)
        {
            setEndGn(mNodes->getCurrentNode(gnwe), CTXID);
        }
    }
}
//HO 

/*HV*/ bool
ContextMemWrapper::
/*HV*/ overlaperateAll(sMDInt minOlap, sMDInt maxWastage)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // There is a tendency to over assemble sequences.
    // we will find that many of the contexts share really long overlaps
    // This function will find (most of) these sequences and then join the contexts together
    //
    // Assumes that a set of contigs have already been made
    // 
    if(0 == mAllContexts.size())
    {
        logError("No contexts to overlaperate!");
        return false;
    }
    else if(1 == mAllContexts.size())
    {
        logInfo("Only 1 context, no need to overlaperate!", 1);
        return true;
    }
    if(NULL == mContigs)
    {
        logError("No Contigs have been made!");
        return false;
    }
    logInfo("Start Overlaperating " << mAllContexts.size() << " contigs", 1);
    
    // fix up the stats
    std::string longest_prelim_contig;
    ContigId longest_CID = mContigs->getNthLongestContig(1);
    mContigs->getContigSequence(&longest_prelim_contig, longest_CID);
    
    // set or override the min olap and max wastage values
    if(-1 != minOlap)
        mMinOlap = (uMDInt)minOlap;
    else
        mMinOlap = mStatsManager->getHighestUpperCut();
    if(-1 != maxWastage)
        mMaxWastage = (uMDInt)maxWastage;
    else
        mMaxWastage = mStatsManager->getReadLength() * 2;
    
    logInfo("Overlaperating with min overlap: " << mMinOlap << " and max wastage: " << mMaxWastage, 1);
    
    // set the cut offs
    mStatsManager->makeCutOffs(SAS_DEF_TOLL_MAX);
    
    // tell the world about it
    mDataSet->logFileStats(1);
    
    // stores the length of the 1 contig in this context vs ContextId
    conLengthQueue all_len_contexts_obj;
    conLengthQueue * all_len_contexts = &all_len_contexts_obj;
    std::map<ContextId, GenericNodeId> con_start_map;
    std::map<ContextId, GenericNodeId> con_next_start_map;
    
    // we need to get the contig sizes and attach them to the contexts.
    // also store other identifying info like end positions etc...
    std::map<ContextId, GenericNodeId>::iterator all_contexts_iter = mAllContexts.begin();
    std::map<ContextId, GenericNodeId>::iterator all_contexts_last = mAllContexts.end();
    NodeBlockElem nbe;
    while(all_contexts_iter != all_contexts_last)
    {
        mNodes->getCapBlock_DN(&nbe, all_contexts_iter->first);
        while(mNodes->nextNBElem(&nbe))
        {
            // we need to store information about each context....
            ContigId current_contig = mNodes->getContig(DUALNODE, nbe.NBE_CurrentNode);
            ContextId this_context = mNodes->getCntxId(nbe.NBE_CurrentNode);
            
            // save the start and next start nodes
            con_start_map[this_context] = mContigs->getStart(current_contig);
            con_next_start_map[this_context] = mContigs->getNextStart(current_contig);
            
            break;
        }
        all_contexts_iter++;
    }
    
    all_contexts_iter = mAllContexts.begin();
    while(all_contexts_iter != mAllContexts.end())
    {
        all_len_contexts->push_back(conLengthPair(all_contexts_iter->first, getLongestContig(all_contexts_iter->first)));
        all_contexts_iter++;
    }
    
    // sort the structure from shortest to longest
    std::sort(all_len_contexts->begin(), all_len_contexts->end(), sort_contexts_by_length_ascending);
    
    if(all_len_contexts->size() == 0)
    {
        logError("No contigs to overlaperate");
        return false;
    }
    
    // now we have made all the contigs and they are sorted ascending
    // all other information has been extracted and loaded...
    
    while(all_len_contexts->size() > 0)
    {
        // PREP DATA STRUCTS    
        // do the next context in the list (this should be the shortest one around...)
        conLengthPair tmp_pair = all_len_contexts->front();
        all_len_contexts->pop_front();
        ContextId current_context = tmp_pair.first;
        
        // get the start and next start nodes so we can start walking.
        GenericNodeId start_GID = con_start_map[current_context];
        GenericNodeId next_start_GID = con_next_start_map[current_context];
        con_start_map.erase(current_context);
        con_next_start_map.erase(current_context);
        
        C_OLAP tmp_CO(GN_NULL_ID);
        
        // TRY OVERLAPERATE
        uMDInt total_length = overlaperateContext(&tmp_CO, &con_start_map, start_GID, next_start_GID, current_context);
        
        // UPDATE LISTS / DATA STRUCTS
        if(0 != total_length)
        {
            con_start_map[tmp_CO.CO_ConA] = tmp_CO.CO_AStart;
            con_next_start_map[tmp_CO.CO_ConA] = tmp_CO.CO_ANextStart;
            
            // if the overlap is contained then the longer context
            // will remain the same length, otherwise we will need to update
            if(!tmp_CO.CO_Contained)
            {
                // we'll need to update the length...
                conLengthQueueIterator clq_iter = all_len_contexts->begin();
                while(clq_iter != all_len_contexts->end())
                {
                    if((*clq_iter).first == tmp_CO.CO_ConA)
                    {
                        (*clq_iter).second = total_length;
                    }
                    clq_iter++;
                }
                
                // The longer context may have become longer, so do a re-sort just to make
                // sure everything is kosher...
                std::sort(all_len_contexts->begin(), all_len_contexts->end(), sort_contexts_by_length_ascending);
            }
        }
        
    } // GOTO next shortset contig (while)
    
    return true;
}
//HO 

/*HV*/ uMDInt
ContextMemWrapper::
/*HV*/ overlaperateContext(C_OLAP * contextOverlap, std::map<ContextId, GenericNodeId> * overlaperatableContexts, GenericNodeId start_GID, GenericNodeId nextStart_GID, ContextId CTXID)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // Overlaperate (if possible) this contig.
    // Return the length of the overlaperated context if sucessful
    // or 0 if there was no overlaperation
    //
    
    std::list<CTX_OLAP_CONTENDER *> contender_list;                 // all of the possible contenders
    std::list<CTX_OLAP_CONTENDER *> confirmed_contender_list;       // confirmed overlaps
    std::list<CTX_OLAP_CONTENDER *>::iterator coc_iter;
    std::map<uMDInt, GenericNodeId> ring_hash;
    std::map<uMDInt, GenericNodeId>::iterator ring_iter;
    std::map<ContextId, GenericNodeId>::iterator overlaperatableContexts_finder;
    
    // ALL V ALL COMPARISON
    // walk along the entire length of the context
    // compare all other contexts using the uninode graph...
    GenericNodeElem gne;
    GenericNodeWalkingElem gnwe(DUALNODE);
    if(mNodes->startWalk(&gnwe, start_GID, nextStart_GID))
    {
        // FIRST NODE SPECIAL CASE
        // we need to duplicate code to handle the first node explicitly...
        // all matches here get made into contenders!
        if(mNodes->getElem(&gne, mNodes->getPrevNode(gnwe)))
        {
            // elem contains only the head
            while(mNodes->getNextElem(&gne))
            {
                // now elem contains a non-head node
                // this is the true start node of the contig for this context
                if(mNodes->isCntxMaster(gne.GNE_GID))
                {
                    // we can only merge this guy with guys we have placed in the overlaperatableContexts
                    // and provided that they are not in this context
                    ContextId query_CTXID = mNodes->getCntxId(gne.GNE_GID);
                    if(CTXID != query_CTXID)
                    {
                        overlaperatableContexts_finder = overlaperatableContexts->find(query_CTXID);
                        if(overlaperatableContexts->end() != overlaperatableContexts_finder)
                        {
                            // huzzah! a match
                            // make a new contender
                            CTX_OLAP_CONTENDER * new_contender = new CTX_OLAP_CONTENDER(mNodes, true, mNodes->getPrevNode(gnwe), gne.GNE_GID, CTXID, query_CTXID, mNodes->getCntxPos(gne.GNE_GID), mStatsManager->getReadLength());
                            PARANOID_ASSERT_PRINT_L4(getLongestContig(query_CTXID) >= getLongestContig(CTXID),  "[WTF - Addnew: " << getLongestContig(query_CTXID) << " >= " << getLongestContig(CTXID));
                            contender_list.push_back(new_contender);
                        }
                    }
                }
            }
        }
        else
            logError("No loop elem found(PREV)");
        // THE REMAINING NODES
        do {
            GenericNodeId curr_walk_node = mNodes->getCurrentNode(gnwe);
            
            // store the absolute value of the offset so we can work out positions relative to both contexts
            // we hash all the information and then re-hash it in a map. This makes
            // for quick lookup in the more expensive part of the algorithm below...
            uMDInt abs_offset = (uMDInt)fAbs(mNodes->getOffset(gnwe));
            
            // hash the context_pos_hash to the node which made it
            ring_hash.clear();
            
            // now we handle the current node as we move along
            if(mNodes->getElem(&gne, mNodes->getCurrentNode(gnwe)))
            {
                // elem contains only the head
                while(mNodes->getNextElem(&gne))
                {
                    // now elem contains a non-head node
                    overlaperatableContexts_finder = overlaperatableContexts->find(mNodes->getCntxId(gne.GNE_GID));
                    if(overlaperatableContexts->end() != overlaperatableContexts_finder)
                    {
                        if(mNodes->isCntxMaster(gne.GNE_GID))
                        {
                            // This is a master node in a context not the current one
                            // It's possible we have a match. We can hash these bozos...
                            ring_hash[mNodes->getCntxPosHash(gne.GNE_GID)] = gne.GNE_GID;
                        }
                    }
                }
            }
            else
                logError("No loop elem found (CURR)");
            
            // we've hashed all the nodes which match at this position
            // so go through all the contenders and check to see if they are still valid
            coc_iter = contender_list.begin();
            while(coc_iter != contender_list.end())
            {
                // we set the default behaviour to delete the contender and
                // iterate through the contender list. If we are given reason to do otherwise then
                // we will change these values
                bool should_delete = true;
                bool should_inc = true;
                
                // contenders are not "real" until they are at least 4 nodes long
                // so we need to do something different depending on how long these
                // guys are
                switch((*coc_iter)->COC_NodeLength)
                {
                    case 1:
                    {
                        // this is the shortest a potential contender can be
                        // it has only one node as a match.
                        // we are still not certain if this guy is ascending or descending...
                        // time to sort that one out...
                        GenericNodeEdgeElem gnee_asc(DUALNODE);
                        uMDInt hash = 0;
                        if(mNodes->getEdges(&gnee_asc, GN_EDGE_STATE_NTRANS, 0, (*coc_iter)->COC_StartLong))
                        {
                            do {
                                // make the hash and see...
                                GenericNodeId olap_node = mNodes->getOlapNode(gnee_asc);
                                hash = mNodes->getCntxPosHash(olap_node);
                                // look for the hashed value in the ring hash
                                ring_iter = ring_hash.find(hash);
                                if(ring_iter != ring_hash.end())
                                {
                                    // determine ascending otr descending
                                    uMDInt new_pos = mNodes->getCntxPos(olap_node);
                                    if(new_pos > (*coc_iter)->COC_LongEndPos)
                                    {
                                        (*coc_iter)->COC_LongAscending = true;
                                    }
                                    else
                                    {
                                        (*coc_iter)->COC_LongAscending = false;
                                    }
                                    // update the contender information
                                    (*coc_iter)->COC_NextStartShort = mNodes->getCurrentNode(gnwe);
                                    (*coc_iter)->COC_NextStartLong = ring_hash[hash];
                                    (*coc_iter)->COC_NodeLength++;
                                    (*coc_iter)->COC_OlapLength += abs_offset;
                                    (*coc_iter)->COC_LongEndPos = mNodes->getCntxPos((*coc_iter)->COC_NextStartLong);
                                    
                                    // remove from the ring
                                    ring_hash.erase(ring_iter);
                                    
                                    // we won't need to delete this guy
                                    should_delete = false;
                                    break;
                                }
                            } while(mNodes->getNextEdge(&gnee_asc));
                        }
                        else
                            logError("No edges for first olaperate: " << mNodes->getCurrentNode(gnwe));
                        break;
                    }
                    case 2:
                    {
                        // we can add a little more information about this one
                        uMDInt hash = 0;
                        if((*coc_iter)->COC_LongAscending)
                            hash= mNodes->getCntxPosHash((*coc_iter)->COC_LongEndPos + abs_offset, (*coc_iter)->COC_LongContext);
                        else
                            hash= mNodes->getCntxPosHash((*coc_iter)->COC_LongEndPos - abs_offset, (*coc_iter)->COC_LongContext);
                        
                        // now we see if we hashed this hash...
                        ring_iter = ring_hash.find(hash);
                        if(ring_iter != ring_hash.end())
                        {
                            (*coc_iter)->COC_NextEndShort = mNodes->getCurrentNode(gnwe);
                            (*coc_iter)->COC_NextEndLong = ring_hash[hash];
                            (*coc_iter)->COC_NodeLength++;
                            (*coc_iter)->COC_OlapLength += abs_offset;
                            (*coc_iter)->COC_LongEndPos = mNodes->getCntxPos((*coc_iter)->COC_NextEndLong);
                            
                            // remove from the ring
                            ring_hash.erase(ring_iter);
                            
                            // we won't need to delete this guy
                            should_delete = false;
                        }
                        break;
                    }
                    case 3:
                    {
                        // we can finalise this one (prelim), we'll have all the information soon
                        uMDInt hash = 0;
                        if((*coc_iter)->COC_LongAscending)
                            hash= mNodes->getCntxPosHash((*coc_iter)->COC_LongEndPos + abs_offset, (*coc_iter)->COC_LongContext);
                        else
                            hash= mNodes->getCntxPosHash((*coc_iter)->COC_LongEndPos - abs_offset, (*coc_iter)->COC_LongContext);
                        
                        // now we see if we hashed this hash...
                        ring_iter = ring_hash.find(hash);
                        if(ring_iter != ring_hash.end())
                        {
                            (*coc_iter)->COC_EndShort = mNodes->getCurrentNode(gnwe);
                            (*coc_iter)->COC_EndLong = ring_hash[hash];
                            (*coc_iter)->COC_NodeLength++;
                            (*coc_iter)->COC_OlapLength += abs_offset;
                            (*coc_iter)->COC_LongEndPos = mNodes->getCntxPos((*coc_iter)->COC_EndLong);
                            
                            // remove from the ring
                            ring_hash.erase(ring_iter);
                            
                            // we won't need to delete this guy
                            should_delete = false;
                        }
                        break;
                    }
                    default:
                    {
                        // longer than 4 nodes. most will be in this situation
                        uMDInt hash = 0;
                        if((*coc_iter)->COC_LongAscending)
                            hash= mNodes->getCntxPosHash((*coc_iter)->COC_LongEndPos + abs_offset, (*coc_iter)->COC_LongContext);
                        else
                            hash= mNodes->getCntxPosHash((*coc_iter)->COC_LongEndPos - abs_offset, (*coc_iter)->COC_LongContext);
                        
                        // now we see if we hashed this hash...
                        ring_iter = ring_hash.find(hash);
                        if(ring_iter != ring_hash.end())
                        {
                            (*coc_iter)->COC_NextEndShort = (*coc_iter)->COC_EndShort;
                            (*coc_iter)->COC_NextEndLong = (*coc_iter)->COC_EndLong;
                            (*coc_iter)->COC_EndShort = mNodes->getCurrentNode(gnwe);
                            (*coc_iter)->COC_EndLong = ring_hash[hash];
                            (*coc_iter)->COC_NodeLength++;
                            (*coc_iter)->COC_OlapLength += abs_offset;
                            (*coc_iter)->COC_LongEndPos = mNodes->getCntxPos((*coc_iter)->COC_EndLong);
                            
                            // remove from the ring
                            ring_hash.erase(ring_iter);
                            
                            // we won't need to delete this guy
                            should_delete = false;
                        }
                        else
                        {
                            // This contender is finished for now.
                            // it is at least 4 long so we need to finalise the information and put it away for use later
                            if(((*coc_iter)->COC_OlapLength >= mMinOlap) || ((*coc_iter)->COC_OlapLength == getLongestContig((*coc_iter)->COC_ShortContext)))
                            {
                                // this guy is long enough to be moved into the completed contenders list
                                // assume that everything was put into order on the last iteration
                                confirmed_contender_list.push_back(*coc_iter);
                                
                                // remove it from this list
                                coc_iter = contender_list.erase(coc_iter);
                                
                                // we won't need to delete this guy
                                should_delete = false;
                                // and we've already moved the iterator
                                should_inc = false;
                            }
                        }
                    }
                }
                // do we have to delete?
                if(should_delete)
                {
                    // delete this guy
                    if(*coc_iter != NULL)
                    {
                        delete *coc_iter;
                        *coc_iter = NULL;
                    }
                    
                    // make sure that we don't ++ the iterator
                    coc_iter = contender_list.erase(coc_iter);
                    should_inc = false;
                }
                
                if(should_inc)
                    coc_iter++;
            }
            
            // any guys left in the ring must be new contenders
            ring_iter = ring_hash.begin();
            while(ring_iter != ring_hash.end())
            {
                CTX_OLAP_CONTENDER * new_contender = new CTX_OLAP_CONTENDER(mNodes, true, mNodes->getCurrentNode(gnwe), ring_iter->second, CTXID, mNodes->getCntxId(ring_iter->second), mNodes->getCntxPos(ring_iter->second), mStatsManager->getReadLength());
                contender_list.push_back(new_contender);
                ring_iter++;
            }
        } while(mNodes->rankStep(&gnwe));
    }
    else
        logError("Could not start walk: " << CTXID);
    
    // CONSOLIDATE ANY OPEN CONTENDERS
    // we have walked the length of the context. All overlaps have been
    // at least partially recorded or discarded). Post-process the leftovers...
    coc_iter = contender_list.begin();
    while(coc_iter != contender_list.end())
    {
        bool valid_olap = false;
        if(((*coc_iter)->COC_OlapLength >= mMinOlap) || ((*coc_iter)->COC_OlapLength == getLongestContig((*coc_iter)->COC_ShortContext)))
        {
            // this guy is long enough to be moved into the completed contenders list
            // assume that everything was put into order on the last iteration
            if((*coc_iter)->COC_NodeLength >= 4)
            {
                valid_olap = true;
                confirmed_contender_list.push_back(*coc_iter);
            }
        }
        else
        {
            logInfo("Rejecting overlap between: " << (*coc_iter)->COC_LongContext << " and " << (*coc_iter)->COC_ShortContext << " as it is only: " << (*coc_iter)->COC_OlapLength << " bases long", 1);
        }
        
        if(!valid_olap)
        {
            // too "short" so delete!
            if(*coc_iter != NULL)
            {
                delete *coc_iter;
                *coc_iter = NULL;
            }
        }
        // remove it from this list
        coc_iter = contender_list.erase(coc_iter);
    }

    // CHOOSE WINNER
    // overlaperate if necessary
    // first we need to choose a winner!
    // Go through all the confirmed contenders and choose
    // the best one. This is a greedy algorithm and could most likely
    // be improved.
    //
    // It works like this:
    //
    // First we look of the contigs with the longest overlap
    // If there is a clear winner then we are happy to just overlaperate these two, provided that it is sane to do so.
    // If two guys have the same overlap the we compare how much sequence will be wasted
    // when we overlaperate. If this is the same, then we look at who will give us the longest final contig
    // and we choose the longest one. If these are the same then we just choose one...
    //
    CTX_OLAP_CONTENDER * olap_winner = NULL;
    uMDInt longest_olap = 0;
    uMDInt total_length = 0;
    uMDInt total_waste  = 0;
    double waste_olap_ratio = HUGE_VAL;
    unsigned int steps_walked  = 0;
    
    // To make the merging process work faster we always assume
    // that we are passing the keeping end of A through to the merging function
    // For contained ones and type 3 (see below) the start remains the same,
    // but for type 2 we will need to flip the overlap around
    bool flip_contender = false;
    
    coc_iter = confirmed_contender_list.begin();
    ContextId sc = (*coc_iter)->COC_ShortContext;
    
    coc_iter = confirmed_contender_list.begin();
    while(coc_iter != confirmed_contender_list.end())
    {
        ContextId lc = (*coc_iter)->COC_LongContext;
        uMDInt this_olap_length = (*coc_iter)->COC_OlapLength;
        
        // OK to compare on longest olap as contained olaps are by default the longest they can be...
        // If the overlaps are within 10% of eachother, then it's worth looking into this further...
        if(this_olap_length >= longest_olap * 0.9)
        {
            double tmp_waste_ratio = HUGE_VAL;
            uMDInt tmp_total_length = 0;
            
            // the overlap was good, what about the wastage?
            uMDInt ssw, sew, lsw, lew;
            ssw = sew = lsw = lew = (mMaxWastage + 1);
            
            // now we need to calculate the wastage at either end...
            bool short_con_asc = (getEndPos(sc) > getStartPos(sc));
            bool long_con_asc = (getEndPos(lc) > getStartPos(lc));
            bool short_olap_dsc = (mNodes->getCntxPos((*coc_iter)->COC_StartShort) > mNodes->getCntxPos((*coc_iter)->COC_EndShort));
            bool long_olap_dsc = (mNodes->getCntxPos((*coc_iter)->COC_StartLong) > mNodes->getCntxPos((*coc_iter)->COC_EndLong));
            
            if(short_con_asc ^ short_olap_dsc)
            {
                // the overlap is in the same direction as the context
                if(mNodes->getCntxPos((*coc_iter)->COC_StartShort) >= getStartPos(sc))
                    ssw =  mNodes->getCntxPos((*coc_iter)->COC_StartShort) - getStartPos(sc);
                else
                    ssw =  getStartPos(sc) - mNodes->getCntxPos((*coc_iter)->COC_StartShort);
                if(getEndPos(sc) >= mNodes->getCntxPos((*coc_iter)->COC_EndShort))
                    sew =  getEndPos(sc) - mNodes->getCntxPos((*coc_iter)->COC_EndShort);
                else
                    sew =  mNodes->getCntxPos((*coc_iter)->COC_EndShort) - getEndPos(sc);
            }
            else
            {
                // the overlap faces backwards down the context
                // this can never happen in the current use but it's put here jic...
                if(getEndPos(sc) >= mNodes->getCntxPos((*coc_iter)->COC_StartShort))
                    ssw =  getEndPos(sc) - mNodes->getCntxPos((*coc_iter)->COC_StartShort);
                else
                    ssw =  mNodes->getCntxPos((*coc_iter)->COC_StartShort) - getEndPos(sc);
                if(mNodes->getCntxPos((*coc_iter)->COC_EndShort) >= getStartPos(sc))
                    sew =  mNodes->getCntxPos((*coc_iter)->COC_EndShort) - getStartPos(sc);
                else
                    sew =  getStartPos(sc) - mNodes->getCntxPos((*coc_iter)->COC_EndShort);
            }
            if(long_con_asc ^ long_olap_dsc)
            {
                // The overlap is in the same direction as the contig has been written
                if(mNodes->getCntxPos((*coc_iter)->COC_StartLong) >= getStartPos(lc))
                    lsw =  mNodes->getCntxPos((*coc_iter)->COC_StartLong) - getStartPos(lc);
                else
                    lsw =  getStartPos(lc) - mNodes->getCntxPos((*coc_iter)->COC_StartLong);
                if(getEndPos(lc) >= mNodes->getCntxPos((*coc_iter)->COC_EndLong))
                    lew =  getEndPos(lc) - mNodes->getCntxPos((*coc_iter)->COC_EndLong);
                else
                    lew =  mNodes->getCntxPos((*coc_iter)->COC_EndLong) - getEndPos(lc);
            }
            else
            {
                // The overlap is in the opposite direction as the contig has been written
                if(getEndPos(lc) >= mNodes->getCntxPos((*coc_iter)->COC_StartLong))
                    lsw =  getEndPos(lc) - mNodes->getCntxPos((*coc_iter)->COC_StartLong);
                else
                    lsw =  mNodes->getCntxPos((*coc_iter)->COC_StartLong) - getEndPos(lc);
                if(mNodes->getCntxPos((*coc_iter)->COC_EndLong) >= getStartPos(lc))
                    lew =  mNodes->getCntxPos((*coc_iter)->COC_EndLong) - getStartPos(lc);
                else
                    lew =  getStartPos(lc) - mNodes->getCntxPos((*coc_iter)->COC_EndLong);
            }
            
            // there are only three ways to put these guys together
            // 1. the short is completly contained within the longer
            //
            //             ----|--SHORT--|----             <= TOTAL_LENGTH = length(long)
            //   --------------|---LONG--|-------------
            //
            // 2. we start with the short and then go into the long
            //
            //   ---------|--SHORT--|----                   <= TOTAL_LENGTH = length(long) + length(short) - olap - lsw - sew
            //        ----|---LONG--|----------------------
            //
            // 3. we start with the long and then go into the short
            //
            //                ----|--SHORT--|----------     <= TOTAL_LENGTH = length(long) + length(short) - olap - ssw - lew
            //   -----------------|---LONG--|--
            //
            //
            // How to identify these cases?
            //
            // 1. ssw <= lsw and sew <= lew
            // 2. lsw < ssw and sew <= lew
            // 3. ssw <= lsw and lew < sew
            int cross_over_limit = 3;
            
            if((sew <= mMaxWastage) && (sew <= lew))
            {
                // end with long
                if((ssw <= mMaxWastage) && (ssw <= lsw))
                {
                    // 1 -- contained (always ok)
                    tmp_total_length = getLongestContig(lc);
                    tmp_waste_ratio = (double)(ssw + sew) / (double)tmp_total_length;
                    if((longest_olap < this_olap_length) || (tmp_waste_ratio < waste_olap_ratio))
                    {
                        longest_olap = this_olap_length;
                        waste_olap_ratio = tmp_waste_ratio;
                        total_length = tmp_total_length;
                        total_waste = ssw + sew;
                        flip_contender = false;
                        olap_winner = *coc_iter;
                        (*coc_iter)->COC_Contained = true;
                    }
                }
                else if((lsw <= mMaxWastage) && (lsw <= ssw))
                {
                    // 2 -- start with short but end with long
                    tmp_total_length = getLongestContig(lc) + getLongestContig(sc) - this_olap_length - lsw - sew;
                    tmp_waste_ratio = (double)(lsw + sew) / (double)tmp_total_length;
                    if((longest_olap < this_olap_length) || (tmp_waste_ratio < waste_olap_ratio))
                    {
                        bool viable_olap = false;
                        int cross_over_count = 0;
                        // if the overlap is a suitable length then there is nothing to worry about
                        if(this_olap_length > mMinOlap)
                            viable_olap = true;
                        else
                        {
                            // This guy is shorter than the "safe" overlap minimum
                            // we should check it out to see if there is a paired read
                            // which spans the overlap. (any length paired read will do)
                            
                            // check that there is enough sequence on the end to search in
                            if((ssw <= (uMDInt)(mStatsManager->getReadLength())) || (lew <= (uMDInt)(mStatsManager->getReadLength()))) {  } //viable_olap = true; }
                            else
                            {
                                // we need to work out what the sweet spot is on the long contig.
                                // if the overlap agrees with the direction of the context then we will need
                                // a position greater then the end of the overlap, otherwise it will be less than the end
                                bool find_gt = false;
                                uMDInt limit = mNodes->getCntxPos((*coc_iter)->COC_EndLong);
                                if(long_olap_dsc)
                                {
                                    // would like it less than the position at the end
                                }
                                else
                                {
                                    // would like a pair greater than the position at the end
                                    find_gt = true;
                                }
                                // regardless of the orientation of the short overlap we will always
                                // walk back away from the start of the short overlap
                                GenericNodeWalkingElem gnwe_olap(DUALNODE);
                                
                                steps_walked  = 0;
                                uMDInt start_short = mNodes->getCntxPos((*coc_iter)->COC_StartShort);
                                uMDInt end_long = mNodes->getCntxPos((*coc_iter)->COC_EndLong);
                                uMDInt olap_length = (*coc_iter)->COC_OlapLength;

                                mNodes->startWalk(&gnwe_olap, (*coc_iter)->COC_NextStartShort, (*coc_iter)->COC_StartShort);
                                do {
                                    if(doesPairedReadSpanOlap(mNodes->getCurrentNode(gnwe_olap), start_short, end_long, olap_length, find_gt, limit, short_olap_dsc, long_olap_dsc, sc, lc))
                                    {
                                        cross_over_count++;
                                        if(cross_over_count == cross_over_limit )
                                        {
                                            viable_olap = true;
                                            break;
                                        }
                                    }
                                    
                                    // no need to walk all the way to the end of the context
                                    steps_walked++;
                                    if((mMinOlap * 2) < steps_walked)
                                        break;
                                } while(mNodes->rankStep(&gnwe_olap));
                            }
                        }
                        if(viable_olap)
                        {
                            longest_olap = this_olap_length;
                            waste_olap_ratio = tmp_waste_ratio;
                            total_length = tmp_total_length;
                            total_waste = lsw + sew;
                            flip_contender = true;
                            olap_winner = *coc_iter;
                            (*coc_iter)->COC_Contained = false;
                        }
                    }
                }
            }
            else if((lew <= mMaxWastage) && (lew <= sew))
            {
                // end with short
                if((ssw <= mMaxWastage) && (ssw <= lsw))
                {
                    // 3 -- start with long but end with short
                    tmp_total_length = getLongestContig(lc) + getLongestContig(sc) - this_olap_length - ssw - lew;
                    tmp_waste_ratio = (double)(ssw + lew) / (double)tmp_total_length;
                    if((longest_olap < this_olap_length) || (tmp_waste_ratio < waste_olap_ratio))
                    {
                        bool viable_olap = false;
                        int cross_over_count = 0;
                        // if the overlap is a suitable length then there is nothing to worry about
                        if(this_olap_length > mMinOlap)
                            viable_olap = true;
                        else
                        {
                            // This guy is shorter than the "safe" overlap minimum
                            // we should check it out to see if there is a paired read
                            // which spans the overlap. (any length paired read will do)
                            // check that there is enough sequence on the end to search in
                            if((sew <= (uMDInt)(mStatsManager->getReadLength())) || (lsw <= (uMDInt)(mStatsManager->getReadLength()))) { } //viable_olap = true; }
                            else
                            {
                                // we need to work out what the sweet spot is on the long contig.
                                // if the overlap agrees with the direction of the context then we will need
                                // a position greater then the end of the overlap, otherwise it will be less than the end
                                bool find_gt = false;
                                uMDInt limit = mNodes->getCntxPos((*coc_iter)->COC_EndShort);
                                if(short_olap_dsc)
                                {
                                    // would like it less than the position at the end
                                }
                                else
                                {
                                    // would like a pair greater than the position at the end
                                    find_gt = true;
                                }
                                
                                // regardless of the orientation of the long overlap we will always
                                // walk back away from the start of the long overlap
                                GenericNodeWalkingElem gnwe_olap(DUALNODE);
                                
                                steps_walked  = 0;
                                uMDInt start_long = mNodes->getCntxPos((*coc_iter)->COC_StartLong);
                                uMDInt end_short = mNodes->getCntxPos((*coc_iter)->COC_EndShort);
                                uMDInt olap_length = (*coc_iter)->COC_OlapLength;

                                mNodes->startWalk(&gnwe_olap, (*coc_iter)->COC_NextStartLong, (*coc_iter)->COC_StartLong);
                                do {
                                    if(doesPairedReadSpanOlap(mNodes->getCurrentNode(gnwe_olap), start_long, end_short, olap_length, find_gt, limit, long_olap_dsc, short_olap_dsc, lc, sc))
                                    {
                                        cross_over_count++;
                                        if(cross_over_count == cross_over_limit )
                                        {
                                            viable_olap = true;
                                            break;
                                        }
                                    }
                                    
                                    // no need to walk all the way to the end of the context
                                    steps_walked++;
                                    if((mMinOlap * 2) < steps_walked)
                                        break;
                                    
                                } while(mNodes->rankStep(&gnwe_olap));
                            }
                        }
                        if(viable_olap)
                        {
                            longest_olap = this_olap_length;
                            waste_olap_ratio = tmp_waste_ratio;
                            total_length = tmp_total_length;
                            total_waste = ssw + lew;
                            flip_contender = false;
                            olap_winner = *coc_iter;
                            (*coc_iter)->COC_Contained = false;
                        }
                    }
                }
                else if((lsw <= mMaxWastage) && (lsw <= ssw))
                {
                    // error! -- start and end with short
                    logError("long appears to be shorter than short");
                }
            }
        }
        coc_iter++;
    }
    
    // OVERLAPERATE                                     
    // do we have a winner?
    // returning start points after merging
    GenericNodeId new_start_GID, new_next_start_GID;
    uMDInt new_start_pos, new_end_pos;
    
    if(olap_winner != NULL)
    {
        // make an overlap object
        contextOverlap->CO_ConA = olap_winner->COC_LongContext;
        contextOverlap->CO_ConB = olap_winner->COC_ShortContext;
        contextOverlap->CO_Contained = olap_winner->COC_Contained;
        contextOverlap->CO_Length = total_length;
        if(!flip_contender)
        {
            contextOverlap->CO_AStart = olap_winner->COC_StartLong;
            contextOverlap->CO_ANextStart = olap_winner->COC_NextStartLong;
            contextOverlap->CO_ANextEnd = olap_winner->COC_NextEndLong;
            contextOverlap->CO_AEnd = olap_winner->COC_EndLong;
            
            contextOverlap->CO_BStart = olap_winner->COC_StartShort;
            contextOverlap->CO_BNextStart = olap_winner->COC_NextStartShort;
            contextOverlap->CO_BNextEnd = olap_winner->COC_NextEndShort;
            contextOverlap->CO_BEnd = olap_winner->COC_EndShort;
        }
        else
        {
            contextOverlap->CO_AStart = olap_winner->COC_EndLong;
            contextOverlap->CO_ANextStart = olap_winner->COC_NextEndLong;
            contextOverlap->CO_ANextEnd = olap_winner->COC_NextStartLong;
            contextOverlap->CO_AEnd = olap_winner->COC_StartLong;
            
            contextOverlap->CO_BStart = olap_winner->COC_EndShort;
            contextOverlap->CO_BNextStart = olap_winner->COC_NextEndShort;
            contextOverlap->CO_BNextEnd = olap_winner->COC_NextStartShort;
            contextOverlap->CO_BEnd = olap_winner->COC_StartShort;
        }
        
        // merge the contexts
        logInfo("About to merge: " << contextOverlap->CO_ConA << " : " << getLongestContig(contextOverlap->CO_ConA) << " and " << contextOverlap->CO_ConB << " : " <<  getLongestContig(contextOverlap->CO_ConB) << ", Olap: " << olap_winner->COC_OlapLength << ", Wastage: " << total_waste<< ", Contained?: " <<  contextOverlap->CO_Contained << ", New Length: " << contextOverlap->CO_Length, 4);
        if(!mergeContexts(contextOverlap, &new_start_GID, &new_next_start_GID, &new_start_pos, &new_end_pos))
        {
            logError("Could not merge");
            total_length = 0;
        }
    }
    else
        total_length = 0;
    
    // REMOVE SHORTER CONTEXT AND SET RETURN VARS    
    if(0 != total_length)
    {
        // the shorter context has been removed from the context manager
        // delete the shorter merged contextId from all the main lists
        std::map<ContextId, GenericNodeId>::iterator all_iter = mAllContexts.begin();
        while(all_iter != mAllContexts.end())
        {
            if(all_iter->first == contextOverlap->CO_ConB)
            {
                mNodes->removeContextBlocks(all_iter->first);
                mAllContexts.erase(all_iter);
                break;
            }
            all_iter++;
        }
        
        // fix these guys up so the context remains valid
        setStartPos(new_start_pos, contextOverlap->CO_ConA);
        setEndPos(new_end_pos, contextOverlap->CO_ConA);
        setLongestContig(total_length, contextOverlap->CO_ConA);
        
        // these are so the calling function can get to these values (if needed)
        contextOverlap->CO_AStart = new_start_GID;
        contextOverlap->CO_ANextStart = new_next_start_GID;
    }
    
    // LOCAL CLEAN UP
    coc_iter = confirmed_contender_list.begin();
    while(coc_iter != confirmed_contender_list.end())
    {
        if(*coc_iter != NULL)
        {
            delete *coc_iter;
            *coc_iter = NULL;
        }
        coc_iter++;
    }
    
    return total_length;
}
//HO 

/*HV*/ bool
ContextMemWrapper::
/*HV*/ mergeContexts(C_OLAP * thisOlap, GenericNodeId * newStartGID, GenericNodeId * newNextStartGID, uMDInt * newStartPos, uMDInt * newEndPos)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // merge two contexts into one
    // We keep the masters of context A.
    // we always lose a few mappings in this process but
    // we gain longer or equal length contigs
    // 
    // This process destroys the shorter context
    //
    if(!isValidAddress(thisOlap->CO_ConA) || !isValidAddress(thisOlap->CO_ConB))
    {
        logError("Invalid address: " << thisOlap->CO_ConA << " : " << thisOlap->CO_ConB);
        return false;
    }
    
    // just some misc vars we need
    GenericNodeEdgeElem gnee(DUALNODE);
    
    // set ourselves up to walk along the context
    bool A_asc = (mNodes->getCntxPos(thisOlap->CO_AStart) < mNodes->getCntxPos(thisOlap->CO_AEnd));
    bool B_asc = (mNodes->getCntxPos(thisOlap->CO_BStart) < mNodes->getCntxPos(thisOlap->CO_BEnd));
    
    // is this going to force b to flip around? (be in reverse compliment?)
    bool b_rev_rev = false;
    
    // for fixing ends after we have merged
    GenericNodeId fix_master, next_fix_master;
    uMDInt fix_mas_pos = 0;
    
    // cut the wasted ends off from the context B if necessary
    if(1 != mNodes->getNtRank(DUALNODE, thisOlap->CO_BStart))
    {
        uMDInt b_end = mNodes->getCntxPos(thisOlap->CO_BEnd);
        if(mNodes->getEdges(&gnee, GN_EDGE_STATE_NTRANS, 0, thisOlap->CO_BStart))
        {
            do {
                // we want to cut away from the overlap
                if(mNodes->getOlapNode(gnee) != thisOlap->CO_BNextStart)
                {
                    // clean up this end of the context
                    detachBranchNoNpm(&(thisOlap->CO_BStart), mNodes->getOlapNode(gnee));
                    break;
                }
            } while(mNodes->getNextEdge(&gnee));
        }
        else { logError("No edges @: " << thisOlap->CO_BStart); return false; }
        
        // CO_BStart is ok, CO_BNextStart may be corrupted...
        // CO_BStart should (must) be a cap now!
        if(1 != mNodes->getNtRank(DUALNODE, thisOlap->CO_BStart))
        {
            logError("Short context does not end in cap after trimming");
            return false;
        }
        if(mNodes->getEdges(&gnee, GN_EDGE_STATE_NTRANS, 0, thisOlap->CO_BStart))
        {
            thisOlap->CO_BNextStart = mNodes->getOlapNode(gnee);
        }
        else { logError("No edges @: " << thisOlap->CO_BStart); logInfo(mNodes->isValidAddress(thisOlap->CO_BStart), 1); mNodes->printEdges(DUALNODE,  thisOlap->CO_BStart); return false; }
        
        // for short overlaps we may have even corrupted the end nodes
        GenericNodeWalkingElem gnwe_coor_fix(DUALNODE);
        if(mNodes->startWalk(&gnwe_coor_fix, thisOlap->CO_BStart, thisOlap->CO_BNextStart))
        {
            do {
                if(b_end == mNodes->getCntxPos(mNodes->getCurrentNode(gnwe_coor_fix)))
                {
                    thisOlap->CO_BNextEnd = mNodes->getPrevNode(gnwe_coor_fix);
                    thisOlap->CO_BEnd = mNodes->getCurrentNode(gnwe_coor_fix);
                    break;
                }
            } while(mNodes->rankStep(&gnwe_coor_fix));
        }
        else
        {
            logError("Could not walk from B");
            return false;
        }
    }
    
    // now we can start walking
    GenericNodeWalkingElem gnweA(DUALNODE);
    GenericNodeWalkingElem gnweB(DUALNODE);
    if(!(mNodes->startWalk(&gnweB, thisOlap->CO_BStart, thisOlap->CO_BNextStart)))
    {
        logError("No walk For: " << thisOlap->CO_BStart << " -> " << thisOlap->CO_BNextStart); return false;
    }
    if(mNodes->startWalk(&gnweA, thisOlap->CO_AStart, thisOlap->CO_ANextStart))
    {
        // we can work out if B is flipped by comparing context reversed flags for the same read
        b_rev_rev = (mNodes->isCntxReversed(mNodes->getPrevNode(gnweA)) ^ mNodes->isCntxReversed(mNodes->getPrevNode(gnweB)));
        
        do {
            // merge the prev nodes now
            fix_mas_pos = mNodes->getCntxPos(mNodes->getPrevNode(gnweA));
            mNodes->moveNode(mNodes->getPrevNode(gnweA), mNodes->getPrevNode(gnweB));
            if(mNodes->getCurrentNode(gnweA) == thisOlap->CO_AEnd) { break; }
            if(mNodes->getCurrentNode(gnweB) == thisOlap->CO_BEnd) { break; }
            if(!mNodes->rankStep(&gnweB)) { break; }
        } while (mNodes->rankStep(&gnweA));
        
        // now we're either at the end of A's overlap (or the end of A completely) or the end of B's overlap
        // in any case we should assume that mNodes->getCurrentNode(gnweA) == thisOlap->CO_AEnd
        // and mNodes->getCurrentNode(gnweB) == thisOlap->CO_BEnd
        
        if(thisOlap->CO_Contained)
        {
            // trim on B and done! (almost)
            if(mNodes->rankStep(&gnweB))
            {
                GenericNodeId prev_B = mNodes->getPrevNode(gnweB);
                PARANOID_ASSERT_PRINT_L2(mNodes->isValidAddress(prev_B), "PREV_B is invalid: " << prev_B);
                detachBranchNoNpm(&prev_B, mNodes->getCurrentNode(gnweB));
                // merge the last guy in B
                fix_master = mNodes->moveNode(mNodes->getCurrentNode(gnweA), prev_B);
            }
            else
            {
                fix_master = mNodes->moveNode(mNodes->getCurrentNode(gnweA), mNodes->getCurrentNode(gnweB));
            }
            GenericNodeEdgeElem gnee_fix(DUALNODE);
            if(mNodes->getEdges(&gnee_fix, GN_EDGE_STATE_NTRANS, 0, fix_master))
            {
                next_fix_master = mNodes->getOlapNode(gnee_fix);
            }
            else
            {
                logError("In contained: No edges for: " << fix_master);
            }
        }
        else
        {
            // trim on A and update the remainder of B
            
            // holders for the final overlapping nodes of A and B
            GenericNodeId final_A, final_B;
            final_B = mNodes->getCurrentNode(gnweB);
            if(mNodes->rankStep(&gnweA))
            {
                final_A = mNodes->getPrevNode(gnweA);
                detachBranchNoNpm(&final_A, mNodes->getCurrentNode(gnweA));
            }
            else
            {
                final_A = mNodes->getCurrentNode(gnweA);
            }
            
            // final_A is now a cap!
            if(1 != mNodes->getNtRank(DUALNODE, final_A))
            {
                logError("Final A is not a cap!");
                return false;
            }
            // position holders etc for translation
            uMDInt last_pos_A = mNodes->getCntxPos(final_A);
            
            uMDInt history_A = mNodes->getCntxHistory(final_A);
            ContextId CTXID_A = mNodes->getCntxId(final_A);
            uMDInt last_pos_B = 0;
            uMDInt pos_diff = 0;
            uMDInt new_pos = 0;
            
            bool keep_walking = false;
            
            if(mNodes->rankStep(&gnweB))
            {
                // step one away from the end of B's overlap, call this the update node
                GenericNodeId update_B = mNodes->getCurrentNode(gnweB);
                
                // save the "B" position of this node before we translate it
                last_pos_B = mNodes->getCntxPos(update_B);
                
                // get the offset from the end of B's overlap to the update node and keep it safe...
                if(B_asc) {
                    pos_diff = mNodes->getCntxPos(mNodes->getCurrentNode(gnweB)) - mNodes->getCntxPos(mNodes->getPrevNode(gnweB));
                }
                else {
                    pos_diff = mNodes->getCntxPos(mNodes->getPrevNode(gnweB)) - mNodes->getCntxPos(mNodes->getCurrentNode(gnweB));
                }
                // now work out the position of the update node in terms of A's context
                if(A_asc) {
                    new_pos = last_pos_A + pos_diff;
                }
                else {
                    new_pos = last_pos_A - pos_diff;
                }
                
                // move onto the next node before we munge update
                if(mNodes->rankStep(&gnweB))
                {
                    keep_walking = true;
                }
                
                // join the update node onto the end of A (ie. final_A)
                // be tricky and set the existing edge from update_B to final_B to NULL
                // this will make a free space for the new edge...
                mNodes->updateEdge_asymmetrical(final_B, GN_NULL_ID, update_B);
                // then add the edge proper
                mNodes->setCntxId(CTXID_A, update_B);
                if(!makeEdge(final_A, update_B)) { logError("Make edge returned false"); }
                // now translate B into A's world
                mNodes->translateNode(CTXID_A, new_pos, history_A, b_rev_rev, update_B);
            }
            // else final_B is really really final
            
            // move the final olap node of B onto A and work out a master pair for end fixing...
            fix_master = mNodes->moveNode(final_A, final_B);
            GenericNodeEdgeElem gnee_fix(DUALNODE);
            if(mNodes->getEdges(&gnee_fix, GN_EDGE_STATE_NTRANS, 0, fix_master))
            {
                do {
                    if(fix_mas_pos == mNodes->getCntxPos(mNodes->getOlapNode(gnee_fix)))
                    {
                        next_fix_master = mNodes->getOlapNode(gnee_fix);
                        break;
                    }
                } while(mNodes->getNextEdge(&gnee_fix));
            }
            else
            {
                logError("In non-contained: No edges for: " << fix_master);
            }
            
            // if there is more of B hanging off the end then we'll need to keep walking along and translating
            // if rank step failed just above then it should fail here again
            // otherwise update is locked into the prev position and the current is a yet untranslated node
            if(keep_walking)
            {
                do {
                    // the prev has been translated and the current is a yet untranslated node
                    if(B_asc) {
                        pos_diff = mNodes->getCntxPos(mNodes->getCurrentNode(gnweB)) - last_pos_B;
                    }
                    else {
                        pos_diff = last_pos_B - mNodes->getCntxPos(mNodes->getCurrentNode(gnweB));
                    }
                    if(A_asc) {
                        new_pos += pos_diff;
                    }
                    else {
                        new_pos -= pos_diff;
                    }
                    last_pos_B = mNodes->getCntxPos(mNodes->getCurrentNode(gnweB));
                    mNodes->translateNode(CTXID_A, new_pos, history_A, b_rev_rev, mNodes->getCurrentNode(gnweB));
                } while(mNodes->rankStep(&gnweB));
            }
        }
    }
    else
    {
        logError("No walk For: " << thisOlap->CO_AStart << " -> " << thisOlap->CO_ANextStart); return false;
    }
    
    // context B has now been merged
    deleteContextId(thisOlap->CO_ConB);
    
    // now we need to update the end values. Assume fix_master and next_fix_master are OK.
    GenericNodeWalkingElem gnwe_fix(DUALNODE);
    
    if(mNodes->startWalk(&gnwe_fix, fix_master, next_fix_master))
    {
        // walk to the end...
        while(mNodes->rankStep(&gnwe_fix));
        *newStartGID = mNodes->getCurrentNode(gnwe_fix);
        *newNextStartGID = mNodes->getPrevNode(gnwe_fix);
        *newStartPos = mNodes->getCntxPos(*newStartGID);
        
        // walk back to grab the end position
        if(mNodes->startWalk(&gnwe_fix, *newStartGID, *newNextStartGID))
        {
            while(mNodes->rankStep(&gnwe_fix));
            *newEndPos = mNodes->getCntxPos(mNodes->getCurrentNode(gnwe_fix));
        }
        else
        {
            logError("Could not start return walk from: " << newStartGID << " : " << newNextStartGID);
            return false;
        }
    }
    else
    {
        logError("Could not start walk from: " << fix_master << " : " << next_fix_master);
        return false;
    }
    return true;
}
//HO 

/*HV*/ bool
ContextMemWrapper::
/*HV*/ doesPairedReadSpanOlap(GenericNodeId GID, uMDInt startA, uMDInt endB, uMDInt olapLength, bool find_gt, uMDInt span_limit, bool A_olapDsc, bool B_olapDsc, ContextId CTX_A, ContextId CTX_B)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // Given a read 'GID', we would like to see if there exists a paired read in the dataset which
    // spans the overlap. This is required for overlaperation to occur when the overlap
    // is too small and other conditions have not been met.
    // 
    // We are given contexts A and B. We always assume that the overlap starts in A and moves into B
    //
    
    // the reads we'll be inspecting will not be double mapped
    // so we need to get the reversed information asap
    
    GenericNodeElem gne_spo, gne_pair_sp;
    if(mNodes->getElem(&gne_spo, GID))
    {
        // gne_spo holds a head node
        while(mNodes->getNextElem(&gne_spo))
        {
            // now gne_spo holds a non-head.
            // mkae sure this guy is not a dummy
            GenericNodeId sp_GID = gne_spo.GNE_GID;
            if(!mNodes->isDummy(sp_GID))
            {
                // not a dummy, so get the paired read and check out all the guys in this ring
                GenericNodeId pair_sp_GID = mNodes->getPair(sp_GID);
                if(mNodes->getElem(&gne_pair_sp, pair_sp_GID))
                {
                    while(mNodes->getNextElem(&gne_pair_sp))
                    {
                        GenericNodeId ring_pair_GID = gne_pair_sp.GNE_GID;
                        // now we see if there is a member of this ring in the overlapping context
                        if(CTX_B == mNodes->getCntxId(ring_pair_GID))
                        {
                            // this guy is in the context B
                            if(find_gt ^ (mNodes->getCntxPos(ring_pair_GID) <= span_limit))
                            {
                                // NOW, this guy lies with one read in either context AND
                                // the reads span the overlap. We just need to check the orientation
                                // and distance and then we can be sure that this is meant to be...
                                
                                // finally we need to know the distance between the two nodes
                                // this needs to take into account the positions of the reads relative to the overlap
                                uMDInt front_length, end_length;
                                if(mNodes->getCntxPos(GID) > startA)
                                    front_length = mNodes->getCntxPos(GID) - startA;
                                else
                                    front_length = startA - mNodes->getCntxPos(GID);
                                if(mNodes->getCntxPos(ring_pair_GID) > endB)
                                    end_length = mNodes->getCntxPos(ring_pair_GID) - endB;
                                else
                                    end_length = endB - mNodes->getCntxPos(ring_pair_GID);
                                
                                // use these two for the call to isMappable
                                uMDInt tdist = front_length + (uMDInt)fAbs(olapLength) + end_length - mStatsManager->getReadLength();
                                InsertLibrary::OrientationType ot = InsertLibrary::TYPE_UNSET;
                                
                                if( (mNodes->isCntxReversed(GID) ^ mNodes->isFileReversed(sp_GID)) ^ A_olapDsc)
                                {
                                    // A faces out
                                    if ( B_olapDsc ^ (mNodes->isCntxReversed(ring_pair_GID) ^ mNodes->isFileReversed(pair_sp_GID)) )
                                    {
                                        // B faces in
                                        ot = InsertLibrary::TYPE_AA;
                                        
                                    }
                                    else
                                    {
                                        // B faces out
                                        ot = InsertLibrary::TYPE_DD;
                                    }
                                }
                                else
                                {
                                    // A faces in
                                    if ( B_olapDsc ^ (mNodes->isCntxReversed(ring_pair_GID) ^ mNodes->isFileReversed(pair_sp_GID)) )
                                    {
                                        // B faces in
                                        ot = InsertLibrary::TYPE_AD;
                                    }
                                    else
                                    {
                                        // B faces out
                                        ot = InsertLibrary::TYPE_AA;
                                    }
                                }
                                
                                if(mStatsManager->isMappable(GID, ot, tdist))
                                {
                                    return true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return false;
}
//HO 

/*HV*/ void
ContextMemWrapper::
/*HV*/ remapPos(uMDInt * currPos, uMDInt * lastPos, GenericNodeId GID, ContextId CTXID)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // remap the context position of this guy and his slaves...
    //
    GenericNodeEdgeElem gnee(DUALNODE);
    if(0 != *lastPos)
    {
        *currPos += (mNodes->getCntxPos(GID) - *lastPos);
    }
    *lastPos = mNodes->getCntxPos(GID);
    mNodes->setCntxPos(*currPos, GID);
    if(mNodes->getEdges(&gnee, GN_EDGE_STATE_TRANS, 0, GID))
    {
        do {
            mNodes->setCntxPos(*currPos, mNodes->getOlapNode(gnee));
        } while(mNodes->getNextEdge(&gnee));
    }
}
//HO 

//******************************************************************************
// NODE MANAGEMENT
//******************************************************************************

/*HV*/ void
ContextMemWrapper::
/*HV*/ makeMaster(uMDInt position, bool reversed, GenericNodeId GID, HistoryId HID, ContextId CTXID)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // Make a generic node a master
    //
    // add the guy to the npm for this context
    NodePositionMapClass * npm = mMasterNPMs[CTXID];
    npm->addElem(GID, position, reversed);
    
    // set the node specific flags
    mNodes->setCntxMaster(true, GID);
    mNodes->setCntxId(CTXID, GID);
    mNodes->setCntxPos(position, GID);
    mNodes->setCntxReversed(reversed, GID);
    mNodes->setCntxHistory(HID, GID);
    mNodes->setCntxReadDepth(1, GID);
    
    // add it to the list of nodes for the context
    mNodes->addMasterToContext(GID);
}
//HO 

/*HV*/ void
ContextMemWrapper::
/*HV*/ makeMasterNoNpm(uMDInt position, bool reversed, GenericNodeId GID, HistoryId HID, ContextId CTXID)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // Make a generic node a master
    //
    // set the node specific flags
    mNodes->setCntxMaster(true, GID);
    mNodes->setCntxId(CTXID, GID);
    mNodes->setCntxPos(position, GID);
    mNodes->setCntxReversed(reversed, GID);
    mNodes->setCntxHistory(HID, GID);
    mNodes->setCntxReadDepth(1, GID);
    
    // add it to the list of nodes for the context
    mNodes->addMasterToContext(GID);
}
//HO 

/*HV*/ bool
ContextMemWrapper::
/*HV*/ makeSlave(GenericNodeId slaveGID, GenericNodeId masterGID)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // Set one GID as a slave to another
    //
    PARANOID_ASSERT_PRINT_L2((mNodes->isValidAddress(masterGID)), masterGID);
    PARANOID_ASSERT_PRINT_L2((mNodes->isCntxMaster(masterGID)), masterGID);
    PARANOID_ASSERT_L3((mNodes->getCntxId(masterGID) != getNullContextId()));
    PARANOID_ASSERT_L3(!(mNodes->isCntxMaster(slaveGID)));
    
    if(!(mNodes->isDummy(masterGID)))
    {
        // set the node specific flags etc...
        mNodes->setCntxId(mNodes->getCntxId(masterGID), slaveGID);
        mNodes->setCntxPos(mNodes->getCntxPos(masterGID), slaveGID);
        mNodes->setCntxReversed(mNodes->isCntxReversed(masterGID), slaveGID);
        mNodes->setCntxHistory(mNodes->getCntxHistory(masterGID), slaveGID);
        
        // make the slave edge
        if(mNodes->makeSlaveEdge_symmetrical(slaveGID, masterGID))
        {
            mNodes->curateSlaveState(slaveGID);
            mNodes->incCntxReadDepth(masterGID);
            return true;
        }
        return false;
    }
    else
    {
        return assimilateDummy(masterGID, slaveGID);
    }
}
//HO 

//******************************************************************************
// DUMMY NODE HANDLING
//******************************************************************************

/*HV*/ GenericNodeId
ContextMemWrapper::
/*HV*/ createNewDummy(uMDInt position, bool reversed, GenericNodeId GID, HistoryId HID, ContextId CTXID)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // create a new dummy node for the context
    //
    GenericNodeId new_dummy = mNodes->addDummyNode(GID);
    makeMaster(position, reversed, new_dummy, HID, CTXID);
    return new_dummy;
}
//HO 

/*HV*/ GenericNodeId
ContextMemWrapper::
/*HV*/ createNewDummyNoNpm(uMDInt position, bool reversed, GenericNodeId GID, HistoryId HID, ContextId CTXID)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // create a new dummy node for the context
    //
    GenericNodeId new_dummy = mNodes->addDummyNode(GID);
    makeMasterNoNpm(position, reversed, new_dummy, HID, CTXID);
    return new_dummy;
}
//HO 

/*HV*/ void
ContextMemWrapper::
/*HV*/ deleteDummy(GenericNodeId dummy)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // delete a dummy node from the nodeset
    //
    PARANOID_ASSERT_PRINT_L2(mNodes->isValidAddress(dummy), "Invalid dummy: " << dummy);
    // remove the dummy from the npm
    NodePositionMapClass * npm = mMasterNPMs[mNodes->getCntxId(dummy)];
    
    npm->deleteElem(dummy);
    
    // delete it from the generic node lists...
    mNodes->deleteDummy(dummy);
    
}
//HO 

/*HV*/ void
ContextMemWrapper::
/*HV*/ deleteDummyNoNpm(GenericNodeId dummy)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // delete a dummy node from the nodeset
    //
    PARANOID_ASSERT_PRINT_L2(mNodes->isValidAddress(dummy), "Invalid dummy: " << dummy);
    // delete it from the generic node lists...
    mNodes->deleteDummy(dummy);
}
//HO 

/*HV*/ void
ContextMemWrapper::
/*HV*/ deleteDummy(GenericNodeId dummy, ContextId CTXID)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // delete a dummy node from the nodeset
    //
    // remove the dummy from the npm
    PARANOID_ASSERT_PRINT_L2(mNodes->isValidAddress(dummy), "Invalid dummy: " << dummy);
    PARANOID_ASSERT_PRINT_L2(isValidAddress(CTXID), "Invalid Context: " << dummy << " : " << CTXID);
    NodePositionMapClass * npm = mMasterNPMs[CTXID];
    
    npm->deleteElem(dummy);
    // delete it from the generic node lists...
    mNodes->deleteDummy(dummy);
    
}
//HO 

/*HV*/ bool
ContextMemWrapper::
/*HV*/ assimilateDummy(GenericNodeId dummy, GenericNodeId newId)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // We need to replace all references to the dummy with the newId which will
    // assume master at the position
    //
    
    NodePositionMapClass * npm = mMasterNPMs[mNodes->getCntxId(dummy)];
    npm->updateElem(dummy, newId);
    if(mNodes->assimilateDummy(dummy, newId))
    {
        mNodes->curateMasterState(newId);
        return true;
    }
    return false;
}
//HO 

//******************************************************************************
// EDGE MANAGEMENT
//******************************************************************************

//HO inline bool makeEdge(GenericNodeId GID_A, GenericNodeId GID_B) { return makeEdge(GID_A, GID_B, false); }
//HO 

/*HV*/ bool
ContextMemWrapper::
/*HV*/ makeEdge(GenericNodeId GID_A, GenericNodeId GID_B, bool suppressError)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // make a master - master edge beween these two generic nodes
    //
    PARANOID_ASSERT_L3((mNodes->isCntxMaster(GID_A)));
    PARANOID_ASSERT_L3((mNodes->isCntxMaster(GID_B)));
    PARANOID_ASSERT_L3((mNodes->getCntxId(GID_A) == mNodes->getCntxId(GID_B)));
    if(mNodes->makeMasterMasterEdge_symmetrical(GID_A, GID_B, suppressError))
    {
        mNodes->curateMasterState(GID_A);
        mNodes->curateMasterState(GID_B);
        return true;
    }
    return false;
}
//HO 

static std::vector<GenericNodeId> CMWG_collateral_nodes;
static std::vector<GenericNodeId> CMWG_pair_nodes;
static std::vector<GenericNodeId>::iterator CMWG_coll_iter;
//HO inline void safeDetachNode(GenericNodeId * startBranch, GenericNodeId dead_GID) { safeDetachNode(startBranch, NULL, dead_GID); }
//HO 

/*HV*/ void
ContextMemWrapper::
/*HV*/ safeDetachNode(GenericNodeId * startBranch, GenericNodeWalkingElem * active_GNWE, GenericNodeId dead_GID)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // Remove a node from the context and make sure all NPMs are left ok
    // insert new dummies as needed
    //
    ContextId CTXID = mNodes->getCntxId(dead_GID);
    NodePositionMapClass * npm = mMasterNPMs[CTXID];
    
    // if we are calling this from within detachBranch then we need to make sure that
    // we do not contaminate the prev or current nodes of that object

    NodeCheckId prev_token = NC_NULL_ID;
    NodeCheckId current_token = NC_NULL_ID;
    
    bool start_is_null = false;
    if(*startBranch == GN_NULL_ID)
        start_is_null = true;
    NodeCheckId start_token = mNodeCheck->checkInNode(*startBranch);

    if(NULL != active_GNWE)
    {
        prev_token = mNodeCheck->checkInNode(mNodes->getPrevNode(*active_GNWE));
        current_token = mNodeCheck->checkInNode(mNodes->getCurrentNode(*active_GNWE));
    }

    // we may have to detach a whole heap more nodes
    CMWG_collateral_nodes.clear();
    CMWG_pair_nodes.clear();

    // need to remove this master and all slaves attached too
    if(mNodes->isCntxMaster(dead_GID))
    {
        // we need to get all the slaves together
        GenericNodeEdgeElem gnee(DUALNODE);
        if(mNodes->getEdges(&gnee, GN_EDGE_STATE_TRANS, 0, dead_GID))
        {
            do {
                // push back the slave and its pair
                GenericNodeId olap_node = mNodes->getOlapNode(gnee);
                CMWG_collateral_nodes.push_back(olap_node);
                CMWG_pair_nodes.push_back(mNodes->getPair(olap_node));
            } while(mNodes->getNextEdge(&gnee));
        }
    }

    if(mNodes->isDummy(dead_GID))
    {
        ContextId tmp_CTXID = mNodes->getCntxId(dead_GID);
        // detach the node and clean up
        mNodes->detachNode(DUALNODE, dead_GID);
        // just delete the dummy now
        // it will be taken out of the lists automatically
        deleteDummy(dead_GID, tmp_CTXID);

    }
    else
    {
        // update the npm here
        if(mNodes->isCntxMaster(dead_GID))
            npm->deleteElem(dead_GID);

        // detach the node and clean up
        // it will be taken out of the lists automatically
        mNodes->detachNode(DUALNODE, dead_GID);
        
        // non-dummies have pairs
        CMWG_pair_nodes.push_back(mNodes->getPair(dead_GID));
    }

    // now we can do all the collateral nodes
    CMWG_coll_iter = CMWG_collateral_nodes.begin();
    while(CMWG_coll_iter != CMWG_collateral_nodes.end())
    {
        // it's just a slave, we only need to detach it and that's all
        mNodes->resetDualNode(*CMWG_coll_iter);
        CMWG_coll_iter++;
    }
    // now we do all the pairs
    CMWG_coll_iter = CMWG_pair_nodes.begin();
    while(CMWG_coll_iter != CMWG_pair_nodes.end())
    {
        if(mNodes->isCntxMaster(*CMWG_coll_iter))
        {
            // this is a bit trickier...
            // Do we have any slaves?
            bool need_swap_start = false;
            if(*CMWG_coll_iter == *startBranch)
            {
                need_swap_start = true;
            }
            
            GenericNodeId replacement_master;
            bool has_slaves = false;
            GenericNodeEdgeElem gnee(DUALNODE);
            if(mNodes->getEdges(&gnee, GN_EDGE_STATE_TRANS, 0, *CMWG_coll_iter))
            {
                // you'll do!
                has_slaves = true;
                replacement_master = mNodes->getOlapNode(gnee);
                // wipe all the edges (assymetrical)
                mNodes->resetDualNode(replacement_master);
                PARANOID_ASSERT_L2(mNodes->isValidAddress(replacement_master));
                mNodes->transferEdges(*CMWG_coll_iter, replacement_master);
                
                // fix all the Context flags
                mNodes->setCntxMaster(true, replacement_master);
                mNodes->setCntxId(mNodes->getCntxId(*CMWG_coll_iter), replacement_master);
                mNodes->setCntxPos(mNodes->getCntxPos(*CMWG_coll_iter), replacement_master);
                mNodes->setCntxReversed(mNodes->isCntxReversed(*CMWG_coll_iter), replacement_master);
                mNodes->setCntxHistory(mNodes->getCntxHistory(*CMWG_coll_iter), replacement_master);
                mNodes->setCntxReadDepth(mNodes->getCntxReadDepth(*CMWG_coll_iter) - 1, replacement_master);
                
                // update the NPM
                npm->updateElem(*CMWG_coll_iter, replacement_master);
            }
            if(!has_slaves)
            {
                // we'll need to put a dummy here in it's place
                replacement_master = createNewDummy(mNodes->getCntxPos(*CMWG_coll_iter), mNodes->isCntxReversed(*CMWG_coll_iter), *CMWG_coll_iter, mNodes->getCntxHistory(*CMWG_coll_iter), mNodes->getCntxId(*CMWG_coll_iter));
                PARANOID_ASSERT_L2(mNodes->isValidAddress(replacement_master));
                
                mNodes->transferEdges(*CMWG_coll_iter, replacement_master);
                
                // the npm has the replacement master in position because of the call to 
                // makeMaster within createNewDummy. We just need to delete the old reference
                npm->deleteElem(*CMWG_coll_iter);
            }
            
            // fix all the lists for the replacement master
            mNodes->curateMasterState(replacement_master);
            
            // we'll promote to master and put it in it's place
            // first transfer all the edges across
            if(need_swap_start)
            {
                *startBranch = replacement_master;
            }
            
            // finally we can reset this node
            // the lists will be updated within the call
            mNodes->resetDualNode(*CMWG_coll_iter);

        }
        else
        {
            // this node is just a slave so we can detach it easily
            // however we need to make sure that we find the master
            // and decrement the read depth here
            GenericNodeEdgeElem gnee_mas(DUALNODE);
            if(mNodes->getEdges(&gnee_mas, GN_EDGE_STATE_TRANS, 0, *CMWG_coll_iter))
            {
                if(mNodes->isValidAddress(mNodes->getOlapNode(gnee_mas)))
                {
                    mNodes->decCntxReadDepth(mNodes->getOlapNode(gnee_mas));
                }
            }
            else
            {
                logError("No master at slaveGID: " << *CMWG_coll_iter);
            }

            mNodes->detachNode(DUALNODE, *CMWG_coll_iter);

        }
        CMWG_coll_iter++;
    }

    // if we have corrupted the given walking elem then now is the time to fix it
    if(NULL != active_GNWE)
    {
        GenericNodeId tmp_prev = mNodeCheck->checkOutNode(true, CTXID, prev_token); 
        GenericNodeId tmp_current = mNodeCheck->checkOutNode(true, CTXID, current_token); 
        mNodes->startWalk(active_GNWE, tmp_prev, tmp_current);
    }
    
    if(!start_is_null)
    { *startBranch = mNodeCheck->checkOutNode(true, CTXID, start_token); }
}
//HO

/*HV*/ void
ContextMemWrapper::
/*HV*/ safeDetachNodeNoNpm(GenericNodeId * startBranch, GenericNodeId dead_GID)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // Remove a node from the context insert new dummies as needed
    //
    ContextId CTXID = mNodes->getCntxId(dead_GID);
    
    // we may have to detach a whole heap more nodes
    CMWG_collateral_nodes.clear();
    CMWG_pair_nodes.clear();
    
    // need to remove this master and all slaves attached too
    if(mNodes->isCntxMaster(dead_GID))
    {
        // we need to get all the slaves together
        GenericNodeEdgeElem gnee(DUALNODE);
        if(mNodes->getEdges(&gnee, GN_EDGE_STATE_TRANS, 0, dead_GID))
        {
            do {
                // push back the slave and its pair
                CMWG_collateral_nodes.push_back(mNodes->getOlapNode(gnee));
                CMWG_pair_nodes.push_back(mNodes->getPair(mNodes->getOlapNode(gnee)));
            } while(mNodes->getNextEdge(&gnee));
        }
    }
    
    if(mNodes->isDummy(dead_GID))
    {
        // detach the node and clean up
        mNodes->detachNode(DUALNODE, dead_GID);
        // just delete the dummy now
        deleteDummyNoNpm(dead_GID);
    }
    else
    {
        // detach the node and clean up
        mNodes->detachNode(DUALNODE, dead_GID);
        // non-dummies have pairs
        CMWG_pair_nodes.push_back(mNodes->getPair(dead_GID));
    }
    
    // now we can do all the collateral nodes
    CMWG_coll_iter = CMWG_collateral_nodes.begin();
    while(CMWG_coll_iter != CMWG_collateral_nodes.end())
    {
        // it's just a slave, we only need to detach it and that's all
        mNodes->resetDualNode(*CMWG_coll_iter);
        CMWG_coll_iter++;
    }
    
    // now we do all the pairs
    CMWG_coll_iter = CMWG_pair_nodes.begin();
    while(CMWG_coll_iter != CMWG_pair_nodes.end())
    {
        if(mNodes->isCntxMaster(*CMWG_coll_iter))
        {
            // this is a bit trickier...
            // Do we have any slaves?
            bool need_swap_start = false;
            if(*CMWG_coll_iter == *startBranch)
            {
                need_swap_start = true;
            }
            
            GenericNodeId replacement_master;
            bool has_slaves = false;
            GenericNodeEdgeElem gnee(DUALNODE);
            if(mNodes->getEdges(&gnee, GN_EDGE_STATE_TRANS, 0, *CMWG_coll_iter))
            {
                // you'll do!
                has_slaves = true;
                replacement_master = mNodes->getOlapNode(gnee);
                // wipe all the edges (assymetrical)
                mNodes->resetDualNode(replacement_master);
                PARANOID_ASSERT_L2(mNodes->isValidAddress(replacement_master));
                mNodes->transferEdges(*CMWG_coll_iter, replacement_master);
                
                // fix all the Context flags
                mNodes->setCntxMaster(true, replacement_master);
                mNodes->setCntxId(mNodes->getCntxId(*CMWG_coll_iter), replacement_master);
                mNodes->setCntxPos(mNodes->getCntxPos(*CMWG_coll_iter), replacement_master);
                mNodes->setCntxReversed(mNodes->isCntxReversed(*CMWG_coll_iter), replacement_master);
                mNodes->setCntxHistory(mNodes->getCntxHistory(*CMWG_coll_iter), replacement_master);
                mNodes->setCntxReadDepth(mNodes->getCntxReadDepth(*CMWG_coll_iter) - 1, replacement_master);
            }
            if(!has_slaves)
            {
                // we'll need to put a dummy here in it's place
                replacement_master = createNewDummyNoNpm(mNodes->getCntxPos(*CMWG_coll_iter), mNodes->isCntxReversed(*CMWG_coll_iter), *CMWG_coll_iter, mNodes->getCntxHistory(*CMWG_coll_iter), mNodes->getCntxId(*CMWG_coll_iter));
                PARANOID_ASSERT_L2(mNodes->isValidAddress(replacement_master));
                mNodes->transferEdges(*CMWG_coll_iter, replacement_master);
            }
            
            // fix all the lists for the replacement master
            mNodes->curateMasterState(replacement_master);
            
            // we'll promote to master and put it in it's place
            // first transfer all the edges across
            if(need_swap_start)
                *startBranch = replacement_master;
            
            // finally we can reset this node
            mNodes->resetDualNode(*CMWG_coll_iter);
        }
        else
        {
            GenericNodeEdgeElem gnee_mas(DUALNODE);
            if(mNodes->getEdges(&gnee_mas, GN_EDGE_STATE_TRANS, 0, *CMWG_coll_iter))
            {
                if(mNodes->isValidAddress(mNodes->getOlapNode(gnee_mas)))
                {
                    mNodes->decCntxReadDepth(mNodes->getOlapNode(gnee_mas));
                }
            }
            else
                logError("No master at slaveGID: " << *CMWG_coll_iter);
            
            mNodes->detachNode(DUALNODE, *CMWG_coll_iter);
        }
        CMWG_coll_iter++;
    }
}
//HO

/*HV*/ void
ContextMemWrapper::
/*HV*/ detachBranch(GenericNodeId * startBranch, GenericNodeId nextStartBranch)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // Detach a whole branch from the tree...
    // does not detach the start of the branch
    //
    // startBranch is a pointer to the node at the start of the branch
    // This is needed because this node may be used in a loop and may
    // not be easily replaced outside if it gets deleted...
    //
    // The philosiphy here is that we walk along a branch until we come to a cross or a cap
    // and we detach the nodes along the way. When we get to a cross we can view this as
    // a new branch and call this function recursively to detach that branch.
    
    // use this guy to walk along the branches
    GenericNodeWalkingElem gnwe(DUALNODE);
    
    // we walk so we use current and previous node variables.
    // if we detach these then we can get into trouble so we need to delay
    GenericNodeId detach_delay_1 = GN_NULL_ID;
    GenericNodeId detach_delay_2 = GN_NULL_ID;
    ContextId CTXID = mNodes->getCntxId(*startBranch);
    NodeCheckId start_token = mNodeCheck->checkInNode(*startBranch);
    
    // use these to measure delays
    int round_number = 0;
    bool branch_longer_than_one = false;
    
    if(mNodes->startWalk(&gnwe, *startBranch, nextStartBranch))
    {
        do {                                                                                                                              
            if(branch_longer_than_one)
            {
                // the walking element and the startBranch are
                // both maintained in the call to safe detach node
                safeDetachNode(startBranch, &gnwe, detach_delay_2);
            }
            else if(round_number == 1)
            {
                // second_round
                branch_longer_than_one = true;
            }
            round_number++;
            
            // in round 0 dd2 = NULL and dd1 = nextStartBranch
            // in round 1 dd2 = nextStartBranch and dd1 = the node after this
            // after this we can detach dd2 note that dd1 does not get detached here...
            detach_delay_2 = mNodes->getPrevNode(gnwe);
            detach_delay_1 = mNodes->getCurrentNode(gnwe);
            
        } while(mNodes->rankStep(&gnwe));
        
        // detach_delay_1 points to a cap or cross, detach_delay_2 points to the guy just before that
        NodeCheckId dd1_token = mNodeCheck->checkInNode(detach_delay_1);
        NodeCheckId dd2_token = mNodeCheck->checkInNode(detach_delay_2);
        
        if(mNodes->getNtRank(DUALNODE, detach_delay_1) > 2)
        {
            // this is a cross. we'll need to detach the branches below
            // just need to check that we actually have more than a double cross going on
            if(branch_longer_than_one)
            {
                sMDInt off_wanted = -1 * mNodes->getReturnOffset(gnwe);
                GenericNodeEdgeElem gnee(DUALNODE);
                if(mNodes->getEdges(&gnee, GN_EDGE_STATE_NTRANS, off_wanted, detach_delay_1))
                {
                    do {
                        detachBranch(&detach_delay_1, mNodes->getOlapNode(gnee));
                    } while(mNodes->getNextEdge(&gnee));
                }
            }
            else
            {
                // we never detached anything.
                // this is a cross linked to a cross
                sMDInt off_wanted = -1 * mNodes->getNtOffset(DUALNODE, *startBranch, nextStartBranch);
                GenericNodeEdgeElem gnee(DUALNODE);
                if(mNodes->getEdges(&gnee, GN_EDGE_STATE_NTRANS, off_wanted, nextStartBranch))
                {
                    do {
                        detachBranch(&detach_delay_1, mNodes->getOlapNode(gnee));
                    } while(mNodes->getNextEdge(&gnee));
                }
            }
        }
        
        // now it's safe to detach these guys ... maybe
        detach_delay_1 = mNodeCheck->checkOutNode(true, CTXID, dd1_token); 
        detach_delay_2 = mNodeCheck->checkOutNode(true, CTXID, dd2_token); 
        
        if(branch_longer_than_one)
        {
            // for very small insert sizes is may be possible that these two
            // guys are actually pairs. watch out!
            if(mNodes->getPair(detach_delay_2) != detach_delay_1)
                safeDetachNode(startBranch, detach_delay_2);
        }
        // if the above was a pair then we will get it here anyways...
        safeDetachNode(startBranch, detach_delay_1);
    }
    else
    {
        logError("Could not detach branch: " << *startBranch << " : " << nextStartBranch);
    }
    
    // this node has had a node removed and must be updated
    *startBranch = mNodeCheck->checkOutNode(true, CTXID, start_token); 
    mNodes->curateMasterState(*startBranch);
}
//HO 

/*HV*/ void
ContextMemWrapper::
/*HV*/ detachBranchNoNpm(GenericNodeId * startBranch, GenericNodeId nextStartBranch)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // Detach a whole branch from the tree...
    // does not detach the start of the branch
    //
    GenericNodeWalkingElem gnwe(DUALNODE);
    GenericNodeId detach_delay_1, detach_delay_2;
    int round_number = 0;
    bool branch_longer_than_one = false;
    if(mNodes->startWalk(&gnwe, *startBranch, nextStartBranch))
    {
        do {
            if(branch_longer_than_one)
            {
                // third round +
                
                safeDetachNodeNoNpm(startBranch, detach_delay_2);
            }
            else if(round_number == 1)
            {
                // second_round only
                branch_longer_than_one = true;
            }
            else
                round_number++;
            detach_delay_2 = detach_delay_1;
            detach_delay_1 = mNodes->getCurrentNode(gnwe);
        } while(mNodes->rankStep(&gnwe));
        
        // now it's safe to detach these guys ... maybe
        if(branch_longer_than_one)
        {
            safeDetachNodeNoNpm(startBranch, detach_delay_2);
        }
        
        safeDetachNodeNoNpm(startBranch, detach_delay_1);
    }
    else
    {
        logError("Could not detach branch: " << startBranch << " : " << nextStartBranch);
    }
    
    // this node has had a node removed and must be updated
    mNodes->curateMasterState(*startBranch);
}
//HO 

//******************************************************************************
// UTILITIES
//******************************************************************************

/*HV*/ void
ContextMemWrapper::
/*HV*/ readNaiveCoverageMap(double cov_cut, std::string covFileName)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // Read in the naive coverage maps
    //
    // reading files
    logInfo("Calculating stretching cut offs", 4 );
    mCovCutPercent = cov_cut;
    ifstream cov_file(covFileName.c_str(), ios::in);
    if (cov_file.is_open()) {
        char current_line[100];
        
        // for getting the info to store in the map
        double coverage;
        uMDInt address;
        ContigId current_CID;
        
        ContigClass * tmp_contigs = new ContigClass();
        PARANOID_ASSERT_L2(mContigs != NULL);
        tmp_contigs->initialise(2);
        
        double mean_coverage = 0;
        double limit_count = 10;
        double done_count = 0;
        
        bool isID = true;                                       
        while (!cov_file.eof()) {
            cov_file >> current_line;
            if(isID) { from_string<uMDInt>(address, current_line, std::hex); isID = false; }
            else { 
                from_string<double>(coverage, current_line, std::dec); isID = true; 
                current_CID = tmp_contigs->getContigId(address);
                mCoverageMap.insert(std::pair<ContigId, double>(current_CID, coverage));
                if(done_count < limit_count)
                {
                    mean_coverage += coverage;
                    done_count++;
                }
            }
        }       
        
        mean_coverage = mean_coverage / limit_count;
        mUpperStretchCoverageCutoff = mean_coverage * (1 + cov_cut);
        mLowerStretchCoverageCutoff = mean_coverage * (1 - cov_cut);
        
        logInfo( "MEAN COVERAGE: " << mean_coverage << " UPPER: " << mUpperStretchCoverageCutoff << " LOWER: " << mLowerStretchCoverageCutoff, 4);
        
        delete tmp_contigs;
    }
    else
        logError("Could not open naive coverage file: " << covFileName);
    
}
//HO 

/*HV*/ bool
ContextMemWrapper::
/*HV*/ validateContext(ContextId CTXID)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // Similar to the validate graph calls for generic and uninodes. But restricted to the nodes
    // within a particulare context
    //
    logInfo("Validating Context: " << CTXID, 2);
    return mNodes->validateDualNodes(CTXID);
}
//HO 

/*HV*/ bool
ContextMemWrapper::
/*HV*/ deleteContext(ContextId CTXID)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // delete the context and clean up.
    //
    // fix all the pointers for GenericNodes
    mNodes->resetDNEdges(true, CTXID);
    mNodes->removeContextBlocks(CTXID);
    
    // delete the context
    return deleteContextId(CTXID);
}
//HO 

/*HV*/ bool
ContextMemWrapper::
/*HV*/ clearNPMs(ContextId CTXID)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // delete any node position maps
    //
    contextNPMLinkerIterator finder = mHeadNPMs.find(CTXID);
    if(finder != mHeadNPMs.end())
    {
        if(finder->second != NULL)
        {
            delete finder->second;
        }
        mHeadNPMs.erase(finder);
    }
    else
    {
        logError("Cannot find a head NPM for: " << CTXID);
        return false;
    }
    
    finder = mMasterNPMs.find(CTXID);
    if(finder != mMasterNPMs.end())
    {
        if(finder->second != NULL)
        {
            delete finder->second;
        }
        mMasterNPMs.erase(finder);
    }
    else
    {
        logError("Cannot find a master NPM for: " << CTXID);
        return false;
    }
    return true;
}
//HO 

//******************************************************************************
// NODECHECK SPECIFIC
//******************************************************************************

/*HV*/ void
ContextMemWrapper::
/*HV*/ updateShoot(contextShoot * theShoot, sMDInt dirn, GenericNodeId GID, ContextId CTXID)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // update the values of a given shoot
    // interfaces with the check in and check out code
    //
     mShootCheck->checkOutNode(true, CTXID, theShoot->first);
    theShoot->first = mShootCheck->checkInNode(GID);
    theShoot->second = dirn;
}
//HO 

/*HV*/ contextShoot
ContextMemWrapper::
/*HV*/ deepCopyShoot(contextShoot * theShoot, ContextId CTXID)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // update the values of a given shoot
    // interfaces with the check in and check out code
    //
    contextShoot ret_shoot;
    ret_shoot.first = mShootCheck->checkInNode(mShootCheck->checkOutNode(false, CTXID,theShoot->first));
    ret_shoot.second = theShoot->second;
    return ret_shoot;
}
//HO 

//HO inline void clearShootTokens() { clearTokens(true); }
//HO 
//HO inline void clearNodeTokens() { clearTokens(false); }
//HO 

/*HV*/ void
ContextMemWrapper::
/*HV*/ clearTokens(bool isShoot)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // clear the NodeCheck of all old values...
    //
    if(isShoot)
    {
        if(mShootCheck != NULL)
            delete mShootCheck;
        mShootCheck = new NodeCheckClass();
        PARANOID_ASSERT_L4(mShootCheck != NULL);

        mShootCheck->initialise((int)SAS_DEF_CTX_NC_SHOOT_SIZE);
        mShootCheck->setObjectPointers(mNodes, &mMasterNPMs, "Shoots");
    }
    else
    {
        if(mNodeCheck != NULL)
            delete mNodeCheck;
        mNodeCheck = new NodeCheckClass();
        PARANOID_ASSERT_L4(mNodeCheck != NULL);
        
        mNodeCheck->initialise((int)SAS_DEF_CTX_NC_NODE_SIZE);
        mNodeCheck->setObjectPointers(mNodes, &mMasterNPMs, "Nodes");
    }
}
//HO 

//******************************************************************************
// CONTIGS AND SEQUENCE INFORMATION
//******************************************************************************

/*HV*/ void
ContextMemWrapper::
/*HV*/ makeAllContigs()
//HO ;
{ // logInfo(__LINE__,1);
    if(0 == mAllContexts.size())
    {
        logError("No contexts to make contigs from");
        return;
    }
    
    // delete any existing contigs
    if(mContigs != NULL)
    {
        delete mContigs;
        mContigs = NULL;
    }
    mContigs = new ContigClass();
    mContigs->initialise(SAS_DEF_ASS_CONTIG_SIZE);
    mContigs->initialise2(mNodes, mStatsManager, DUALNODE);
    
    // there should only be one contig per context, so we need to splice
    // quite a few times
    ContigId this_CID;
    std::map<ContextId, GenericNodeId>::iterator all_contexts_iter = mAllContexts.begin();
    std::map<ContextId, GenericNodeId>::iterator all_contexts_last = mAllContexts.end();
    while(all_contexts_iter != all_contexts_last)
    {
        this_CID = mContigs->splice(all_contexts_iter->first);
        
        // store the length and position data in the context
        // make sure that the contig is arranged from 
        // lowest Context pos to highest...
        if(mNodes->getCntxPos(mContigs->getStart(this_CID)) > mNodes->getCntxPos(mContigs->getEnd(this_CID)))
            mContigs->reverseContig(this_CID);
        setStartPos(mNodes->getCntxPos(mContigs->getStart(this_CID)), all_contexts_iter->first);
        setEndPos(mNodes->getCntxPos(mContigs->getEnd(this_CID)), all_contexts_iter->first);
        setLongestContig(mContigs->getContigSequenceLengthManual(this_CID), all_contexts_iter->first);
        all_contexts_iter++;
        
    }
    
    // sort them here
    mContigs->sortContigs();
}
//HO 

/*HV*/ void
ContextMemWrapper::
/*HV*/ printAllContigs(std::string fileName, int * n50, int * totalBases, int * longestContig, int * numContigs)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // Print all the contigs to file and get the stats
    // assume that Contigs have been made and sorted in a previous call
    // to makeAllContigs()
    //
    if(mContigs == NULL)
    {
        logError("No Contigs to print");
        return;
    }
    
    // print them out
    mContigs->printContigs(fileName);
    
    #if __USE_RI
    // print the mappings if need be...
    if(printMappings)
        mContigs->printMappings(mappingName);
    #endif
    
    // get the stats together:
    mContigs->makeContigStats();
    *numContigs = mContigs->getNumContigs();
    *n50 = mContigs->getN50();
    *longestContig = mContigs->getLongestContigLength();
    *totalBases = mContigs->getTotalBases();
}
//HO 

/*HV*/ std::string
ContextMemWrapper::
/*HV*/ getContigEnd(uMDInt len, bool isStart, bool rc, ContextId CTXID)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // return the last bb bases of the Context's contig
    // reversed if necessary
    //
    if(mContigs == NULL)
    {
        logError("No contigs to cut from");
        return "";
    }
    
    GenericNodeId start_node;
    
    if(isStart)
        start_node = getStartGn(CTXID);
    else
        start_node = getEndGn(CTXID);
    
    //std::cout << len << ", " << rc << ", " << start_node << ", " << CTXID << " : " << (mNodes->getContig(DUALNODE, start_node)) << std::endl;
    return mContigs->getContigEnd(len, rc, start_node, mNodes->getContig(DUALNODE, start_node));
}
//HO 

/*HV*/ void
ContextMemWrapper::
/*HV*/ getContigSequence(std::string * sequence, int trimFront, int trimEnd, bool doGC, GenericNodeId * startNode, bool doRC, ContextId CTXID)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // get a Contig sequence
    //
    mContigs->getContigSequence(sequence, trimFront, trimEnd, doGC, startNode, doRC, mNodes->getContig(DUALNODE, getStartGn(CTXID)));
}
//HO 

/*HV*/ void
ContextMemWrapper::
/*HV*/ revCmpSequence(std::string * sequence)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // get the reverse complement of a sequence
    //
    int i = sequence->length();
    char out_buff[i];
    char *pOut_buff = out_buff;
    const char * in_buff_end = sequence->c_str() + i - 1;
    
    // just do it!!!
    while(i)
    {
        switch(*in_buff_end)
        {
            case SAS_CE_D_0:
                *pOut_buff = SAS_CE_R_0;
                break;
            case SAS_CE_D_1:
                *pOut_buff = SAS_CE_R_1;
                break;
            case SAS_CE_D_2:
                *pOut_buff = SAS_CE_R_2;
                break;
            case SAS_CE_D_3:
                *pOut_buff = SAS_CE_R_3;
                break;
        }
        i--;
        pOut_buff++;
        in_buff_end--;
    }
    sequence->assign(out_buff, i);
}
//HO 

//HO inline void cutRawSeq(std::string * raw_seq, std::ostringstream * buffer) { mContigs->cutRawSeq(raw_seq, buffer); }
//HO 

//******************************************************************************
// PRINTING AND OUTPUT
//******************************************************************************

/*HV*/ void
ContextMemWrapper::
/*HV*/ printContextGraph(ContextId CTXID)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // print a graphviz style graph of the entire context
    //
    // go through all the Generic nodes and filter for the ones in this context
    std::cout << "graph " << CTXID << " { " << std::endl;
    NodeBlockElem nbe;
    mNodes->getCTXBlock_DN(&nbe, CTXID);
    
    while(mNodes->nextNBElem(&nbe))
    {
        // we have a guy in this context
        mNodes->printLocalGraph(DUALNODE, nbe.NBE_CurrentNode);
    }
    std::cout << "}" << std::endl;
}
//HO 

//HO inline void printAllEdges(ContextId CTXID, bool showBanished) { mNodes->printAllEdges(CTXID, showBanished); }
//HO 

/*HV*/ void
ContextMemWrapper::
/*HV*/ printShoots(shootQueue * shoots, ContextId CTXID)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // print all the shoots
    //
    shootQueueIterator shoot_iter = shoots->begin();
    shootQueueIterator shoot_last = shoots->end();
    if(shoot_iter != shoot_last)
    {
        std::cout << "Shoots for: " << CTXID << std::endl;
    }
    while(shoot_iter != shoot_last)
    {
        GenericNodeId tmp_shoot_GID = mShootCheck->checkOutNode(false, CTXID, shoot_iter->first);
        std::cout << tmp_shoot_GID << " : " << shoot_iter->second << " pos: " << mNodes->getCntxPos(tmp_shoot_GID) << " : " << mNodes->getSequence(tmp_shoot_GID) << std::endl;
        shoot_iter++;
    }
}
//HO 

/*HV*/ void
ContextMemWrapper::
/*HV*/ printShoot(contextShoot shoot, ContextId CTXID)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // print a shoot
    //
    GenericNodeId tmp_shoot_GID = mShootCheck->checkOutNode(false, CTXID, shoot.first);
    std::cout << tmp_shoot_GID << " : " << shoot.second << " pos: " << mNodes->getCntxPos(tmp_shoot_GID) << " : " << mNodes->getSequence(tmp_shoot_GID) << std::endl;
}
//HO 

/*HV*/ void
ContextMemWrapper::
/*HV*/ logShoot(contextShoot shoot, ContextId CTXID, int llevel)
//HO ;
{ // logInfo(__LINE__,1);
    //-----
    // print a shoot
    //
    GenericNodeId tmp_shoot_GID = mShootCheck->checkOutNode(false, CTXID, shoot.first);
    logInfo(tmp_shoot_GID << " : " << shoot.second << " pos: " << mNodes->getCntxPos(tmp_shoot_GID) << " : " << mNodes->getSequence(tmp_shoot_GID), llevel);
}
//HO 
