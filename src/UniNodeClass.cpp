/******************************************************************************
**
** File: UniNodeClass.cpp
**
*******************************************************************************
**
** Layer 3 Implementation file for use with projects including MemManager.cpp
**
** Public interface for the uninode object
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

// local includes
#include "UniNodeMemWrapper.h"
#include "intdef.h"
#include "IdTypeDefs.h"
#include "UniNodeClass.h"
#include "ReadStoreClass.h"

/******************************************************************************
** #INCLUDES #DEFINES AND PRIVATE VARS
******************************************************************************/
//HO typedef UN_EDGE_ELEM UniNodeEdgeElem;
//HO 
//HO uMDInt mNaiveOffset;
//HO 
/******************************************************************************
** INTIALISATION AND SAVING
******************************************************************************/

/*HV*/ void
UniNodeClass::
/*HV*/ extendedDestructor(void)
//HO ;
{
    //-----
    // Add anything you would like to happen when the destructor is called
    //
}
//HO 

/*HV*/ bool
UniNodeClass::
/*HV*/ extendedInitialise(uMDInt originalNumberOfBlocks )
//HO ;
{
    //-----
    // Add anything you would like to happen on a regular intialisation call
    //
    mNaiveOffset = SAS_DEF_UN_OFFSET_DEF_MIN;
    return true;
}
//HO 

/*HV*/ bool
UniNodeClass::
/*HV*/ extendedInitialise(std::string fileName)
//HO ;
{
    //-----
    // Add anything you would like to happen when reloading a saved instance
    //
    mNaiveOffset = SAS_DEF_UN_OFFSET_DEF_MIN;
    return true;
}
//HO 

/*HV*/ bool
UniNodeClass::
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
//HO inline void setObjectPointers(ReadStoreClass * RSC) { mData->setObjectPointers(RSC); }
//HO 

//HO inline void setCurrentMaxOffset(uMDInt mo) { mData->setCurrentMaxOffset(mo); }
//HO 

//HO inline void setNaiveOffset(uMDInt no) { mNaiveOffset = no; mData->setNaiveOffset(no); }
//HO 
/******************************************************************************
** HOUSEKEEPING
******************************************************************************/
    /*
    ** Inline wrapper for adding a node UniNode object
    */
//HO inline UniNodeId addNode(ReadStoreId RID) { return mData->addNode(RID); }
//HO 

//HO inline bool getUniNodeId(UniNodeId * UID, std::string * sequence) { return mData->getUniNodeId(UID, sequence); }
//HO 

//HO inline void killKmerLists(void) { mData->killKmerLists(); }
//HO 
/******************************************************************************
** NAVIGATION
******************************************************************************/
    /*
    ** Get the start of the edges for a node
    */
//HO inline bool getEdges(UN_EDGE_ELEM * ee, uMDInt stateWanted, sMDInt offsetWanted, UniNodeId baseNode) { return mData->getEdges(ee, stateWanted, offsetWanted, baseNode); }
//HO 

    /*
    ** Get hold of the cap nodes
    */
//HO inline bool getNextEdge(UN_EDGE_ELEM * ee) { return mData->getNextEdge(ee); }
//HO 

    /*
    ** Forpath walking. Start at a position in the graph
    */
/*HV*/ bool
UniNodeClass::
/*HV*/ startWalk(UN_WALKING_ELEM * data, UniNodeId startNode, UniNodeId nextNode)
//HO ;
{
    //-----
    // Start a walk at the startNode in the direction of the nextNode.
    // ie, set the first edge.
    //
    // we only ever walk onlong non-transitive edges
    data->UNWE_StateWanted = UN_EDGE_STATE_NTRANS;
    data->UNWE_State = UN_EDGE_STATE_NTRANS;

    // set prev to be the start node and curr to be the following node
    data->UNWE_CurrentNode = nextNode;
    data->UNWE_PrevNode = startNode;
    
    // We never use these
    data->UNWE_WhichData = mData->getNullUniNodeDataId();
    data->UNWE_WhichOlap = 0;
    
    // now find the edge connecting the current node to the prev node
    UN_EDGE_ELEM tmp_edge;
    if(mData->getEdges(&tmp_edge, UN_EDGE_STATE_NTRANS, 0, data->UNWE_CurrentNode))
    {
        do {
            if(tmp_edge.UNEE_OlapNode == data->UNWE_PrevNode)
            {
                // we will need to walk in the opposite direction to this...
                data->UNWE_OffsetWanted = -1 * tmp_edge.UNEE_Offset;

                // make these guys fit
                data->UNWE_C2POffset = tmp_edge.UNEE_Offset;
                data->UNWE_P2COffset = tmp_edge.UNEE_ReturnOffset;
                
                return true;
            }
        } while(mData->getNextEdge(&tmp_edge));
    }
    logError("Walk could not be started: " << startNode << " --> " << nextNode);
    return false;
}
//HO 
//VO 

    /*
    ** step one node in the graph if next node is both available (non-cap)
    ** and un ambiguous (non-cross). Two methods here depending on what you
    ** want to do
    */
