//            File: StatsManager.cpp
// Original Author: Michael Imelfort
// --------------------------------------------------------------------
//
// OVERVIEW:
// Implementation of StatsManager methods.
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
#include <cmath>
#include <sys/statvfs.h>
#include <iomanip>
#include <limits.h>
#include <unistd.h>
#include <sstream>
#include <string.h>
 
// local includes
#include "StatsManager.h"
#include "LoggerSimp.h"
#include "intdef.h"
#include "IdTypeDefs.h"
#include "default_parameters.h"
#include "NodePositionMapClass.h"
#include "GenericNodeClass.h"
#include "Dataset.h"

using namespace std;

  // Constructor/Destructor
StatsManager::StatsManager(void)
{
    //-----
    // default constructor
    //

    // by default use everything
    mIgnoreTrap = SAS_DEF_MIN_EDGE_OFFSET;

    // write in the input vars
    mReadLength = 0;

    // initialise to 0
    mNumDumped = 0;
    mNumDupes = 0;

    // set the GenericNodeClass to NULL
    mNodes = NULL;
    mDataSet = NULL;

    mOverShoots = NULL;
    mUnderShoots = NULL;
}

StatsManager::~StatsManager(void)
{
    //-----
    // default destructor
    //
}

void StatsManager::initialise(DataSet * dataSet, bool optimiseForArch, unsigned int basesPerInt, uMDInt ignoreTrap)
{
    //-----
    // initialise some of the variables
    //
    mDataSet    = dataSet;
    mNumReads   = mDataSet->getTotalNumReads();
    mReadLength = mDataSet->calculateReadLength(optimiseForArch, basesPerInt);
    if(fAbs(ignoreTrap) > SAS_DEF_MIN_EDGE_OFFSET) { mIgnoreTrap = fAbs(ignoreTrap); }
    
    mUnderShoots = mDataSet->getUnderShoots();
    mOverShoots = mDataSet->getOverShoots();

    PARANOID_ASSERT_PRINT_L4(NULL != mOverShoots, "OS- NULL");
    PARANOID_ASSERT_PRINT_L4(NULL != mUnderShoots, "US- NULL");
}

  // operations
    
std::string StatsManager::getStdoutFromCommand(std::string cmd)
{
  // setup
    std::string data;
    FILE *stream;
    char buffer[100];

  // do it
    stream = popen(cmd.c_str(), "r");
    while ( fgets(buffer, 100, stream) != NULL )
        data.append(buffer);
    pclose(stream);

  // exit
    return data;
}

//*************************************************************************************
// OLD STATS
//*************************************************************************************

void StatsManager::getStats(std::string fileName, int percentToCut, int whichEntry)
{
    //-----
    // calculate the means and standard deviations for the assembly
    // we do this using the longest contig
    // Kinda assumes that we're dealing with naive contigs only
    //

    logInfo("Calculating mean insert lengths using sequence: " << whichEntry << " in: ", 2);
    logInfo(fileName, 2);
    logInfo("Original estimates: ", 2);
    mDataSet->logFileStats(2);

    // get the correct sequence from the fasta file
    std::string con_seq;
    std::string sequence = "";
    fstream fasta_file;
    int f_counter = whichEntry;
    bool in_fasta = false;
    
    fasta_file.open(fileName.c_str(), ios::in);
    if(fasta_file.is_open())
    {
        std::string line = "";
        stringstream buffer;
        
        while(getline(fasta_file , line))
        {
            if(line.substr(0, 1) == ">")
            {
                if(in_fasta)    // we are done!
                {
                    sequence = buffer.str();
                    fasta_file.close();
                    break;
                }
                // this is a fasta identifier
                f_counter--;
                if(0 == f_counter)
                {
                    // this is the right one!
                    in_fasta = true;
                    // skip to the next line
                    if(!getline(fasta_file , line)) { fasta_file.close(); return; }
                }
            }
            if(in_fasta)
                buffer << line;
        }
        if(sequence == "")
        {
        // only one sequence in the file
            sequence = buffer.str();
        }        
    }
    else
    {
        logError("Could not open file: " << fileName);
        return;
    }
    if(!in_fasta)
    {
        logError("Sequence: " << whichEntry << " not found");
        fasta_file.close();
        return;
    }
    getStats(sequence, percentToCut);
}

