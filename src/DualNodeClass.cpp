/******************************************************************************
**
** File: DualNodeClass.cpp
**
*******************************************************************************
**
** Layer 3 Implementation file for use with projects including MemManager.cpp
** Supplies the public interfaces for the DualNode Class. These are needed by
** GenericNode Class.
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
#include "DualNodeMemWrapper.h"
#include "intdef.h"
#include "IdTypeDefs.h"
#include "DualNodeClass.h"
#include "MemManager.h"

using namespace std;


/******************************************************************************
** #INCLUDES #DEFINES AND PRIVATE VARS
******************************************************************************/
//INC typedef DN_EDGE_ELEM DualNodeEdgeElem;
//INC 

/******************************************************************************
** INTIALISATION AND SAVING
******************************************************************************/

/*HV*/ void
DualNodeClass::
/*HV*/ extendedDestructor(void)
//HO ;
{
    //-----
    // Add anything you would like to happen when the destructor is called
    //
}
//HO 

/*HV*/ bool
DualNodeClass::
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
DualNodeClass::
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
DualNodeClass::
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
//HO inline void setObjectPointers(UniNodeClass * UNC) { mData->setObjectPointers(UNC); }
//HO 

/******************************************************************************
** HOUSEKEEPING
******************************************************************************/
    /*
    ** Inline wrapper for adding a node UniNode object
    */
//HO inline DualNodeId addNode(GenericNodeId node) { return mData->addNode(node); }
//HO 

    /*
    ** Inline wrapper for adding a node UniNode object
    */
//HO inline bool deleteDualNodeId(DualNodeId ID) { return mData->deleteDualNodeId( ID ); }
//HO 
#if __USE_RI
//HO inline std::string getReadIdString(DualNodeId DID) { return mData->getReadIdString( DID ); }
//HO
#endif

/******************************************************************************
** NAVIGATION
******************************************************************************/
    /*
    ** Get the start of the edges for a node
    */
//HO inline bool getEdges(DN_EDGE_ELEM * ee, uMDInt stateWanted, sMDInt offsetWanted, DualNodeId baseNode) { return mData->getEdges(ee, stateWanted, offsetWanted, baseNode); }
//HO 

    /*
    ** Get hold of the cap nodes
    */
//HO inline bool getNextEdge(DN_EDGE_ELEM * ee) { return mData->getNextEdge(ee); }
//HO 

    /*
    ** Forpath walking. Start at a position in the graph
    */
/*HV*/ bool
DualNodeClass::
/*HV*/ startWalk(DN_WALKING_ELEM * data, DualNodeId startNode, DualNodeId nextNode)
//HO ;
{
    //-----
    // Start a walk at the startNode in the direction of the nextNode.
    // ie, set the first edge.
    //
    // we only ever walk onlong non-transitive edges
    data->DNWE_StateWanted = DN_EDGE_STATE_MASTER;
    data->DNWE_State = DN_EDGE_STATE_MASTER;

    // set base to be the forward node and start to be the following node
    data->DNWE_CurrentNode = nextNode;
    data->DNWE_PrevNode = startNode;

    // these are unused
    data->DNWE_WhichData = mData->getNullDualNodeDataId();
    data->DNWE_WhichOlap = 0;
    
    // now find the edge connecting the current node to the prev node
    DN_EDGE_ELEM tmp_edge;
    if(mData->getEdges(&tmp_edge, DN_EDGE_STATE_MASTER, 0, nextNode))
    {
        do {
            if(tmp_edge.DNEE_OlapNode == startNode)
            {
                // we will need to walk in the opposite direction to this...
                data->DNWE_OffsetWanted = -1 * tmp_edge.DNEE_Offset;
                
                // make these guys fit
                data->DNWE_C2POffset = tmp_edge.DNEE_Offset;
                data->DNWE_P2COffset = tmp_edge.DNEE_ReturnOffset;

                return true;
            }
        } while(mData->getNextEdge(&tmp_edge));
    }
    logError("Walk could not be started: " << startNode << " --> " << nextNode);
    return false;
}
//HO 
    
    /*
    ** For stepping over a cross and beyond (where non-ambiguous)
    */
/*HV*/ bool
DualNodeClass::
/*HV*/ jumpStartWalk(DN_WALKING_ELEM * data, DualNodeId prevNode, DualNodeId crossNode)
//HO ;
{
    //-----
    // Start walking from the crossNode in the opposite direction to prevNode
    //
    // This function as written assumes that the next_start node chosen is the only
    // option at the crossnode...   ...is this safe?
    // 
    
    // we need to work out the direction we'll be going in
    sMDInt tmp_off = 0;
    DN_EDGE_ELEM tmp_edge;
    if(mData->getEdges(&tmp_edge, DN_EDGE_STATE_MASTER, 0, crossNode))
    {
        do {
            if(tmp_edge.DNEE_OlapNode == prevNode)
            {
                tmp_off = -1 * tmp_edge.DNEE_Offset;
                break;
            }
        } while(mData->getNextEdge(&tmp_edge));
    }
    
    // now we can get the forward running edge
    if(mData->getEdges(&tmp_edge, DN_EDGE_STATE_MASTER, tmp_off, crossNode))
        return startWalk(data, crossNode, tmp_edge.DNEE_OlapNode);
    
    return false;
}
//HO 

    /*
    ** step one node in the graph if next node is both available (non-cap)
    ** and un ambiguous (non-cross). Two methods here depending on what you
    ** want to do
    */
