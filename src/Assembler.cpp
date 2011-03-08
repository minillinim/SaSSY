//            File: Assembler.cpp
// Original Authors: Michael Imelfort and Dominic Eales
// OVERVIEW:
// Implementation of Assembler methods.
// --------------------------------------------------------------------
// Copyright (C) 2009 2010 2011 Michael Imelfort and Dominic Eales
//
// This file is part of the Sassy Assembler Project.
//
// Sassy is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Sassy is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Sassy.  If not, see <http://www.gnu.org/licenses/>.
// --------------------------------------------------------------------
//
//                        A
//                       A B
//                      A B R
//                     A B R A
//                    A B R A C
//                   A B R A C A
//                  A B R A C A D
//                 A B R A C A D A
//                A B R A C A D A B 
//               A B R A C A D A B R  
//              A B R A C A D A B R A 
//

// system includes
#include <math.h>
#include <sys/statvfs.h>
#include <iomanip>
#include <limits.h>
#include <unistd.h>
#include <vector>
#include <sstream>
#include <cstring>
#include <string>

// local includes
#include "Assembler.h"
#include "GenericNodeClass.h"
#include "ContigClass.h"
#include "ContextClass.h"
#include "StatsManager.h"
#include "Dataset.h"
#include "map_ext.h"
#include "LoggerSimp.h"
#include "IdTypeDefs.h"
#include "default_parameters.h"
#include "intdef.h"
#include "FormattedFileReader.h"
#include "Utils.h"
#include "stlext.h"
#include "nodeblockdef.h"
#include "Scaffolder.h"

  // Constructor/Destructor

Assembler::Assembler(void)
{
    //-----
    // default constructor
    //
    mGenericNodes = NULL;
    mContigs = NULL;
    mContexts = NULL;
    mStatsManager = NULL;
    mDataSet = &mExtraDataSet;
    
    mNaiveOffset = SAS_DEF_UN_OFFSET_DEF_MIN;
    mExtOffset = SAS_DEF_UN_OFFSET_DEF_MAX;
    
    mOptimiseForArch = false;
    mStrict = true;
    mScaffGraph = false;
    mCsv = false;
    mIgnoreTrap = SAS_DEF_MIN_EDGE_OFFSET;
    setOutputDirectory( "./sassyout" );
    
    // by default, let the context manager decide
    mOlapMinOlap = -1;
    mOlapMaxWastage = -1;
    mCovCutPercent = -1;
}

Assembler::~Assembler(void)
{
    //-----
    // default destructor
    //

    // delete the main objects
    logInfo("Deleting main objects" , 3);

    if(mContigs != NULL)
        delete mContigs;
    mContigs = NULL;

    if(mContexts != NULL)
        delete mContexts;
    mContexts = NULL;
    
    if(mGenericNodes != NULL)
        delete mGenericNodes;
    mGenericNodes = NULL;
    
    if(mStatsManager != NULL)
        delete mStatsManager;
    mStatsManager = NULL;
    
    if(mDataSet != &mExtraDataSet)
        delete mDataSet;
    mDataSet = NULL;
}

void Assembler::setRunParameters(DataSet * dataSet)
{
    //-----
    // initialisation
    //

    mDataSet = dataSet;

    // Make stats object
    if(mStatsManager != NULL)
        delete mStatsManager;
    mStatsManager = new StatsManager();
    PARANOID_ASSERT_L4(mStatsManager != NULL);
    mStatsManager->initialise(mDataSet, mOptimiseForArch, RS_BASES_PER_INT, mIgnoreTrap);
}

void Assembler::setOutputDirectory(std::string outputDir)
{
    size_t slashPos = outputDir.rfind( '/' );
    if ( ( slashPos == std::string::npos ) ||
           ( slashPos != outputDir.size()-1 ) )
    {
        outputDir += '/';
    }
    mOutputDir = outputDir;
}

bool Assembler::intialise(bool strictVal, bool scaffGraph, bool csv, uMDInt ignoreTrap) 
{ 
    mStrict = strictVal; 
    mIgnoreTrap = ignoreTrap; 
    mScaffGraph = scaffGraph;
    mCsv = csv;
    
    mDataSet->setIgnoreTrap(mIgnoreTrap);
    return true; 
}

//********************************************************
  // GENERAL OPERATIONS (USER COMMANDS)