void StatsManager::getStats(std::string sequence, int percentToCut)
{
    //-----
    // calculate the means and standard deviations for the assembly
    // we do this using the longest contig
    // Kinda assumes that we're dealing with naive contigs only
    //

    logInfo("Stats making sequence is: " << sequence.length() << " long", 5);
     
    // map all the head GIDs to the sequence we just gotted
    NodePositionMapClass npm;
    npm.initialise(SAS_DEF_CTX_NPM_SIZE);
    vector<GenericNodeId> mapped_nodes;
    vector<uMDInt> mapped_positions;
    
    int seq_length = sequence.length() - mReadLength + 1;
    int seq_used = 0;
    uMDInt pos = (uMDInt)(mReadLength / 2);
    sMDInt tdist;
    uMDInt dist;
    std::string kmer;
    GenericNodeId GID;
    
    while(seq_used < seq_length)
    {
        kmer = sequence.substr(seq_used, mReadLength);
        if(mNodes->getGn(&GID, &kmer))
        {
            NPMHeadElem tmp_npm_elem;
            tmp_npm_elem.NPMHE_GID = GID;
            tmp_npm_elem.NPMHE_Position = pos;
            if(mNodes->getSequence(GID) != kmer)
                tmp_npm_elem.NPMHE_Reversed = true;
            else
                tmp_npm_elem.NPMHE_Reversed = false;
            npm.addElem(&tmp_npm_elem, GID);
            mapped_nodes.push_back(GID);
            mapped_positions.push_back(pos);
        }
        seq_used++;
        pos++;
    }

    //
    // if there are two or three libraries we need two or three means
    // so we need to work out which container to put the information into
    // do this:
    //
    //  0----------A---------------------------------------B----------------------
    //                        C
    //                        |
    //                     cut here
    //
    // Such that 0A:0B == AC:CB
    //
    // pivot on these values
    //

    // we don't need to know how many libraries we're dealing with
    // we just process for each fileset separately

    logInfo("Start mapping", 6);
    
    std::vector<FileSet> * all_fs = mDataSet->getFileSets();
    std::vector<FileSet>::iterator all_fs_iter = all_fs->begin();
    while(all_fs_iter != all_fs->end())
    {
        // get the current FSID
        FileSetId current_fsid = all_fs_iter->getFSID();
        
        // we need to know how many insert libraries there are in this file set
        std::vector<InsertLibrary> * all_ins_libs = all_fs_iter->getInsLibs();
        int num_libraries = all_ins_libs->size();
        
        logInfo("Calculating stats for fileSet: " << current_fsid << " with: " << num_libraries << " insert libraries", 4);

        // use this data structure to work out the orientation
        // type of each library
        int ** orientation_array = new int * [num_libraries];
        PARANOID_ASSERT_L3(orientation_array != NULL);

        for(int i = 0; i < num_libraries; i++)
        {
            orientation_array[i] = new int[4];
            PARANOID_ASSERT_L3(orientation_array[i] != NULL);

            for(int j = 0; j < 4; j++)
            {
                orientation_array[i][j] = 0;
            }
        }

        // use these data structures to work out distribution stats
        double * mean_cutoffs = new double[num_libraries];
        PARANOID_ASSERT_L3(mean_cutoffs != NULL);

        vector<int> ** sequence_bucket = new vector<int>*[num_libraries];
        PARANOID_ASSERT_L3(sequence_bucket != NULL);

        // we assume that the vector of insert libraries is sorted in order of ascending mean
        double * means = new double[num_libraries];
        PARANOID_ASSERT_L3(means != NULL);

        int index = 0;
        std::vector<InsertLibrary>::iterator all_ins_iter = all_ins_libs->begin();
        while(all_ins_iter != all_ins_libs->end())
        {
            means[index] = all_ins_iter->_mean;
            index++;
            all_ins_iter++;
        }

        int bucket_counter = 0;
        // we'll only go in this while loop when there is more than one library
        while(bucket_counter < num_libraries - 1)
        {
            mean_cutoffs[bucket_counter] = (means[bucket_counter+1] - means[bucket_counter]) / (means[bucket_counter+1] / means[bucket_counter] + 1) + means[bucket_counter];
            //mean_cutoffs[bucket_counter] = (means[bucket_counter+1] - means[bucket_counter]) / 2 + means[bucket_counter];
            sequence_bucket[bucket_counter] = new vector<int>;
            PARANOID_ASSERT_L3(sequence_bucket[bucket_counter] != NULL);

            bucket_counter++;
        }

        // do the last bucket and cut off
        mean_cutoffs[bucket_counter] = HUGE_VAL;
        sequence_bucket[bucket_counter] = new vector<int>;
        PARANOID_ASSERT_L3(sequence_bucket[bucket_counter] != NULL);

    
        // delete the temp holder
        delete [] means;
        means = NULL;
        
        // go through all the nodes and try to map the GIDs
        vector<GenericNodeId>::iterator node_iter = mapped_nodes.begin();
        vector<GenericNodeId>::iterator node_last = mapped_nodes.end();
        vector<uMDInt>::iterator pos_iter = mapped_positions.begin();
        GenericNodeElem gn_elem;

        // the positions of the nodes in the context
        NPMHeadElem near_npm_elem, far_npm_elem;
        GenericNodeId far_head, near_GID, far_GID;
    
        while(node_iter != node_last)
        {
            // get the right head
            if(npm.getHead(&near_npm_elem, *node_iter))
            {
                while(near_npm_elem.NPMHE_Position != *pos_iter)
                {
                    npm.getNextHead(&near_npm_elem);
                }
            }
        
            // go through all the Dualnodes at this position
            if(mNodes->getElem(&gn_elem, *node_iter))
            {
                // the elem now holds only the head node
                while(mNodes->getNextElem(&gn_elem))
                {
                    // the elem now holds a non-head node - check to see it was from the current file
                    near_GID = gn_elem.GNE_GID;
                    if(mNodes->getFileId(near_GID) == current_fsid)
                    {
                        // get the paired read
                        far_GID = mNodes->getPair(near_GID);
                        far_head = mNodes->getHead(far_GID);
                        if(npm.getHead(&far_npm_elem, far_head))
                        {
                            do {
                                tdist = (sMDInt)far_npm_elem.NPMHE_Position - (sMDInt)near_npm_elem.NPMHE_Position;
                                dist = (uMDInt)(fAbs(tdist));
                            } while(npm.getNextHead(&far_npm_elem));
                        }

                        
                        if(npm.getHead(&far_npm_elem, far_head))
                        {
                            // both guys lie on the same sequence, get the distance and put it in the right hole
                            // also get the absolute value of the true distance
                            tdist = (sMDInt)far_npm_elem.NPMHE_Position - (sMDInt)near_npm_elem.NPMHE_Position;
                            dist = (uMDInt)(fAbs(tdist));

                            int count_down = num_libraries - 2;
                            bool added = false;
                            while(count_down >= 0)
                            {
                                if(dist > mean_cutoffs[count_down])
                                {
                                    (sequence_bucket[count_down + 1])->push_back(dist);
                                    orientation_array[count_down + 1][all_fs_iter->calcOrientationType(mNodes->isFileReversed(near_GID) ^ near_npm_elem.NPMHE_Reversed, mNodes->isFileReversed(far_GID) ^ far_npm_elem.NPMHE_Reversed, tdist)]++;
                                    added = true;
                                    break;
                                }
                                count_down--;
                            }
                            if(!added)
                            {
                                (sequence_bucket[0])->push_back(dist);
                                orientation_array[0][all_fs_iter->calcOrientationType(mNodes->isFileReversed(near_GID) ^ near_npm_elem.NPMHE_Reversed, mNodes->isFileReversed(far_GID) ^ far_npm_elem.NPMHE_Reversed, tdist)]++;
                            }
                        }
                    }
                }
            }
            node_iter++;
            pos_iter++;
        }

        // for each library we see which entry (type) in orientation_array is the highest
        // and then we can assign that type to the library
        index = 0;
        all_ins_iter = all_ins_libs->begin();
        while(all_ins_iter != all_ins_libs->end())
        {
            int highest = 0;
            InsertLibrary::OrientationType type = InsertLibrary::TYPE_UNSET;
            for(int j = 0; j < 4; j++)
            {
                if(orientation_array[index][j] > highest)
                {
                    highest = orientation_array[index][j];
                    type = (InsertLibrary::OrientationType)j;
                }
            }

            // set the type
            all_ins_iter->_orientation = type;
            index++;
            all_ins_iter++;
        }

        // types are done! Now we will need to sort the vector holding the distances
        // so we can work out the mean and stdev
    
        // sort the vectors    
        for(int i = 0; i < num_libraries; i++)
        {
            sort((sequence_bucket[i])->begin(), (sequence_bucket[i])->end());
        }

        // cut out some of the top and bottom of values...
        // this removes any spurious outliers

        if(0 != percentToCut)
        {
            for(int i = 0; i < num_libraries; i++)
            {
                int low_cutter = (sequence_bucket[i])->size() / 100 * percentToCut;
                int high_cutter = (sequence_bucket[i])->size() - low_cutter;
                int counter = 0;
                vector<int>::iterator values_iter = (sequence_bucket[i])->begin();
                vector<int>::iterator values_last = (sequence_bucket[i])->end();
                while(values_iter != values_last)
                {
                    if(counter < low_cutter || counter > high_cutter)
                    {
                        *values_iter = 0;
                    }
                    counter++;
                    values_iter++;
                }
            }
        }

        // now set the mean and stdev for each library
        index = 0;
        all_ins_iter = all_ins_libs->begin();
        while(all_ins_iter != all_ins_libs->end())
        {
            vector<int>::iterator values_iter = (sequence_bucket[index])->begin();
            vector<int>::iterator values_last = (sequence_bucket[index])->end();
            // get the mean...
            double mean = 0;
            double stdev = 0;
            double num_non_zero = 0;
            while(values_iter != values_last)
            {
                if(0 != *values_iter)
                {
                    num_non_zero++;
                    mean += (double)(*values_iter);
                }
                values_iter++;
            }
            if(0 != num_non_zero)
            {
                mean /= num_non_zero;
                values_iter = (sequence_bucket[index])->begin();

                // get the stdev...
                while(values_iter != values_last)
                {
                    if(*values_iter)
                        stdev += pow(((double)(*values_iter) - mean),2);
                    values_iter++;
                }
                stdev /= (double)(num_non_zero);
                stdev = sqrt(stdev);
            }
            else
            {
                // couldn't map all that well.
                // keep the current estimate for the mean and 
                // make the stdev +/- 10%
                mean = all_ins_iter->_mean;
                stdev = all_ins_iter->_mean / 10;
            }

            // set the vars
            all_ins_iter->_mean = mean;
            if(0 == stdev)
                all_ins_iter->_stdev =  all_ins_iter->_mean / 10;
            else
                all_ins_iter->_stdev = stdev;
            
            all_ins_iter->makeCutOffs((double)SAS_DEF_MEAN_RANGE);
            
            index++;
            all_ins_iter++;
        }

        // clean up
        for(int i = 0; i < num_libraries; i++)
        {
            delete sequence_bucket[i];
        }
    
        delete [] sequence_bucket;
        sequence_bucket = NULL;

        delete [] mean_cutoffs;
        mean_cutoffs = NULL;
        
        for(int i = 0; i < num_libraries; i++)
        {
            delete orientation_array[i];
        }
        delete [] orientation_array;
        orientation_array = NULL;
        
        // next file set
        all_fs_iter++;
    }

    logInfo("(seq) Final stats:" , 2);
    mDataSet->logFileStats(2);
}