/*HV*/ bool
DualNodeClass::
/*HV*/ rankStep(DN_WALKING_ELEM * data)
//HO ;
{
    //-----
    // Test the rank of the current_node (base_node) and act accordingly
    // VEry robust, can not really stray far
    //
    switch(mData->getNtRank(data->DNWE_CurrentNode))
    {
        case 2:
        {
            // DN_NODE_TYPE_PATH
            // now find the edge connecting the current node to the prev node
            DN_EDGE_ELEM tmp_edge;
            if(mData->getEdges(&tmp_edge, DN_EDGE_STATE_MASTER, data->DNWE_OffsetWanted, data->DNWE_CurrentNode))
            {

                // set prev to be the old_current and curr to be the overlaping node
                data->DNWE_PrevNode = data->DNWE_CurrentNode;
                data->DNWE_CurrentNode = tmp_edge.DNEE_OlapNode;
                
                // we will need to walk in the opposite direction to this...
                data->DNWE_OffsetWanted = -1 * tmp_edge.DNEE_ReturnOffset;
                
                // make these guys fit
                data->DNWE_C2POffset = tmp_edge.DNEE_ReturnOffset;
                data->DNWE_P2COffset = tmp_edge.DNEE_Offset;
                return true;
            }
            logError("Could not continue walk: " << data->DNWE_PrevNode << " --> " << data->DNWE_CurrentNode);
            break;
        }
        case 0:
            // UN_NODE_TYPE_UNSET or detached? Should be banished
            logError("Trying to get NtRank for RANK: 0 " << mData->sayNodeTypeLikeAHuman(getNodeType(data->DNEE_BaseNode)) << ": " << data->DNEE_BaseNode);
            break;
        default:
            // UN_NODE_TYPE_CROSS OR CAP
            break;
    }
    return false;
}
//HO 

/******************************************************************************
** EDGE STATES GET AND SET
******************************************************************************/
    /*
    ** Detach the edge betwen two nodes
    */
//HO inline bool detachEdgeBetween(DualNodeId nodeA, DualNodeId nodeB) { return mData->detachEdgeBetween_symmetrical(nodeA, nodeB); }
//HO 
    
    /*
    ** Banish the edge betwen two nodes
    */
//HO inline bool banishEdgeBetween(DualNodeId nodeA, DualNodeId nodeB) { return mData->banishEdgeBetween_symmetrical(nodeA, nodeB); }
//HO 

    /*
    ** Get the offset between two nodes
    */
//HO inline sMDInt getOffsetBetweenNodes(DualNodeId nodeB, EDGE_STATE state, DualNodeId nodeA) { return mData->getOffsetBetweenNodes(nodeB, state, nodeA); }
//HO 

    /*
    ** get the number of non-transitive nodes
    */
//HO inline uMDInt getTRank(DualNodeId DID) { return mData->getTRank(DID); }
//HO 

/******************************************************************************
** EDGE MAKING AND MANAGEMENT
******************************************************************************/

    /*
    ** add a master master edge between two dualnodes
    */
//HO inline bool addEdgeBetween(sMDInt offsetAtoB, DualNodeId nodeA, sMDInt offsetBtoA, DualNodeId nodeB) { return mData->addEdgeBetween(offsetAtoB, nodeA, offsetBtoA, nodeB); }
//HO 

    /*
    ** add a master slave edge between two dualnodes
    */
//HO inline bool addSlaveEdgeBetween(DualNodeId nodeA, DualNodeId nodeB) { return mData->addSlaveEdgeBetween(nodeA, nodeB); }
//HO 

    /*
    ** add a master master edge between two dualnodes but one way only
    */
//HO inline bool addEdge_asymmetrical(EDGE_STATE state, DualNodeId newNode, sMDInt offsetFromBase, bool offsetMatches, DualNodeId baseNode) { return mData->addEdge_asymmetrical(state, newNode, offsetFromBase, offsetMatches, baseNode); }
//HO 

    /*
    ** update the edge information when a dummy is assimilated
    */
//HO inline bool updateEdge_asymmetrical(DualNodeId oldNode, DualNodeId newNode, DualNodeId baseNode) { return mData->updateEdge_asymmetrical(oldNode, newNode, baseNode); }
//HO 

    /*
    ** Inline wrapper for shallow sorting the nodes
    */
//HO inline void resetEdges(DualNodeId DID) { mData->resetEdges(DID); }
//HO 
    
    /*
    ** Inline wrapper for shallow sorting the nodes
    */
//HO inline bool sortEdges(DualNodeId DID) { return mData->shallowSortEdges(DID); }
//HO 

/******************************************************************************
** NODE TYPE MANAGEMENT
******************************************************************************/
    /*
    ** inline wrapper fo detaching a node
    */
//HO inline bool detachNode(map<GenericNodeId, bool> * detMap, DualNodeId DID) { return mData->detachNode(detMap, DID); } 
//HO 
//HO inline bool detachNode(DualNodeId DID) { return mData->detachNode(DID); }
//HO 

    /*
    ** And for banisheing a node
    */
//HO inline bool banishNode(DualNodeId DID) { return mData->banishNode(DID); }
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
//HO inline std::string sayNodesTypeLikeAHuman(DualNodeId DID) { return mData->sayNodeTypeLikeAHuman(getNodeType(DID)); }
//HO 

    /*
    ** Print the edges at a node
    */
//HO inline void printEdges(DualNodeId DID) { mData->printEdges(DID); }
//HO 

    /*
    ** Print the local grahviz type graph at a node
    */
//HO inline void printLocalGraph(DualNodeId DID) { mData->printLocalGraph(DID); }
//HO 

//HO inline void printNodeInfo(DualNodeId DID) { mData->printNodeInfo(DID); }
//HO 

