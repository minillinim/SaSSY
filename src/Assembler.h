//            File: Assembler.h
// Original Author: Michael Imelfort
// --------------------------------------------------------------------
//
// OVERVIEW:
// This file contains the class definition and functions for assembling
// short read data
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

#ifndef Assembler_h
 #define Assembler_h

// system includes
#include <map>
#include <vector>

// local includes
#include "StatsManager.h"
#include "Triple.h"
#include "LoggerSimp.h"
#include "IdTypeDefs.h"
#include "GenericNodeClass.h"
#include "ContigClass.h"
#include "ContextClass.h"
#include "Dataset.h"
                 
class Assembler {
public:

  // Constructor/Destructor
    Assembler(void);
    ~Assembler(void);
    void setRunParameters(DataSet * dataSet);
    
    void setOutputDirectory(std::string outputDir);
    inline std::string getOutputDirectory(void) { return mOutputDir; };

    bool intialise(bool strictVal, bool scaffGraph, bool csv, uMDInt ignoreTrap);
    
//********************************************************
  // GENERAL OPERATIONS (USER COMMANDS)
//********************************************************
    // choreographs the assembly process
    void doFullRunAssembley(uMDInt naiveOffset, uMDInt extOffset, bool overlap, sMDInt olapMinOlap, sMDInt olapMaxWastage, uMDInt scaff_min_links, uMDInt scaff_min_ins, double cov_perc);
    // load state from a file skipping stages one and two
    void assembleFromSavedData(std::string loadDir, uMDInt extOffset, bool overlap, sMDInt olapMinOlap, sMDInt olapMaxWastage, uMDInt scaff_min_links, uMDInt scaff_min_ins, double cov_perc);
    // only do the first two stages and then save
    void loadRawDataThenSave(std::string saveDir, uMDInt naiveOffset, uMDInt extOffset);

//********************************************************
  // STAGE WRAPPERS
//********************************************************

    void stage1(void);                                              // LOAD AND PROCESS DATA, MAKE OVERLAPS
    void stage2(void);                                              // MAKING THE REFINED GRAPH AND UN_Contigs
    void stage3(void);                                              // MAKING CLOSURES AND EXTENDING UN_Contigs into MN_Contigs
    void stage4(void);                                              // FINISHING, ANALYSIS AND STATS

//********************************************************
  // STAGE 1        LOAD AND PROCESS DATA, MAKE OVERLAPS
//********************************************************

      // load data
    bool loadRawData(void);                         // load the data from a file
    bool addNode(std::string * forward, std::string * reverse, std::string * insName, std::string * laneNum, std::string * tileNum, std::string * X_Cord, std::string * Y_Cord, std::string * mPlex, FileSetId FSID);

//********************************************************
  // STAGE 2    MAKING THE REFINED GRAPH AND NAIVE CONTIGS
//********************************************************
    
      // making / analysis of UN_Contigs
    void makeContigs(uMDInt * n50, NODE_CLASS nc);                                // make some Contigs for a particular node type
    void makeContigs(uMDInt * n50, NODE_CLASS nc, ContextId CTXID);               // make some Contigs for a particular node type restricted to a context
    void resetUniNodeEdges(void);                                   // reset the uninode edges and delete the contig object

//********************************************************
  // STAGE 3        CONTEXTS
//********************************************************
    
//********************************************************
  // STAGE 4        FINISHING
//********************************************************
    
//********************************************************
  // MISC
//********************************************************

        // file IO
    void saveState(std::string saveFile);                           // save the state of the Assembler to file
    void loadState(std::string loadFile);                           // load an Assembler State from file

protected:

    bool mOptimiseForArch;
    bool mStrict;
    uMDInt mIgnoreTrap;
    
    // directories
    std::string mOutputDir;

    // main objects
    GenericNodeClass * mGenericNodes;                               // abstraction of different node types
    ContigClass * mContigs;                                         // generic contig management class
    ContextClass * mContexts;                                       // object for holding and manipulating contexts
    StatsManager * mStatsManager;                                   // for collecting and parsing all run stats
    DataSet * mDataSet;                                             // for storing all the information about the data

    // offsets
    uMDInt mMaxOffset;                                              // max allowable offset
    uMDInt mMinOffset;                                              // min allowable offset

    uMDInt mNaiveOffset;                                            // offset naive contigs were made at
    uMDInt mExtOffset;                                              // the highest offset used during edge construction
    double mCovCutPercent;                                          // used to work out coverage
    
    // Finishing vars
    bool mOverlaperate;                                             // overlap cut off
    sMDInt mOlapMinOlap;                                            // paa these onto the Context manager
    sMDInt mOlapMaxWastage;

    uMDInt mScaffMinLinks;                                          // scaffolding link cut off
    uMDInt mScaffMinIns;                                            // scaffolding insert size cut off

    bool mScaffGraph;                                               // draw the scaffold graph?
    bool mCsv;                                                      // output scaffold csv?
    
    // crud
    DataSet   mExtraDataSet;
};

#endif //Assembler_h