void StatsManager::updateStats(std::string * sequence, bool cutStart, bool cutEnd, ContextId CTXID)
{
    //-----
    // update the means and standard deviations for the assembly
    // we do this using the longest contig
    // this function uses the values stored in mMeans, so if you want you can call it againa and again...
    // we assume that a context has been built and that we're comparing to the alpha contig
    //
    
    // we only need to consider the end of the sequence we are
    // trying to map to. So we can take a substring if need be
    uMDInt huc = mDataSet->getHighestUpperCut() * 6;
    std::string working_sequence;
    if(cutStart)
    {
        if(sequence->length() > huc)
            working_sequence = sequence->substr(huc, sequence->length() - huc);
        else
            working_sequence = *sequence;
    }
    else if(cutEnd)
    {
        if(sequence->length() > huc)
            working_sequence = sequence->substr(sequence->length() - huc, huc);
        else
            working_sequence = *sequence;
        
    }
    else
        working_sequence = *sequence;
    
    // map all the head GIDs to the input sequence
    NodePositionMapClass npm;
    npm.initialise(SAS_DEF_CTX_NPM_SIZE);
    vector<GenericNodeId> mapped_nodes;
    vector<uMDInt> mapped_positions;
    
    int seq_length = working_sequence.length() - mReadLength + 1;
    int seq_used = 0;
    uMDInt pos = (uMDInt)(mReadLength / 2);
    sMDInt tdist;
    uMDInt dist;
    std::string kmer;
    GenericNodeId GID;
    while(seq_used < seq_length)
    {
        kmer = working_sequence.substr(seq_used, mReadLength);
        if(mNodes->getGn(&GID, &kmer))
        {
            NPMHeadElem tmp_npm_elem;
            tmp_npm_elem.NPMHE_GID = GID;
            tmp_npm_elem.NPMHE_Position = pos;
            if(mNodes->getSequence(GID) != kmer)
                tmp_npm_elem.NPMHE_Reversed = true;
            else
                tmp_npm_elem.NPMHE_Reversed = false;
            npm.addElem(&tmp_npm_elem, GID);
            mapped_nodes.push_back(GID);
            mapped_positions.push_back(pos);
        }
        seq_used++;
        pos++;
    }
    
    //
    // if there are two or three libraries we need two or three means
    // so we need to work out which container to put the information into
    // do this:
    //
    //  0----------A---------------------------------------B----------------------
    //                        C
    //                        |
    //                     cut here
    //
    // Such that 0A:0B == AC:CB
    //
    // pivot on these values
    //
    
    // we don't need to know how many libraries we're dealing with
    // we just process for each fileset separately
    
    // we need some other disposable maps so that we can 
    // work out averages
    std::map<int, double> cum_os_by_rank;
    std::map<int, double> cum_us_by_rank;
    std::map<int, int> num_os_by_rank;
    std::map<int, int> num_us_by_rank;
    
    // don't forget to intialise them...
    int num_rank_bands = mDataSet->getNumRankBands();
    while(num_rank_bands > 0)
    {
        cum_os_by_rank[num_rank_bands] = 0;
        cum_us_by_rank[num_rank_bands] = 0;
        num_os_by_rank[num_rank_bands] = 0;
        num_us_by_rank[num_rank_bands] = 0;
        num_rank_bands--;
    }
    
    logInfo("Start mapping: [" << mapped_nodes.size() << " :"  << mapped_positions.size() << "]", 6);
    
    std::vector<FileSet> * all_fs = mDataSet->getFileSets();
    std::vector<FileSet>::iterator all_fs_iter = all_fs->begin();
    while(all_fs_iter != all_fs->end())
    {
        // get the current FSID
        FileSetId current_fsid = all_fs_iter->getFSID();
        
        // we need to know how many insert libraries there are in this file set
        std::vector<InsertLibrary> * all_ins_libs = all_fs_iter->getInsLibs();
        int num_libraries = all_ins_libs->size();
        
        logInfo("Calculating stats for fileSet: " << current_fsid << " with: " << num_libraries << " insert libraries", 6);
        
        // use this data structure to work out the orientation
        // type of each library
        int ** orientation_array = new int * [num_libraries];
        PARANOID_ASSERT_L3(orientation_array != NULL);
        
        
        for(int i = 0; i < num_libraries; i++)
        {
            orientation_array[i] = new int[4];
            PARANOID_ASSERT_L3(orientation_array[i] != NULL);
            
            for(int j = 0; j < 4; j++)
            {
                orientation_array[i][j] = 0;
            }
        }
        
        // use these data structures to work out distribution stats
        double * mean_cutoffs = new double[num_libraries];
        PARANOID_ASSERT_L3(mean_cutoffs != NULL);
        
        vector<int> ** sequence_bucket = new vector<int>*[num_libraries];
        PARANOID_ASSERT_L3(sequence_bucket != NULL);
        
        // we assume that the vector of insert libraries is sorted in order of ascending mean
        double * means = new double[num_libraries];
        PARANOID_ASSERT_L3(means != NULL);
        
        int index = 0;
        std::vector<InsertLibrary>::iterator all_ins_iter = all_ins_libs->begin();
        while(all_ins_iter != all_ins_libs->end())
        {
            means[index] = all_ins_iter->_mean;
            index++;
            all_ins_iter++;
        }
        
        int bucket_counter = 0;
        // we'll only go in this while loop when there is more than one library
        while(bucket_counter < num_libraries - 1)
        {
            mean_cutoffs[bucket_counter] = (means[bucket_counter+1] - means[bucket_counter]) / (means[bucket_counter+1] / means[bucket_counter] + 1) + means[bucket_counter];
            //mean_cutoffs[bucket_counter] = (means[bucket_counter+1] - means[bucket_counter]) / 2 + means[bucket_counter];
            sequence_bucket[bucket_counter] = new vector<int>;
            PARANOID_ASSERT_L3(sequence_bucket[bucket_counter] != NULL);
            
            bucket_counter++;
        }
        
        // do the last bucket and cut off
        mean_cutoffs[bucket_counter] = HUGE_VAL;
        sequence_bucket[bucket_counter] = new vector<int>;
        PARANOID_ASSERT_L3(sequence_bucket[bucket_counter] != NULL);
        
        // delete the temp holder
        delete [] means;
        means = NULL;
        
        // go through all the nodes and try to map the GIDs
        vector<GenericNodeId>::iterator node_iter = mapped_nodes.begin();
        vector<GenericNodeId>::iterator node_last = mapped_nodes.end();
        vector<uMDInt>::iterator pos_iter = mapped_positions.begin();
        GenericNodeElem gn_elem;
        
        // the positions of the nodes in the context
        NPMHeadElem near_npm_elem, far_npm_elem;
        GenericNodeId far_head, near_GID, far_GID;
        bool near_reversed, far_reversed;
        
        while(node_iter != node_last)
        {
            // get the right head
            if(npm.getHead(&near_npm_elem, *node_iter))
            {
                while(near_npm_elem.NPMHE_Position != *pos_iter)
                {
                    npm.getNextHead(&near_npm_elem);
                }
            }
            
            // go through all the Dualnodes at this position
            if(mNodes->getElem(&gn_elem, *node_iter))
            {
                // the elem now holds only the head node
                while(mNodes->getNextElem(&gn_elem))
                {
                    // the elem now holds a non-head node - check to see it was from the current file
                    // and that the context matches
                    near_GID = gn_elem.GNE_GID;
                    if(mNodes->getCntxId(near_GID) == CTXID)
                    {
                        if(mNodes->getFileId(near_GID) == current_fsid)
                        {
                            // get the paired read
                            far_GID = mNodes->getPair(near_GID);
                            far_head = mNodes->getHead(far_GID);
                            if(npm.getHead(&far_npm_elem, far_head))
                            {
                                do
                                {
                                    // both guys lie on the same sequence, get the distance and put it in the right hole
                                    // also get the absolute value of the true distance
                                    tdist = (sMDInt)far_npm_elem.NPMHE_Position - (sMDInt)near_npm_elem.NPMHE_Position;
                                    dist = (uMDInt)(fAbs(tdist));
                                    near_reversed = mNodes->isFileReversed(near_GID) ^ near_npm_elem.NPMHE_Reversed;
                                    far_reversed = mNodes->isFileReversed(far_GID) ^ far_npm_elem.NPMHE_Reversed;
                                    if(isMappable(near_GID, far_GID, near_reversed, far_reversed, tdist))
                                    {
                                        // we could map this guy according to the last stats
                                        int count_down = num_libraries - 2;
                                        bool added = false;
                                        while(count_down >= 0)
                                        {
                                            if(dist > mean_cutoffs[count_down])
                                            {
                                                (sequence_bucket[count_down + 1])->push_back(dist);
                                                orientation_array[count_down + 1][all_fs_iter->calcOrientationType(near_reversed, far_reversed, tdist)]++;
                                                added = true;
                                                break;
                                            }
                                            count_down--;
                                        }
                                        if(!added)
                                        {
                                            (sequence_bucket[0])->push_back(dist);
                                            orientation_array[0][all_fs_iter->calcOrientationType(near_reversed, far_reversed, tdist)]++;
                                        }
                                        // we only need to map it once...
                                        break;
                                    }
                                } while(npm.getNextHead(&far_npm_elem));
                            }
                        }
                    }
                }
            }
            node_iter++;
            pos_iter++;
        }
        
        // for each library we see which entry (type) in orientation_array is the highest
        // and then we can assign that type to the library
        index = 0;
        all_ins_iter = all_ins_libs->begin();
        while(all_ins_iter != all_ins_libs->end())
        {
            int highest = 0;
            InsertLibrary::OrientationType type = InsertLibrary::TYPE_UNSET;
            for(int j = 0; j < 4; j++)
            {
                if(orientation_array[index][j] > highest)
                {
                    highest = orientation_array[index][j];
                    type = (InsertLibrary::OrientationType)j;
                }
            }
            
            // set the type
            all_ins_iter->_orientation = type;
            index++;
            all_ins_iter++;
        }
        
        // types are done! Now we will need to sort the vector holding the distances
        // so we can work out the mean and stdev
        
        // sort the vectors    
        for(int i = 0; i < num_libraries; i++)
        {
            sort((sequence_bucket[i])->begin(), (sequence_bucket[i])->end());
        }
        
        // cut out some of the top and bottom of values...
        // this removes any spurious outliers
        int percent_to_cut = 5;
        for(int i = 0; i < num_libraries; i++)
        {
            int low_cutter = (sequence_bucket[i])->size() / (100 / percent_to_cut);
            if(0 != low_cutter)
            {
                int high_cutter = (sequence_bucket[i])->size() - low_cutter;
                int middle = high_cutter - low_cutter - 1;
                int counter = 0;
                vector<int>::iterator values_iter = (sequence_bucket[i])->begin();
                vector<int>::iterator values_last = (sequence_bucket[i])->end();
                while(values_iter != values_last)
                {
                    if(counter == low_cutter)
                    {
                        values_iter += middle;
                        counter += middle;
                    }
                    else if(counter < low_cutter || counter > high_cutter)
                    {
                        *values_iter = 0;
                    }
                    counter++;
                    values_iter++;
                }
            }
        }
        
        // now set the mean and stdev for each library      
        index = 0;
        all_ins_iter = all_ins_libs->begin();
        while(all_ins_iter != all_ins_libs->end())
        {
            vector<int>::iterator values_iter = (sequence_bucket[index])->begin();
            vector<int>::iterator values_last = (sequence_bucket[index])->end();
            // get the mean...
            double mean = 0;
            double stdev = 0;
            double num_non_zero = 0;
            while(values_iter != values_last)
            {
                if(0 != *values_iter)
                {
                    num_non_zero++;
                    mean += (double)(*values_iter);
                }
                values_iter++;
            }
            mean /= num_non_zero;
            values_iter = (sequence_bucket[index])->begin();
            
            // get the stdev...
            while(values_iter != values_last)
            {
                if(*values_iter)
                    stdev += pow(((double)(*values_iter) - mean),2);
                values_iter++;
            }
            stdev /= (double)(num_non_zero);
            stdev = sqrt(stdev);
            
            // set the vars, maybe...
            if(num_non_zero > 0)
            {
                all_ins_iter->_mean = mean;
                if(0 == stdev)
                    all_ins_iter->_stdev = mean / 10;
                else
                    all_ins_iter->_stdev = stdev;
            }
            else
            {
                // the mean should be set "somewhere" so to stop things going bad
                // just set the stdev to 10% of the mean and get going...
                all_ins_iter->_stdev = (all_ins_iter->_mean/10);
            }
            
            all_ins_iter->_beta = -1;
            index++;
            all_ins_iter++;
        }
        
        // clean up
        for(int i = 0; i < num_libraries; i++)
        {
            delete sequence_bucket[i];
        }
        
        delete [] sequence_bucket;
        sequence_bucket = NULL;
        
        delete [] mean_cutoffs;
        mean_cutoffs = NULL;
        
        for(int i = 0; i < num_libraries; i++)
        {
            delete [] orientation_array[i];
        }
        delete [] orientation_array;
        orientation_array = NULL;
        
        // next file set
        all_fs_iter++;
    }
    
    // We will work out the over and undershoots...
    // Get hold of these guys. We are doing this by rank band
    // so we need to work out averages AFTER the band is complete
    mUnderShoots->clear();
    mOverShoots->clear();
    
    // now we can do the averages
    std::map<int, double>::iterator cum_os_by_rank_iter = cum_os_by_rank.begin();
    std::map<int, double>::iterator cum_os_by_rank_last = cum_os_by_rank.end();
    std::map<int, int>::iterator num_os_by_rank_iter = num_os_by_rank.begin();
    while(cum_os_by_rank_iter != cum_os_by_rank_last)
    {
        if(num_os_by_rank_iter->second != 0)
        {
            (*mOverShoots)[cum_os_by_rank_iter->first] = cum_os_by_rank_iter->second / num_os_by_rank_iter->second;
        }
        else
        {
            (*mOverShoots)[cum_os_by_rank_iter->first] = 0;
        }
        cum_os_by_rank_iter++;
        num_os_by_rank_iter++;
    }
    
    std::map<int, double>::iterator cum_us_by_rank_iter = cum_us_by_rank.begin();
    std::map<int, double>::iterator cum_us_by_rank_last = cum_us_by_rank.end();
    std::map<int, int>::iterator num_us_by_rank_iter = num_us_by_rank.begin();
    while(cum_us_by_rank_iter != cum_us_by_rank_last)
    {
        if(num_us_by_rank_iter->second != 0)
        {
            (*mUnderShoots)[cum_us_by_rank_iter->first] = cum_us_by_rank_iter->second / num_us_by_rank_iter->second;
        }
        else
        {
            (*mUnderShoots)[cum_us_by_rank_iter->first] = 0;
        }
        cum_us_by_rank_iter++;
        num_us_by_rank_iter++;
    }
    
    logInfo("(update) Final stats:" , 5);
    mDataSet->logFileStats(5);
}