//********************************************************

void Assembler::doFullRunAssembley(uMDInt naiveOffset, uMDInt extOffset, bool overlap, sMDInt olapMinOlap, sMDInt olapMaxWastage, uMDInt scaff_min_links, uMDInt scaff_min_ins, double cov_perc)
{
    //-----
    // this is the function called from main
    // this choreographs the entire process
    //
    logInfo("Full assembly started", 1);
    if(naiveOffset >= SAS_DEF_UN_OFFSET_MIN_MIN) { mNaiveOffset = naiveOffset; } else { logError( "Naive offset: " << naiveOffset << " must be greater than or eq: " << SAS_DEF_UN_OFFSET_MIN_MIN); }
    if(extOffset <= SAS_DEF_UN_OFFSET_MAX_MAX) { mExtOffset = extOffset; } else { logError( "Max offset: " << extOffset << " must be less than or eq: " << SAS_DEF_UN_OFFSET_MAX_MAX); }
    
    mOverlaperate = overlap;
    mScaffMinLinks = scaff_min_links;
    mScaffMinIns = scaff_min_ins;
    mOlapMinOlap = olapMinOlap;
    mOlapMaxWastage = olapMaxWastage;
    mCovCutPercent = cov_perc;
    
    stage1();    
    stage2();                                                       // MAKE Naive Contigs
    stage3();                                                       // MAKE CLOSURES and more...
    stage4();                                                       // FINISHING
    logInfo("Assembler exiting", 1);
}

void Assembler::assembleFromSavedData(std::string loadDir, uMDInt extOffset, bool overlap, sMDInt olapMinOlap, sMDInt olapMaxWastage, uMDInt scaff_min_links, uMDInt scaff_min_ins, double cov_perc)
{
    //-----
    // Skip stages one and two, load state from fileName
    //
    logInfo("Assembly from saved data store: " << loadDir + " started." , 1);
    loadState(loadDir);
    
    if(extOffset == mExtOffset)
    {
        // do nothing
    }
    else if (extOffset < mExtOffset)
    {
        logInfo(mExtOffset << " = " << extOffset ,1);
        mExtOffset = extOffset;
        // reset the uninode edges, keep the contig Ids... ...maybe...
        std::cout << "Reset all the labelled edges" << std::endl;
        mGenericNodes->resetUNEdges(true);
        // now we re-label
        std::cout << "Re-Labeling non-transitive edges up to maximum offset: " << mExtOffset << std::endl;
        mGenericNodes->setExtOffset(mExtOffset);        
        mGenericNodes->buildUniNodeGraph(mExtOffset);
    }
    else
    { logError( "Cannot set Max offset greater than previous highest: " << mExtOffset); }
    
    mOverlaperate = overlap;
    mScaffMinLinks = scaff_min_links;
    mScaffMinIns = scaff_min_ins;
    mOlapMinOlap = olapMinOlap;
    mOlapMaxWastage = olapMaxWastage;
    mCovCutPercent = cov_perc;
    
    stage3();
    stage4();
    logInfo("Assembler exiting", 1);
}

void Assembler::loadRawDataThenSave(std::string saveDir, uMDInt naiveOffset, uMDInt extOffset)
{
    //-----
    // do only stages 1 and 2 then save to fileName
    //

    logInfo("Assembly with save state: " << saveDir + " started.", 1);
    if(naiveOffset >= SAS_DEF_UN_OFFSET_MIN_MIN) { mNaiveOffset = naiveOffset; } else { logError( "Naive offset: " << naiveOffset << " must be greater than or eq: " << SAS_DEF_UN_OFFSET_MIN_MIN); }
    if(extOffset <= SAS_DEF_UN_OFFSET_MAX_MAX) { mExtOffset = extOffset; } else { logError( "Max offset: " << extOffset << " must be less than or eq: " << SAS_DEF_UN_OFFSET_MAX_MAX); }
    
    stage1();                                                       // LOAD DATA AND PROCESS
    stage2();                                                       // MAKE Naive Contigs
    saveState(saveDir);
    logInfo("Assembler exiting", 1);
}

//********************************************************
  // STAGE WRAPPERS
//********************************************************

