/******************************************************************************
**
** File: ContigMemWrapper.cpp
**
*******************************************************************************
**
** Layer 2 code for the generic contig class
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
#include <sstream>
#include <fstream>
#include <string>

#include <queue>
//INC #include <queue>
//INC 

#include <map>
//INC #include <map>
//INC 

//INC #include "nodeblockdef.h"
//INC 
#include "nodeblockdef.h"

//INC #include "Dataset.h"
//INC 
#include "Dataset.h"

//INC #include "StatsManager.h"
//INC 
#include "StatsManager.h"

// local includes
#include "intdef.h"
#include "IdTypeDefs.h"
#include "IdTypeSTL_ext.h"
#include "IdTypeStructs.h"
#include "ContigMemWrapper.h"
#include "default_parameters.h"

using namespace std;

/******************************************************************************
** #INCLUDES #DEFINES AND PRIVATE VARS
******************************************************************************/
#include "GenericNodeClass.h"
//INC #include "GenericNodeClass.h"
//INC 
#include "ReadStoreClass.h"
//INC #include "ReadStoreClass.h"
//INC 
/*INC*/ typedef std::vector<ContigId> contigList;
/*INC*/ 
/*INC*/ typedef std::vector<ContigId>::iterator contigListIterator;
/*INC*/ 
//PV int mReadLength;
//PV 
//PV NODE_CLASS mNc;
//PV 
//PV ContextId mCurrentContext;
//PV 
//PV GenericNodeClass * mNodes;
//PV 
//PV ReadStoreClass * mReads;
//PV 
//PV DataSet * mDataSet;
//PV 
//PV StatsManager * mStatsManager;
//PV 
//PV std::vector<ContigId> * mContigs;      /*S*/
//PV 
//PV std::map<ContigId, double> mCoverageMap;
//PV 
//PV std::map<ContigId, double> mGCMap;
//PV 
/******************************************************************************
        ** INITIALISATION AND SAVING
******************************************************************************/

/*HV*/ void
ContigMemWrapper::
/*HV*/ extendedDestructor(void)
//HO ;
{
    //-----
    // Add anything you would like to happen when the destructor is called
    //
    if(mContigs != NULL)
    {
        delete mContigs;
        mContigs = NULL;
    }
}
//HO 

/*HV*/ bool
ContigMemWrapper::
/*HV*/ extendedInitialise(uMDInt originalNumberOfBlocks )
//HO ;
{
    //-----
    // Add anything you would like to happen on a regular intialisation call
    //
    // set these guys to NULL
    mNodes = NULL;
    mReads = NULL;

    mContigs = new contigList();
    PARANOID_ASSERT_L2(mContigs != NULL);
    return true;
}
//HO 

/*HV*/ bool
ContigMemWrapper::
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
ContigMemWrapper::
/*HV*/ extendedSave(std::string fileName)
//HO ;
{
    //-----
    // Add anything you would like to happen when saving (including saving private vars)
    //
    return true;
}
//HO 

/*HV*/ void
ContigMemWrapper::
/*HV*/ initialise2(GenericNodeClass * GNC, StatsManager * SM, NODE_CLASS nc)
//HO ;
{
    //-----
    // Set the pointers to the external objects
    //
    mStatsManager = SM;
    mDataSet = mStatsManager->getDS();
    mNodes = GNC;
    mReads = mNodes->getReads();
    mNc = nc;
    mReadLength = mReads->getReadLength();
}
//HO 

//HO bool sort_contig_length(const ContigId & left, const ContigId & right)
//HO {
//HO     return getSeqLength(left) < getSeqLength(right);
//HO }

bool sort_by_contig_length(const std::pair<ContigId, uMDInt> & left, const std::pair<ContigId, uMDInt> & right)
{
    return right.second < left.second;
}

/******************************************************************************
** HOUSEKEEPING
******************************************************************************/
//HO inline void setCurrentContextId(ContextId CTXID) { mCurrentContext = CTXID; }
//HO 
//HO inline ContextId getCurrentContextId(void) { return mCurrentContext; }
//HO 

    /*
    ** Get the lists of cap and cross nodes for a node type
    */
/*HV*/ ContigId
ContigMemWrapper::
/*HV*/ getNthLongestContig(int nth)
//HO ;
{
    //-----
    // Get the nth longest contig
    //
    contigListIterator contig_iter = mContigs->begin();
    while(contig_iter != mContigs->end())
    {
        if(0 == --nth)
            return *contig_iter;
        contig_iter++;
    }
    return CTG_NULL_ID;
}
//HO 

//HO inline int getNumContigs(void) { return mContigs->size(); }
//HO 

/*HV*/ GenericNodeId 
ContigMemWrapper::
/*HV*/ getTrueStartNode(ContigId CID)
//HO ;
{
    //-----
    // check the reversed flags before we do anything rash
    //
    if(isReadFromStart(CID))
        return getStartNode(CID);
    return getEndNode(CID);
}
//HO 

/*HV*/ GenericNodeId 
ContigMemWrapper::
/*HV*/ getTrueNextStartNode(ContigId CID)
//HO ;
{
    //-----
    // check the reversed flags before we do anything rash
    //
    if(isReadFromStart(CID))
        return getNextStartNode(CID);
    return getNextEndNode(CID);
}
//HO 

/*HV*/ GenericNodeId 
ContigMemWrapper::
/*HV*/ getTrueEndNode(ContigId CID)
//HO ;
{
    //-----
    // check the reversed flags before we do anything rash
    //
    if(isReadFromStart(CID))
        return getEndNode(CID);
    return getStartNode(CID);
}
//HO 

/*HV*/ GenericNodeId 
ContigMemWrapper::
/*HV*/ getTrueNextEndNode(ContigId CID)
//HO ;
{
    //-----
    // check the reversed flags before we do anything rash
    //
    if(isReadFromStart(CID))
        return getNextEndNode(CID);
    return getNextStartNode(CID);
}
//HO 

/******************************************************************************
** Get the cap and cross blocks
******************************************************************************/

/*HV*/ void
ContigMemWrapper::
/*HV*/ getCapBlock(NodeBlockElem * nbe)
//HO ;
{
    //-----
    // The nodeClass (mNc) MUST before this is called
    // if mNc == DUALNODE then the variable
    // mCurrentContext MUST be valid before this is called
    //
    switch(mNc)
    {
        case UNINODE:
            mNodes->getCapBlock_UN(nbe);
            break;
        case DUALNODE:
            mNodes->getCapBlock_DN(nbe, mCurrentContext);
            break;
    }
}
//HO 