//*************************************************************************************
// NEW* IMPROVED STATS
//*************************************************************************************

void StatsManager::cumuloGetStats(std::string fileName, int percentToCut, uMDInt minCutoff, int numSeqs)
{
    //-----
    // calculate the means and standard deviations for the assembly
    // Use all contigs in the given file which are equal to or longer
    // than the minCutoff.
    //
    // NOTE: Assumes the input file is sorted in descendin order of length
    //
    
    logInfo("Calculating mean insert lengths using at most " << numSeqs << " sequences that are longer than: " << minCutoff << " in: ", 2);
    logInfo(fileName, 2);
    logInfo("Original estimates: ", 2);
    mDataSet->logFileStats(2);

    fstream fasta_file;
    fasta_file.open(fileName.c_str(), ios::in);
    if(fasta_file.is_open())
    {
        // Process for each fileset separately.We need to pass these 
        // arrays through to the cumulative function for updating
        std::vector<FileSet> * all_fs = mDataSet->getFileSets();
        int num_filesets = all_fs->size();
        int *** orientation_array_per_fs = new int ** [num_filesets];
        double ** mean_cutoffs_per_fs = new double * [num_filesets];
        vector<int> *** sequence_bucket_per_fs = new vector<int>**[num_filesets];
        int index = 0;
        
// INITIALISE ALL THE DATA CONTAINERS WE'LL NEED

        std::vector<FileSet>::iterator all_fs_iter = all_fs->begin();
        int file_set_number = 0;
        while(all_fs_iter != all_fs->end())
        {
            //
            // if there are two or three libraries we need two or three means
            // so we need to work out which container to put the information into
            // do this:
            //
            //  0----------A---------------------------------------B----------------------
            //                        C
            //                        |
            //                     cut here
            //
            // Such that 0A:0B == AC:CB
            //
            // pivot on these values
            //
            // we need to know how many insert libraries there are in this file set
            std::vector<InsertLibrary> * all_ins_libs = all_fs_iter->getInsLibs();
            int num_libraries = all_ins_libs->size();
            
            // use this data structure to work out the orientation
            // type of each library
            int ** orientation_array = new int * [num_libraries];
            PARANOID_ASSERT_L3(orientation_array != NULL);
            
            for(int i = 0; i < num_libraries; i++)
            {
                orientation_array[i] = new int[4];
                PARANOID_ASSERT_L3(orientation_array[i] != NULL);
                
                for(int j = 0; j < 4; j++)
                {
                    orientation_array[i][j] = 0;
                }
            }
            orientation_array_per_fs[file_set_number] = orientation_array;
            
            // use these data structures to work out distribution stats
            double * mean_cutoffs = new double[num_libraries];
            PARANOID_ASSERT_L3(mean_cutoffs != NULL);
            mean_cutoffs_per_fs[file_set_number] = mean_cutoffs;
            
            vector<int> ** sequence_bucket = new vector<int>*[num_libraries];
            PARANOID_ASSERT_L3(sequence_bucket != NULL);
            sequence_bucket_per_fs[file_set_number] = sequence_bucket;
            
            // we assume that the vector of insert libraries is sorted in order of ascending mean
            double * means = new double[num_libraries];
            PARANOID_ASSERT_L3(means != NULL);
            
            index = 0;
            std::vector<InsertLibrary>::iterator all_ins_iter = all_ins_libs->begin();
            while(all_ins_iter != all_ins_libs->end())
            {
                means[index] = all_ins_iter->_mean;
                index++;
                all_ins_iter++;
            }
            
            // we'll only go in this while loop when there is more than one library
            int bucket_counter = 0;
            while(bucket_counter < num_libraries - 1)
            {
                mean_cutoffs[bucket_counter] = (means[bucket_counter+1] - means[bucket_counter]) / (means[bucket_counter+1] / means[bucket_counter] + 1) + means[bucket_counter];
                sequence_bucket[bucket_counter] = new vector<int>;
                PARANOID_ASSERT_L3(sequence_bucket[bucket_counter] != NULL);
                bucket_counter++;
            }
            
            // do the last bucket and cut off
            mean_cutoffs[bucket_counter] = HUGE_VAL;
            sequence_bucket[bucket_counter] = new vector<int>;
            PARANOID_ASSERT_L3(sequence_bucket[bucket_counter] != NULL);
            
            // delete the temp holder
            delete [] means;
            means = NULL;
         
            file_set_number++;
            all_fs_iter++;
        }

// LOAD THE SEQUENCES AND POPULATE THE DATA CONTAINERS

        int sequences_parsed = 0;
        bool in_fasta = false;
        std::string line;
        bool done = false;
        
        // keep this as a pointer
        stringstream buffer; new stringstream;

        // read the file
        while(getline(fasta_file, line))
        {
            // check if it's a fasta header
            if(line.substr(0, 1) == ">")
            {
                if(in_fasta)
                {
                    if(buffer.str().length() >= minCutoff && sequences_parsed <= numSeqs)
                    {
                        // process this sequence
                        sequences_parsed++;
                        logInfo("Updating stats with sequence: " << sequences_parsed << " Length: " << buffer.str().length(), 6);
                        populateStatsVars(&orientation_array_per_fs, &mean_cutoffs_per_fs, &sequence_bucket_per_fs, buffer.str());
                    }
                    else
                    {
                        // check to see we have parsed some sequences...
                        if(0 == sequences_parsed)
                        {
                            // none done yet.
                            // we need to do at least one sequence...
                            sequences_parsed++;
                            logError("Forced to make stats with longest contig. Length: " << buffer.str().length());
                            populateStatsVars(&orientation_array_per_fs, &mean_cutoffs_per_fs, &sequence_bucket_per_fs, buffer.str());
                        }

                        // this sequence, and all others are too short
                        // time to quit
                        done = true;
                        break;
                    }
                    
                    // clear the buffer
                    buffer.str("");
                    buffer.clear();
                }
            }
            else
            {
                in_fasta = true;
                buffer << line;
            }
        }
        
        // perhaps we need to do the last one...
        if(!done && in_fasta)
        {
            if(buffer.str().length() >= minCutoff && sequences_parsed <= numSeqs)
            {
                // process this sequence
                sequences_parsed++;
                logInfo("Updating stats with sequence: " << sequences_parsed << " Length: " << buffer.str().length(), 6);
                populateStatsVars(&orientation_array_per_fs, &mean_cutoffs_per_fs, &sequence_bucket_per_fs, buffer.str());
            }
            else
            {
                // check to see we have parsed some sequences...
                if(0 == sequences_parsed)
                {
                    // none done yet.
                    // we need to do at least one sequence...
                    sequences_parsed++;
                    logError("Forced to make stats with longest contig. Length: " << buffer.str().length());
                    populateStatsVars(&orientation_array_per_fs, &mean_cutoffs_per_fs, &sequence_bucket_per_fs, buffer.str());
                }
            }
        }
         
        // clean up
        fasta_file.close();

// CALCULATE THE STATISTICS        
        
        all_fs_iter = all_fs->begin();
        file_set_number = 0;
        while(all_fs_iter != all_fs->end())
        {
            // we need to know how many insert libraries there are in this file set
            std::vector<InsertLibrary> * all_ins_libs = all_fs_iter->getInsLibs();
            int num_libraries = all_ins_libs->size();
            
            // get the right arrays...
            int ** orientation_array = orientation_array_per_fs[file_set_number];
            vector<int> ** sequence_bucket = sequence_bucket_per_fs[file_set_number];
            
            // for each library we see which entry (type) in orientation_array is the highest
            // and then we can assign that type to the library
            index = 0;
            std::vector<InsertLibrary>::iterator all_ins_iter = all_ins_libs->begin();
            while(all_ins_iter != all_ins_libs->end())
            {
                int highest = 0;
                InsertLibrary::OrientationType type = InsertLibrary::TYPE_UNSET;
                for(int j = 0; j < 4; j++)
                {
                    if(orientation_array[index][j] > highest)
                    {
                        highest = orientation_array[index][j];
                        type = (InsertLibrary::OrientationType)j;
                    }
                }
                
                // set the type
                all_ins_iter->_orientation = type;
                index++;
                all_ins_iter++;
            }
            
            // types are done! Now we will need to sort the vector holding the distances
            // so we can work out the mean and stdev
            
            // sort the vectors    
            for(int i = 0; i < num_libraries; i++)
            {
                sort((sequence_bucket[i])->begin(), (sequence_bucket[i])->end());
            }
            
            // cut out some of the top and bottom of values...
            // this removes any spurious outliers
            
            if(0 != percentToCut)
            {
                for(int i = 0; i < num_libraries; i++)
                {
                    int low_cutter = (sequence_bucket[i])->size() / 100 * percentToCut;
                    int high_cutter = (sequence_bucket[i])->size() - low_cutter;
                    int counter = 0;
                    vector<int>::iterator values_iter = (sequence_bucket[i])->begin();
                    vector<int>::iterator values_last = (sequence_bucket[i])->end();
                    while(values_iter != values_last)
                    {
                        if(counter < low_cutter || counter > high_cutter)
                        {
                            *values_iter = 0;
                        }
                        counter++;
                        values_iter++;
                    }
                }
            }
            
            
            // now set the mean and stdev for each library
            index = 0;
            all_ins_iter = all_ins_libs->begin();
            while(all_ins_iter != all_ins_libs->end())
            {
                vector<int>::iterator values_iter = (sequence_bucket[index])->begin();
                vector<int>::iterator values_last = (sequence_bucket[index])->end();
                // get the mean...
                double mean = 0;
                double stdev = 0;
                double num_non_zero = 0;
                while(values_iter != values_last)
                {
                    if(0 != *values_iter)
                    {
                        num_non_zero++;
                        mean += (double)(*values_iter);
                    }
                    values_iter++;
                }
                if(0 != num_non_zero)
                {
                    mean /= num_non_zero;
                    values_iter = (sequence_bucket[index])->begin();
                    
                    // get the stdev...
                    while(values_iter != values_last)
                    {
                        if(*values_iter)
                            stdev += pow(((double)(*values_iter) - mean),2);
                        values_iter++;
                    }
                    stdev /= (double)(num_non_zero);
                    stdev = sqrt(stdev);
                }
                else
                {
                    // couldn't map all that well.
                    // keep the current estimate for the mean and 
                    // make the stdev +/- 10%
                    mean = all_ins_iter->_mean;
                    stdev = all_ins_iter->_mean / 10;
                }
                
                // set the vars
                all_ins_iter->_mean = mean;
                if(0 == stdev)
                    all_ins_iter->_stdev =  all_ins_iter->_mean / 10;
                else
                    all_ins_iter->_stdev = stdev;
                
                all_ins_iter->makeCutOffs((double)SAS_DEF_MEAN_RANGE);
                
                index++;
                all_ins_iter++;
            }
            // next file set
            all_fs_iter++;
            file_set_number++;
        }
        
// RANK THE INSERT LIBRARIES AND CALCULATE OVER - UNDER SHOOTS

        mDataSet->rankInsertLibs();
        
        // we need some other disposable maps so that we can 
        // work out averages
        std::map<int, double> cum_os_by_rank;
        std::map<int, double> cum_us_by_rank;
        std::map<int, double> num_os_by_rank;
        std::map<int, double> num_us_by_rank;
        
        // don't forget to intialise them...
        int num_rank_bands = mDataSet->getNumRankBands();
        while(num_rank_bands > 0)
        {
            cum_os_by_rank[num_rank_bands] = 0;
            cum_us_by_rank[num_rank_bands] = 0;
            num_os_by_rank[num_rank_bands] = 0;
            num_us_by_rank[num_rank_bands] = 0;
            num_rank_bands--;
        }
        
        all_fs_iter = all_fs->begin();
        file_set_number = 0;
        while(all_fs_iter != all_fs->end())
        {
            // we need to know how many insert libraries there are in this file set
            std::vector<InsertLibrary> * all_ins_libs = all_fs_iter->getInsLibs();
            
            // get the right arrays...
            vector<int> ** sequence_bucket = sequence_bucket_per_fs[file_set_number];
            
            index = 0;
            std::vector<InsertLibrary>::iterator all_ins_iter = all_ins_libs->begin();
            while(all_ins_iter != all_ins_libs->end())
            {
                vector<int>::iterator values_iter = (sequence_bucket[index])->begin();
                vector<int>::iterator values_last = (sequence_bucket[index])->end();
                while(values_iter != values_last)
                {
                    if(0 != *values_iter)
                    {
                        double delta = (double)(fAbs(*values_iter)) - all_ins_iter->_mean;
                        double z_score = fAbs(delta)/all_ins_iter->_stdev;
                        if(delta > 0)
                        {
                            // the insert is larger than the mean
                            cum_os_by_rank[all_ins_iter->_sizeRank] += z_score;
                            num_os_by_rank[all_ins_iter->_sizeRank]++;
                        }
                        else if(delta < 0)
                        {
                            // the insert is smaller than the mean
                            cum_us_by_rank[all_ins_iter->_sizeRank] += z_score;
                            num_us_by_rank[all_ins_iter->_sizeRank]++;
                        }
                    }
                    values_iter++;
                }
                index++;
                all_ins_iter++;
            }
            // next file set
            all_fs_iter++;
            file_set_number++;
        }
        
        // clear any old values
        mUnderShoots->clear();
        mOverShoots->clear();
        
        // now we can do the averages
        std::map<int, double>::iterator cum_os_by_rank_iter = cum_os_by_rank.begin();
        std::map<int, double>::iterator cum_os_by_rank_last = cum_os_by_rank.end();
        std::map<int, double>::iterator num_os_by_rank_iter = num_os_by_rank.begin();
        while(cum_os_by_rank_iter != cum_os_by_rank_last)
        {
            if(num_os_by_rank_iter->second != 0)
            {
                (*mOverShoots)[cum_os_by_rank_iter->first] = cum_os_by_rank_iter->second / num_os_by_rank_iter->second;
            }
            else
            {
                (*mOverShoots)[cum_os_by_rank_iter->first] = 0;
            }
            cum_os_by_rank_iter++;
            num_os_by_rank_iter++;
        }
        
        std::map<int, double>::iterator cum_us_by_rank_iter = cum_us_by_rank.begin();
        std::map<int, double>::iterator cum_us_by_rank_last = cum_us_by_rank.end();
        std::map<int, double>::iterator num_us_by_rank_iter = num_us_by_rank.begin();
        while(cum_us_by_rank_iter != cum_us_by_rank_last)
        {
            if(num_us_by_rank_iter->second != 0)
            {
                (*mUnderShoots)[cum_us_by_rank_iter->first] = cum_us_by_rank_iter->second / num_us_by_rank_iter->second;
            }
            else
            {
                (*mUnderShoots)[cum_us_by_rank_iter->first] = 0;
            }
            cum_us_by_rank_iter++;
            num_us_by_rank_iter++;
        }
        
// DELETE THE DATA CONTAINERS        
        
        all_fs_iter = all_fs->begin();
        file_set_number = 0;
        while(all_fs_iter != all_fs->end())
        {
            // we need to know how many insert libraries there are in this file set
            std::vector<InsertLibrary> * all_ins_libs = all_fs_iter->getInsLibs();
            int num_libraries = all_ins_libs->size();
            
            for(int i = 0; i < num_libraries; i++)
            {
                delete sequence_bucket_per_fs[file_set_number][i];
            }
            
            delete [] sequence_bucket_per_fs[file_set_number];
            
            delete [] mean_cutoffs_per_fs[file_set_number];
            
            for(int i = 0; i < num_libraries; i++)
            {
                delete [] orientation_array_per_fs[file_set_number][i];
            }
            delete orientation_array_per_fs[file_set_number];
        
            all_fs_iter++;
            file_set_number++;    
        }
        
        delete [] orientation_array_per_fs;
        delete [] mean_cutoffs_per_fs;
        delete [] sequence_bucket_per_fs;
    }
    else
    {
        logError("Could not open file: " << fileName);
        return;
    }
    
// TELL THE WORLD ABOUT IT

    logInfo("Final stats:" , 2);
    mDataSet->logFileStats(2);
    
}