void Assembler::stage1(void)
{
    //-----
    // LOAD AND PROCESS DATA, MAKE OVERLAPS, BASIC CLEANING
    //
    //
    // NONE OF THE NODE LISTS CAN BE TRUSTED AT THIS POINT
    //
    // make the data storage
    
    // create the output directories

    // declare the generic node interace
    std::cout << "Initialising main data structures..." << std::flush;
    mGenericNodes = new GenericNodeClass();
    PARANOID_ASSERT_L4(mGenericNodes != NULL);
    mGenericNodes->initialise((int)(SAS_DEF_ASS_GN_MULT * mStatsManager->getNumReads()));
    mGenericNodes->initialise2(mStatsManager->getReadLength(), mStatsManager->getNumReads(), mNaiveOffset, mExtOffset);
    mStatsManager->setGNC(mGenericNodes);

    logInfo("Successfully initialised main data objects", 2);
    std::cout << "           ...Success!\nStart Loading raw data..." << std::flush;
    
    // load the data
    bool isLoaded = loadRawData();
    if(!isLoaded)
    {
        logError("Failed to load data from dataset");
        std::cout << "                      ...Failed!" << std::endl;
        return;
    }
    else
    {
        logInfo("Loaded dataset sucessfully", 1);
    }

    std::cout << "                      ...Success!\nStart building base graph structures..." << std::flush;
    
    // Make base edges
    mGenericNodes->makeBaseUniNodeEdges();
    
    std::cout << "        ...Success!\nStart basic cleaning..." << std::flush;

    // Data is loaded, duplicates are removed, Edge states and node types can be trusted
    // mark Uninodes with too few edges or with no DualNode
    mGenericNodes->banishSparseFloatingUniNodes();

    std::cout << "                        ...Success!\n---\nGraph building time!\n---" << std::endl;
    logInfo("Stage 1 complete", 1);
}

void Assembler::stage2(void)
{
    //-----
    // MAKING THE REFINED GRAPH AND NAIVE CONTIGS
    //

    std::cout << "Labeling non-transitive edges up to maximum offset: " << mNaiveOffset << std::endl;
    
    // select from raw edges to make adjacent edges (ie. build the actual graph)
    mGenericNodes->buildUniNodeGraph(mNaiveOffset);

    // set this amount as the naive offset:
    mGenericNodes->setNaiveOffset(mNaiveOffset);
    
    // optional, validate the graph structure
    //mGenericNodes->validateDataStructs();

    std::cout << "Cleaning and making Naive contigs" << std::endl;
    // clean the naive UniNodeId
    mGenericNodes->cleanUniNodeGraph(SAS_DEF_UN_SHAVE_DEPTH);
    
    // optional, validate the graph structure
    //mGenericNodes->validateDataStructs();
   
    // make the contigs
    if(mContigs != NULL)
        delete mContigs;
    mContigs = new ContigClass();
    PARANOID_ASSERT_L4(mContigs != NULL);
    mContigs->initialise(SAS_DEF_ASS_CONTIG_SIZE);
    mContigs->initialise2(mGenericNodes, mStatsManager, UNINODE);
    mContigs->splice();
    mContigs->sortContigs();

    // print them to standard file
    std::string file_name = mOutputDir + "out" + SAS_DEF_NAIVE_CONTIG_EXT;
    mContigs->printContigs(file_name);

    // print out the naive contig stats
    mContigs->makeContigStats();
    mContigs->writeNaiveCoverageMap(SAS_DEF_ASS_COV_FILE);
    std::cout << "Naive contigs stats ~ total contigs: " << mContigs->getNumContigs() << ", n50: " << mContigs->getN50() << ", longest: " << mContigs->getLongestContigLength() << ", total bases: " << mContigs->getTotalBases() << std::endl;
    logInfo("Naive contigs stats ~ total contigs: " << mContigs->getNumContigs() << ", n50: " << mContigs->getN50() << ", longest: " << mContigs->getLongestContigLength() << ", total bases: " << mContigs->getTotalBases(), 1);

    // get the stats for this assembly
    mStatsManager->cumuloGetStats(file_name, 15, mDataSet->getHighestUpperCut(), 10);
    
    // make supplementary naive contigs
    std::string supp_file_name = mOutputDir + "out" + SAS_DEF_SUPP_CONTIG_EXT;
    int num_supp = 0;
    num_supp = mContigs->makeSupplementaryNaiveContigs(supp_file_name);
    std::cout << "Made: " << num_supp << " supplementary contigs" << std::endl;
    
    // reset the uninode edges, keep the contig Ids... ...maybe...
    std::cout << "Reset all the labelled edges" << std::endl;
    mGenericNodes->resetUNEdges(true);

    // then we delete the contig object becuase it is now useless
    if(mContigs != NULL)
        delete mContigs;
    mContigs = NULL;
    
    // optional, validate the graph structure
    //mGenericNodes->validateDataStructs();

    // now we re-label
    std::cout << "Labeling non-transitive edges up to maximum offset: " << mExtOffset << std::endl;
    mGenericNodes->buildUniNodeGraph(mExtOffset);

    // optional, validate the graph structure
    //mGenericNodes->validateDataStructs();
    
    logInfo("Stage 2 complete", 1);
}