/*HV*/ bool
UniNodeClass::
/*HV*/ rankStep(UN_WALKING_ELEM * data)
//HO ;
{
    //-----
    // Test the rank of the current_node (base_node) and act accordingly
    // VEry robust, can not really stray far
    //
    PARANOID_ASSERT_L2((getNodeType(data->UNEE_BaseNode)!= UN_NODE_TYPE_UNSET));
    PARANOID_ASSERT_L2((getNodeType(data->UNEE_BaseNode)!= UN_NODE_TYPE_DETACHED));
    PARANOID_ASSERT_L2((getNodeType(data->UNEE_BaseNode)!= UN_NODE_TYPE_BANISHED));
    
    switch(mData->getNtRank(data->UNWE_CurrentNode))
    {
        case 2:
        {
            // UN_NODE_TYPE_PATH
            // now find the edge connecting the current node to the prev node
            UN_EDGE_ELEM tmp_edge;
            if(mData->getEdges(&tmp_edge, UN_EDGE_STATE_NTRANS, data->UNWE_OffsetWanted, data->UNWE_CurrentNode))
            {
                // set prev to be the old_current and curr to be the overlaping node
                data->UNWE_PrevNode = data->UNWE_CurrentNode;
                data->UNWE_CurrentNode = tmp_edge.UNEE_OlapNode;
                
                // we will need to walk in the opposite direction to this...
                data->UNWE_OffsetWanted = -1 * tmp_edge.UNEE_ReturnOffset;
                
                // make these guys fit
                data->UNWE_C2POffset = tmp_edge.UNEE_ReturnOffset;
                data->UNWE_P2COffset = tmp_edge.UNEE_Offset;
                return true;
                    
            }
            // if something goes wrong with the walking code, check here!
            // logError("Could not continue walk: " << data->UNWE_PrevNode << " --> " << data->UNWE_CurrentNode);
            break;
        }
        case 0:
            // UN_NODE_TYPE_UNSET or detached? Should be banished
            logError("Trying to get NtRank for RANK: 0 " << mData->sayNodeTypeLikeAHuman(getNodeType(data->UNEE_BaseNode)) << ": " << data->UNEE_BaseNode);
            break;
        default:
            // UN_NODE_TYPE_CROSS OR CAP
            break;
    }
    return false;
}
//HO 
//VO 

    /*
    ** Step one node in the graph provided that we are still walking on the same 
    ** uninode contig. Ie. the head ofr both prev and current will both
    ** have the same contig ID and we only step forward if this can be maintained
    */
/*HV*/ bool
UniNodeClass::
/*HV*/ contigStep(UN_WALKING_ELEM * data)
//HO ;
{
    //-----
    // Keep walking along a given contig in the direction specified by
    // the data elem...
    // 
    // We assume that this method will be used when the contigs have been made
    // on an offset which is either the same or stricter as the current uninode graph
    // thus all non-transitive edges are conserved...
    // Thus crossnodes in the currently existing graph may not be crossnodes in the graph that contigs
    // were made for, but all crossnodes in the graph which the contigs were made for
    // should be present in the currently existing graph.
    //
    // So this is the same as for rank step BUT we don't just stop at crossnodes,
    // we can walk trhough if the ContigId matches
    //
    // We can't deal with a null contig ID here so the caller has to make
    // sure that that cannot happen...
    //
    PARANOID_ASSERT_L2((getNodeType(data->UNEE_BaseNode)!= UN_NODE_TYPE_UNSET));
    PARANOID_ASSERT_L2((getNodeType(data->UNEE_BaseNode)!= UN_NODE_TYPE_DETACHED));
    PARANOID_ASSERT_L2((getNodeType(data->UNEE_BaseNode)!= UN_NODE_TYPE_BANISHED));
    
    ContigId CID =  getContig(data->UNWE_CurrentNode);
    
    UN_EDGE_ELEM tmp_edge;
    if(mData->getEdges(&tmp_edge, UN_EDGE_STATE_NTRANS, data->UNWE_OffsetWanted, data->UNWE_CurrentNode))
    {
        do {
            if(0 == tmp_edge.UNEE_Offset)
            {
                logError("Crazy stuff happening here - walk could not be started: " << data->UNWE_PrevNode << " --> " << data->UNWE_CurrentNode);
                return false;
            }
            if(getContig(tmp_edge.UNEE_OlapNode) == CID)
            {
                // set prev to be the old_current and curr to be the overlaping node
                data->UNWE_PrevNode = data->UNWE_CurrentNode;
                data->UNWE_CurrentNode = tmp_edge.UNEE_OlapNode;
                
                // we will need to walk in the opposite direction to this...
                data->UNWE_OffsetWanted = -1 * tmp_edge.UNEE_ReturnOffset;
                
                // make these guys fit
                data->UNWE_C2POffset = tmp_edge.UNEE_ReturnOffset;
                data->UNWE_P2COffset = tmp_edge.UNEE_Offset;

                return true;
            }
        } while(mData->getNextEdge(&tmp_edge));
    }
    return false;
}
//HO 
//VO 