void StatsManager::populateStatsVars(int **** orientationArrayPerFs, double *** meanCutoffsPerFs, vector<int> **** sequenceBucketPerFs, std::string sequence)
{
    //-----
    // map paired reads to the given sequence and update the cumulative counters
    //
    int seq_length = sequence.length() - mReadLength + 1;
    int seq_used = 0;
    uMDInt pos = (uMDInt)(mReadLength / 2);
    sMDInt tdist;
    uMDInt dist;
    std::string kmer;
    GenericNodeId GID;
        
    // map all the head GIDs to the sequence we just gotted
    NodePositionMapClass npm;
    npm.initialise(SAS_DEF_CTX_NPM_SIZE);
    vector<GenericNodeId> mapped_nodes;
    vector<uMDInt> mapped_positions;
    
    while(seq_used < seq_length)
    {
        kmer = sequence.substr(seq_used, mReadLength);
        if(mNodes->getGn(&GID, &kmer))
        {
            NPMHeadElem tmp_npm_elem;
            tmp_npm_elem.NPMHE_GID = GID;
            tmp_npm_elem.NPMHE_Position = pos;
            if(mNodes->getSequence(GID) != kmer)
                tmp_npm_elem.NPMHE_Reversed = true;
            else
                tmp_npm_elem.NPMHE_Reversed = false;
            npm.addElem(&tmp_npm_elem, GID);
            mapped_nodes.push_back(GID);
            mapped_positions.push_back(pos);
        }
        seq_used++;
        pos++;
    }
    
    // Using the heads, map actual paired reads
    std::vector<FileSet> * all_fs = mDataSet->getFileSets();
    std::vector<FileSet>::iterator all_fs_iter = all_fs->begin();
    int file_set_number = 0;
    while(all_fs_iter != all_fs->end())
    {
        // get the right arrays...
        int ** orientation_array = (*orientationArrayPerFs)[file_set_number];
        double * mean_cutoffs = (*meanCutoffsPerFs)[file_set_number];
        vector<int> ** sequence_bucket = (*sequenceBucketPerFs)[file_set_number];
        // we need to know how many insert libraries there are in this file set
        std::vector<InsertLibrary> * all_ins_libs = all_fs_iter->getInsLibs();
        int num_libraries = all_ins_libs->size();
        
        // get the current FSID
        FileSetId current_fsid = all_fs_iter->getFSID();
        
        // go through all the nodes and try to map the GIDs
        vector<GenericNodeId>::iterator node_iter = mapped_nodes.begin();
        vector<uMDInt>::iterator pos_iter = mapped_positions.begin();
        
        GenericNodeElem gn_elem;
        
        // the positions of the nodes in the context
        NPMHeadElem near_npm_elem, far_npm_elem;
        GenericNodeId far_head, near_GID, far_GID;
        
        while(node_iter != mapped_nodes.end())
        {
            // get the right head
            if(npm.getHead(&near_npm_elem, *node_iter))
            {
                while(near_npm_elem.NPMHE_Position != *pos_iter)
                {
                    npm.getNextHead(&near_npm_elem);
                }
            }
            
            // go through all the Dualnodes at this position
            // if(mNodes->getElem(&gn_elem, *node_iter))
            if(mNodes->getElem(&gn_elem, near_npm_elem.NPMHE_GID))
            {
                // the elem now holds only the head node
                while(mNodes->getNextElem(&gn_elem))
                {
                    // the elem now holds a non-head node - check to see it was from the current file
                    near_GID = gn_elem.GNE_GID;
                    if(mNodes->getFileId(near_GID) == current_fsid)
                    {
                        // get the paired read
                        far_GID = mNodes->getPair(near_GID);
                        far_head = mNodes->getHead(far_GID);
                        if(npm.getHead(&far_npm_elem, far_head))
                        {
                            // both guys lie on the same sequence, get the distance and put it in the right hole
                            // also get the absolute value of the true distance
                            tdist = (sMDInt)far_npm_elem.NPMHE_Position - (sMDInt)near_npm_elem.NPMHE_Position;
                            dist = (uMDInt)(fAbs(tdist));
                            
                            int count_down = num_libraries - 2;
                            bool added = false;
                            while(count_down >= 0)
                            {
                                if(dist > mean_cutoffs[count_down])
                                {
                                    (sequence_bucket[count_down + 1])->push_back(dist);
                                    orientation_array[count_down + 1][all_fs_iter->calcOrientationType(mNodes->isFileReversed(near_GID) ^ near_npm_elem.NPMHE_Reversed, mNodes->isFileReversed(far_GID) ^ far_npm_elem.NPMHE_Reversed, tdist)]++;
                                    added = true;
                                    break;
                                }
                                count_down--;
                            }
                            if(!added)
                            {
                                (sequence_bucket[0])->push_back(dist);
                                orientation_array[0][all_fs_iter->calcOrientationType(mNodes->isFileReversed(near_GID) ^ near_npm_elem.NPMHE_Reversed, mNodes->isFileReversed(far_GID) ^ far_npm_elem.NPMHE_Reversed, tdist)]++;
                            }
                        }
                    }
                }
            }
            node_iter++;
            pos_iter++;
        }
        // next file set
        all_fs_iter++;
        file_set_number++;
    }
}