void Assembler::stage3(void)
{
    //-----
    // MAKING CLOSURES/CONTEXTS
    //
    // we assume that the naive contigs are sorted.
    // so we start with the longest naive contig
    //
    // get the stats for this assembly
    
    int number_naive_contigs;
    int number_supp_contigs;

    std::string naive_file_name= mOutputDir + "out" + SAS_DEF_NAIVE_CONTIG_EXT;
    std::string supp_file_name = mOutputDir + "out" + SAS_DEF_SUPP_CONTIG_EXT;
    
    // get the number of naive contigs
    std::cout << "Loading seed contigs from: " << naive_file_name<< std::endl;
    std::string line_count_command = "grep \">\" " + naive_file_name+ " | wc -l";
    std::string command_output = mStatsManager->getStdoutFromCommand(line_count_command);
    // string to int
    std::istringstream buffer(command_output);
    buffer >> number_naive_contigs;
    logInfo("There are: " << number_naive_contigs << " naive contig(s) in: " << naive_file_name, 2);

    // get the number of supplementary contigs
    std::cout << "Loading supplementary contigs from: " << supp_file_name<< std::endl;
    line_count_command = "grep \">\" " + supp_file_name+ " | wc -l";
    command_output = mStatsManager->getStdoutFromCommand(line_count_command);
    // string to int
    std::istringstream buffer2(command_output);
    buffer2 >> number_supp_contigs;
    logInfo("There are: " << number_supp_contigs << " supplementary contig(s) in: " << supp_file_name, 2);

    // get the stats for the naive contigs given... ( dataset will automagically rank these guys)
    mStatsManager->cumuloGetStats(naive_file_name, 20, mDataSet->getHighestUpperCut(), 10);
    
    // set the insert libraries to a string. Then we can reset quickly laterz
    mDataSet->setString();

    // kill any existing context managers
    // and make a new one!
    if(mContexts != NULL)
        delete mContexts;
    mContexts = new ContextClass();
    PARANOID_ASSERT_L4(mContexts != NULL);
    mContexts->initialise((int)SAS_DEF_ASS_CONTEXT_SIZE);
    mContexts->setObjectPointers(mGenericNodes, mStatsManager);
    mContexts->setStrict(mStrict);
    mContexts->setOffsets(mExtOffset, mNaiveOffset);

    if(-1 != mCovCutPercent)
        mContexts->readNaiveCoverageMap(mCovCutPercent, SAS_DEF_ASS_COV_FILE);
    
    // now make the contexts
    std::string ext_file_name = mOutputDir + "out";
    
    // make as many doubly mapped as we can from the naive file
    logInfo("Start making double mapped naive contigs" ,2);
    std::cout << "Start making double mapped naive contigs" << std::endl;
    mContexts->makeDualContexts(ext_file_name, number_naive_contigs, true, false);
    
    // make as many as we can from the supplementary file
    logInfo("Start making double mapped supplementary contigs" ,2);
    std::cout << "Start making double mapped supplementary contigs" << std::endl;
    mContexts->makeDualContexts(ext_file_name, number_supp_contigs, false, true);

    
    logInfo("Stage 3 complete", 1);
}