/******************************************************************************
** EDGE STATES GET AND SET
******************************************************************************/
    /*
    ** Mark the edge betwen two nodes as non-transitive
    */
//HO inline bool promoteEdgeBetween(UniNodeId nodeA, UniNodeId nodeB) { return mData->promoteEdgeBetween_symmetrical(nodeA, nodeB); }
//HO 
    
    /*
    ** Mark the edge betwen two nodes as transitive
    */
//HO inline bool demoteEdgeBetween(UniNodeId nodeA, UniNodeId nodeB) { return mData->demoteEdgeBetween_symmetrical(nodeA, nodeB); }
//HO 
    
    /*
    ** Detach the edge betwen two nodes
    */
//HO inline bool detachEdgeBetween(UniNodeId nodeA, UniNodeId nodeB) { return mData->detachEdgeBetween_symmetrical(nodeA, nodeB); }
//HO 
    
    /*
    ** Banish the edge betwen two nodes
    */
//HO inline bool banishEdgeBetween(UniNodeId nodeA, UniNodeId nodeB) { return mData->banishEdgeBetween_symmetrical(nodeA, nodeB); }
//HO 

    /*
    ** Get the offset from node A to node B
    */
//HO inline sMDInt getOffsetBetweenNodes(UniNodeId nodeB, EDGE_STATE state, UniNodeId nodeA) { return mData->getOffsetBetweenNodes(nodeB, state, nodeA); }
//HO 

/******************************************************************************
** EDGE MAKING AND MANAGEMENT
******************************************************************************/
    /*
    ** Inline wrappers for making the edges for a UniNode
    */
//HO inline void prepForEdgeMaking(uMDInt kmerMapSize) { mData->prepForEdgeMaking(kmerMapSize); }
//HO 
//HO inline bool makeEdges(std::vector<UniNodeId> * banishVec, UniNodeId UID) { return mData->makeEdges(banishVec, UID); }
//HO

    /*
    ** Inline wrapper for labelling non-transitive edges
    */
//HO inline bool labelNonTransitiveEdges(std::vector<UniNodeId> * banishVec, int extOffset, UniNodeId UID) { return mData->labelNonTransitiveEdges(banishVec, extOffset, UID); }
//HO 

    /*
    ** Inline wrapper for shallow sorting the nodes
    */
//HO inline void resetEdges(bool keepContigId, UniNodeId UID) { mData->resetEdges(keepContigId, UID); }
//HO 
    
    /*
    ** Inline wrapper for shallow sorting the nodes
    */
//HO inline bool sortEdges(UniNodeId UID) { return mData->shallowSortEdges(UID); }
//HO 

/******************************************************************************
** NODE TYPE MANAGEMENT
******************************************************************************/
    /*
    ** inline wrapper fo detaching a node
    */
//HO inline bool detachNode(map<GenericNodeId, bool> * detMap, UniNodeId UID) { return mData->detachNode(detMap, UID); } 
//HO 
//HO inline bool detachNode(UniNodeId UID) { return mData->detachNode(UID); }
//HO 

    /*
    ** And for banisheing a node
    */
//HO inline bool banishNode(UniNodeId UID) { return mData->banishNode(UID); }
//HO 

/******************************************************************************
** OUTPUT AND PRINTING
******************************************************************************/

    /*
    ** Get a nice version of the state of an edge
    */
//HO inline std::string sayEdgeStateLikeAHuman(uMDInt state) { return mData->sayEdgeStateLikeAHuman(state); }
//HO 
    
    /*
    ** Get a nice version of the type of a node
    */
//HO inline std::string sayNodesTypeLikeAHuman(UniNodeId UID) { return mData->sayNodeTypeLikeAHuman(getNodeType(UID)); }
//HO 

    /*
    ** Print the edges at a node
    */
//HO inline void printEdges(UniNodeId UID) { mData->printEdges(UID); }
//HO 

    /*
    ** Print the sequence for this node
    */
//HO inline std::string getSequence(UniNodeId UID) { return mData->getSequence(UID); }
//HO 