//*************************************************************************************
// BACK TO WORK
//*************************************************************************************

bool StatsManager::isMappable(GenericNodeId near_GID, GenericNodeId far_GID, bool seq1Reversed, bool seq2Reversed, sMDInt distance1to2)
{
    //-----
    // this function looks at the variables mUpperCuts and mLowerCuts
    // and decides if the distance given is an acceptable distance for
    // mapping and making closures
    //
    // Reversed flags indicate the orientation of the read in the context as being read from start to finish (wrt the distance between the nodes +/-)
    // Are they reversed from how they are saved? ie. are the reads as read in the contexts current direction in lowest lexigographically orientated

    //TODO this is a crappy bug fix. need to do better work laterz...
    // This fixes a problem where paired reads which are too close to eachother 
    // cause segfaults during delete operations...
    if((uMDInt)fAbs(distance1to2) < (uMDInt)(mIgnoreTrap)) { return false; }
    
    std::vector<InsertLibrary> * file_libs = mDataSet->getInsLibs(mNodes->getFileId(near_GID));
    std::vector<InsertLibrary>::iterator libs_iter = file_libs->begin();
    while(libs_iter != file_libs->end())
    {
        if(libs_iter->doesMatchLibraryStats(seq1Reversed, seq2Reversed, distance1to2))
        {
            return true;
        }
        libs_iter++;
    }
    return false;
}

