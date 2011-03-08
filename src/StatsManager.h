//            File: StatsManager.h
// Original Author: Michael Imelfort
// --------------------------------------------------------------------
//
// OVERVIEW:
// This file contains the class definitions for the object which
// handles stats for the assembly
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

#ifndef StatsManager_h
 #define StatsManager_h

// system includes
                 
// Local includes
#include "intdef.h"
#include "IdTypeDefs.h"
#include "GenericNodeClass.h"
#include "TreeHistory.h"
#include "Dataset.h"
 
class StatsManager {

public:

  // Constructor/Destructor
    StatsManager(void);
    ~StatsManager(void);
    void initialise(DataSet * dataSet, bool optimiseForArch, unsigned int basesPerInt, uMDInt ignoreTrap);
     
  // get methods
    inline int getReadLength(void)      { return mReadLength; }
    inline int getNumDumped(void)       { return mNumDumped; }
    inline int getNumDupes(void)        { return mNumDupes; }
    inline int getNumReads(void)        { return mNumReads; }
    inline DataSet * getDS(void)        { return mDataSet; }
    inline int getNumRankBands(void) { return mDataSet->getNumRankBands(); }
    inline uMDInt getLowestUpperCut(void) { return mDataSet->getLowestUpperCut(); }
    inline uMDInt getHighestUpperCut(void) { return mDataSet->getHighestUpperCut(); }
    inline uMDInt getConCutoffLength(void) { return (mDataSet->getLowestUpperCut() + getReadLength()); }
    
  // set methods
    inline void incNumDumped(void)      { mNumDumped++; }
    inline void incNumDupes(void)       { mNumDupes++; }
    inline void setNumDupes(int dupes)  { mNumDupes = dupes; }
    inline void setGNC(GenericNodeClass * GNC) { mNodes = GNC; }
    inline void setDS(DataSet * DS) { mDataSet = DS; mUnderShoots = mDataSet->getUnderShoots(); mOverShoots = mDataSet->getOverShoots(); }
    inline void setIgnoreTrap(uMDInt ignoreTrap) { if(fAbs(ignoreTrap) > SAS_DEF_MIN_EDGE_OFFSET) { mIgnoreTrap = fAbs(ignoreTrap); } }
    
  // operations
    std::string getStdoutFromCommand(std::string cmd);              // just what it says
    inline void makeCutOffs(double range) { mDataSet->setCutOffs(range); }
    
    // OLD
    void getStats(std::string fileName, int percentToCut, int whichEntry);            // calculate the means and standard deviations for the assembly
    void getStats(std::string sequence, int percentToCut);          // same as above but takes a string instead
    inline void updateStats(std::string * sequence, ContextId CTXID) { updateStats(sequence, false, true, CTXID); }
    void updateStats(std::string * sequence, bool cutStart, bool cutEnd, ContextId CTXID); // update the means and standard deviations for the assembly based on the sequence
    
    // NEW
    void cumuloGetStats(std::string fileName, int percentToCut, uMDInt minCutoff, int numSeqs);
    void populateStatsVars(int **** orientationArrayPerFs, double *** meanCutoffsPerFs, vector<int> **** sequenceBucketPerFs, std::string sequence);
    
    bool isMappable(GenericNodeId near_GID, GenericNodeId far_GID, bool seq1Reversed, bool seq2Reversed, sMDInt distance1to2); // lets us know if were allowed to map a paired read
    bool isMappable(GenericNodeId GID, InsertLibrary::OrientationType ot, uMDInt dist); // checks to see if this orientation and distance match the stats for the file fro this GID

  // force stuff
    double calcForce(GenericNodeId near_GID, GenericNodeId far_GID, bool seq1Reversed, bool seq2Reversed, sMDInt distance1to2, int rank); // calculate the net force acroos the mapping for this node
    bool updatePerArmMaps(GenericNodeId near_GID, GenericNodeId far_GID, bool seq1Reversed, bool seq2Reversed, sMDInt distance1to2, int rank, GenericNodeId key_GID, std::map<GenericNodeId, double> * numOver, std::map<GenericNodeId, double> * cumOver, std::map<GenericNodeId, double> * numUnder, std::map<GenericNodeId, double> * cumUnder);
    double calculateProfileDistance(int rank, double overShoots, double underShoots); // calculate the distance of the shoots from the profile for the given rank
    double forceEquation(double z_score);                           // where the force for a given z score is actually calculated
    
  // fileIO
    void save(std::string saveFile);                                // save the stats
    void load(std::string loadFile);                                // load the stats

private:
        // settings / parameters
    int mNumReads;                                                  // the number of lines in the data file
    int mReadLength;
    uMDInt mIgnoreTrap;
                
        // for stats
    int mNumDumped;                                                 // the number of reads dumped during loading becuase they contained the same reads
    int mNumDupes;                                                  // the number of DualNodeClasss which have the same ends
    vector<double> mBetas;                                          // equalibrium betas for filesets (by order of insert size)

        // for when working out stats
    GenericNodeClass * mNodes;                                      // a link to the generic node class for this assembly
    DataSet * mDataSet;                                             // for holding all the info about this dataset

        // for each ranked band of insert, what is the average stdev for overshoots and also for undershoots?
    std::map<int, double> * mOverShoots;
    std::map<int, double> * mUnderShoots;
};

#endif //StatsManager_h