void Assembler::stage4(void)
{
    //-----
    // FINISHING
    //
    // locate and consolidate overlaps in contigs

    int n50, total_bases, longest_contig, num_contigs;
    
    std::cout << "Overlaperating extended contigs" << std::endl;
    std::cout << "Reticulating splines..." << std::endl;

    std::string advan_file_name = mOutputDir + "out" + SAS_DEF_ADVANCED_CONTIG_EXT;
    std::string final_file_name = mOutputDir + "out" + SAS_DEF_FINAL_CONTIG_EXT;
    
    // print all the pre-overlaperated contigs to file
    mContexts->makeAllContigs();
    // Scaffolder will use these guy, so renumber the Contexts
    // set the start and end GenericNodes
    if(!mOverlaperate && (0 != mScaffMinLinks))
        mContexts->renumber(true);
    mContexts->printAllContigs(advan_file_name, &n50, &total_bases, &longest_contig, &num_contigs);
    std::cout << "Pre_olap Contigs stats ~ total contigs: " << num_contigs << " n50: " << n50 << " longest: " << longest_contig << " total bases: " << total_bases << std::endl;
    logInfo("Pre_olap Contigs stats ~ total contigs: " << num_contigs << " n50: " << n50 << " longest: " << longest_contig << " total bases: " << total_bases, 1);
    
    // overlaperate
    if(mOverlaperate)
    {
        // get the stats for the advanvced contigs given...
        mStatsManager->cumuloGetStats(advan_file_name, 20, mDataSet->getHighestUpperCut(), 10);
        
        mContexts->overlaperate(mOlapMinOlap, mOlapMaxWastage);
        
        // print all the overlaperated contigs to file
        mContexts->makeAllContigs();
        // If we're going to scaffold, renumber the Contexts
        // set the start and end GenericNodes
        if(0 != mScaffMinLinks)
            mContexts->renumber(true);
        mContexts->printAllContigs(final_file_name, &n50, &total_bases, &longest_contig, &num_contigs);
        std::cout << "Final Contigs stats ~ total contigs: " << num_contigs << " n50: " << n50 << " longest: " << longest_contig << " total bases: " << total_bases << std::endl;
        logInfo("Final Contigs stats ~ total contigs: " << num_contigs << " n50: " << n50 << " longest: " << longest_contig << " total bases: " << total_bases, 1);
    }
    
    // The scaffolder
    if(0 != mScaffMinLinks)
    {
        if(mOverlaperate)
        {
            // get the stats for the final contigs given...
            mStatsManager->cumuloGetStats(final_file_name, 20, mDataSet->getHighestUpperCut(), 10);
        }
        else
        {   
            // get the stats for the advanvced contigs given...
            mStatsManager->cumuloGetStats(advan_file_name, 20, mDataSet->getHighestUpperCut(), 10);
        }
        
        Scaffolder mScaff(mOutputDir, mScaffMinLinks, mScaffMinIns,  mScaffGraph, mCsv);
        mScaff.setObjectPointers(mGenericNodes, mStatsManager, mContexts);
        mScaff.scaffoldAll();
    }
    
    logInfo("Stage 4 complete", 1);
}

//********************************************************
  // STAGE 1        LOAD AND PROCESS DATA, MAKE OVERLAPS
//********************************************************

struct LoadRawData_FFR_CallbackData {
    // Global to all file-sets
    uMDInt      totalReads;
    Assembler * pAssembler;
    uint_16t    readLength;
    uMDInt      lastPrintVal;

    // File-set specific
    std::map<std::string,std::string> * pUnpairedReads;
    FileSetId                           fileSetId;
    uMDInt                              maxNumReads;
    uMDInt                              numReadsFromFileset;
};