bool StatsManager::isMappable(GenericNodeId GID, InsertLibrary::OrientationType ot, uMDInt dist)
{
    //-----
    // Simpler version of the function call above
    //
    //TODO this is a crappy bug fix. need to do better work laterz...
    // This fixes a problem where paired reads which are too close to eachother 
    // cause segfaults during delete operations...
    if(dist < mIgnoreTrap) { return false; }
    
    std::vector<InsertLibrary> * file_libs = mDataSet->getInsLibs(mNodes->getFileId(GID));
    std::vector<InsertLibrary>::iterator libs_iter = file_libs->begin();
    while(libs_iter != file_libs->end())
    {
        if(libs_iter->isDistOrientOK(ot, dist))
        {
            return true;
        }
        libs_iter++;
    }
    return false;
}

double StatsManager::calcForce(GenericNodeId near_GID, GenericNodeId far_GID, bool seq1Reversed, bool seq2Reversed, sMDInt distance1to2, int rank)
{
    //-----
    // calculate the net force acroos the mapping for this node givent he rank we're looking for
    //
    // get the right insert library
    std::vector<InsertLibrary *> * theseIns = mDataSet->getInsertLib(rank);
    
    if(theseIns != NULL)
    {
        std::vector<InsertLibrary *>::iterator theseIns_iter = theseIns->begin();
        std::vector<InsertLibrary *>::iterator theseIns_last = theseIns->end();
        while(theseIns_iter != theseIns_last)
        {
            // and test
            if((*theseIns_iter)->doesMatchLibraryStats(seq1Reversed, seq2Reversed, distance1to2, mNodes->getFileId(near_GID)))
            {
                double delta = (double)(fAbs(distance1to2)) - (*theseIns_iter)->_mean;
                double z_score = fAbs(delta)/(*theseIns_iter)->_stdev;
                if(delta > 0)
                {
                    // the insert is larger than the mean
                    return forceEquation(z_score);
                }
                else if(delta < 0)
                {
                    // the insert is smaller than the mean
                    return forceEquation(z_score) * (*theseIns_iter)->_beta;
                }
                else
                {
                    return 0;
                }
            }
            theseIns_iter++;
        }
        // didn't match any of the required insert libraries
        return HUGE_VAL;
    }
    logError("Rank: " << rank << " doesn't correspond to any insert libary");
    return HUGE_VAL;
}

