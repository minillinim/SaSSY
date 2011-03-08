/******************************************************************************
**
** File: Scaffolder.cpp
**
*******************************************************************************
** 
** VERSION 1 of a scaffolding algorithm. Probably won't work well...
** 
** The main ideas: 
** 
** 1. Only use the longest InsertLibrary in each fileset.
** 
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

// system includes
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits.h>
#include <cmath>
#include <string>
#include <math.h>

// local includes
#include "Scaffolder.h"
#include "StatsManager.h"
#include "GenericNodeClass.h"
#include "ContextClass.h"
#include "Dataset.h"

#include "intdef.h"
#include "IdTypeDefs.h"
#include "paranoid.h"
#include "LoggerSimp.h"
#include "char_encodings.h"

// Used in the smith waterman - type algorithm below
// quick max
#define SAS_SW_MAX2(x,y)       ((x)<(y) ? (y) : (x))
#define SAS_SW_MAX3(x,y,z)     (SAS_SW_MAX2(x,y)<(z) ? (z) : SAS_SW_MAX2(x,y))

// Build similarity matrix
#define SAS_SW_MATCH    (1.1)
#define SAS_SW_MISMATCH (-1)
#define SAS_SW_GAP(K)   (-(37 + (4*K)))

#define SAS_SW_SIMINIT  { {SAS_SW_MATCH, SAS_SW_MISMATCH, SAS_SW_MISMATCH, SAS_SW_MISMATCH}, \
                          {SAS_SW_MISMATCH, SAS_SW_MATCH, SAS_SW_MISMATCH, SAS_SW_MISMATCH}, \
                          {SAS_SW_MISMATCH, SAS_SW_MISMATCH, SAS_SW_MATCH, SAS_SW_MISMATCH}, \
                          {SAS_SW_MISMATCH, SAS_SW_MISMATCH, SAS_SW_MISMATCH, SAS_SW_MATCH} }

#define SAS_SW_MAX_WEIGHT (3.5)
#define SAS_SW_EXPONENT   (3.5)

// Data structures used in the SW alg below

// Use weights to favour overlaps towards the ends
static double SAS_SW_STR_1_WEIGHTS[SAS_DEF_CONT_END_LEN + 1];
static double SAS_SW_STR_2_WEIGHTS[SAS_DEF_CONT_END_LEN + 1];

// Build similarity matrix
static double SAS_SW_SIM[SAS_CE_NUM_NUCS][SAS_CE_NUM_NUCS] = SAS_SW_SIMINIT;

// main matrix and traceback matricies
static int SAS_SW_TRACEBACK_1[SAS_DEF_CONT_END_LEN + 1][SAS_DEF_CONT_END_LEN + 1];
static int SAS_SW_TRACEBACK_2[SAS_DEF_CONT_END_LEN + 1][SAS_DEF_CONT_END_LEN + 1];
static double SAS_SW_MAIN_MATRIX[SAS_DEF_CONT_END_LEN + 1][SAS_DEF_CONT_END_LEN + 1];

// consensus sequence
static char SAS_SW_CONSENSUS[2 * (SAS_DEF_CONT_END_LEN + 1)];

//**************************************************
// VALID SCAFFOLDS

void V_Scaff::printContents()
{
    //-----
    // Print the contents of the class to STD::COUT
    //
    std::string lies = "beyond";
    std::string dirn = "opposite";
    if(VS_IsBefore) { lies = "before"; }
    if(VS_IsSameDirn) { dirn = "same"; }
    std::cout << VS_Linker << " lies " << lies << " " << VS_Base << " in the " << dirn << " direction, with gap: " << VS_Gap << std::endl;
    std::cout << VS_BeforeScaff << " : " << VS_BeyondScaff << " : " << VS_PathAgrees << std::endl;
}

void V_Scaff::logContents(int logLevel)
{
    //-----
    // Print the contents of the class to the log file
    //
    std::string lies = "beyond";
    std::string dirn = "opposite";
    if(VS_IsBefore) { lies = "before"; }
    if(VS_IsSameDirn) { dirn = "same"; }
    logInfo(VS_Linker << " lies " << lies << " " << VS_Base << " in the " << dirn << " direction, with gap: " << VS_Gap << " Cons: " << VS_NumConnections, logLevel);
}

void V_Scaff::printVSGraph(void)
{
    //-----
    // print a graphviz style graph of this valid scaffold to std::out
    //
    if(VS_IsSameDirn) 
    {
        // ---L-->  ---B-->   B o--^ L
        if(VS_IsBefore) { std::cout << "\t\"" << VS_Base << "\" -> \"" << VS_Linker << "\" [arrowtail = dot, arrowhead = odiamond, label = \"" << VS_Gap << ":" << VS_NumConnections << "\"];" << std::endl; }
        // ---B-->  ---L-->   B ^--o L
        else { std::cout << "\t\"" << VS_Base << "\" -> \"" << VS_Linker << "\" [arrowtail = odiamond, arrowhead = dot, label = \"" << VS_Gap << ":" << VS_NumConnections << "\"];" << std::endl; }
    }
    else 
    {
        // <--L---  ---B-->  B o--o L
        if(VS_IsBefore) { std::cout << "\t\"" << VS_Linker << "\" -> \"" << VS_Base << "\" [arrowtail = dot, arrowhead = dot, label = \"" << VS_Gap << ":" << VS_NumConnections << "\"];" << std::endl; }
        // ---B-->  <---L---  B ^--^ L
        else { std::cout << "\t\"" << VS_Base << "\" -> \"" << VS_Linker << "\" [arrowtail = odiamond, arrowhead = odiamond, label = \"" << VS_Gap << ":" << VS_NumConnections << "\"];" << std::endl; }
    }
    
    
}

//**************************************************
// SIMPLE SCAFFOLDS

void S_Scaff::setPairKey()
{
    //-----
    // create a key using both Contexts
    //
    if(SS_Linker < SS_Base)
    {
        // Base > Linker.
        // We need to swap them.
        if(SS_IsSameDirn)
            SS_IsBefore = !SS_IsBefore;
        
        // do the swap
        uMDInt tmp_pos = SS_BasePos;
        uMDInt tmp_dist = SS_BaseDist;
        SS_BasePos = SS_LinkerPos;
        SS_BaseDist = SS_LinkerDist;
        SS_LinkerPos = tmp_pos;
        SS_LinkerDist = tmp_dist;

        ContextId tmp = SS_Base;
        SS_Base = SS_Linker;
        SS_Linker = tmp;
    }
    
    // base is lower than linker...
    SS_PairKey = SS_Base.getGuts() * 100000000 + SS_Linker.getGuts();
    
    uMDInt type_key = SS_PairKey;   
    if(SS_IsBefore)
        type_key *= 2;
    if(SS_IsSameDirn)
        type_key += 5;
    
    SS_TypeKey = type_key;
}

void S_Scaff::printContents()
{
    //-----
    // Print the contents of the class to STD::COUT
    //
    std::string lies = "beyond";
    std::string dirn = "opposite";
    if(SS_IsBefore) { lies = "before"; }
    if(SS_IsSameDirn) { dirn = "same"; }
    std::cout << SS_Linker << " lies " << lies << " " << SS_Base << " in the " << dirn << " direction, with gap: " << SS_Gap << " PK: " << SS_PairKey << " TK: " << SS_TypeKey << std::endl;
    std::cout << "Hit from FileSet: " << SS_FileId << " BasePos: " << SS_BasePos << " BaseDist: " << SS_BaseDist << " LinkerPos: " << SS_LinkerPos << " LinkerDist: " << SS_LinkerDist << std::endl;
}

void S_Scaff::printCSV()
{
    //-----
    // Print the contents of the class to STD::COUT in csv format
    //
    std::cout << SS_Base << "," << SS_Linker << "," << SS_IsSameDirn << "," << SS_IsBefore << "," << SS_FileId << "," << SS_BasePos << "," << SS_BaseDist << "," << SS_LinkerPos << "," << SS_LinkerDist << "," << SS_Gap << "," << SS_PairKey << "," << SS_TypeKey << std::endl;
}

void S_Scaff::logContents(int logLevel)
{
    //-----
    // Print the contents of the class to the log file
    //
    std::string lies = "beyond";
    std::string dirn = "opposite";
    if(SS_IsBefore) { lies = "before"; }
    if(SS_IsSameDirn) { dirn = "same"; }
    logInfo(SS_Linker << " lies " << lies << " " << SS_Base << " in the " << dirn << " direction, with gap: " << SS_Gap << " PK: " << SS_PairKey << " TK: " << SS_TypeKey << " USE_COUNT: " << SS_IncludeInCount , logLevel);
    logInfo("Hit from FileSet: " << SS_FileId << " BasePos: " << SS_BasePos << " BaseDist: " << SS_BaseDist << " LinkerPos: " << SS_LinkerPos << " LinkerDist: " << SS_LinkerDist, logLevel);
}

//**************************************************
// SCAFFOLD CLASS

// Constructor/Destructor

Scaffolder::Scaffolder(std::string outDir, uMDInt minLinks, uMDInt minIns, bool doScaffGraph, bool doCsv)
{
    //-----
    // default constructor
    //
    mOutputDir = outDir;
    mReadLength = 0;
    mMinLinks = minLinks;
    mMinIns = minIns;
    mDoScaffGraph = doScaffGraph;
    mDoCsv = doCsv;
    
    mNodes = NULL;
    mStatsManager = NULL;
    mContexts = NULL;
    mDataSet = NULL;
    mAllContexts = NULL;  
}

Scaffolder::~Scaffolder(void) 
{
    //-----
    // default destructor
    //
    clearSimpleScaffolds();
    clearValidScaffolds();
    clearScaffolds();
}

void Scaffolder::setObjectPointers(GenericNodeClass * GNC, StatsManager * SM, ContextClass * CC)
{
    mNodes = GNC;
    mStatsManager = SM;
    mReadLength = mStatsManager->getReadLength();
    mDataSet = mStatsManager->getDS();
    mContexts = CC;
    CTX_NULL_ID = mContexts->getNullContextId();
    mCurrentContext = CTX_NULL_ID;
    mAllContexts = mContexts->getAllContexts();
    getFileStats();
    logFileStats(1);
    
    // Fill the weighting matricies...
    double param = (SAS_SW_MAX_WEIGHT - 1) / pow(SAS_DEF_CONT_END_LEN, SAS_SW_EXPONENT);
    SAS_SW_STR_1_WEIGHTS[0] = 0; SAS_SW_STR_2_WEIGHTS[0] = 0;
    for(int i = 1; i<= SAS_DEF_CONT_END_LEN ; i++)
    {
        SAS_SW_STR_1_WEIGHTS[i] = 1 + param * pow((double)i,SAS_SW_EXPONENT);
        SAS_SW_STR_2_WEIGHTS[SAS_DEF_CONT_END_LEN - i + 1] = 1 + param * pow((double)i,SAS_SW_EXPONENT);
    }
}

// operations

bool Scaffolder::getElem(SC_ELEM * data, GenericNodeId head)
{
    //-----
    // Get the first element in the loop
    // Sets this guy to the head
    //
    data->SCE_GID = head;
    data->SCE_DID = mNodes->getDualNode(head);
    data->SCE_Orientation = mNodes->isFileReversed(head);
    data->SCE_Master = mNodes->isCntxMaster(head);
    data->SCE_Dummy = mNodes->isDummy(head);
    data->SCE_isHead = mNodes->isHead(head);
    data->SCE_CNTX = mNodes->getCntxId(head);
    data->SCE_CNTXPos = mNodes->getCntxPos(head);
    data->SCE_CNTXRev = mNodes->isCntxReversed(head);
    data->SCE_Head = head;
    data->SCE_NextHit = mNodes->getNextElem(head);
    return true;
}

bool Scaffolder::getNextElem(SC_ELEM * data)
{
    //-----
    // Get the next element in the loop, without a ContextId
    // Uses recurrsion...
    //
    if(data->SCE_Head != data->SCE_NextHit && GenericNodeId() != data->SCE_Head)
    {
        if(CTX_NULL_ID == mNodes->getCntxId(data->SCE_NextHit))
        {
            data->SCE_GID = data->SCE_NextHit;
            data->SCE_DID = mNodes->getDualNode(data->SCE_NextHit);
            data->SCE_Orientation = mNodes->isFileReversed(data->SCE_NextHit);
            data->SCE_Master = mNodes->isCntxMaster(data->SCE_NextHit);
            data->SCE_Dummy = mNodes->isDummy(data->SCE_NextHit);
            data->SCE_isHead = mNodes->isHead(data->SCE_NextHit);
            data->SCE_CNTX = mNodes->getCntxId(data->SCE_NextHit);
            data->SCE_CNTXPos = mNodes->getCntxPos(data->SCE_NextHit);
            data->SCE_CNTXRev = mNodes->isCntxReversed(data->SCE_NextHit);
            data->SCE_NextHit = mNodes->getNextElem(data->SCE_NextHit);
            return true;
        }
        else
        {
            data->SCE_NextHit = mNodes->getNextElem(data->SCE_NextHit);
            return getNextElem(data);
        }
    }
    return false;
}

bool Scaffolder::getFileStats(void)
{
    //-----
    // get the means, orientationsm, etc.. for the longest InsertLibraries in each FileSet
    //
    if(mDataSet->getFileStats(&mLongestInserts, &mUpperCuts, &mLowerCuts, &mMeans))
    {
        // work out which filesets are usable
        logInfo("Rejecting all reads from FileSets with insert less than: " << mMinIns << " for the scaffolding stage", 1);
        std::vector<FileSetId> usables;
        std::map<FileSetId, uMDInt>::iterator means_iter = mMeans.begin();
        while(means_iter != mMeans.end())
        {
            if(means_iter->second >= mMinIns) { mUsableFileSets[means_iter->first] = true; usables.push_back(means_iter->first); }
            else { mUsableFileSets[means_iter->first] = false; }
            means_iter++;
        }
        if(usables.size() == 0) { logError("Insert cut off of: " << mMinIns << " renders all reads unusable for scaffolding"); return false; }
        else 
        {
            logInfo("Using filesets: ", 1);
            std::vector<FileSetId>::iterator usable_iter = usables.begin();
            while(usable_iter != usables.end())
            {
                logInfo(*usable_iter, 1);
                usable_iter++;
            }
        }
        return true;
    }
    return false;
}

bool Scaffolder::isBeyond(bool isStart, bool isReversed, InsertLibrary::OrientationType OT)
{
    //-----
    // For finding only guys who point BEYOND
    // the current working end
    //
    switch(OT)
    {
        case InsertLibrary::TYPE_DD:
        {
            // <---- ---->
            if(isStart)             // <====    ---====>-----Rev = 0----------------------------------------->
                return !isReversed;
            else                    //          ---------------------------------------Rev = 1-----<====----->    ====>
                return isReversed;
        }
        case InsertLibrary::TYPE_AD:
        {
            // ----> <----
            if(isStart)             // ====>    ---<====-----Rev = 1----------------------------------------->
                return isReversed;
            else                    //          ---------------------------------------Rev = 0-----====>----->    <====
                return !isReversed;
        }
        default:
            return false;
    }
    return false;
}

std::string Scaffolder::scaffoldSW(int * overlap, uMDInt * start_1, uMDInt * end_1, uMDInt * start_2, uMDInt * end_2, const std::string& str1, const std::string& str2, uMDInt mainLen)
{
    //-----
    // Smith waterman for nuc's 
    // Based on code from: http://www.cs.bc.edu/~clote/ComputationalMolecularBiology/smithWaterman.c
    // but changed significantly
    // 
    // Uses weights to favour overlaps at the end of STR_1 and the start of STR_2
    // Could be called gapless but this is not true. If there were a *great* overlap
    // which had a gap, then a gap it would be...
    // 
    // uses the encodings in char_encodings.h
    // 
    // Str1 and Str2 must be the same length.
    // 
    // Similarity matrix is simple 1/-6 match mismatch
    // Gap penalty is -35
    // Weights are quartic within the range 1 - 4 across the domain 1 -> SAS_DEF_CONT_END_LEN
    // We only use as much of the weights array as we need to
    //
    // This is smith waterman so we are optimising on:
    //                     _
    //                    |
    //                    | H(i-1,j-1) + sigma(a_i,b_j)  (diagonal)
    // H(i,j) =  MAX of   | H(i-1,j)   + delta           (up)
    //                    | H(i,j-1)   + delta           (left)
    //                    | 0
    //                    | _
    // 
    // Return the consensus sequence with the overlap in the middle
    // 
    // GO!
    double temp_max, global_max;
    int max_str_1, max_str_2;  
    double diag, down, right;
    temp_max = global_max = max_str_1 = max_str_2 = 0;
    uMDInt front_off = SAS_DEF_CONT_END_LEN - mainLen;
    
    // fill the main and traceback matricies
    // reset the consensus array
    for (int i = 0; i <= SAS_DEF_CONT_END_LEN; i++) { 
        SAS_SW_MAIN_MATRIX[i][0] = 0;
        SAS_SW_MAIN_MATRIX[0][i] = 0;
        
        SAS_SW_TRACEBACK_1[i][0] = -1;
        SAS_SW_TRACEBACK_1[0][i] = -1;
        
        SAS_SW_TRACEBACK_2[i][0] = -1;
        SAS_SW_TRACEBACK_2[0][i] = -1;
        
        SAS_SW_CONSENSUS[(2 * i)] = 'N';
        SAS_SW_CONSENSUS[(2 * i) + 1] = 'N';
    }

    int gap_len_1 = 0;
    int gap_len_2 = 0;

    for (uMDInt i = 1; i <= mainLen; i++)
    {
        for (uMDInt j = 1; j <= mainLen; j++) 
        {
            // work out the values
            diag    = SAS_SW_MAIN_MATRIX[i-1][j-1] + (SAS_SW_SIM[NUC2INT(str1[i - 1])][NUC2INT(str2[j - 1])] * SAS_SW_STR_1_WEIGHTS[i + front_off] * SAS_SW_STR_2_WEIGHTS[j]);
            down    = SAS_SW_MAIN_MATRIX[i-1][j] + SAS_SW_GAP(gap_len_2);
            right   = SAS_SW_MAIN_MATRIX[i][j-1] + SAS_SW_GAP(gap_len_1);
            
            temp_max = SAS_SW_MAX3(diag,down,right);
            
            if (temp_max <= 0)  
            {
                gap_len_2 = 0;
                gap_len_1 = 0;                
                SAS_SW_MAIN_MATRIX[i][j] = 0;
                SAS_SW_TRACEBACK_1[i][j] = -1;
                SAS_SW_TRACEBACK_2[i][j] = -1;
            }
            else if (temp_max == diag) 
            {
                gap_len_2 = 0;
                gap_len_1 = 0;                
                SAS_SW_MAIN_MATRIX[i][j] = diag;
                SAS_SW_TRACEBACK_1[i][j] = i-1;
                SAS_SW_TRACEBACK_2[i][j] = j-1;
            }
            else if (temp_max == down) 
            {
                gap_len_2++;
                gap_len_1 = 0;
                SAS_SW_MAIN_MATRIX[i][j] = down;
                SAS_SW_TRACEBACK_1[i][j] = i-1;
                SAS_SW_TRACEBACK_2[i][j] = j;
            }
            else  
            {
                gap_len_2 = 0;
                gap_len_1++;
                SAS_SW_MAIN_MATRIX[i][j] = right;
                SAS_SW_TRACEBACK_1[i][j] = i;
                SAS_SW_TRACEBACK_2[i][j] = j-1;
            }
            
            // update the global maximum in H
            if (temp_max > global_max)
            {
                global_max = temp_max;
                max_str_1 = i;
                max_str_2 = j;
            }
        }
    }
    
    *end_1 = max_str_1;
    *end_2 = max_str_2;
    
    // Walk back and find the longest alignment
    // if there are gaps, take the shortest consensus!
    bool j_move, i_move;
    j_move = i_move = true;
    *overlap = 0;
    int used_1 = 0;
    int used_2 = 0;
    int temp_max_1, temp_max_2;
    
    while (max_str_1 > 0 && max_str_2 > 0 && SAS_SW_MAIN_MATRIX[max_str_1][max_str_2] > 0)
    {
        if (i_move && j_move) 
        {
            SAS_SW_CONSENSUS[*overlap] = str1[max_str_1 - 1];
            (*overlap)++;
            used_1++;
            used_2++;
        }
        else if(i_move)
        {
            used_1++;
        }
        else if(j_move)
        {
            used_2++;
        }
            
        
        i_move = (max_str_1 > SAS_SW_TRACEBACK_1[max_str_1][max_str_2]);
        j_move = (max_str_2 > SAS_SW_TRACEBACK_2[max_str_1][max_str_2]);
        
        // backup max_str_1 and max_str_2 becuse they are being changed!
        temp_max_1 = max_str_1;
        temp_max_2 = max_str_2;
        max_str_1 = SAS_SW_TRACEBACK_1[temp_max_1][temp_max_2];
        max_str_2 = SAS_SW_TRACEBACK_2[temp_max_1][temp_max_2];
    }
    
    *start_1 = *end_1 - used_1;
    *start_2 = *end_2 - used_2;
    
    // stringify consensus
    stringstream cons_stream;
    cons_stream << str1.substr(0, *start_1);
    for (int i = ((*overlap) - 1); i >= 0; i--) { cons_stream << SAS_SW_CONSENSUS[i]; }
    cons_stream << str2.substr(*end_2, mainLen - *end_2);
    return cons_stream.str();
}

double Scaffolder::getGC(std::string * sequence)
{
    //-----
    // get the GC content of a sequence
    // This makes no sense in colorspace
    //
    #ifdef SOLID_DATA_
        return 0;
    #else
        double GCs = 0;
        double len = (double)sequence->length();
        for(int i = 0; i < len; i++)
        {
            switch((*sequence)[i])
                case SAS_CE_D_1:
                case SAS_CE_D_2:
                {
                    GCs++;
                    break;
                }
        }
        return GCs / len;
    #endif
}

// completly clear the scaffold lists

void Scaffolder::clearSimpleScaffolds(void)
{
    //-----
    // Do just that
    //
    std::vector< S_Scaff * >::iterator ss_iter = mSimpleScaffolds.begin();
    while(ss_iter != mSimpleScaffolds.end())
    {
        if(NULL != *ss_iter)
            delete *ss_iter;
        *ss_iter = NULL;
        ss_iter++;
    }
    mSimpleScaffolds.clear();
}

void Scaffolder::clearValidScaffolds(void)
{
    //-----
    // Do just that
    //
    std::vector< V_Scaff * >::iterator vs_iter = mValidScaffolds.begin();
    while(vs_iter != mValidScaffolds.end())
    {
        if(NULL != *vs_iter)
            delete *vs_iter;
        *vs_iter = NULL;
        vs_iter++;
    }
    mValidScaffolds.clear();
}

void Scaffolder::clearScaffolds(void)
{
    //-----
    // Delete main Scaffold list
    //
    std::vector< std::vector< V_Scaff * > * >::iterator scaff_iter = mScaffolds.begin();
    while(scaff_iter != mScaffolds.end())
    {
        if(NULL != *scaff_iter)
        {
            delete *scaff_iter;
            *scaff_iter = NULL;
        }
        scaff_iter++;
    }
    mScaffolds.clear();
}

// clean up the scaffold lists

void Scaffolder::purgeSimpleScaffolds(void)
{
    //-----
    // purge the static maps of any potential scaffolds
    // with less than the requisite number of links
    // 
    // Make the valid scaffolds, and only use filesets with
    // mean insert size longer than the cutoff to add to their count
    //
    // The scaffolds ahve been clustered  into groups using a sort on their
    // pairkey and typekey fields...
    //
    std::vector< S_Scaff * >::iterator ss_iter = mSimpleScaffolds.begin();
    std::vector< S_Scaff * >::iterator go_back_iter;
    
    uMDInt current_PK = 0;
    uMDInt current_TK = 0;
    unsigned int counter = 0;

    // to identify the first round...
    bool just_started = true;

    // go through and delete any who have less than the required number.
    while(ss_iter != mSimpleScaffolds.end())
    {
        if(*ss_iter != NULL)
        {
            bool is_same = true;
            if((*ss_iter)->SS_PairKey != current_PK)
                is_same = false;
            else if((*ss_iter)->SS_TypeKey != current_TK)
                is_same = false;

            if(!is_same)
            {
                if(!just_started)
                {
                    // check to see if there we enough of these guys
                    if(mMinLinks > counter)
                    {
                        // there are some guys to delete
                        while(go_back_iter != ss_iter)
                        {
                            if(*go_back_iter != NULL)
                            {
                                delete *go_back_iter;
                                *go_back_iter = NULL;
                            }
                            go_back_iter++;
                        }
                    }
                    else
                    {
                        // set the max for this pairkey...
                        std::map<uMDInt, unsigned int>::iterator pkmm_iter = mPairKeyMaxMap.find(current_PK);
                        if(pkmm_iter == mPairKeyMaxMap.end())
                        {
                            // first time we've seen this pairkey
                            mPairKeyDelMap[current_PK] = false;
                            mPairKeyMaxMap[current_PK] = counter;
                            mPairTypeMaxMap[current_PK] = current_TK;
                        }
                        else
                        {
                            // perhaps a new max?
                            if(counter == pkmm_iter->second)
                            {
                                // here is a guy, equal to the max
                                // do nothing yet, but mark the maxmap for deletion
                                mPairKeyDelMap[current_PK] = true;
                            }
                            else if(counter > pkmm_iter->second)
                            {
                                // new_max!
                                mPairKeyDelMap[current_PK] = false;
                                mPairKeyMaxMap[current_PK] = counter;
                                mPairTypeMaxMap[current_PK] = current_TK;
                            }
                        }
                    }
                    if((*ss_iter)->SS_PairKey != current_PK)
                    {
                        // pairkey has changed.
                        // check the delete key. If it is still set to true then there were
                        // two equal contenders for the most common scaffold type
                        // so we cannot use it. Set the max map to an impossible value
                        if(mPairKeyDelMap[current_PK])
                        {
                            mPairKeyMaxMap[current_PK] = 0;
                            mPairTypeMaxMap[current_PK] = 0;
                        }
                    }
                }
                else
                    just_started = false;
                
                // use the command line variable (min_ins) to determine if this guy should be included in the count...
                if((*ss_iter)->SS_IncludeInCount)    
                    counter = 1;
                current_TK = (*ss_iter)->SS_TypeKey;
                current_PK = (*ss_iter)->SS_PairKey;
                go_back_iter = ss_iter;
            }
            else
            {
                if((*ss_iter)->SS_IncludeInCount)
                    counter++;
            }
        }
        ss_iter++;
    }
    // do the last one
    if(!just_started)
    {
        // check to see if there we enough of these guys
        if(mMinLinks > counter)
        {
            // there are some guys to delete
            while(go_back_iter != ss_iter)
            {
                if(*go_back_iter != NULL)
                {
                    delete *go_back_iter;
                    *go_back_iter = NULL;
                }
                go_back_iter++;
            }
        }
        else
        {
            // set the max for this pairkey...
            std::map<uMDInt, unsigned int>::iterator pkmm_iter = mPairKeyMaxMap.find(current_PK);
            if(pkmm_iter == mPairKeyMaxMap.end())
            {
                // first time we've seen this pairkey
                mPairKeyDelMap[current_PK] = false;
                mPairKeyMaxMap[current_PK] = counter;
                mPairTypeMaxMap[current_PK] = current_TK;
            }
            else
            {
                // perhaps a new max?
                if(counter == pkmm_iter->second)
                {
                    // here is a guy, equal to the max
                    // do nothing yet, but mark the maxmap for deletion
                    mPairKeyDelMap[current_PK] = true;
                }
                else if(counter > pkmm_iter->second)
                {
                    // new_max!
                    mPairKeyDelMap[current_PK] = false;
                    mPairKeyMaxMap[current_PK] = counter;
                    mPairTypeMaxMap[current_PK] = current_TK;
                }
            }
        }
        // check the delete key. If it is still set to true then there were
        // two equal contenders for the most common scaffold type
        // so we cannot use it. Set the max map to an impossible value
        if(mPairKeyDelMap[current_PK])
        {
            mPairKeyMaxMap[current_PK] = 0;
            mPairTypeMaxMap[current_PK] = 0;
        }
    }
    
    // delete any that were not the most commonly occuring configuration
    // and find the median value for the most common
    std::vector<sMDInt> median_vec;
    current_TK = 0;
    V_Scaff * new_VS = NULL;
    ss_iter = mSimpleScaffolds.begin();
    
    while(ss_iter != mSimpleScaffolds.end())
    {
        if(*ss_iter != NULL)
        {
            if((*ss_iter)->SS_TypeKey != current_TK)
            {
                // change of type_key
                if(0 != current_TK)
                {
                    if(NULL != new_VS)
                    {
                        new_VS->VS_Gap = median_vec[(median_vec.size() / 2)];
                        mValidScaffolds.push_back(new_VS);
                    }
                    new_VS = NULL;      
                    // because the list is sorted, it's ok to clear these guys here...
                    median_vec.clear();
                    current_TK = 0;
                }
            }
            if(mPairTypeMaxMap[(*ss_iter)->SS_PairKey] != (*ss_iter)->SS_TypeKey)
            {   
                delete *ss_iter;
                *ss_iter = NULL;
            }
            else
            {
                current_TK = (*ss_iter)->SS_TypeKey;
                median_vec.push_back((*ss_iter)->SS_Gap);
                if(NULL == new_VS )
                {
                    //first_time!
                    new_VS = new V_Scaff;
                    new_VS->VS_Base = (*ss_iter)->SS_Base;
                    new_VS->VS_Linker = (*ss_iter)->SS_Linker;
                    new_VS->VS_IsBefore = (*ss_iter)->SS_IsBefore;
                    new_VS->VS_IsSameDirn = (*ss_iter)->SS_IsSameDirn;
                    new_VS->VS_NumConnections = mPairKeyMaxMap[(*ss_iter)->SS_PairKey];
                }
            }
        }
        ss_iter++;
    }
    if(0 != current_TK)
    {
        if(NULL != new_VS)
        {
            new_VS->VS_Gap = median_vec[(median_vec.size() / 2)];
            mValidScaffolds.push_back(new_VS);
        }
        new_VS = NULL;      
    }
    // simple scaffolds are no longer needed
    clearSimpleScaffolds();
}

void Scaffolder::purgeValidScaffolds(void)
{
    //-----
    // purge the static maps of any potential scaffolds
    // with less than the requisite number of links
    //
    std::vector< V_Scaff * >::iterator vs_iter = mValidScaffolds.begin();
    
    std::multimap<ContextId, V_Scaff *> cont_V_mmap;
    std::map<V_Scaff *, bool> kill_list;
    
//*******************************************************     
// Set up base data structures
    while(vs_iter != mValidScaffolds.end())
    {
        if(*vs_iter != NULL)
        {
            // we need to know which Contexts are referenced
            // This is not overly efficient, but not a mortal sin either...
            mUsedContexts.insert(std::pair<ContextId, bool>((*vs_iter)->VS_Base, true));
            mUsedContexts.insert(std::pair<ContextId, bool>((*vs_iter)->VS_Linker, true));
            
            // now populate the multimap, Context -> Scaff, Scaff, Scaff, ...
            cont_V_mmap.insert(std::pair<ContextId, V_Scaff *>((*vs_iter)->VS_Base, (*vs_iter)));
            cont_V_mmap.insert(std::pair<ContextId, V_Scaff *>((*vs_iter)->VS_Linker, (*vs_iter)));
        }
        vs_iter++;
    }
    
//*******************************************************     
// Choose best scaffolds
    std::pair<std::multimap<ContextId, V_Scaff *>::iterator, std::multimap<ContextId, V_Scaff *>::iterator> cont_V_mmap_iter;
    std::map<ContextId, bool>::iterator used_conts_iter = mUsedContexts.begin();
    while(used_conts_iter != mUsedContexts.end())
    {
        V_Scaff * best_b4 = NULL;
        V_Scaff * best_by = NULL;
        uMDInt most_b4_connections = 0;
        uMDInt most_by_connections = 0;
        unsigned int num_b4 = 0;
        unsigned int num_by = 0;
        bool is_before = true;
        
        cont_V_mmap_iter = cont_V_mmap.equal_range(used_conts_iter->first);
        
        // pick the best links in either direction...
        for (std::multimap<ContextId, V_Scaff *>::iterator sing_iter = cont_V_mmap_iter.first; sing_iter != cont_V_mmap_iter.second; ++sing_iter)
        {
            if(sing_iter->second != NULL)
            {
                if(kill_list.find(sing_iter->second) == kill_list.end())
                {
                    // whether or not this guy actually *is* before the other
                    // one depends on if it is a base or a linker...
                    // if it is a base, we can use the flag as is.
                    // if it is a linker, we'll need to modify it a bit
                    if(sing_iter->second->VS_Base == used_conts_iter->first)
                    {
                        is_before = sing_iter->second->VS_IsBefore;
                    }
                    else
                    {
                        if(sing_iter->second->VS_IsSameDirn)
                            is_before = !sing_iter->second->VS_IsBefore;
                        else
                            is_before = sing_iter->second->VS_IsBefore;
                    }
                    
                    if(is_before)
                    {
                        if(sing_iter->second->VS_NumConnections > most_b4_connections)
                        {
                            // new best!
                            most_b4_connections = sing_iter->second->VS_NumConnections;
                            best_b4 = sing_iter->second;
                            num_b4++;
                        }
                    }
                    else
                    {
                        if(sing_iter->second->VS_NumConnections > most_by_connections)
                        {
                            // new best!
                            most_by_connections = sing_iter->second->VS_NumConnections;
                            best_by = sing_iter->second;
                            num_by++;
                        }
                    }
                }
            }
        }

        // We will delete all but the best. Make a list!
        for (std::multimap<ContextId, V_Scaff *>::iterator sing_iter = cont_V_mmap_iter.first; sing_iter != cont_V_mmap_iter.second; ++sing_iter)
        {
            if(sing_iter->second != NULL)
            {
                if(sing_iter->second->VS_Base == used_conts_iter->first)
                {
                    is_before = sing_iter->second->VS_IsBefore;
                }
                else
                {
                    if(sing_iter->second->VS_IsSameDirn)
                        is_before = !sing_iter->second->VS_IsBefore;
                    else
                        is_before = sing_iter->second->VS_IsBefore;
                }
                if(is_before)
                {
                    if(best_b4 != NULL)
                    {
                        if(sing_iter->second != best_b4)
                            kill_list.insert(std::pair<V_Scaff *, bool>(sing_iter->second, true));
                    }
                }
                else
                {
                    if(best_by != NULL)
                    {
                        if(sing_iter->second != best_by)
                            kill_list.insert(std::pair<V_Scaff *, bool>(sing_iter->second, true));
                    }
                }
            }
        }
        used_conts_iter++;
    }
    
    // Delete!
    vs_iter = mValidScaffolds.begin();
    while(vs_iter != mValidScaffolds.end())
    {
        if(*vs_iter != NULL)
        {
            if(kill_list.find(*vs_iter) != kill_list.end())
            {
                // it's in the kill list!
                delete *vs_iter;
                *vs_iter = NULL;
            }
        }
        vs_iter++;
    }
    
//*******************************************************     
// Make scaffold chains
    // We want to chain all the remaining valid scaffolds together
    // these will be stored in the main scaffold object
    cont_V_mmap.clear();
    vs_iter = mValidScaffolds.begin();
    while(vs_iter != mValidScaffolds.end())
    {
        // re-populate this map. No Context should appear in more than 2 V_S's
        if(*vs_iter != NULL)
        {
            cont_V_mmap.insert(std::pair<ContextId, V_Scaff *>((*vs_iter)->VS_Base, (*vs_iter)));
            cont_V_mmap.insert(std::pair<ContextId, V_Scaff *>((*vs_iter)->VS_Linker, (*vs_iter)));
        }
        vs_iter++;
    }
    
    // set the rank of each VS. 
    // Those with rank = 1 lie at the ends of chains, those with rank 2 lie in the middle...
    used_conts_iter = mUsedContexts.begin();
    while(used_conts_iter != mUsedContexts.end())
    {
        ContextId current_CID = used_conts_iter->first;
        V_Scaff * first_scaff = NULL;
        bool first_context_was_base = false;
        bool is_first_scaff = true;
        cont_V_mmap_iter = cont_V_mmap.equal_range(current_CID);
        for (std::multimap<ContextId, V_Scaff *>::iterator sing_iter = cont_V_mmap_iter.first; sing_iter != cont_V_mmap_iter.second; ++sing_iter)
        {
            if(current_CID == sing_iter->second->VS_Base)
            {
                // The other V_Scaff lies behind this guy
                if(is_first_scaff)
                {
                    // we don't know who to point to so just save the values
                    first_context_was_base = true;
                    first_scaff = sing_iter->second;
                    is_first_scaff = false;
                }
                else
                {
                    // This is not the first one...
                    first_scaff->VS_Rank++;
                    sing_iter->second->VS_Rank++;
                    
                    sing_iter->second->VS_BeforeScaff = first_scaff; 
                    if(first_context_was_base) { first_scaff->VS_BeforeScaff = sing_iter->second; }
                    else                       { first_scaff->VS_BeyondScaff = sing_iter->second; }
                }
            }
            else // current_CID == sing_iter->second->VS_Linker
            {
                // The other V_Scaff lies beyond this guy
                if(is_first_scaff)
                {
                    // we don't know who to point to so just save the values
                    first_context_was_base = false;
                    first_scaff = sing_iter->second;
                    is_first_scaff = false;
                }
                else
                {
                    // This is not the first one...
                    first_scaff->VS_Rank++;
                    sing_iter->second->VS_Rank++;
                    
                    sing_iter->second->VS_BeyondScaff = first_scaff;
                    if(first_context_was_base) { first_scaff->VS_BeforeScaff = sing_iter->second; }
                    else                       { first_scaff->VS_BeyondScaff = sing_iter->second; }
                }
            }
        }
        used_conts_iter++;
    }

  // Go through the graph now and make the chains
  // first get all the non-circular chains
  // at the end of this if there are any circles, their rank will be 2
  // all ofther guys will ave rank = -1
    vs_iter = mValidScaffolds.begin();
    while(vs_iter != mValidScaffolds.end())
    {
        if(*vs_iter != NULL)
        {
            if((*vs_iter)->VS_Rank == 1)
            {
                // we've hit the start of a chain
                // does the path through this scaffold
                // agree with the native ordering?
                // 
                // Either the base or the linker must have
                // cont_V_mmap_iter.count == 1
                //
                // Set up the pathAgress variable so we can start walking
                if(1 == cont_V_mmap.count((*vs_iter)->VS_Base))
                {
                    // the BASE context only has one scaff struct
                    // Thus the linker must have 2, B-->L so the path agrees!
                    (*vs_iter)->VS_PathAgrees = true;
                }
                else
                { 
                    (*vs_iter)->VS_PathAgrees = false;
                }

                // make scaffold chains
                std::vector< V_Scaff * > * tmp_scaff = new std::vector< V_Scaff * >;
                V_Scaff * working_scaff = *vs_iter;
                
                do {
                    logInfo("PB: B: " << working_scaff->VS_Base << " L: " << working_scaff->VS_Linker << " PA: " << working_scaff->VS_PathAgrees, 8);
                    tmp_scaff->push_back(working_scaff);
                    // lower the rank so we'll never re-process this guy
                    working_scaff->VS_Rank = -1;
                } while(scaffoldStep(&working_scaff));
                
                // the working scaff is a cap
                mScaffolds.push_back(tmp_scaff);
            }
            else if ((*vs_iter)->VS_Rank == 0)
            {
                // scaffold consists of only two contexts
                std::vector< V_Scaff * > * tmp_scaff = new std::vector< V_Scaff * >;
                tmp_scaff->push_back(*vs_iter);
                mScaffolds.push_back(tmp_scaff);
                (*vs_iter)->VS_Rank = -1;
            }
        }
        vs_iter++;
    }
    
  // Now get the circles
    vs_iter = mValidScaffolds.begin();
    std::map<V_Scaff *, bool> used_map;
    
    while(vs_iter != mValidScaffolds.end())
    {
        if(*vs_iter != NULL)
        {
            if(2 == (*vs_iter)->VS_Rank)
            {
                // we've hit the start of a circular chain
                // 
                // Add it to the used_map
                V_Scaff * working_scaff = *vs_iter;
                used_map.insert(std::pair<V_Scaff *, bool>(working_scaff, true));
                
                // its a circle so we ALWAYS set the first 
                // as path agrees!
                
                // make scaffold chains
                std::vector< V_Scaff * > * tmp_scaff = new std::vector< V_Scaff * >;
                
                // push the first one on
                tmp_scaff->push_back(working_scaff);
                working_scaff->VS_Rank = -1;
                scaffoldStep(&working_scaff);
                
                do {
                    if(used_map.find(working_scaff) != used_map.end())
                        break;
                    tmp_scaff->push_back(working_scaff);
                    working_scaff->VS_Rank = -1;
                } while(scaffoldStep(&working_scaff));
                
                mScaffolds.push_back(tmp_scaff);
            }
        }
        vs_iter++;
    }
}

bool Scaffolder::scaffoldStep(V_Scaff ** workingScaff)
{
    //-----
    // Step one!
    // 
    // Working Scaff must have it's "agree" flag set correctly
    // The "agree" flag of the new working flag will be set here...
    //
    V_Scaff * ret_scaff = NULL;
    if((*workingScaff)->VS_PathAgrees)
    {
        // Running from base -> link
        // Next scaff will be the beyond Node
        ret_scaff = (*workingScaff)->VS_BeyondScaff;
    }
    else
    {
        // Running fromm Linker to base
        // Next Scaff is behind
        ret_scaff = (*workingScaff)->VS_BeforeScaff;
    }

    if(NULL != ret_scaff)
    {
        if(ret_scaff->VS_BeforeScaff == (*workingScaff))
            ret_scaff->VS_PathAgrees = true;
        else
            ret_scaff->VS_PathAgrees = false;
        
        *workingScaff = ret_scaff;
        return true;
    }
    
    // at the end of the chain...
    return false;
}

// scaffolding

bool Scaffolder::scaffoldAll(void)
{
    //-----
    // The main entry for scaffolding
    //
    // see if we're wasing or time
    if(0 == mAllContexts->size())
    {
        logError("No contexts to scaffold!");
        return false;
    }
    else if(1 == mAllContexts->size())
    {
        logInfo("Only 1 context, no need to scaffold!", 1);
        return true;
    }
    
    // how far from the end of a context we are willing to walk.
    uMDInt walk_limit = mDataSet->getHighestUpperCut();
    
    logInfo("Start Scaffolding " << mAllContexts->size() << " contigs", 1);
    logInfo("Walking up to: " << walk_limit << " bases into each Context", 5);
    
    // go through each Context
    std::map<ContextId, GenericNodeId>::iterator all_contexts_iter = mAllContexts->begin();
    std::map<ContextId, GenericNodeId>::iterator all_contexts_last = mAllContexts->end();
    while(all_contexts_iter != all_contexts_last)
    {

// Do initial analysis
        // each context has two capNodes
        mCurrentContext = all_contexts_iter->first;
        NB_ELEM nbe;
        mNodes->getCapBlock_DN(&nbe, mCurrentContext);
        bool is_start = true;
        GenericNodeId master_GID;
        while(mNodes->nextNBElem(&nbe))
        {
            // walk in from each Cap
            GenericNodeElem gne;
            GenericNodeWalkingElem gnwe(DUALNODE);
            GenericNodeId start_GID = nbe.NBE_CurrentNode;
            GenericNodeId nextStart_GID = mNodes->getNullGenericNodeId();
            GenericNodeEdgeElem gnee(DUALNODE);
            if(mNodes->getEdges(&gnee, GN_EDGE_STATE_NTRANS, 0, start_GID))
            {
                nextStart_GID = mNodes->getOlapNode(gnee);
                uMDInt current_step = 0;
                if(mNodes->startWalk(&gnwe, start_GID, nextStart_GID))
                {
                    // process prev_node
                    master_GID = mNodes->getPrevNode(gnwe);
                    processAtMaster(is_start, mNodes->getHead(master_GID), mNodes->isCntxReversed(master_GID), mNodes->getCntxPos(master_GID), mCurrentContext);
                    do {
                        //process current node
                        master_GID = mNodes->getCurrentNode(gnwe);
                        processAtMaster(is_start, mNodes->getHead(master_GID), mNodes->isCntxReversed(master_GID), mNodes->getCntxPos(master_GID), mCurrentContext);
                        if(++current_step > walk_limit) { break; }
                    } while(mNodes->rankStep(&gnwe));
                }
            }
            else
            {
                logError("No edges at CAP");
            }
            is_start = false;
        }
        all_contexts_iter++;
    }
    
    // Summarise the scaffold analysis
    // first sort...
    std::sort(mSimpleScaffolds.begin(), mSimpleScaffolds.end(), S_Scaff::compareSScaffs());

    logSimpleScaffolds(101);
    
    // print the csv file if need be
    if(mDoCsv)
        printCSV();
    
    // delete any entries with less than the required number of members
    purgeSimpleScaffolds();
    
    // tell everyone about it
    logValidScaffolds(10);
    if(mDoScaffGraph)
        printVSGraph();
    
    // make sure everyone has only one in edge and one out edge
    // make the final scaffold lists
    purgeValidScaffolds();

    // tell everyone about it
    logValidScaffolds(6);
    if(mDoScaffGraph)
        printVSGraph();
    
    // print the scaffolds to file
    std::string file_name = mOutputDir + "out" + SAS_DEF_SCAFF_CONTIG_EXT;
    printAllScaffolds(file_name);
    
    printStats();
    
    return true;
}

bool Scaffolder::processAtMaster(bool isStart, GenericNodeId head_GID, bool cntxRev, uMDInt position, ContextId CTXID)
{
    //-----
    // Process all the nodes at a given master
    //
    SC_ELEM sce;
    GenericNodeId single_check_GID;
    
    // This is the local form of getElem which only returns GNs in the head ring
    // which have not yet been set to a Context.
    if(getElem(&sce, head_GID))
    {
        // sce holds the head
        while(getNextElem(&sce)) {
            // sce holds a non-head non-assigned DualNode
            
            // get info about the current Working GID
            GenericNodeId working_GID = sce.SCE_GID;
            FileSetId WFID = mNodes->getFileId(working_GID);
            
            InsertLibrary::OrientationType working_OT = mLongestInserts[WFID];
            bool working_rev = (mNodes->isFileReversed(working_GID) ^ cntxRev);
            
            // only get guys who match the orientation type of the longest insert
            // and even then, only guys who REALLY agree with it.
            // we only want to see overlaps with guys which lie  beyond / behind the current
            // working end. ONLY TYPE 0 and TYPE 2 orientation will make it through
            // 
            if(!mNodes->isDummy(working_GID) && isBeyond(isStart, working_rev, working_OT))
            {
                // get the pair
                GenericNodeId working_pair_GID = mNodes->getPair(working_GID);
                
                // check we haven't used it yet
                if(working_GID < working_pair_GID)
                    single_check_GID = working_GID;
                else
                    single_check_GID = working_pair_GID;
                
                if(mUsedNodes.find(single_check_GID) == mUsedNodes.end())
                {
                    // working_pair_GID has no context ID or position. 
                    // get it's ring and check out where it "could" be located...
                    GenericNodeElem gne_pair;
                    if(mNodes->getElem(&gne_pair, working_pair_GID))
                    {
                        // there may be a number of guys at any position
                        // we could only map the pair there once so we only take one from
                        // each position... (we need to make a universal key here too, becuase we could
                        // have the same key in multiple Contexts...
                        std::map<uMDInt, bool> pos_used_map;
                        bool working_pair_GID_file_reversed = mNodes->isFileReversed(working_pair_GID);
                        
                        // once again, skip past the head...
                        while(mNodes->getNextElem(&gne_pair))
                        {
                            GenericNodeId template_pair_GID = gne_pair.GNE_GID;
                            ContextId template_pair_CTXID = mNodes->getCntxId(template_pair_GID);
                            
                            // check that this guy is mapped to a context, and not the working context
                            if(CTX_NULL_ID != template_pair_CTXID && CTXID != template_pair_CTXID)
                            {
                                // for each guy in the ring with a ContextId
                                uMDInt template_pair_position = mNodes->getCntxPos(template_pair_GID);
                                uMDInt template_pos_cntx_key = template_pair_CTXID.getGuts() * 100000000 + template_pair_position;
                                
                                if(pos_used_map.find(template_pos_cntx_key) == pos_used_map.end())
                                {
                                    // first time we've seen this guy
                                    pos_used_map[template_pos_cntx_key] = true;
                                    
                                    // Use the file reversed for the working_pair, but the context reversed of the template_pair
                                    // because we want to see how the pair WOULD map at various positions in the template_context
                                    bool working_pair_rev = (working_pair_GID_file_reversed ^ mNodes->isCntxReversed(template_pair_GID));
                                    
                                    sMDInt working_eaten = 0;
                                    sMDInt working_pair_eaten = 0;
                                    
                                    bool same_dirn = true;
                                    
                                    if(isStart)
                                    {
                                        //
                                        //   --------------0---->
                                        //               ====>     ----<====----1------>
                                        //   <-------------1-----
                                        //
                                        // OR
                                        //
                                        //   --------------1---->
                                        //               <====     ----====>----0------>
                                        //   <-------------0-----
                                        //
                                        
                                        working_eaten = position;
                                        if(working_pair_rev == working_rev)
                                        {
                                            same_dirn = false;
                                            working_pair_eaten = template_pair_position;
                                        }
                                        else
                                        {
                                            same_dirn = true;
                                            working_pair_eaten = mContexts->getEndPos(template_pair_CTXID) - template_pair_position;
                                        }
                                    }
                                    else
                                    {
                                        //
                                        //   --------------0---->
                                        //               ====>     <----<====----0-----
                                        //   <-------------1-----
                                        //
                                        // OR
                                        //
                                        //   --------------1---->
                                        //               <====     <----====>----1-----
                                        //   <-------------0-----
                                        //
                                        
                                        working_eaten = mContexts->getEndPos(CTXID) - position;
                                        if(working_pair_rev == working_rev)
                                        {
                                            same_dirn = false;
                                            working_pair_eaten = mContexts->getEndPos(template_pair_CTXID) - template_pair_position;
                                        }
                                        else
                                        {
                                            same_dirn = true;
                                            working_pair_eaten = template_pair_position;
                                        }
                                    }
                                    // we need to place some limits on the gap. It can be -ve, but not more than
                                    // the readLength. Otherwise we would have caught it in the overlapperation stage
                                    sMDInt total_eaten = working_eaten + working_pair_eaten + mReadLength;
                                    if((total_eaten <= (sMDInt)(mUpperCuts[WFID])) && (total_eaten >= (sMDInt)(mLowerCuts[WFID])))
                                    {
                                        // This guy links!
                                        S_Scaff * tmp_scaff = new S_Scaff;

                                        tmp_scaff->SS_Base = mCurrentContext;
                                        tmp_scaff->SS_Linker = template_pair_CTXID;
                                        
                                        tmp_scaff->SS_BasePos = position;
                                        tmp_scaff->SS_BaseDist = working_eaten;
                                        tmp_scaff->SS_LinkerPos = template_pair_position;
                                        tmp_scaff->SS_LinkerDist = working_pair_eaten;
                                        tmp_scaff->SS_FileId = WFID;
                                        tmp_scaff->SS_IncludeInCount = mUsableFileSets[WFID];
                                        
                                        tmp_scaff->SS_Gap = (sMDInt)(mMeans[WFID]) - total_eaten;
                                        tmp_scaff->SS_IsBefore = isStart;
                                        tmp_scaff->SS_IsSameDirn = same_dirn;
                                        tmp_scaff->setPairKey();
                                        mSimpleScaffolds.push_back(tmp_scaff);
                                        
                                        // add it to the used nodes list
                                        mUsedNodes[single_check_GID] = true;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            
        }
    }
    return true;
}

// file + IO

void Scaffolder::logFileStats(int logLevel)
{
    //-----
    // Log all the fileStats
    //
    std::map<FileSetId, InsertLibrary::OrientationType>::iterator ins_iter = mLongestInserts.begin();
    logInfo("Stats used in the scaffolding process:", logLevel);
    while(ins_iter != mLongestInserts.end())
    {
        logInfo("FSID: " << ins_iter->first << " O: " << ins_iter->second << " U: " << mUpperCuts[ins_iter->first] << " L: " << mLowerCuts[ins_iter->first] << " M: " << mMeans[ins_iter->first], logLevel);
        ins_iter++;
    }
}

void Scaffolder::logValidScaffolds(int logLevel)
{
    //-----
    // go through and list all the valid scaffolds
    //
    if(willLog(logLevel))
    {
        logInfo("Valid Scaffolds", logLevel);
        logInfo("==============================", logLevel);
        std::vector< V_Scaff * >::iterator vs_iter = mValidScaffolds.begin();
        while(vs_iter != mValidScaffolds.end())
        {
            if(*vs_iter != NULL)
                (*vs_iter)->logContents(logLevel);
            vs_iter++;
        }
        logInfo("==============================", logLevel);
    }
}

void Scaffolder::logSimpleScaffolds(int logLevel)
{
    //-----
    // Do just that
    //
    if(willLog(logLevel))
    {
        logInfo("Simple Scaffolds", logLevel);
        logInfo("==============================", logLevel);
        std::vector< S_Scaff * >::iterator ss_iter = mSimpleScaffolds.begin();
        while(ss_iter != mSimpleScaffolds.end())
        {
            if(NULL != *ss_iter)
                (*ss_iter)->logContents(logLevel);
            ss_iter++;
        }
        logInfo("==============================", logLevel);
    }
}

void Scaffolder::printCSV(void)
{
    //-----
    // Do just that
    //
    std::cout << "\"Base\",\"Linker\",\"IsSameDirn\",\"IsBefore\",\"FileId\",\"BasePos\",\"BaseDist\",\"LinkerPos\",\"LinkerDist\",\"Gap\",\"PairKey\",\"TypeKey\"" << std::endl;
    std::vector< S_Scaff * >::iterator ss_iter = mSimpleScaffolds.begin();
    while(ss_iter != mSimpleScaffolds.end())
    {
        if(NULL != *ss_iter)
            (*ss_iter)->printCSV();
        ss_iter++;
    }
}

void Scaffolder::printVSGraph(void)
{
    //-----
    // print a graphviz style graph of the valid scaffolds 
    // to std::out
    //
    std::cout << "digraph {\n\tnode [shape=ellipse,fixedsize=true,width=3];" << std::endl;
    std::vector< V_Scaff * >::iterator vs_iter = mValidScaffolds.begin();
    while(vs_iter != mValidScaffolds.end())
    {
        if(*vs_iter != NULL)
            (*vs_iter)->printVSGraph();
        vs_iter++;
    }
    std::cout << "}" << std::endl;
}

void Scaffolder::printAllScaffolds(std::string fileName)
{
    //-----
    // Print all the Scaffolds to file
    //
    
    mUsedContexts.clear();
    
    // open the file
    ofstream my_file(fileName.c_str(), ofstream::binary);
    
    // print the multi context scaffolds
    std::vector< std::vector< V_Scaff * > * >::iterator scaff_iter = mScaffolds.begin();
    mScaffNum = 1;
    
    logInfo("=========================", 4);
    
    while(scaff_iter != mScaffolds.end())
    {
        if(NULL != *scaff_iter)
        {
            printScaffold(&my_file, *scaff_iter, mScaffNum);
            mScaffNum++;
        }
        scaff_iter++;
    }
    
    
    // get the unused contexts:
    std::vector<ContextId> unused_contexts;
    // go through each Context
    std::map<ContextId, GenericNodeId>::iterator all_contexts_iter = mAllContexts->begin();
    std::map<ContextId, GenericNodeId>::iterator all_contexts_last = mAllContexts->end();
    while(all_contexts_iter != all_contexts_last)
    {
        if(mUsedContexts.find(all_contexts_iter->first) == mUsedContexts.end())
            unused_contexts.push_back(all_contexts_iter->first);
        all_contexts_iter++;
    }
    
    std::vector<ContextId>::iterator uc_iter = unused_contexts.begin();
    std::vector<ContextId>::iterator uc_last = unused_contexts.end();
    logInfo("Unscaffolded:", 4);
    logInfo("=========================", 4);
    while(uc_iter != uc_last)
    {
        logInfo(*uc_iter, 4);
        std::string sequence;
        std::ostringstream SCAFF_buffer;
        mContexts->getContigSequence(&sequence, 0, 0, false, NULL, false, *uc_iter);
        
        #ifdef SOLID_DATA_
            SCAFF_buffer << ">ScaffoldId:" << mScaffNum << "_LEN_" << sequence.length() << "_Ns_0_CTXTS_1_SINGULAR\n";
        #else
            SCAFF_buffer << ">ScaffoldId:" << mScaffNum << "_LEN_" << sequence.length() << "_Ns_0_GC_" << getGC(&sequence) << "_CTXTS_1_SINGULAR\n";
        #endif
        
        mScaffSizes.push_back(sequence.length());
        
        mContexts->cutRawSeq(&sequence, &SCAFF_buffer);
        my_file << SCAFF_buffer.str();        
        mScaffNum++;
        uc_iter++;
    }

    logInfo("=========================", 4);

    // close the file
    my_file.close();
}

void Scaffolder::printScaffold(ofstream * outFile, std::vector< V_Scaff * > * currentScaff, int scaffNum)
{
    //-----
    // Print this scaffold to file
    //
    // Lots and lots of reversing flags used here...
    // TO recap:
    //
    // |-------------------------------------------------|
    // |  i    |               isBefore                  |
    // |  S    |         1         |          0          |
    // |  S ---|-------------------|---------------------|
    // |  a  1 | ---L-->  ---B-->  |  ---B-->  ---L-->   |
    // |  m ---|-------------------|---------------------|
    // |  e  0 | <--L---  ---B-->  |  ---B-->  <--L---   |
    // |-------------------------------------------------|
    //
    // But now we need to take account of the path agrees flag too
    // This gives:
    //
    // ||                                             pathAgress                                      ||
    // ||                                                                                             ||
    // ||                        1                        ||                     0                    ||
    // ||-------------------------------------------------||------------------------------------------||
    // ||                 Base comes first                ||              Linker comes first          ||
    // ||-------------------------------------------------||------------------------------------------||
    // ||  i    |               isBefore                  ||                isBefore                  ||
    // ||  S    |         1         |          0          ||          1         |          0          ||
    // ||  S ---|-------------------|---------------------||--------------------|---------------------||
    // ||  a  1 | <--B---  <--L---  |  ---B-->  ---L-->   ||  ---L-->  ---B-->  |  <--L---  <--B---   ||
    // ||  m ---|-------------------|---------------------||--------------------|---------------------||
    // ||  e  0 | <--B---  ---L-->  |  ---B-->  <--L---   ||  <--L---  ---B-->  |  ---L-->  <--B---   ||
    // ||-------------------------------------------------||------------------------------------------||
    //
    // And the following logic:
    // 
    // First round:
    // 
    // if(pathAgress)
    // {
    //     PRINT OUT BASE
    //     
    //     if(isBefore) { PRINT RC }
    //     else { PRINT NORMAL }
    // }
    // else
    // {
    //     PRINT OUT LINKER
    //     
    //     if(isBefore == isSame) { PRINT NORMAL }
    //     else { PRINT RC }
    //     
    // }
    // 
    // Subsequent rounds:
    // 
    // if(pathAgress)
    // {
    //     PRINT OUT LINKER
    //     
    //     if(isBefore == isSame) { PRINT RC }
    //     else { PRINT NORMAL }
    // }
    // else
    // {
    //     PRINT OUT BASE
    //     
    //     if(isBefore) { PRINT NORMAL }
    //     else { PRINT RC }
    //     
    // }
    
    logInfo("Scaffold: " << scaffNum, 4);
    logInfo("=========================", 4);
    
    // for walking across the scaffold
    std::vector< V_Scaff * >::iterator cs_iter = currentScaff->begin();
    V_Scaff * prev_scaff = *cs_iter;
    ContextId prev_CTXID = CTX_NULL_ID;
    ContextId curr_CTXID = CTX_NULL_ID;
    GenericNodeId prev_GN;
    GenericNodeId curr_GN;
    
    // main buffer for holding the scaffold sequence
    std::ostringstream MAIN_buffer;
   
    // Calculate overlap or pad with Ns?
    bool use_mid = true;
    int num_Ns = 0;

    // for use in marking and trimming circular contexts
    bool is_circular = false;
    ContextId first_CTXID = CTX_NULL_ID;
    int last_MID_length = 0;
    
    // trimming on the start and end of contigs
    uMDInt trim_last_front = 0;
    uMDInt main_length = 0;
    uMDInt start_1, end_1, start_2, end_2;
    start_1 = end_1 = start_2 = end_2 = 0;
    std::string curr_start_str, prev_end_str;
    
    // relative direction of contigs in the scaffold
    bool prev_isStart_end, prev_rc, current_isStart_start, current_isStart_end, current_rc;
    
    // We are either going to print out the scaffs separated by N's OR we're going to 
    // try to overlap them. We'll try to overlap them if the gap is less than the readLength
    // if an overlap is found, then it needs to be within readLength bases from either end for us to 
    // use it...
    
    // buffer the first Context
    // These flags are used to determine the direction of the context
    // within the scaffold
    // if(pathAgress)
    // {
    //     PRINT OUT BASE
    //     
    //     if(isBefore) { PRINT RC }
    //     else { PRINT NORMAL }
    // }
    // else
    // {
    //     PRINT OUT LINKER
    //     
    //     if(isBefore == isSame) { PRINT NORMAL }
    //     else { PRINT RC }
    //     
    // }
    if(prev_scaff->VS_PathAgrees)
    {
        prev_CTXID = prev_scaff->VS_Base;
        if(prev_scaff->VS_IsBefore)
        {
            logInfo("<--" << prev_scaff->VS_Base << "---", 4);
            prev_isStart_end = true; prev_rc = true;
        }
        else
        {
            logInfo("---" << prev_scaff->VS_Base << "-->", 4);
            prev_isStart_end = false; prev_rc = false;
        }
    }
    else
    {
        prev_CTXID = prev_scaff->VS_Linker;
        if(prev_scaff->VS_IsBefore == prev_scaff->VS_IsSameDirn)
        {
            logInfo("---" << prev_scaff->VS_Linker << "-->", 4);
            prev_isStart_end = false; prev_rc = false;
        }
        else
        {
            logInfo("<--" << prev_scaff->VS_Linker << "---", 4);
            prev_isStart_end = true; prev_rc = true;
        }
    }
    prev_GN = mContexts->getStartGn(prev_CTXID);
    first_CTXID = prev_CTXID;
    
    // Now go through all the remaining contexts in this scaffold
    while(cs_iter != currentScaff->end())
    {
        mUsedContexts.insert(std::pair<ContextId, bool>((*cs_iter)->VS_Base, true));
        mUsedContexts.insert(std::pair<ContextId, bool>((*cs_iter)->VS_Linker, true));
        
        // find the direction of this context within the scaffold
        // the c_end_flags will be used in the next round
        // if(pathAgress)
        // {
        //     PRINT OUT LINKER
        //     
        //     if(isBefore == isSame) { PRINT RC }
        //     else { PRINT NORMAL }
        // }
        // else
        // {
        //     PRINT OUT BASE
        //     
        //     if(isBefore) { PRINT NORMAL }
        //     else { PRINT RC }
        //     
        // }
        if((*cs_iter)->VS_PathAgrees)
        {
            curr_CTXID = (*cs_iter)->VS_Linker;
            if((*cs_iter)->VS_IsBefore == (*cs_iter)->VS_IsSameDirn)
            {
                logInfo("<--" << (*cs_iter)->VS_Linker << "---", 4);
                current_isStart_start = false; current_rc = true;
                current_isStart_end = true;
            }
            else
            {
                logInfo("---" << (*cs_iter)->VS_Linker << "-->", 4);
                current_isStart_start = true; current_rc = false;
                current_isStart_end = false;
            }
        }
        else
        {
            curr_CTXID = (*cs_iter)->VS_Base;
            if((*cs_iter)->VS_IsBefore)
            {
                logInfo("---" << (*cs_iter)->VS_Base << "-->", 4);
                current_isStart_start = true; current_rc = false;
                current_isStart_end = false;
            }
            else
            {
                logInfo("<--" << (*cs_iter)->VS_Base << "---", 4);
                current_isStart_start = false; current_rc = true;
                current_isStart_end = true;
            }
        }
        
        curr_GN = mContexts->getStartGn(curr_CTXID);
        
        // see if these guys overlap.
        // get the middle sequence if they do
        
        // if the gap is less than the read length, (including negative gaps)
        // then we calculate the overlap, otherwise we don't bother...
        if((*cs_iter)->VS_Gap <= (int)mReadLength)
        {
            use_mid = true;
        
            int overlap = 0;
            std::ostringstream MID_buffer;

            // need to know what the minimum length is
            // The trimming langth needs to be greater than or equal
            // to half of the contig length...
            main_length = SAS_DEF_CONT_END_LEN;
            if( (mContexts->getLongestContig(prev_CTXID) / 2) < main_length)
                main_length = (mContexts->getLongestContig(prev_CTXID) / 2);
            if( (mContexts->getLongestContig(curr_CTXID) / 2) < main_length)
                main_length = (mContexts->getLongestContig(curr_CTXID) / 2);
            
            // cut the start and end strings and do smith waterman
            prev_end_str = mContexts->getContigEnd(main_length, prev_isStart_end, prev_rc, prev_CTXID);
            curr_start_str = mContexts->getContigEnd(main_length, current_isStart_start, current_rc, curr_CTXID);
            MID_buffer << scaffoldSW(&overlap, &start_1, &end_1, &start_2, &end_2, prev_end_str, curr_start_str, main_length);
            
            
            if((0 >= overlap) && ((*cs_iter)->VS_Gap > 1))
            {
                // we need to pad gap with Ns
                use_mid = false;
                main_length = 0;
            }
            
            // add this guy to the main buffer
            std::string sequence;
            // we have a choice on which variable we'll choose to tweak.
            // Keep the startGN as the start of the Context always. Use the RC flag to tuen the Context around.
            // getContigSequence(std::string * sequence, int trimFront, int trimEnd, bool doGC, GenericNodeId * startNode, bool doRC, ContextId CTXID)
            //mContexts->getContigSequence(&sequence, trim_last_front, main_length, false, &prev_GN, prev_rc, prev_CTXID);
            mContexts->getContigSequence(&sequence, trim_last_front, main_length, false, NULL, prev_rc, prev_CTXID);
            MAIN_buffer << sequence;
            
            if(!use_mid)
            {
                for(int i = 0; i < (*cs_iter)->VS_Gap; i++)
                {
                    MAIN_buffer << SAS_CE_N_CHAR;
                    num_Ns++;
                }
                // keep track of the last MID added, because this guy MAY be circular
                last_MID_length = (*cs_iter)->VS_Gap;
                
            }
            else
            {
                std::string MID_sequence(MID_buffer.str());
                MAIN_buffer << MID_sequence;
                if(0 >= overlap)
                    num_Ns++;
                // keep track of the last MID added, because this guy MAY be circular
                last_MID_length = MID_sequence.length();
                
            }
            
            // if we trimmed the end of the prev we'll have to trim the 
            // front of the next
            trim_last_front = main_length;
        }
        else
        {
            use_mid = false;
            
            // these guys shouldn't overlap --> pad with Ns
            std::string sequence = "";

            // get the base, accounting for if it needed to be trimmed
            //mContexts->getContigSequence(&sequence, trim_last_front, 0, false, &prev_GN, prev_rc, prev_CTXID);
            mContexts->getContigSequence(&sequence, trim_last_front, 0, false, NULL, prev_rc, prev_CTXID);
            
            MAIN_buffer << sequence;
            
            // make sure the gap is non-negative
            // it should be but hey! -- even if the gap is 0, put one N there...
            int gap = 1;
            if(0 < (*cs_iter)->VS_Gap)
                gap = (*cs_iter)->VS_Gap;
            
            for(int i = 0; i < gap; i++)
            {
                MAIN_buffer << SAS_CE_N_CHAR;
                num_Ns++;
            }
            
            // keep track of the last MID added, because this guy MAY be circular
            last_MID_length = gap;
            
            // next time we won't trim the front...
            trim_last_front = 0;
            
        }
        
        // get set up for the next V_Scaff
        prev_CTXID = curr_CTXID;
        prev_GN = curr_GN;
        prev_isStart_end = current_isStart_end;
        prev_rc = current_rc;
        
        cs_iter++;
    }
    
    std::string scaff_seq;
    
    // Now we need to fix up the last sequence
    if(prev_CTXID == first_CTXID)
    {
        // circular scaffold!
        // the start an end will overlap by a bit. We'll need to cut that off...
        is_circular = true;
        int trimmer = 0;
        
        if(use_mid)
        {
            // need to work out how much to trim
            // MID has been added and will look like:
            //
            // |<--- main_length --->|
            //             |<--- main_length --->|
            // 11111111111111111111111
            //             22222222222XXXXXXXXXXXX
            //                       ^
            //                       |
            //                      end_2
            //
            // Need to remove the Xs
            
            trimmer = main_length - end_2;
        }
        else
        {
            // just remove the last N's we added...
            trimmer = last_MID_length;
        }
        
        scaff_seq = MAIN_buffer.str();
        scaff_seq = scaff_seq.substr(0, scaff_seq.length() - trimmer);
        
    }
    else
    {
        // just tack on the last context
        std::string sequence;
        //mContexts->getContigSequence(&sequence, trim_last_front, 0, false, &prev_GN, prev_rc, prev_CTXID);
        mContexts->getContigSequence(&sequence, trim_last_front, 0, false, NULL, prev_rc, prev_CTXID);
        MAIN_buffer << sequence;
        scaff_seq = MAIN_buffer.str();
        
    }
    
    // now we have the scaffolded sequence!
    std::ostringstream SCAFF_buffer;
    if(is_circular)
    {
        #ifdef SOLID_DATA_
            SCAFF_buffer << ">ScaffoldId:" << scaffNum << "_LEN_" << scaff_seq.length() << "_Ns_" << num_Ns << "_CTXTS_" << (currentScaff->size()) << "_CIRCULAR_SCAFF\n";
        #else
            SCAFF_buffer << ">ScaffoldId:" << scaffNum << "_LEN_" << scaff_seq.length() << "_Ns_" << num_Ns << "_GC_" << getGC(&scaff_seq) << "_CTXTS_" << (currentScaff->size()) << "_CIRCULAR_SCAFF\n";
        #endif
    }
    else
    {
        #ifdef SOLID_DATA_
            SCAFF_buffer << ">ScaffoldId:" << scaffNum << "_LEN_" << scaff_seq.length() << "_Ns_" << num_Ns << "_CTXTS_" << (currentScaff->size() + 1) << "_REGULAR\n";
        #else
            SCAFF_buffer << ">ScaffoldId:" << scaffNum << "_LEN_" << scaff_seq.length() << "_Ns_" << num_Ns << "_GC_" << getGC(&scaff_seq) << "_CTXTS_" << (currentScaff->size() + 1) << "_REGULAR\n";
        #endif
    }
    
    // for calculating stats...
    mScaffSizes.push_back(scaff_seq.length());
    
    mContexts->cutRawSeq(&scaff_seq, &SCAFF_buffer);
    *outFile << SCAFF_buffer.str();
    
    logInfo("=========================", 4);
    
}

void Scaffolder::printStats(void)
{
    int total_bases = 0;
    int n50 = 0;
    int longest_scaffold = 0;
    
    std::sort(mScaffSizes.rbegin(), mScaffSizes.rend());
    std::vector<int>::iterator ss_iter = mScaffSizes.begin();
    std::vector<int>::iterator ss_last = mScaffSizes.end();
    while(ss_iter != ss_last)
    {
        total_bases += *ss_iter;
        if(*ss_iter > longest_scaffold)
            longest_scaffold = *ss_iter;
        ss_iter++;
    }
    
    int halvsies = total_bases / 2;
    ss_iter = mScaffSizes.begin();
    while(ss_iter != ss_last)
    {
        halvsies -= *ss_iter;
        if(halvsies <= 0)
        {
            n50 = *ss_iter;
            break;
        }
        ss_iter++;
    }
    std::cout << "Scaffold stats ~ total scaffolds: " << (mScaffNum - 1) << " n50: " << n50 << " longest: " << longest_scaffold << " total bases: " << total_bases << std::endl;
    logInfo("Scaffold stats ~ total scaffolds: " << (mScaffNum - 1) << " n50: " << n50 << " longest: " << longest_scaffold << " total bases: " << total_bases, 1);
}