bool LoadRawData_FFR_Callback(std::map<std::string,std::string> recordInfo, void *callbackdata)
{
    // get a callback data pointer
    LoadRawData_FFR_CallbackData * pCbData = (LoadRawData_FFR_CallbackData*)callbackdata;
    std::map<std::string,std::string>::iterator mapIter;

    // create a unique read ID
#define READ_ID_SEPARATOR "-"
    std::string readId = "";
    if("UNKNOWN" == recordInfo["unique-id"])
    {
        readId =
            recordInfo["instrument-name"] + READ_ID_SEPARATOR +
            recordInfo["lane-number"] + READ_ID_SEPARATOR +
            recordInfo["tile-number"] + READ_ID_SEPARATOR +
            recordInfo["x-coord"] + READ_ID_SEPARATOR +
            recordInfo["y-coord"] + READ_ID_SEPARATOR +
            recordInfo["multiplex-index"];
    }
    else
    {
        readId = recordInfo["unique-id"];
    }

    // trim the read
    recordInfo["sequence-data"].erase( pCbData->readLength );

    // check if the other pair is in the map
    mapIter = pCbData->pUnpairedReads->find(readId);

    if ( mapIter != pCbData->pUnpairedReads->end() )
    {
        // we found it!! --> add the pair
        std::string * forward;
        std::string * reverse;

        if("UNKNOWN" == recordInfo["pair-reference"])
        {
            // 2nd read in is reverse!
            forward = &(mapIter->second);
            reverse = &recordInfo["sequence-data"];
        }
        else
        {
            // assuming pair references of 1 or 2
            unsigned int pairRef;
            from_string<unsigned int>(pairRef, recordInfo["pair-reference"], std::dec);
            if(2 == pairRef)
            {
                forward = &(mapIter->second);
                reverse = &recordInfo["sequence-data"];
            }
            else
            {
                forward = &recordInfo["sequence-data"];
                reverse = &(mapIter->second);
            }
        }

        // add the node
        pCbData->pAssembler->addNode( forward,
                                      reverse,
                                      &recordInfo["instrument-name"],
                                      &recordInfo["tile-number"],
                                      &recordInfo["lane-number"],
                                      &recordInfo["x-coord"],
                                      &recordInfo["y-coord"],
                                      &recordInfo["multiplex-index"],
                                      pCbData->fileSetId );

        // matched this sequence to it's pair so delete it from the unpaired list
        pCbData->pUnpairedReads->erase( mapIter );

        // increment number of paired reads
        pCbData->totalReads++;
        pCbData->numReadsFromFileset++;
        
        // break out if we've read all our required reads
        if ( pCbData->numReadsFromFileset == pCbData->maxNumReads ) {
            return false;
        }

        // print out to user
#if SIZE_OF_INT == 64
# define CHECKVAL 0xFFFFFFFFFFFC0000
#else
# define CHECKVAL 0xFFFC0000
#endif
        if ( pCbData->lastPrintVal != (pCbData->totalReads & CHECKVAL) ) {
            logInfo( "... " << pCbData->totalReads << " paired reads loaded", 2);
            pCbData->lastPrintVal = (pCbData->totalReads & CHECKVAL);
        }
    }
    else {
        // else didn't find it's pair in the unpaired list
        // add the sequence to the unpaired list
        (*(pCbData->pUnpairedReads))[readId] = recordInfo["sequence-data"];
    }
    
    /* Other fields we could use.... for later
    recordInfo["quality-data"]
    recordInfo["quality-format"]
    */
    return true;
}

    // loading - making overlaps
bool Assembler::loadRawData(void)
{
    LoadRawData_FFR_CallbackData cbData;
    std::map<std::string,std::string> unpairedReads;
    std::vector<FileSet> * pFileSets;
    bool ffrRetVal;

    cbData.pAssembler = this;
    cbData.totalReads = 0;
    cbData.pUnpairedReads = &unpairedReads;
    cbData.readLength = mDataSet->getReadLength();
    cbData.lastPrintVal = 0;

    logInfo( "Begin load data (effective data-set read length: " << cbData.readLength << ")", 1 );

    // loop on all file sets
    pFileSets = mDataSet->getFileSets();
    V_FOR_EACH( (*pFileSets), FileSet, fsIter ) {

        FormattedFileReader ffr;

        logInfo( "Loading paired reads from file-set `" << fsIter->GetFullPathname() << "'", 1 );

        // init the file specific variables
        cbData.pUnpairedReads->clear();
        cbData.fileSetId = fsIter->getFSID();
        cbData.numReadsFromFileset = 0;
        cbData.maxNumReads = mDataSet->getMaxReadsForFileSet(cbData.fileSetId);

        // load the data
        
        ffrRetVal = ffr.LoadFileset( (*fsIter), LoadRawData_FFR_Callback, &cbData);
        if ( ffrRetVal == false ) {
            logInfo( "Skipping File-set.. cannot process `" << fsIter->GetFullPathname() << "'", 1 );
            continue;
        }

        logInfo( "Loaded " << cbData.numReadsFromFileset << " paired reads from file-set `" << fsIter->GetFullPathname() << "'", 1 );
    }

    logInfo( "Total paired reads loaded: " << cbData.totalReads, 1 );
    
    return true;
}