bool StatsManager::updatePerArmMaps(GenericNodeId near_GID, GenericNodeId far_GID, bool seq1Reversed, bool seq2Reversed, sMDInt distance1to2, int rank, GenericNodeId key_GID, std::map<GenericNodeId, double> * numOver, std::map<GenericNodeId, double> * cumOver, std::map<GenericNodeId, double> * numUnder, std::map<GenericNodeId, double> * cumUnder)
{
    //-----
    // Used to work out the distribution of inserts across an arm of an extended branch
    //
    // get the right insert library
    std::vector<InsertLibrary *> * theseIns = mDataSet->getInsertLib(rank);
    
    if(theseIns != NULL)
    {
        std::vector<InsertLibrary *>::iterator theseIns_iter = theseIns->begin();
        std::vector<InsertLibrary *>::iterator theseIns_last = theseIns->end();
        while(theseIns_iter != theseIns_last)
        {
            // and test
            if((*theseIns_iter)->doesMatchLibraryStats(seq1Reversed, seq2Reversed, distance1to2, mNodes->getFileId(near_GID)))
            {
                double delta = (double)(fAbs(distance1to2)) - (*theseIns_iter)->_mean;
                double z_score = fAbs(delta)/(*theseIns_iter)->_stdev;
                if(delta >= 0)
                {
                    // the insert is larger than the mean
                    (*numOver)[key_GID]++;
                    (*cumOver)[key_GID] += z_score;
                    return true;
                }
                else if(delta < 0)
                {
                    // the insert is smaller than the mean
                    (*numUnder)[key_GID]++;
                    (*cumUnder)[key_GID] += z_score;
                    return true;
                }
                else
                {
                    return 0;
                }
            }
            theseIns_iter++;
        }
        // didn't match any of the required insert libraries
        return false;
    }
    logError("Rank: " << rank << " doesn't correspond to any insert libary");   
    
    // didn't match any of the required insert libraries
    return false;
}

double StatsManager::calculateProfileDistance(int rank, double overShoots, double underShoots)
{
    //-----
    // calculate the distance of the shoots from the profile for the given rank
    //
    // get the profile for this rank
    PARANOID_ASSERT_PRINT_L4(NULL != mOverShoots, "OS- NULL");
    PARANOID_ASSERT_PRINT_L4(NULL != mUnderShoots, "US- NULL");

    std::map<int, double>::iterator os_iter = mOverShoots->find(rank);
    std::map<int, double>::iterator us_iter = mUnderShoots->find(rank);
    if(os_iter != mOverShoots->end())
    {
        // good old distance equation
        return sqrt( pow(os_iter->second - overShoots, 2) + pow(us_iter->second - underShoots, 2));
    }
    else
    {
        //logError("rank: " << rank << " is not valid. Size of ");
        
    }
    return HUGE_VAL;
}

double StatsManager::forceEquation(double z_score)
{
    //-----
    // Work out the Q-function for a positive z score
    //
    return (1/(0.5 - (erf(z_score/M_SQRT2)/2)) - 2);
}
        // fileIO

void StatsManager::save(std::string saveFile)
{
    //-----
    // save an StatsManager to file
    //
    ofstream myFile(saveFile.c_str(), ofstream::binary);
    //logInfo(mReadLength << " : " << mNumReads << " : " << mNumDumped << " : " << mNumDupes ,1);
    myFile.write(reinterpret_cast<char *>(&mReadLength), sizeof(int));
    myFile.write(reinterpret_cast<char *>(&mNumReads), sizeof(int));
    myFile.write(reinterpret_cast<char *>(&mNumDumped), sizeof(int));
    myFile.write(reinterpret_cast<char *>(&mNumDupes), sizeof(int));
    myFile.close();
}

void StatsManager::load(std::string loadFile)
{
    //-----
    // Load an StatsManager from File
    //
    ifstream myFile(loadFile.c_str(), ifstream::binary);
    myFile.read(reinterpret_cast<char *>(&mReadLength), sizeof(int));
    myFile.read(reinterpret_cast<char *>(&mNumReads), sizeof(int));
    myFile.read(reinterpret_cast<char *>(&mNumDumped), sizeof(int));
    myFile.read(reinterpret_cast<char *>(&mNumDupes), sizeof(int));
    //logInfo(mReadLength << " : " << mNumReads << " : " << mNumDumped << " : " << mNumDupes ,1);
    myFile.close();
}