/*HV*/ void
ContigMemWrapper::
/*HV*/ getCrossBlock(NodeBlockElem * nbe)
//HO ;
{
    switch(mNc)
    {
        case UNINODE:
            mNodes->getCrossBlock_UN(nbe);
            break;
        case DUALNODE:
            mNodes->getCrossBlock_DN(nbe, mCurrentContext);
            break;
    }
}
//HO 

/******************************************************************************
** MAKE AND SORT CONTIGS
******************************************************************************/

/*HV*/ ContigId
ContigMemWrapper::
/*HV*/ startSplice(void)
//HO ;
{
    //-----
    // Returns some random ContigId
    //
    return splice();
}
//HO        

/*HV*/ ContigId
ContigMemWrapper::
/*HV*/ startSplice(ContextId CTXID)
//HO ;
{
    //-----
    // returns The ContigID if it is a Context based splice.
    // Otherwise some random ContigId
    //
    setCurrentContextId(CTXID);
    return splice();
}
//HO        

/*HV*/ ContigId
ContigMemWrapper::
/*HV*/ splice(void)
//HO ;
{
    //-----
    // here we make the (naive) Contigs.
    // we start at cap or cross nodes and we keep walking until we hit a cap or a cross node
    // all nodes between the two ends must have rank 2, and are part of the RN_Contig
    // the curr_cap and current_cross node lists were made in RN_Contig::processGenericNodeIds above
    // first, go through all the cross nodes and add rank information
    // and cap nodes with info on whether they're found or not
    //
    // returns The ContigID if it is a Context based splice.
    // Otherwise some random ContigId
    //
    //
    logInfo("Making Contigs", 40);

    // all the information we need to store about a RN_Contig
    int num_contigs = 0;
    int length = 0;
    double mean = 0;
    GenericNodeId start;
    GenericNodeId next_start;
    
    ContigId ret_CID = CTG_NULL_ID;

    // get the caps and crosses
    NodeBlockElem cap_nodes, cross_nodes;
    getCapBlock(&cap_nodes);

    // set all caps to unused
    while(mNodes->nextNBElem(&cap_nodes))
    {
        mNodes->setCapUsed(false, cap_nodes.NBE_CurrentNode);
    }
        
    // if a crossnode is adjacent to a crossnode then we won't make an RN_Contig
    // so the max number of RN_Contigs a cross node can be in is equal to the rank
    // of the node - record the number of cross nodes that are adjacent to the
    // current node, it's important.

    int unusable = 0;
    int curr_rank = 0;
    
    GN_EDGE_ELEM tmp_edge(mNc);
    GN_WALKING_ELEM walking_edge(mNc);
    
    getCrossBlock(&cross_nodes);
    while(mNodes->nextNBElem(&cross_nodes))
    {
        // is this even a cross node?
        
        curr_rank = mNodes->getNtRank(mNc, cross_nodes.NBE_CurrentNode);
        PARANOID_ASSERT_PRINT_L2(curr_rank > 2, "CrossNode with rank less than 3: " << (mNodes->isDummy(cross_nodes.NBE_CurrentNode)) << " : " << cross_nodes.NBE_CurrentNode);

        // go through all the adjacent nodes to the current cross node
        unusable = 0;
        if(mNodes->getEdges(&tmp_edge, GN_EDGE_STATE_NTRANS, 0, cross_nodes.NBE_CurrentNode))
        {
            do {
                GenericNodeId olap_node_GN = mNodes->getOlapNode(tmp_edge);
                if(olap_node_GN == cross_nodes.NBE_CurrentNode)
                    logError("cross links to self");

                switch(mNodes->getNtRank(mNc, olap_node_GN))
                {
                    case 2:
                    {
                        // path, so all is ok.
                        break;
                    }
                    case 1:
                    {
                        // we have a cross joined to a cap
                        // we won't make a contig here
                        unusable++;
                        // we need to mark this cap as done
                        // this should only ever happen once
                        if(mNodes->isCapUsed(olap_node_GN))
                            logError("cap_used true too early");
                        mNodes->setCapUsed(true, olap_node_GN);
                        break;
                    }
                    case 0:
                    {
                        logError("Detached node is non-transitive");
                        break;
                    }
                    default:
                    {
                            // we have a cross joined to a cross, increment the counter
                        unusable++;
                        if(mNodes->getNodeType(mNc, olap_node_GN) != GN_NODE_TYPE_CROSS)
                            logError("Rank 3+ node not listed as cross");
                        break;
                    }
                }

            } while(mNodes->getNextEdge(&tmp_edge));
        }

                // store the updated rank information
        PARANOID_ASSERT_PRINT_L2((curr_rank - unusable) >= 0, "effective rank less than 0");
        mNodes->setCrossCounter((curr_rank - unusable), cross_nodes.NBE_CurrentNode);
    }

    logInfo("Splicing", 40);

    // now we can start from the cap list
    // go through and make a contig, we will hit a cap or we will hit a cross

    getCapBlock(&cap_nodes);
    while(mNodes->nextNBElem(&cap_nodes))
    {
        // only do guy's we haven't done already
        if(!mNodes->isCapUsed(cap_nodes.NBE_CurrentNode))
        {
            // we haven't done this guy yet
            // mark this guy as done
            mNodes->setCapUsed(true, cap_nodes.NBE_CurrentNode);

            // set ContigId here
            start = cap_nodes.NBE_CurrentNode;
            if(!mNodes->getEdges(&tmp_edge, GN_EDGE_STATE_NTRANS, 0, cap_nodes.NBE_CurrentNode))
            {
                logError("Can't walk out from cap: " << cap_nodes.NBE_CurrentNode);
            }
            else
            {
                // reset the vars
                ContigId new_id = newContigId();
                
                // For Context based Contigs. This will only be called once
                // so set the return ContigId here...
                ret_CID = new_id;
                
                num_contigs++;
                length = 1;
                mean = 0;
                next_start = mNodes->getOlapNode(tmp_edge);
                if(mNodes->startWalk(&walking_edge, cap_nodes.NBE_CurrentNode, next_start))
                {
                    mNodes->setContig(mNc, new_id, mNodes->getPrevNode(walking_edge));
                    do {
                        //mean += getReadDepthAt(mNodes->getCurrentNode(walking_edge));
                        length++;
                        mNodes->setContig(mNc, new_id, mNodes->getCurrentNode(walking_edge));
                    } while(mNodes->rankStep(&walking_edge));

                    GenericNodeId current_GN = mNodes->getCurrentNode(walking_edge);
                    switch(mNodes->getNtRank(mNc, current_GN))
                    {
                        case 1:
                            // ends at a cap
                            if(mNodes->isCapUsed(current_GN))
                            {
                                logError("contig from cap ends at used cap");
                            }
                            mNodes->setCapUsed(true, current_GN);
                            // set the contig ID for caps only.
                            mNodes->setContig(mNc, new_id, current_GN);
                            break;
                        case 0:
                            // ends in pain
                            logError("navigation failed");
                            break;
                        case 2:
                            // ends in pain
                            logError("Nav ended on a path node");
                            break;
                        default:
                        {
                            // ends at a cross
                            PARANOID_ASSERT_PRINT_L2((mNodes->getCrossCounter(current_GN) >= 0), "contig from cap ends at used cross");
                            mNodes->decCrossCounter(current_GN);
                            break;
                        }
                    }

                    mean = mean / (double)length;
                    setStartNode(start, new_id);
                    // set the contig ID for caps only.
                    mNodes->setContig(mNc, new_id, start);
                    setNextStartNode(next_start, new_id);
                    setEndNode(current_GN, new_id);
                    setNextEndNode(mNodes->getPrevNode(walking_edge), new_id);
                    setNodeLength(length, new_id);
                    //setCoverage((uMDInt)mean, new_id);
                    setCoverage(0, new_id);
                    mContigs->push_back(new_id);
                }
            }
        }
    }

    // finally we go through the cross nodes, any with their rank not dimished to zero we can make a contig from
    getCrossBlock(&cross_nodes);
    while(mNodes->nextNBElem(&cross_nodes))
    {
        if(mNodes->getCrossCounter(cross_nodes.NBE_CurrentNode) > 0)
        {
            // we'll need the list of adjacent nodes
            if(!(mNodes->getEdges(&tmp_edge, GN_EDGE_STATE_NTRANS, 0, cross_nodes.NBE_CurrentNode)))
            {
                logError("no edges for the cross node");
                return CTG_NULL_ID;
            }
        }
        else if(mNodes->getCrossCounter(cross_nodes.NBE_CurrentNode) < 0)
            logError("effective rank is negative");

        start = cross_nodes.NBE_CurrentNode;

        while(mNodes->getCrossCounter(cross_nodes.NBE_CurrentNode) > 0)
        {
            // we haven't completely done this guy yet
            // mark another round
            mNodes->decCrossCounter(cross_nodes.NBE_CurrentNode);

            // reset the vars
            ContigId new_id = newContigId();
            num_contigs++;
            length = 1;
            mean = 0;

            // need to get a path that hasn't been used yet
            // this will be the next_start node
            // check the contig ID's of all the adjacent nodes
            // we need a num_contigs of 0 and a rank of two to continue
            // this must exist cause the effective rank is > 0

            bool found_next_ok = false;
            do {
                GenericNodeId olap_node_GN = mNodes->getOlapNode(tmp_edge);
                if(CTG_NULL_ID == mNodes->getContig(mNc, olap_node_GN) && 2 == mNodes->getNtRank(mNc, olap_node_GN))
                {
                    // an unused path node
                    next_start = olap_node_GN;
                    // move onto the next olap for the following round...
                    mNodes->getNextEdge(&tmp_edge);
                    found_next_ok = true;
                    break;
                }
            } while(mNodes->getNextEdge(&tmp_edge));

            if(!found_next_ok)
            {
                logError("it's not ok!!!");
            }
            else
            {
                if(mNodes->startWalk(&walking_edge, start, next_start))
                {
                    do {
                        //mean += getReadDepthAt(walking_edge.UNWE_CurrentNode);
                        length++;
                        mNodes->setContig(mNc, new_id, mNodes->getCurrentNode(walking_edge));
                    } while(mNodes->rankStep(&walking_edge));

                    GenericNodeId current_GN = mNodes->getCurrentNode(walking_edge);
                    switch(mNodes->getNtRank(mNc, current_GN))
                    {
                        case 1:
                                // ends at a cap
                            if(mNodes->isCapUsed(current_GN))
                                logError("contig from cap ends at used cap");
                            mNodes->setCapUsed(true, current_GN);
                            // set the contig ID for caps only.
                            mNodes->setContig(mNc, new_id, current_GN);
                            break;
                        case 0:
                                // ends in pain
                            logError("navigation failed");
                            break;
                        case 2:
                            logError("Finished walking at a path");
                            break;
                        default:
                                // ends at a cross
                            PARANOID_ASSERT_PRINT_L2((mNodes->getCrossCounter(current_GN) >= 0), "contig from cross ends at used cross");
                            mNodes->decCrossCounter(current_GN);
                            break;
                    }
                    mean = mean / (double)length;
                    setStartNode(start, new_id);
                    setNextStartNode(next_start, new_id);
                    setEndNode(current_GN, new_id);
                    setNextEndNode(mNodes->getPrevNode(walking_edge), new_id);
                    setNodeLength(length, new_id);
                    if(mNc == UNINODE)
                        setCoverage((uMDInt)mean, new_id);
                    mContigs->push_back(new_id);
                }
            }
        }
    }
    logInfo("Made " << num_contigs << " Contigs", 40);

    // finally, finally. We set all the cross nodes to have a null Contig ID
    // this avoids any ambiguities laterz...
    getCrossBlock(&cross_nodes);
    while(mNodes->nextNBElem(&cross_nodes))
    {
        mNodes->setContig(mNc, CTG_NULL_ID, cross_nodes.NBE_CurrentNode);
    }
    
    return ret_CID;
}
//HO 

    static std::vector< std::pair<ContigId, uMDInt> > CMWG_sorting_vector;
    static std::vector< std::pair<ContigId, uMDInt> >::iterator CMWG_sv_iter;