bool Assembler::addNode(std::string * forward, std::string * reverse, std::string * insName, std::string * tileNum, std::string * laneNum, std::string * X_Cord, std::string * Y_Cord, std::string * mPlex, FileSetId FSID)
{
    //-----
    // we always add both the read and it's pair and the reverse (pair as a read and read as a pair).
    // that is we consider forward with reverse a pair, and the opposite too
    //
    // this function takes a foray through make edges before it returns
    //
    // In the instance that only one string loads then we don't bother to erase it. However
    // this UniNode will have no accompanying DualNode. So we must search for these and clean them
    // From the dataset just after we call this function. Chances are we'll see them in this function
    //
    //

    return mGenericNodes->addNode(forward, reverse, insName, tileNum, laneNum, X_Cord, Y_Cord, mPlex, FSID);
}

//********************************************************
  // STAGE 2    MAKING THE REFINED GRAPH AND NAIVE CONTIGS
//********************************************************

//********************************************************
  // STAGE 3        CLOSURES/CONTEXTS
//********************************************************

//********************************************************
  // STAGE 4        FINISHING
//********************************************************

//********************************************************
  // END STAGES
//********************************************************

    // fileIO

void Assembler::saveState(std::string saveDir)
{
    //-----
    // save an StatsManager to file
    //
    logInfo("Start saving assembly in directory: " << saveDir, 1);
    
    // we assume the directory has already been made to accommodate the logfile...
    // still we will give it a go here to be sure...
    RecursiveMkdir(saveDir);
    saveDir = saveDir + "/";
    
    ofstream myFile((saveDir + "Assembler").c_str(), ofstream::binary);
    
    myFile.write(reinterpret_cast<char *>(&mNaiveOffset), sizeof(uMDInt));
    myFile.write(reinterpret_cast<char *>(&mExtOffset), sizeof(uMDInt));
    
    // close the file
    myFile.close();

    logInfo("Saving GenericNodes", 2);
    if(mGenericNodes != NULL)
        mGenericNodes->save(saveDir + "GenericNodes");

    logInfo("Saving Statistics", 2);
    if(mStatsManager != NULL)
        mStatsManager->save(saveDir + "StatsManager");
    
    logInfo("Saving DataSet", 5);
    if(mDataSet != NULL)
        mDataSet->save(saveDir + "DataSet");
    
    logInfo("Save complete", 1);
}

void Assembler::loadState(std::string loadDir)
{
    //-----
    // Load an StatsManager from File
    //
    logInfo("Start loading assembly from directory: " << loadDir, 1);
    loadDir = loadDir + "/";
    
    // Open file to store local vars...
    ifstream myFile((loadDir + "Assembler").c_str(), ifstream::binary);

    // load these guys...
    myFile.read(reinterpret_cast<char *>(&mNaiveOffset), sizeof(uMDInt));
    myFile.read(reinterpret_cast<char *>(&mExtOffset), sizeof(uMDInt));
    
    // close the file
    myFile.close();

    // load the big guns
    logInfo("Loading GenericNodes", 3);
    if(mGenericNodes != NULL)
        delete mGenericNodes;
    mGenericNodes = new GenericNodeClass();
    PARANOID_ASSERT_L4(mGenericNodes != NULL);
    mGenericNodes->initialise(loadDir + "GenericNodes");
    mGenericNodes->initialiseOnLoad();
    
    logInfo("Loading Statistics", 3);
    if(mStatsManager != NULL)
        delete mStatsManager;
    mStatsManager = new StatsManager();
    PARANOID_ASSERT_L4(mStatsManager != NULL);
    mStatsManager->load(loadDir + "StatsManager");
    mStatsManager->setGNC(mGenericNodes);
    mStatsManager->setIgnoreTrap(mIgnoreTrap);
    
    if(mDataSet != &mExtraDataSet)
        delete mDataSet;
    
    mDataSet = new DataSet();
    PARANOID_ASSERT_L4(mDataSet != NULL);
    mDataSet->load(loadDir + "DataSet");
    
    // set the dataSet for the statsmanager
    mStatsManager->setDS(mDataSet);
    mDataSet->setIgnoreTrap(mIgnoreTrap);
    
    logInfo("Ready to rock and roll...", 2);
}

