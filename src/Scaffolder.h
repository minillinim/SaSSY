/******************************************************************************
**
** File: Scaffolder.h
**
*******************************************************************************
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
** Copyright (C) 2011 Michael Imelfort and Dominic Eales
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

#ifndef Scaffolder_h
    #define Scaffolder_h

// system includes 
#include <iostream>
#include <string>

// local includes
#include "StatsManager.h"
#include "GenericNodeClass.h"
#include "ContextClass.h"
#include "Dataset.h"

#include "intdef.h"
#include "IdTypeDefs.h"
#include "paranoid.h"
#include "LoggerSimp.h"

// Need an element which is like the vanilla GN_ELEM
// but oly returns those which have no CTXID
// EAASY -- Hijack the generic node element
#define SC_ELEM         GN_ELEM
#define SCE_GID         GNE_GID 
#define SCE_UID         GNE_UID 
#define SCE_DID         GNE_DID 
#define SCE_Dummy       GNE_Dummy 
#define SCE_Master      GNE_Master 
#define SCE_isHead      GNE_isHead 
#define SCE_Orientation GNE_Orientation 
#define SCE_CNTX        GNE_CNTX 
#define SCE_CNTXPos     GNE_CNTXPos 
#define SCE_CNTXRev     GNE_CNTXRev 
#define SCE_NextHit     GNE_NextHit 
#define SCE_Head        GNE_Head 

// Need a struct which will hold the validated (though perhaps not non-ambiguous)
// scaffs. Use a class
class V_Scaff {
    public:
        
  // Constructor / Destructor
    V_Scaff() { VS_BeyondScaff = NULL; VS_BeforeScaff = NULL; VS_Rank = 0; VS_PathAgrees = true; }
    ~V_Scaff() {}
    
  // File + IO
    void printContents();
    void logContents(int logLevel);
    void printVSGraph(void);                // print a graphviz style graph of this valid scaffold to std::out
        
  // Member vars
    ContextId VS_Base;                      // the base context "from"
    ContextId VS_Linker;                    // the "to" Context
    bool VS_IsBefore;                       // does the "to" lie before the "from" - does the linker lie before the base?
    bool VS_IsSameDirn;                     // is the "to" in the same direction as the "from"
    sMDInt VS_Gap;                          // what is the gap between the two?
    uMDInt VS_NumConnections;               // the number of connections making up this link
    
  // Just to make things even more complex.
  // Put each guy into a linked list context
  // Direction is from the base -> linker...
    V_Scaff * VS_BeyondScaff;
    V_Scaff * VS_BeforeScaff;
    
  // used when making chains
    int VS_Rank;
    bool VS_PathAgrees;
  
};

// This class stores individual scaffoldlinks
// Once setPairKey is called. Base will always be lower than Linker...
class S_Scaff {
    public:
        
        // for sorting...
        struct compareSScaffs : std::binary_function<const S_Scaff&, const S_Scaff&, bool> {
            bool operator ()(S_Scaff *left, S_Scaff *right) 
            {
                // first sort on the pairkey
                if(left->SS_PairKey < right->SS_PairKey)
                {
                    return true;
                }
                else if(right->SS_PairKey < left->SS_PairKey)
                {
                    return false;
                }
                else
                {
                    // if pairkey is equal, sort on before Vs beyond
                    if(!right->SS_IsBefore && left->SS_IsBefore)
                    {
                        return true;
                    }
                    else if(right->SS_IsBefore && !left->SS_IsBefore)
                    {
                        return false;
                    }
                    else
                    {
                        // next sort on same direction
                        if(!right->SS_IsSameDirn && left->SS_IsSameDirn)
                        {
                            return true;
                        }
                        else if(right->SS_IsSameDirn && !left->SS_IsSameDirn)
                        {
                            return false;
                        }
                        else
                        {
                            // finally, sort on the gap
                            return left->SS_Gap < right->SS_Gap;
                        }
                    }
                }
            }
        };
        
        // Constructor / Destructor
        S_Scaff() {}
        ~S_Scaff() {}
        
        void setPairKey();
        
        // File + IO
        void printContents();
        void logContents(int logLevel);
        void printCSV(void);
        
        // Member vars
        bool SS_IncludeInCount;                 // use this guy when getting the counts
        
        ContextId SS_Base;                      // the base context "from"
        ContextId SS_Linker;                    // the "to" Context
        bool SS_IsBefore;                       // does the "to" lie before the "from"
        bool SS_IsSameDirn;                     // is the "to" in the same direction as the "from"
        sMDInt SS_Gap;                          // what is the gap between the two?

        FileSetId SS_FileId;                    // which file did this hit come from?
        uMDInt SS_BaseDist;                     // how far in from the ends were the hits
        uMDInt SS_LinkerDist;
        uMDInt SS_BasePos;                      // Where did these guys hit?
        uMDInt SS_LinkerPos;

        uMDInt SS_PairKey;                      // key used to identify both contexts in the hash
        uMDInt SS_TypeKey;                      // key used to identify before / after /same /opp differences
};

class Scaffolder {

    public:

  // Constructor / Destructor
    Scaffolder(std::string outDir, uMDInt minLinks, uMDInt minIns, bool doScaffGraph, bool doCsv);
    ~Scaffolder(void);
    void setObjectPointers(GenericNodeClass * GNC, StatsManager * SM, ContextClass * CC);
    
  // operations
    bool getElem(SC_ELEM * data, GenericNodeId head);                     // get the head for getting unassigned DualNodes
    bool getNextElem(SC_ELEM * data);                                     // get the next unassigned DualNode
    bool getFileStats(void);                                              // get the means, orientationsm, etc.. for the longest InsertLibraries in each FileSet
    bool isBeyond(bool isStart, bool isReversed, InsertLibrary::OrientationType OT);
    bool scaffoldStep(V_Scaff ** working_scaff);                           // walk along a scaffold chain
    std::string scaffoldSW(int * overlap, uMDInt * start_1, uMDInt * end_1, uMDInt * start_2, uMDInt * end_2, const std::string& str1, const std::string& str2, uMDInt mainLen);
    double getGC(std::string * sequence);
    
  // completly clear the scaffold lists
    void clearSimpleScaffolds(void);
    void clearValidScaffolds(void);
    void clearScaffolds(void);
    
  // clean up the scaffold lists
    void purgeSimpleScaffolds(void);
    void purgeValidScaffolds(void);
    
  // scaffolding
    bool scaffoldAll(void);
    bool processAtMaster(bool isStart, GenericNodeId head_GID, bool cntxRev, uMDInt position, ContextId CTXID);
  
  // file + IO
    void logFileStats(int logLevel);
    void logValidScaffolds(int logLevel);
    void logSimpleScaffolds(int logLevel);
    void printVSGraph(void);                                                // print a graphviz style graph of all valid scaffolds to std::out
    void printCSV(void);
    
    void printAllScaffolds(std::string fileName);
    void printScaffold(ofstream * outFile, std::vector< V_Scaff * > * currentScaff, int scaffNum);

    void printStats(void);
    
    protected:
        
    private:
    GenericNodeClass * mNodes;
    StatsManager * mStatsManager;
    ContextClass * mContexts;
    DataSet * mDataSet;
    
    // minimum number of links needed to make a scaffold
    uMDInt mMinLinks;
    uMDInt mMinIns;
    
    // directory to write scaffold out to
    std::string mOutputDir;
    bool mDoScaffGraph;
    bool mDoCsv;
    
    // the current context during intial scaffold analysis
    ContextId mCurrentContext;
    
    // save asking a million times
    ContextId CTX_NULL_ID;
    std::map<ContextId, GenericNodeId> * mAllContexts;  
    uMDInt mReadLength;
    
    // For storing InsertLibrray info
    // What is the orientation type of the longest Insert Library
    // for a given FileSetId?
    // Also, the upper and lower cutoffs and the means...
    std::map<FileSetId, InsertLibrary::OrientationType> mLongestInserts;
    std::map<FileSetId, uMDInt> mUpperCuts;
    std::map<FileSetId, uMDInt> mLowerCuts;
    std::map<FileSetId, uMDInt> mMeans;
    std::map<FileSetId, bool> mUsableFileSets;
    
    // we don't want to use any node more than once...
    std::map<GenericNodeId, bool> mUsedNodes;
    
    std::vector< V_Scaff * > mValidScaffolds;
    std::vector< S_Scaff * > mSimpleScaffolds;
    std::vector< std::vector< V_Scaff * > * > mScaffolds;
    
    std::vector< int > mScaffSizes;
    std::map< ContextId, bool > mUsedContexts;
    int mScaffNum;
    
    // these three maps are used to determine the most commonly occurring
    // configuration. Used in purge maps function
    std::map<uMDInt, unsigned int> mPairKeyMaxMap;
    std::map<uMDInt, bool> mPairKeyDelMap;
    std::map<uMDInt, uMDInt> mPairTypeMaxMap;
};

#endif // Scaffolder_h