/*HV*/ void
ContigMemWrapper::
/*HV*/ sortContigs()
//HO ;
{
    //-----
    // Get the sequences of the contigs and then sort them all...
    //
    logInfo("Sorting Contigs", 40);

    contigListIterator contig_iter = mContigs->begin();

    if(mContigs->size() == 1)
    {
        setSeqLength((uMDInt)(getContigSequenceLengthManual(*contig_iter)), *contig_iter);
        return;
    }
    else if(mContigs->size() == 0)
        return;

    CMWG_sorting_vector.clear();
    while(contig_iter != mContigs->end())
    {
        // set the length of the sequence here...
        int len = getContigSequenceLengthManual(*contig_iter);
        setSeqLength((uMDInt)(len), *contig_iter);

        // push back onto the sorting vector
        CMWG_sorting_vector.push_back(std::pair<ContigId, uMDInt>(*contig_iter, (uMDInt)(len)));

        contig_iter++;
    }

    // now actually sort
    std::sort(CMWG_sorting_vector.begin(), CMWG_sorting_vector.end(), sort_by_contig_length);

    // clear this list and then we'll fill it again
    mContigs->clear();
    
    CMWG_sv_iter = CMWG_sorting_vector.begin();
    while(CMWG_sv_iter != CMWG_sorting_vector.end())
    {
        mContigs->push_back(CMWG_sv_iter->first);
        CMWG_sv_iter++;
    }
    logInfo("done.", 40);
}
//HO 

/*HV*/ void
ContigMemWrapper::
/*HV*/ reverseContig(ContigId CID)
//HO ;
{
    //-----
    // reverse a contig
    //
    if(isReadFromStart(CID)) { setReadFromStart(false, CID); }
    else { setReadFromStart(true, CID); }
}
//HO 

/******************************************************************************
** NAIVE++
******************************************************************************/
/*HV*/ int
ContigMemWrapper::
/*HV*/ makeSupplementaryNaiveContigs(std::string fileName)
//HO ;
{
    //-----
    // Join together short naive contigs to make longer
    // sequences which are closable..
    // 
    // The criteria is that each contig must be shorter than the lowestUpperCut for the dataset
    // and that the sum of the length of the two contigs minus the readlength must be greater.
    //
    // write all the supps to file
    //
    logInfo("Printing Supplementry Naive Contigs to: " << fileName, 4);
    ofstream my_file(fileName.c_str(), ofstream::binary);

    int num_made = 0;
    int max_rank = 4;                                      // the maximum rank we'll consider
    NodeBlockElem nbe;
    GenericNodeElem gne;
    GenericNodeEdgeElem gnee(UNINODE);
    GenericNodeWalkingElem gnwe(UNINODE);
    ContigId current_contig;

    // get our bounds for whether we keep or reject a simple context seed
    uMDInt lowestUpper = mDataSet->getLowestUpperCut();

    // store prospective contigs vs their lengths
    std::map<ContigId, uMDInt> con_map;
    std::map<ContigId, uMDInt>::iterator con_map_iter;

    // go through all the cross nodes
    mNodes->getCrossBlock_UN(&nbe);
    while(mNodes->nextNBElem(&nbe))
    {
        // make sure that this cross node does not have rank greater than max_rank
        // we don't want to mess with guys that have 1,000,000,000 edges
        if(mNodes->getNtRank(UNINODE, nbe.NBE_CurrentNode) <= max_rank)
        {
            // clear any older results
            con_map.clear();
            // get all the outgoing edges in contigs which are too small to be seeds
            // store the good guys in the map
            if(mNodes->getEdges(&gnee, GN_EDGE_STATE_NTRANS, 1, nbe.NBE_CurrentNode))
            {
                do {
                    current_contig = mNodes->getContig(UNINODE, mNodes->getOlapNode(gnee));
                    uMDInt this_seq_length = getSeqLength(current_contig);
                    if(this_seq_length != 0 && this_seq_length < lowestUpper)
                    {
                        con_map[current_contig] = this_seq_length;
                    }
                } while(mNodes->getNextEdge(&gnee));
            }
            // now we scan through all the incoming edges
            if(mNodes->getEdges(&gnee, GN_EDGE_STATE_NTRANS, -1, nbe.NBE_CurrentNode))
            {
                do {
                    current_contig = mNodes->getContig(UNINODE, mNodes->getOlapNode(gnee));
                    uMDInt this_seq_length = getSeqLength(current_contig);
                    // make sure this guy is too short to be a seed on it's own...
                    if(this_seq_length != 0 && this_seq_length < lowestUpper)
                    {
                        // now we need to check and see if there are any dualnodes in this
                        // uninode contig with pairs in the map we made just above
                        if(mNodes->startWalk(&gnwe, nbe.NBE_CurrentNode, mNodes->getOlapNode(gnee)))
                        {
                            bool force_break = false;
                            do {
                                if(mNodes->getElem(&gne, mNodes->getUniNode(mNodes->getCurrentNode(gnwe))))
                                {
                                    // this is the head
                                    while(mNodes->getNextElem(&gne))
                                    {
                                        // look for this contig in the map
                                        con_map_iter = con_map.find(mNodes->getContig(UNINODE, mNodes->getPair(gne.GNE_GID)));
                                        if(con_map_iter != con_map.end())
                                        {
                                            // it's in the map, check that the combo of these two guys is long enough 
                                            // to make a viable seed
                                            if((con_map_iter->second + this_seq_length - (uMDInt)mReadLength) > lowestUpper)
                                            {
                                                // we can make a seed!
                                                std::string raw_seq1, raw_seq2;
                                                getContigSequence(&raw_seq2, 0, 0, con_map_iter->first);
                                                
                                                // make sure these two guys are aligned ok...
                                                // there are four possible ways these guys can be arranged
                                                if(getTrueStartNode(con_map_iter->first) == getTrueStartNode(current_contig))
                                                {
                                                    if(getTrueEndNode(con_map_iter->first) != getTrueEndNode(current_contig))
                                                    {
                                                        // reverse current_contig and cc goes first
                                                        reverseContig(current_contig);
                                                        getContigSequence(&raw_seq1, 0, 0, current_contig);
                                                        num_made++;
                                                        my_file << ">" << num_made << "\n" << raw_seq1 << (raw_seq2.substr(mReadLength, raw_seq2.length() - mReadLength)) << std::endl;
                                                    }
                                                    force_break = true;
                                                    break;
                                                }
                                                else if(getTrueEndNode(con_map_iter->first) == getTrueEndNode(current_contig))
                                                {
                                                    if(getTrueStartNode(con_map_iter->first) != getTrueStartNode(current_contig))
                                                    {
                                                        // reverse current_contig and cc goes last
                                                        reverseContig(current_contig);
                                                        getContigSequence(&raw_seq1, 0, 0, current_contig);
                                                        num_made++;
                                                        my_file << ">" << num_made << "\n" << raw_seq2 << (raw_seq1.substr(mReadLength, raw_seq1.length() - mReadLength)) << std::endl;
                                                    }
                                                    force_break = true;
                                                    break;
                                                }
                                                else if(getTrueStartNode(con_map_iter->first) == getTrueEndNode(current_contig))
                                                {
                                                    if(getTrueEndNode(con_map_iter->first) != getTrueStartNode(current_contig))
                                                    {
                                                        // cc goes first
                                                        getContigSequence(&raw_seq1, 0, 0, current_contig);
                                                        num_made++;
                                                        my_file << ">" << num_made << "\n" << raw_seq1 << (raw_seq2.substr(mReadLength, raw_seq2.length() - mReadLength)) << std::endl;
                                                    }
                                                    force_break = true;
                                                    break;
                                                }
                                                else //(getTrueEndNode(con_map_iter->first) == getTrueStartNode(current_contig))
                                                {
                                                    if(getTrueStartNode(con_map_iter->first) != getTrueEndNode(current_contig))
                                                    {
                                                        // cc goes last
                                                        getContigSequence(&raw_seq1, 0, 0, current_contig);
                                                        num_made++;
                                                        my_file << ">" << num_made << "\n" << raw_seq2 << (raw_seq1.substr(mReadLength, raw_seq1.length() - mReadLength)) << std::endl;
                                                    }
                                                    force_break = true;
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                }
                                // we only need to find one pair to make the seed, so break.
                                if(force_break)
                                    break;
                            } while(mNodes->rankStep(&gnwe));
                        }
                    }
                } while(mNodes->getNextEdge(&gnee));
            }
        }
    }
    
    my_file.close();
    logInfo("Done", 40);

    return num_made;
}
//HO

/******************************************************************************
** PRINTING AND OUTPUT
******************************************************************************/

/*HV*/ void
ContigMemWrapper::
/*HV*/ printAllContigInfo(void)
//HO ;
{
    //-----
    // Print the information about all contigs
    //
    contigListIterator contig_iter = mContigs->begin();
    while(contig_iter != mContigs->end())
    {
        printContigInfo(*contig_iter);
        contig_iter++;
    }
}
//HO 

/*HV*/ void
ContigMemWrapper::
/*HV*/ printContigInfo(ContigId CID)
//HO ;
{
    //-----
    // Print all the information about a contig
    //
    std::cout << CID <<
            " : " << getTrueStartNode(CID) <<
            " -> " << getTrueNextStartNode(CID) <<
            " -> " << getTrueNextEndNode(CID) <<
            " -> " << getTrueEndNode(CID) <<
            " N: " << getNodeLength(CID) <<
            " C: " << getCoverage(CID) << std::endl;
}
//HO 

/*HV*/ void
ContigMemWrapper::
/*HV*/ printAllContigs(std::string fileName)
//HO ;
{
    //-----
    // Print all the contigs to file or to standard out
    //
    logInfo("Printing Contigs to: " << fileName, 4);
    ofstream my_file(fileName.c_str(), ofstream::binary);
    std::string tmp_str;
    contigListIterator contig_iter = mContigs->begin();
    while(contig_iter != mContigs->end())
    {
        getFastaSequence(&tmp_str, *contig_iter);
        my_file << tmp_str;
        contig_iter++;
    }
    my_file.close();
    logInfo("Done", 40);
}
//HO 

#if __USE_RI

/*HV*/ void
ContigMemWrapper::
/*HV*/ printAllMappings(std::string fileName)
//HO ;
{
    //-----
    // Print all the contigs read mappings to file or to standard out
    //
    logInfo("Printing Mappings to: " << fileName, 4);
    ofstream my_file(fileName.c_str(), ofstream::binary);
    contigListIterator contig_iter = mContigs->begin();
    while(contig_iter != mContigs->end())
    {
        printMapping(&my_file, *contig_iter);
        contig_iter++;
    }
    my_file.close();
}
//HO 

/*HV*/ void
ContigMemWrapper::
/*HV*/ printMapping(ofstream * printFile, ContigId CID)
//HO ;
{
    //-----
    // Print the read mappings for a contig to file
    //
    // get us a walking edge
    GenericNodeWalkingElem walking_edge(mNc);

    uMDInt position = 0;
    uMDInt context_pos = 0;
    if(mNodes->startWalk(&walking_edge, getTrueStartNode(CID), getTrueNextStartNode(CID)))
    {
        // do the first node explicitly
        context_pos = mNodes->getCntxPos(mNodes->getPrevNode(walking_edge));
        if(!mNodes->isDummy(mNodes->getPrevNode(walking_edge)))
        {
            printMappingAtMaster(printFile, mNodes->getPrevNode(walking_edge), 0 ,CID);
        }
        while(mNodes->rankStep(&walking_edge)){
            // do the rest of the nodes
            uMDInt new_pos = mNodes->getCntxPos(mNodes->getCurrentNode(walking_edge));
            if(new_pos > context_pos)
                position += (new_pos - context_pos);
            else
                position += (context_pos - new_pos);
            context_pos = new_pos;
            if(!mNodes->isDummy(mNodes->getCurrentNode(walking_edge)))
                printMappingAtMaster(printFile, mNodes->getCurrentNode(walking_edge), position ,CID);
        }
    }
}
//HO 

/*HV*/ void
ContigMemWrapper::
/*HV*/ printMappingAtMaster(ofstream * printFile, GenericNodeId GID, uMDInt pos, ContigId CID)
//HO ;
{
    //-----
    // Given a master node at a position which is NOt a dummy
    // print the mapping info for this guy and for all the slaves...
    //
    // first do this guy...
    *printFile << getMappingString(GID, pos, CID) << std::endl;
    // now do the slaves
    GenericNodeEdgeElem gnee(DUALNODE);
    if(mNodes->getEdges(&gnee, GN_EDGE_STATE_TRANS, 0, GID))
    {
        do {
            *printFile << getMappingString(mNodes->getOlapNode(gnee), pos, CID) << std::endl;
        } while(mNodes->getNextEdge(&gnee));
    }
}
//HO 

/*HV*/ std::string
ContigMemWrapper::
/*HV*/ getMappingString(GenericNodeId GID, uMDInt pos, ContigId CID)
//HO ;
{
    //-----
    // get the string for a mapping
    //
    stringstream tmp_stream;
    char strand;
    if(mNodes->isCntxReversed(GID) ^ mNodes->isFileReversed(GID))
        strand = '-';
    else
        strand = '+';
    
    tmp_stream << CID << "\t" << mNodes->getReadId(GID) << "\t" << pos << "\t" << (pos + mReadLength) << "\t" << strand << "\t" << mNodes->getReadId(mNodes->getPair(GID));
    return tmp_stream.str();
}
//HO 
#endif

/*HV*/ void
ContigMemWrapper::
/*HV*/ printContig(std::string fileName, ContigId CID)
//HO ;
{
    //-----
    // Print all the contigs to file or to standard out
    //
    string tmp_str;
    ofstream my_file(fileName.c_str(), ofstream::binary);
    getFastaSequence(&tmp_str, CID);
    my_file << tmp_str;
    my_file.close();
}
//HO 

/*HV*/ void
ContigMemWrapper::
/*HV*/ getFastaSequence(std::string * sequence, ContigId CID)
//HO ;
{
    //-----
    // Get the sequence for a contig in pretty fasta format
    //
    
    // get the raw sequence
    std::string raw_seq;
    getContigSequence(&raw_seq, 0, 0, true, NULL, false, CID);

    // make the fasta header
    std::ostringstream buffer;
    if(mNc == UNINODE)
    {
        std::string x_s = "_C";
        std::string x_e = "_C";
        if(mNodes->getNodeType(mNc, getTrueStartNode(CID)) == GN_NODE_TYPE_CROSS)
            x_s = "_X";
        if(mNodes->getNodeType(mNc, getTrueEndNode(CID)) == GN_NODE_TYPE_CROSS)
            x_e = "_X";
    #ifdef SOLID_DATA_
        buffer << ">" << CID << "_S_" << getTrueStartNode(CID) << x_s << "_E_" << getTrueEndNode(CID) << x_e << "_LEN_" << getSeqLength(CID) << "_COV_" << mCoverageMap[CID] << "\n";
    #else
        buffer << ">" << CID << "_S_" << getTrueStartNode(CID) << x_s << "_E_" << getTrueEndNode(CID) << x_e << "_LEN_" << getSeqLength(CID)  << "_GC_" << mGCMap[CID] << "_COV_" << mCoverageMap[CID] << "\n";
    #endif
    }
    else
    {
    #ifdef SOLID_DATA_
        buffer << ">" << mNodes->getCntxId(getTrueEndNode(CID)) << "_LEN_" << getSeqLength(CID) << "_COV_" << mCoverageMap[CID] << "\n";
    #else
        buffer << ">" << mNodes->getCntxId(getTrueEndNode(CID)) << "_LEN_" << getSeqLength(CID) << "_COV_" << mCoverageMap[CID] << "_GC_" << mGCMap[CID] << "\n";
    #endif
    }
    
    // now fastafy it
    cutRawSeq(&raw_seq, &buffer);
    *sequence = buffer.str();
}
//HO 

/*HV*/ void
ContigMemWrapper::
/*HV*/ cutRawSeq(std::string * raw_seq, std::ostringstream * buffer)
//HO ;
{
    //-----
    // Cut down the raw sequence into the "fasta" type
    // split lines
    //
    int start_index = 0;
    int total_length = raw_seq->length();
    
    while(1)
    {
        if(total_length <= (SAS_DEF_FASTA_LINE_LENGTH + start_index))
        {
            *buffer << raw_seq->substr(start_index) << "\n";
            break;
        }
        else
        {
            *buffer << raw_seq->substr(start_index, SAS_DEF_FASTA_LINE_LENGTH) << "\n";
        }
        start_index += SAS_DEF_FASTA_LINE_LENGTH;
    }
}

/*HV*/ int
ContigMemWrapper::
/*HV*/ getContigSequenceLengthManual(ContigId CID)
//HO ;
{
    //-----
    // Get the length without building the sequence
    //
    int len = mReadLength;
    
    // get us a walking edge
    GenericNodeWalkingElem walking_edge(mNc);

    if(mNodes->startWalk(&walking_edge, getTrueStartNode(CID), getTrueNextStartNode(CID)))
    {
        len += fAbs(mNodes->getNtOffset(mNc, mNodes->getCurrentNode(walking_edge), mNodes->getPrevNode(walking_edge)));
        while(mNodes->rankStep(&walking_edge)){
            len += fAbs(mNodes->getNtOffset(mNc, mNodes->getCurrentNode(walking_edge), mNodes->getPrevNode(walking_edge)));
        }
    }
    return len;
}
//HO 

//HO inline void getContigSequence(std::string * sequence, int trimFront, int trimEnd, ContigId CID) { getContigSequence(sequence, trimFront, trimEnd, false, NULL, false, CID); }
//HO
//HO inline void getContigSequence(std::string * sequence, bool doGC, ContigId CID) { getContigSequence(sequence, 0, 0, doGC, NULL, false, CID); }
//HO

/*HV*/ void
ContigMemWrapper::
/*HV*/ getContigSequence(std::string * sequence, int trimFront, int trimEnd, bool doGC, GenericNodeId * startNode, bool doRC, ContigId CID)
//HO ;
{
    //-----
    // Get the sequence for a contig
    //
    std::stringstream buffer;
    std::string test_str_1, test_str_2, tmp_str;
    sMDInt offset;

    // reset the coverage and GC maps
    mCoverageMap[CID] = 0;
    mGCMap[CID] = -1;
    
    // get us a walking edge
    GenericNodeWalkingElem walking_edge(mNc);
    
    bool start_at_end = false;
    
    // work out what we're doing with the start node
    GenericNodeId start_GID, next_start_GID;
    if(NULL != startNode)
    {
        // the user specified a startnode
        start_GID = *startNode;
        if(*startNode == getTrueStartNode(CID))
            next_start_GID = getTrueNextStartNode(CID);
        else
            next_start_GID = getTrueNextEndNode(CID); start_at_end = true;
    }
    else
    {
        // any end will do...
        start_GID = getTrueStartNode(CID);
        next_start_GID = getTrueNextStartNode(CID);
    }

    if(mNodes->startWalk(&walking_edge, start_GID, next_start_GID))
    {
        tmp_str = mNodes->getSequence(start_GID);
        
        offset = mNodes->getOffset(walking_edge);
        
        if(offset < 0)
        {
            mReads->revCmpSequence(&tmp_str);
        }

        offset = fAbs(offset);
        
        buffer << tmp_str;
        if(mNc == DUALNODE)
        {
            if(!mNodes->isDummy(start_GID))
            {
                mCoverageMap[CID] += mNodes->getCntxReadDepth(start_GID);
            }
        }
        else
            mCoverageMap[CID] += mNodes->getReadDepth(start_GID);
        
        test_str_1 = (tmp_str).substr(offset, mReadLength - offset);
        tmp_str =  mNodes->getSequence(mNodes->getCurrentNode(walking_edge));
        
        test_str_2 = (tmp_str).substr(0, mReadLength - offset);
        if(test_str_1 != test_str_2)
        {
            mReads->revCmpSequence(&tmp_str);
        }

        buffer << tmp_str.substr(mReadLength - offset, offset);
        if(mNc == DUALNODE)
        {
            if(!mNodes->isDummy(mNodes->getCurrentNode(walking_edge)))
            {
                mCoverageMap[CID] += mNodes->getCntxReadDepth(mNodes->getCurrentNode(walking_edge));
            }
        }
        else
            mCoverageMap[CID] += mNodes->getReadDepth(mNodes->getCurrentNode(walking_edge));

        while(mNodes->rankStep(&walking_edge))
        {
            offset = mNodes->getOffset(walking_edge);
            if(offset < 0)
                offset *= -1;

            test_str_1 = (tmp_str).substr(offset, mReadLength - offset);
            tmp_str =  mNodes->getSequence(mNodes->getCurrentNode(walking_edge));
            test_str_2 = (tmp_str).substr(0, mReadLength - offset);
            
            if(test_str_1 != test_str_2)
            {
                mReads->revCmpSequence(&tmp_str);
            }
            
            buffer << tmp_str.substr(mReadLength - offset, offset);
            if(mNc == DUALNODE)
            {
                if(!mNodes->isDummy(mNodes->getCurrentNode(walking_edge)))
                {
                    mCoverageMap[CID] += mNodes->getCntxReadDepth(mNodes->getCurrentNode(walking_edge));
                }
            }
            else
                mCoverageMap[CID] += mNodes->getReadDepth(mNodes->getCurrentNode(walking_edge));
        }
    }
    
    // make the sequence - reverse complement if need be
    if(doRC)
        *sequence = revCmpSequence(buffer.str());
    else
        *sequence = buffer.str();

    // trim if need be
    int original_length = sequence->length();
    if(0 != trimFront || 0 != trimEnd)
    {
        int rem_length = original_length - trimFront - trimEnd;
        *sequence = sequence->substr(trimFront, rem_length);
    }
        
    // now average out the coverage map
    if(sequence->length() != 0)
        mCoverageMap[CID] = mCoverageMap[CID] * mReadLength / original_length;
    
    // now get the GC (if asked for)
    if(doGC)
        mGCMap[CID] = getGC(sequence);
}
//HO 

/*HV*/ std::string
ContigMemWrapper::
/*HV*/ getContigEnd(uMDInt len, bool rc, GenericNodeId startNode, ContigId CID)
//HO ;
{
    //-----
    // return the last bb bases of the Context's contig
    // reversed if necessary
    //
    if(!isValidAddress(CID))
    {
        logError("Invalid: " << CID);
        return "Invalid CONTIG ID";
    }
    
    std::stringstream buffer;
    std::string test_str_1, test_str_2, tmp_str;
    sMDInt offset;
    GenericNodeId next_start_node = mNodes->getNullGenericNodeId();
    
    // get us a walking edge
    GenericNodeWalkingElem walking_edge(mNc);
    if(startNode == getTrueStartNode(CID)) { next_start_node = getTrueNextStartNode(CID); }
    else if (startNode == getTrueEndNode(CID)) { next_start_node = getTrueNextEndNode(CID); rc = !rc; }
    else{ logError("Not a node of this contig"); }
    
    if(mNodes->startWalk(&walking_edge, startNode, next_start_node))
    {
        tmp_str = mNodes->getSequence(startNode);
        
        offset = mNodes->getOffset(walking_edge);
        
        if(offset < 0)
        {
            mReads->revCmpSequence(&tmp_str);
            offset *= -1;
        }
        
        buffer << tmp_str;
        
        // get the length of the buffer
        if(buffer.str().length() >= len)
        {
            if(rc)
                return revCmpSequence(buffer.str().substr(0, len));
            else
                return buffer.str().substr(0, len);
        }
        
        test_str_1 = (tmp_str).substr(offset, mReadLength - offset);
        tmp_str =  mNodes->getSequence(mNodes->getCurrentNode(walking_edge));
        
        test_str_2 = (tmp_str).substr(0, mReadLength - offset);
        if(test_str_1 != test_str_2)
        {
            mReads->revCmpSequence(&tmp_str);
        }
        
        buffer << tmp_str.substr(mReadLength - offset, offset);
        // get the length of the buffer
        if(buffer.str().length() >= len)
        {
            if(rc)
                return revCmpSequence(buffer.str().substr(0, len));
            else
                return buffer.str().substr(0, len);
        }
        
        while(mNodes->rankStep(&walking_edge))
        {
            offset = mNodes->getOffset(walking_edge);
            if(offset < 0)
                offset *= -1;
            
            test_str_1 = (tmp_str).substr(offset, mReadLength - offset);
            tmp_str =  mNodes->getSequence(mNodes->getCurrentNode(walking_edge));
            test_str_2 = (tmp_str).substr(0, mReadLength - offset);
            
            if(test_str_1 != test_str_2)
            {
                mReads->revCmpSequence(&tmp_str);
            }
            
            buffer << tmp_str.substr(mReadLength - offset, offset);
            if(buffer.str().length() >= len)
            {
                if(rc)
                    return revCmpSequence(buffer.str().substr(0, len));
                else
                    return buffer.str().substr(0, len);
            }
        }
    }
    else
    {
        logError("Could not start walk");
        return "COULD NOT START WALK";
    }
    if(rc)
        return revCmpSequence(buffer.str().substr(0, len));
    else
        return buffer.str().substr(0, len);
    
    return "END_ERROR";
}
//HO 

/*HV*/ double
ContigMemWrapper::
/*HV*/ getGC(std::string * sequence)
//HO ;
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

/*HV*/ void
ContigMemWrapper::
/*HV*/ printContigGraph(bool show_names)
//HO ;
{
    //-----
    // print the basic contig graph
    //
    // get the crossNodes
    printContigGraph2(show_names);
}
//HO 


/*HV*/ void
ContigMemWrapper::
/*HV*/ printContigGraph(bool show_names, ContextId CTXID)
//HO ;
{
    //-----
    // print the basic contig graph
    //
    // get the crossNodes
    setCurrentContextId(CTXID);
    printContigGraph2(show_names);
}
//HO 


/*HV*/ void
ContigMemWrapper::
/*HV*/ printContigGraph2(bool show_names)
//HO ;
{
    //-----
    // print the basic contig graph
    //

    // get the latest caps and crosses
    contigListIterator contig_iter = mContigs->begin();

    NodeBlockElem cross_nodes;
    getCrossBlock(&cross_nodes);

    int map_limit = 10;
    string heatPalette[11] = { "color=11, ", "color=10, ", "color=9, ", "color=8, ", "color=7, ", "color=6, ", "color=5, ", "color=4, ", "color=3, ", "color=2, ", "color=1, "};
    int heat_map[11] = {33,43,53,63,93,123,183,300,800,1600,2300};

    if(show_names)
    {
        std::cout << "graph contigs {\n  rankdir=LR;\n  size=\"150,150\";\n  node [shape = circle];" << std::endl;
        
        while(mNodes->nextNBElem(&cross_nodes))
        {
            GenericNodeEdgeElem gnee(mNc);
            if(mNodes->getEdges(&gnee, GN_EDGE_STATE_NTRANS, 0, cross_nodes.NBE_CurrentNode))
            {
                do {
                    if(mNodes->getNodeType(mNc, mNodes->getOlapNode(gnee)) == GN_NODE_TYPE_CROSS)
                    {
                        std::cout << "  \"" << cross_nodes.NBE_CurrentNode << "\" -- \"" << mNodes->getOlapNode(gnee) << "\";" << std::endl;
                    }
                } while(mNodes->getNextEdge(&gnee));
            }
        }
        while(contig_iter != mContigs->end())
        {
            int indexer = 0;
            int len = getSeqLength(*contig_iter);
            
            do {
                if(indexer >= map_limit)
                    break;
                indexer++;
            } while(heat_map[indexer] < len);
            std::cout << "  \"" << *contig_iter << "\" [colorscheme=rdylgn11, " << (heatPalette[indexer]) << "shape=doublecircle, label=\"" << *contig_iter << "_" << getSeqLength(*contig_iter) << "\"];" << std::endl;
            std::cout << "  \"" << getTrueStartNode(*contig_iter) << "\" -- \"" << *contig_iter << "\";" << std::endl;
            std::cout << "  \"" << getTrueEndNode(*contig_iter) << "\" -- \"" << *contig_iter << "\";" << std::endl;
            contig_iter++;
        }
    }
    else
    {
        std::cout << "graph contigs {\n  rankdir=LR;\n  size=\"150,150\";\n" << std::endl;

        while(mNodes->nextNBElem(&cross_nodes))
        {
            GenericNodeEdgeElem gnee(mNc);
            if(mNodes->getEdges(&gnee, GN_EDGE_STATE_NTRANS, 0, cross_nodes.NBE_CurrentNode))
            {
                do {
                    if(mNodes->getNodeType(mNc, mNodes->getOlapNode(gnee)) == GN_NODE_TYPE_CROSS)
                    {
                        std::cout << "  \"" << cross_nodes.NBE_CurrentNode << "\" [shape=point, label=\"\"];" << std::endl;
                        std::cout << "  \"" << cross_nodes.NBE_CurrentNode << "\" -- \"" << mNodes->getOlapNode(gnee) << "\";" << std::endl;
                    }
                } while(mNodes->getNextEdge(&gnee));
            }
        }
        while(contig_iter != mContigs->end())
        {
            int indexer = 0;
            int len = getSeqLength(*contig_iter);
            
            do {
                if(indexer >= map_limit)
                    break;
                indexer++;
            } while(heat_map[indexer] < len);
            std::cout << "  \"" << *contig_iter << "\" [colorscheme=rdylgn11, " << (heatPalette[indexer]) << "shape=doublecircle, label=\"" << *contig_iter << "_" << getSeqLength(*contig_iter) << "\"];" << std::endl;
            
            if((mNodes->getNodeType(mNc, getTrueStartNode(*contig_iter)) == GN_NODE_TYPE_CAP) || (mNodes->getNodeType(mNc, getTrueStartNode(*contig_iter)) == GN_NODE_TYPE_CROSS))
            {
                std::cout << "  \"" << getTrueStartNode(*contig_iter) << "\" [shape=point, label=\"\"];" << std::endl;
            }
            if((mNodes->getNodeType(mNc, getTrueEndNode(*contig_iter)) == GN_NODE_TYPE_CAP) || (mNodes->getNodeType(mNc, getTrueEndNode(*contig_iter)) == GN_NODE_TYPE_CROSS))
            {
                std::cout << "  \"" << getTrueEndNode(*contig_iter) << "\" [shape=point, label=\"\"];" << std::endl;
            }
            std::cout << "  \"" << getTrueStartNode(*contig_iter) << "\" -- \"" << *contig_iter << "\";" << std::endl;
            std::cout << "  \"" << getTrueEndNode(*contig_iter) << "\" -- \"" << *contig_iter << "\";" << std::endl;
            contig_iter++;
        }
    }
    std::cout << "}" << std::endl;
}
//HO 

/*HV*/ bool
ContigMemWrapper::
/*HV*/ makeContigStats(int * n50, int * longestContig, int * totalBases)
//HO ;
{
    //-----
    // update our list of contig stats, we assume these guys are sorted in descending order
    //
    *n50 = 0;
    *longestContig = 0;
    *totalBases = 0;
    contigListIterator all_contigs_iter = mContigs->begin();
    contigListIterator all_contigs_last = mContigs->end();
    while(all_contigs_iter != all_contigs_last)
    {
        int tmp_length = (int)getSeqLength(*all_contigs_iter);
        *totalBases += tmp_length;
        if(tmp_length > *longestContig)
            *longestContig = tmp_length;
        all_contigs_iter++;
    }

    int n50_cut = *totalBases/2;
    // now get the n50
    all_contigs_iter = mContigs->begin();
    while(all_contigs_iter != all_contigs_last)
    {
        n50_cut -= (int)getSeqLength(*all_contigs_iter);
        if(n50_cut < 0)
        {
            *n50 = (int)getSeqLength(*all_contigs_iter);
            return true;
        }
        all_contigs_iter++;
    }
    return false;
}
//HO 

/*HV*/ std::string
ContigMemWrapper::
/*HV*/ revCmpSequence(std::string sequence)
//HO ;
{
    //-----
    // get the reverse complement of a sequence
    //
    int i = sequence.length();
    const char * in_buff_end = sequence.c_str() + i - 1;
    
    std::stringstream buffer;
    
    // just do it!!!
    while(0 != i)
    {
        switch(*in_buff_end)
        {
            case SAS_CE_D_0:
                buffer << SAS_CE_R_0;
                break;
            case SAS_CE_D_1:
                buffer << SAS_CE_R_1;
                break;
            case SAS_CE_D_2:
                buffer << SAS_CE_R_2;
                break;
            case SAS_CE_D_3:
                buffer << SAS_CE_R_3;
                break;
        }
        i--;
        in_buff_end--;
    }
    return buffer.str();
}
//HO 

/*HV*/ void
ContigMemWrapper::
/*HV*/ writeNaiveCoverageMap(std::string fileName)
//HO ;
{
    //-----
    // print out the ContigId vs coverage to a txt file
    //
    logInfo("Printing Contig coverages to: " << fileName, 4);
    ofstream my_file(fileName.c_str(), ofstream::binary);
    std::string tmp_str;
    contigListIterator contig_iter = mContigs->begin();
    while(contig_iter != mContigs->end())
    {
        my_file << hex << contig_iter->getGuts() << dec << "\t" << mCoverageMap[*contig_iter] << std::endl;
        contig_iter++;
    }
    my_file.close();
    logInfo("Done", 40);
}
//HO 

