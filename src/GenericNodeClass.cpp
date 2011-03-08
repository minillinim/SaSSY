/******************************************************************************
**
** File: GenericNodeClass.cpp
**
*******************************************************************************
**
** Layer 3 Implementation file for use with projects including MemManager.cpp
**
** Public interface for the GenericNode Class Cleaning functions are here!!!
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
/*INC*/ #include <iostream>
/*INC*/ 

// local includes
//INC #include "UniNodeClass.h"
//INC 
#include "UniNodeClass.h"

//INC #include "DualNodeClass.h"
//INC 
#include "DualNodeClass.h"

//INC #include "ReadStoreClass.h"
//INC 
#include "ReadStoreClass.h"

//INC #include "IdTypeStructs.h"
//INC 
#include "IdTypeStructs.h"
                 
//INC #include "nodeblockdef.h"
//INC 
#include "nodeblockdef.h"

#include "GenericNodeMemWrapper.h"
#include "GenericNodeClass.h"
#include "intdef.h"
#include "IdTypeDefs.h"
#include "TreeHistory.h"
#include "LoggerSimp.h"

    /*
    ** This is the edge walking object
    */
        
//INC class GN_WALKING_ELEM {
//INC 
//INC     public:
//INC 
//INC         GN_WALKING_ELEM(unsigned int nc) { mNc = nc; }
//INC 
//INC 
//INC         void printContents(void)
//INC 
//INC         {
//INC 
//INC               switch(mNc) { case UNINODE: { mUNWE.printContents(); break; } case DUALNODE: { mDNWE.printContents(); break; } }
//INC 
//INC         }
//INC 
//INC         void printContents(int i)
//INC 
//INC         {
//INC 
//INC               switch(mNc) { case UNINODE: { mUNWE.printContents(i); std::cout << "----" << std::endl; break; } case DUALNODE: { mDNWE.printContents(i); break; } }
//INC 
//INC         }
//INC 
//INC         unsigned int mNc;
//INC 
//INC         UN_WALKING_ELEM mUNWE;
//INC 
//INC         DN_WALKING_ELEM mDNWE;
//INC 
//INC };
//INC 
//HO GenericNodeId getCurrentNode(GN_WALKING_ELEM gwe) { switch(gwe.mNc) { case UNINODE: { return mUniNodes->getGn(gwe.mUNWE.UNWE_CurrentNode); break; } case DUALNODE: { return mDualNodes->getGn(gwe.mDNWE.DNWE_CurrentNode); break; } } return GN_NULL_ID; }
//HO 
//HO GenericNodeId getPrevNode(GN_WALKING_ELEM gwe) { switch(gwe.mNc) { case UNINODE: { return mUniNodes->getGn(gwe.mUNWE.UNWE_PrevNode); break; } case DUALNODE: { return mDualNodes->getGn(gwe.mDNWE.DNWE_PrevNode); break; } } return GN_NULL_ID; }
//HO 
//HO sMDInt getOffset(GN_WALKING_ELEM gwe) { switch(gwe.mNc) { case UNINODE: { return gwe.mUNWE.UNWE_P2COffset; } case DUALNODE: { return gwe.mDNWE.DNWE_P2COffset; } } return 0; }
//HO 
//HO sMDInt getReturnOffset(GN_WALKING_ELEM gwe) { switch(gwe.mNc) { case UNINODE: { return gwe.mUNWE.UNWE_C2POffset; } case DUALNODE: { return gwe.mDNWE.DNWE_C2POffset; } } return 0; }
//HO 

/*INC*/ typedef GN_ELEM GenericNodeElem;
/*INC*/ 
/*INC*/ typedef GN_EDGE_ELEM GenericNodeEdgeElem;
/*INC*/ 
/*INC*/ typedef GN_WALKING_ELEM GenericNodeWalkingElem;
/*INC*/ 

//HO inline GenericNodeId getBaseNode(GN_EDGE_ELEM gnee) { return mData->getBaseNode(gnee); }
//HO 
//HO inline GenericNodeId getOlapNode(GN_EDGE_ELEM gnee) { return mData->getOlapNode(gnee); }
//HO 
//HO inline sMDInt getOffset(GN_EDGE_ELEM gnee) { return mData->getOffset(gnee); }
//HO 
//HO inline sMDInt getReturnOffset(GN_EDGE_ELEM gnee) { return mData->getReturnOffset(gnee); }
//HO 
//HO inline uMDInt getState(GN_EDGE_ELEM gnee) { return mData->getState(gnee); }
//HO 

/*INC*/ typedef NB_ELEM NodeBlockElem;
/*INC*/ 

//PV GenericNodeId GN_NULL_ID;
//PV 

    // Main data objects used to store sequences and graph edges and nodes (copies of)
//PV ReadStoreClass * mReads;
//PV 
//PV UniNodeClass * mUniNodes;
//PV 
//PV DualNodeClass * mDualNodes;
//PV 
// to store the current context we're working on. Saves passing it around too much
//PV ContextId mCurrentContext;
//PV     
/******************************************************************************
** INTIALISATION AND SAVING
******************************************************************************/

/*HV*/ void
GenericNodeClass::
/*HV*/ extendedDestructor(void)
//HO ;
{
    //-----
    // Add anything you would like to happen when the destructor is called
    //
}
//HO 

/*HV*/ bool
GenericNodeClass::
/*HV*/ extendedInitialise(uMDInt originalNumberOfBlocks )
//HO ;
{
    //-----
    // Add anything you would like to happen on a regular intialisation call
    //
    GN_NULL_ID = getNullGenericNodeId();
    return true;
}
//HO 

/*HV*/ bool
GenericNodeClass::
/*HV*/ extendedInitialise(std::string fileName)
//HO ;
{
    //-----
    // Add anything you would like to happen when reloading a saved instance
    //
    GN_NULL_ID = getNullGenericNodeId();
    return true;
}
//HO 

/*HV*/ bool
GenericNodeClass::
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
    ** Get a copy of all the node classes etc. from the lower layer
    */
//HO inline void initialise2(int RL, int NL, uMDInt mx, uMDInt mn) { mData->initialise2(RL, NL, mx, mn); mData->getNodeClasses(&mUniNodes, &mDualNodes, &mReads); }
//HO 

//HO inline void initialiseOnLoad(void) { mData->getNodeClasses(&mUniNodes, &mDualNodes, &mReads); }
//HO 

/******************************************************************************
** HOUSEKEEPING
******************************************************************************/
//HO inline void setExtOffset(uMDInt eo) { mData->setExtOffset(eo); }
//HO 
//HO inline void setNaiveOffset(uMDInt no) { mData->setNaiveOffset(no); }
//HO 
//HO inline GenericNodeId newGenericNodeId(void) { return mData->newGenericNodeId(); }
//HO 
//HO inline bool deleteGenericNodeId(GenericNodeId ID) { return mData->deleteGenericNodeId( ID ); }
//HO 
//HO inline int getNumNodes(NODE_CLASS nc, std::string node_type) { return mData->getNumNodes(nc, node_type); }
//HO 
//HO inline void setCurrentContextId(ContextId CTXID) { mCurrentContext = CTXID; }
//HO
//HO inline ContextId getCurrentContextId(void) { return mCurrentContext; }
//HO 
//HO inline void curateMasterState(GenericNodeId GID) { mData->curateMasterState(GID); }
//HO 
//HO inline void curateSlaveState(GenericNodeId GID) { mData->curateSlaveState(GID); }
//HO
//HO inline void addMasterToContext(GenericNodeId GID) { mData->addMasterToContext(GID); }
//HO
//HO inline uMDInt getReadDepth(GenericNodeId GID) { return mUniNodes->getReadDepth(getUniNode(GID)); }
//HO 
/*HV*/ bool
GenericNodeClass::
/*HV*/ getGn(GenericNodeId * GID, std::string * sequence)
//HO ;
{
    //-----
    // get the generic node Id for a particular sequence
    //
    UniNodeId UID;
    if(mUniNodes->getUniNodeId(&UID, sequence))
    {
        *GID = mUniNodes->getGn(UID);
        if(GN_NULL_ID == *GID )
        {
            return false;
        }
        PARANOID_ASSERT_PRINT_L2((UID != mUniNodes->getNullUniNodeId()), "NULL UID for sequence: " << *sequence);
        PARANOID_ASSERT_PRINT_L2((*GID != GN_NULL_ID), "NULL GID for UID: " << UID << " with sequence: " << *sequence);
        return true;
    }
    return false;
}
//HO 

    /*
    ** Return a pointer to the readStore
    */
//HO ReadStoreClass * getReads(void) { return mData->getReads(); }
//HO 

/******************************************************************************
** ADDING
******************************************************************************/
//HO inline bool addNode(std::string * seq_1, std::string * seq_2, std::string * insName, std::string * tileNum, std::string * laneNum, std::string * X_Cord, std::string * Y_Cord, std::string * mPlex, FileSetId FSID) { return mData->addNode(seq_1, seq_2, insName, tileNum, laneNum, X_Cord, Y_Cord, mPlex, FSID); }
//HO 
//HO inline GenericNodeId addDummyNode(GenericNodeId GID) { return mData->addDummyNode(getUniNode(GID)); }
//HO 

/******************************************************************************
** UPDATING
******************************************************************************/
//HO inline bool assimilateDummy(GenericNodeId dummy, GenericNodeId newId) { return mData->assimilateDummyNode(dummy, newId); }
//HO 

/******************************************************************************
** DELETING
******************************************************************************/
//HO inline void deleteDummy(GenericNodeId dummy) { mData->deleteDummyNode(dummy); }
//HO 
//HO inline bool deleteDualNode(DualNodeId DID) { return mData->deleteDualNode(mDualNodes->getGn(DID)); }
//HO 
//HO inline bool deleteDualNode(GenericNodeId GID) { return mData->deleteDualNode(GID); }
//HO 

/******************************************************************************
** VALIDATING
******************************************************************************/
//HO inline bool validateDataStructs(void) { if( mData->validateGenericNodes() ) { return mData->validateUniNodes(); } return false; }
//HO 
//HO inline bool validateGenericNodes(void) { return mData->validateGenericNodes(); }
//HO 
//HO inline bool validateUniNodes(void) { return mData->validateUniNodes(); }
//HO 
//HO inline bool validateDualNodes(ContextId CTXID) { return mData->validateDualNodes(CTXID); }
//HO 

/******************************************************************************
** NAVIGATION
******************************************************************************/
//HO inline bool getElem(GenericNodeElem * data, UniNodeId UID) { return mData->getElem(data, UID); }
//HO 
//HO inline bool getElem(GenericNodeElem * data, GenericNodeId GID) { return mData->getElem(data, getUniNode(GID)); }
//HO 
//HO inline bool getNextElem(GenericNodeElem * data) { return mData->getNextElem(data); }
//HO 
//HO inline bool getNextElem(GenericNodeElem * data, bool yopu) { return mData->getNextElem(data, yopu); }
//HO 

    /*
    ** Forpath walking. Start at a position in the graph
    */
/*HV*/ bool
GenericNodeClass::
/*HV*/ startWalk(GN_WALKING_ELEM * data, GenericNodeId startNode, GenericNodeId nextNode)
//HO ;
{
    //-----
    // Start a walk at the startNode in the direction of the nextNode.
    // ie, set the first edge.
    //
    switch(data->mNc)
    {
        case UNINODE:
        {
            return mUniNodes->startWalk(&(data->mUNWE), getUniNode(startNode), getUniNode(nextNode));
        }
        case DUALNODE:
        {
            return mDualNodes->startWalk(&(data->mDNWE), getDualNode(startNode), getDualNode(nextNode));
        }
        default:
            return false;
    }
}
//HO

    /*
    ** Forpath walking. Start at a position in the graph
    */
/*HV*/ bool
GenericNodeClass::
/*HV*/ jumpStartWalk(GN_WALKING_ELEM * data, GenericNodeId prevNode, GenericNodeId crossNode)
//HO ;
{
    //-----
    // Start a walk at the startNode in the direction of the nextNode.
    // ie, set the first edge.
    //
    switch(data->mNc)
    {
        case UNINODE:
        {
            logError("Can't call jump start on unpaired data");
            return false;
        }
        case DUALNODE:
        {
            return mDualNodes->jumpStartWalk(&(data->mDNWE), getDualNode(prevNode), getDualNode(crossNode));
        }
        default:
            return false;
    }
}
//HO 

    /*
    ** step one node in the graph if next node is both available (non-cap)
    ** and un ambiguous (non-cross). Depends strictly on the rank of the node and
    ** ignores the type set
    */
/*HV*/ bool
GenericNodeClass::
/*HV*/ rankStep(GN_WALKING_ELEM * data)
//HO ;
{
    switch(data->mNc)
    {
        case UNINODE:
        {
            return mUniNodes->rankStep(&(data->mUNWE));
        }
        case DUALNODE:
        {
            return mDualNodes->rankStep(&(data->mDNWE));
        }
        default:
            return false;
    }
}
//HO 

    /*
    ** Step one node in the graph provided that we are still walking on the same 
    ** uninode contig. Ie. the head ofr both prev and current will both
    ** have the same contig ID and we only step forward if this can be maintained
    */
/*HV*/ bool
GenericNodeClass::
/*HV*/ contigStep(GN_WALKING_ELEM * data)
//HO ;
{
    switch(data->mNc)
    {
        case UNINODE:
        {
            return mUniNodes->contigStep(&(data->mUNWE));
        }
        case DUALNODE:
        {
            logError("No contig walk available for dualnodes!");
        }
        default:
            return false;
    }
}
//HO 

//HO inline bool getEdges(GN_EDGE_ELEM * data, uMDInt edgeState, sMDInt offSign, GenericNodeId GID) { return mData->getEdges(data, edgeState, offSign, GID); }
//HO 
//HO inline bool getNextEdge(GN_EDGE_ELEM * data) { return mData->getNextEdge(data); }
//HO 
/******************************************************************************
** EDGE STATES GET AND SET
******************************************************************************/
    /*
    ** Get the edge ranks of nodes
    */
/*HV*/ int
GenericNodeClass::
/*HV*/ getNtRank(NODE_CLASS nc, GenericNodeId GID)
//HO ;
{
    switch(nc)
    {
        case UNINODE:
        {
            return mUniNodes->getNtRank(getUniNode(GID));
        }
        case DUALNODE:
        {
            return mDualNodes->getNtRank(getDualNode(GID));
        }
        default:
            return -1;
    }
}
//HO 

/*HV*/ int
GenericNodeClass::
/*HV*/ getTRank(NODE_CLASS nc, GenericNodeId GID)
//HO ;
{
    switch(nc)
    {
        case UNINODE:
        {
            return mUniNodes->getTRank(getUniNode(GID));
        }
        case DUALNODE:
        {
            if(!isCntxMaster(GID))
                return 1;
            return mDualNodes->getTRank(getDualNode(GID));
        }
        default:
            return -1;
    }
}
//HO 

    /*
    ** Get the offset from base to olap
    */
/*HV*/ sMDInt
GenericNodeClass::
/*HV*/ getNtOffset(NODE_CLASS nc, GenericNodeId GID_olap, GenericNodeId GID_base)
//HO ;
{
    switch(nc)
    {
        case UNINODE:
        {
            return mUniNodes->getOffsetBetweenNodes(getUniNode(GID_olap), UN_EDGE_STATE_NTRANS, getUniNode(GID_base));
        }
        case DUALNODE:
        {
            return mDualNodes->getOffsetBetweenNodes(getDualNode(GID_olap), DN_EDGE_STATE_MASTER, getDualNode(GID_base));
        }
    }
    return sMDInt(0);
}
//HO 

/******************************************************************************
** NODEBLOCK STUFF
******************************************************************************/
//HO inline void getUniNodeBlock(NB_ELEM * nbe)        { mData->getUniNodeBlock(nbe); }
//HO 
//HO inline void getUniNodeBlockOnly(NB_ELEM * nbe)        { mData->getUniNodeBlockOnly(nbe); }
//HO 
//HO inline void getNonBanishedBlock_UN(NB_ELEM * nbe) { mData->getNonBanishedBlock_UN(nbe); }
//HO 
//HO inline void getNonBanishedBlockOnly_UN(NB_ELEM * nbe) { mData->getNonBanishedBlockOnly_UN(nbe); }
//HO 
//HO inline void getBanishedBlock_UN(NB_ELEM * nbe)    { mData->getBanishedBlock_UN(nbe); }
//HO 
//HO inline void getAttachedBlock_UN(NB_ELEM * nbe)    { mData->getAttachedBlock_UN(nbe); }
//HO 
//HO inline void getAttachedBlockOnly_UN(NB_ELEM * nbe)    { mData->getAttachedBlockOnly_UN(nbe); }
//HO 
//HO inline void getDetachedBlock_UN(NB_ELEM * nbe)    { mData->getDetachedBlock_UN(nbe); }
//HO 
//HO inline void getCapBlock_UN(NB_ELEM * nbe)         { mData->getCapBlock_UN(nbe); }
//HO 
//HO inline void getPathBlock_UN(NB_ELEM * nbe)        { mData->getPathBlock_UN(nbe); }
//HO 
//HO inline void getCrossBlock_UN(NB_ELEM * nbe)       { mData->getCrossBlock_UN(nbe); }
//HO 

//HO inline void addContextBlocks(ContextId CTXID)     {mData->addContextBlocks(CTXID); }
//HO 
//HO inline void removeContextBlocks(ContextId CTXID)  {mData->removeContextBlocks(CTXID); }
//HO 

//HO inline void getDualNodeBlock(NB_ELEM * nbe)       { mData->getDualNodeBlock(nbe); }
//HO 
//HO inline void getDualNodeBlockOnly(NB_ELEM * nbe)       { mData->getDualNodeBlockOnly(nbe); }
//HO 
//HO inline void getNonBanishedBlock_DN(NB_ELEM * nbe) { mData->getNonBanishedBlock_DN(nbe); }
//HO 
//HO inline void getNonBanishedBlockOnly_DN(NB_ELEM * nbe) { mData->getNonBanishedBlockOnly_DN(nbe); }
//HO 
//HO inline void getBanishedBlock_DN(NB_ELEM * nbe)    { mData->getBanishedBlock_DN(nbe); }
//HO 
//HO inline void getAttachedBlock_DN(NB_ELEM * nbe)    { mData->getAttachedBlock_DN(nbe); }
//HO 
//HO inline void getAttachedBlockOnly_DN(NB_ELEM * nbe)    { mData->getAttachedBlockOnly_DN(nbe); }
//HO 
//HO inline void getDetachedBlock_DN(NB_ELEM * nbe)    { mData->getDetachedBlock_DN(nbe); }
//HO 
//HO inline void getCTXBlock_DN(NB_ELEM * nbe, ContextId CTXID) { mData->getCTXBlock_DN(nbe, CTXID); }
//HO 
//HO inline void getCapBlock_DN(NB_ELEM * nbe, ContextId CTXID) { mData->getCapBlock_DN(nbe, CTXID); }
//HO 
//HO inline void shuffleCapForward_DN(GenericNodeId cap_GID, ContextId CTXID) { mData->shuffleCapForward_DN(cap_GID, CTXID); }
//HO 
//HO inline void getPathBlock_DN(NB_ELEM * nbe, ContextId CTXID) { mData->getPathBlock_DN(nbe, CTXID); }
//HO 
//HO inline void getCrossBlock_DN(NB_ELEM * nbe, ContextId CTXID) { mData->getCrossBlock_DN(nbe, CTXID); }
//HO 

//HO inline bool nextNBElem(NB_ELEM * nbe)             { return mData->nextNBElem(nbe); }
//HO 
//HO inline bool addNodeFront(GenericNodeId GID, NB_TYPE nbt) { return mData->addNodeFront(GID, nbt); }
//HO 
//HO inline bool removeNode(NB_ELEM * activeElem) { return mData->removeNode(activeElem); }
//HO 
//HO inline bool transferNode(NB_ELEM * activeElem, NB_TYPE nbt) { return mData->transferNode(activeElem, nbt); }
//HO 

/******************************************************************************
** EDGE MAKING AND MANAGEMENT
******************************************************************************/
//HO inline bool makeBaseUniNodeEdges(void) { return mData->makeBaseUniNodeEdges(); }
//HO 
//HO inline bool buildUniNodeGraph(int max_offset) { if (mData->buildUniNodeGraph(max_offset)) { sortUNEdges(); return true; } return false; }
//HO 
//HO inline void sortUNEdges(void) { return mData->sortUNEdges(); }
//HO 
//HO inline void sortDNEdges(ContextId CTXID) { return mData->sortDNEdges(CTXID);}
//HO 
//HO inline void sortEdges(GenericNodeId GID) { mData->sortEdges(GID); }
//HO 
//HO inline bool updateEdge_asymmetrical(GenericNodeId oldNode, GenericNodeId newNode, GenericNodeId baseNode) { return mDualNodes->updateEdge_asymmetrical(getDualNode(oldNode), getDualNode(newNode), getDualNode(baseNode)); }
//HO 

    /*
    ** Add a master-master edge between two dualnodes
    */
/*HV*/ bool
GenericNodeClass::
/*HV*/ makeMasterMasterEdge_symmetrical(GenericNodeId GID_A, GenericNodeId GID_B, bool suppressError)
//HO ;
{
    //-----
    // make a master master edge between these two cats
    //
    sMDInt offsetAtoB = mUniNodes->getOffsetBetweenNodes(getUniNode(GID_B), UN_EDGE_STATE_NTRANS, getUniNode(GID_A));
    sMDInt offsetBtoA = mUniNodes->getOffsetBetweenNodes(getUniNode(GID_A), UN_EDGE_STATE_NTRANS, getUniNode(GID_B));
    PARANOID_ASSERT_PRINT_L2((offsetAtoB != 0) && (offsetBtoA != 0), GID_A << " : " << GID_B << " : " << offsetAtoB << " : " << offsetBtoA);
    if(offsetBtoA == 0 || offsetAtoB == 0)
    {
        if(!suppressError)
        {
            logError("offset is 0 in make edge! " << GID_A << " : " << isValidAddress(GID_A) << " : "  << GID_B << " : " << isValidAddress(GID_B) << " : " << offsetAtoB << " : " << offsetBtoA);
            printEdges(DUALNODE, GID_A);
            printEdges(UNINODE, GID_A);
            printEdges(DUALNODE, GID_B);
            printEdges(UNINODE, GID_B);
        }
        return false;
    }
    return makeMasterMasterEdge_symmetrical(offsetAtoB, GID_A, offsetBtoA, GID_B);
}
//HO 

    /*
    ** Add a master-master edge between two dualnodes
    */
/*HV*/ bool
GenericNodeClass::
/*HV*/ makeMasterMasterEdge_symmetrical(sMDInt offsetAtoB, GenericNodeId GID_A, sMDInt offsetBtoA, GenericNodeId GID_B)
//HO ;
{
    //-----
    // just call the dualnode version directly
    //
    return mDualNodes->addEdgeBetween(offsetAtoB, getDualNode(GID_A), offsetBtoA, getDualNode(GID_B));
}
//HO 

    /*
    ** Add a master-slave edge between two dualnodes
    */
//HO inline bool makeSlaveEdge_symmetrical(GenericNodeId slave, GenericNodeId master) { return mDualNodes->addSlaveEdgeBetween(getDualNode(master), getDualNode(slave)); }
//HO 

//HO inline void resetUNEdges(bool keepContigIds) { mData->resetUniNodeEdges(keepContigIds); }
//HO 
//HO inline void resetDNEdges(bool resetHeads, ContextId CTXID) { mData->resetDualNodeEdges(resetHeads, CTXID); }
//HO 
//HO inline GenericNodeId moveNode(GenericNodeId masterGID, GenericNodeId GID) { return mData->moveNode(masterGID, GID); }
//HO 
//HO inline void translateNode(ContextId CTXID, uMDInt pos, uMDInt history, bool revRev, GenericNodeId GID) { mData->translateNode(CTXID, pos, history, revRev, GID); }
//HO 
//HO inline void resetDualNode(GenericNodeId GID) { mData->resetDualNode(GID); }
//HO 

    /*
    ** Reset all the contig settings for dualnodes
    */
//HO inline void resetDNContigs(ContextId CTXID) { mData->resetDNContigs(CTXID); }
//HO 
//HO inline void transferEdges(GenericNodeId oldGID, GenericNodeId replacementGID) { mData->transferEdges(oldGID, replacementGID); }
//HO 


    /*
    ** Detach a node from the graph
    */

// 
//  FIRST: Generic call to detachNode, with and without list
//

/*HV*/ void
GenericNodeClass::
/*HV*/ detachNode(nodeBlock_GN * detachedNodes, NODE_CLASS nc, GenericNodeId GID)
//HO ;
{
    //-----
    // Detach a single node and update the lists it's in...
    //
    switch(nc)
    {
        case UNINODE:
        {
            detachUniNode(detachedNodes, GID);
            break;
        }
        case DUALNODE:
        {
            detachDualNode(detachedNodes, GID);
            break;
        }
    }
}
//HO 

/*HV*/ void
GenericNodeClass::
/*HV*/ detachNode(NODE_CLASS nc, GenericNodeId GID)                                                                    
//HO ;
{
    //-----
    // Detach a single node and update the lists it's in...
    //
    switch(nc)
    {
        case UNINODE:
        {
            detachUniNode(GID);
            break;
        }
        case DUALNODE:
        {
            detachDualNode(GID);            
            break;
        }
    }
}
//HO 

/*HV*/ void
GenericNodeClass::
/*HV*/ detachNode(nodeBlock_GN * detachedNodes, NODE_CLASS nc, NodeBlockElem * nbe)
//HO ;
{
    //-----
    // pretty wrapper for calling detach node
    //
    switch(nc)
    {
        case UNINODE:
        {
            detachUniNode(detachedNodes, nbe);
            break;
        }
        case DUALNODE:
        {
            detachDualNode(detachedNodes, nbe);
            break;
        }
    }
}
//HO 

/*HV*/ void
GenericNodeClass::
/*HV*/ detachNode(NODE_CLASS nc, NodeBlockElem * nbe)
//HO ;
{
    //-----
    // pretty wrapper for calling detach node
    //
    switch(nc)
    {
        case UNINODE:
        {
            detachUniNode(nbe);
            break;
        }
        case DUALNODE:
        {
            detachDualNode(nbe);
            break;
        }
    }
}
//HO 

// 
//  UniNode
//

/*HV*/ void
GenericNodeClass::
/*HV*/ detachUniNode(nodeBlock_GN * detachedNodes, GenericNodeId GID)
//HO ;
{
    //-----
    // Detach a UniNode from the list and make
    // sure all contained blocks (nodeLists)
    // are handled
    //
    detachedNodes->clear();
    mUniNodes->detachNode(detachedNodes, getUniNode(GID));
    mData->transferNode(GID, DETACHEDNODES_UN);
}
//HO 

/*HV*/ void
GenericNodeClass::
/*HV*/ detachUniNode(GenericNodeId GID)
//HO ;
{
    //-----
    // Detach a UniNode from the list and make
    // sure all contained blocks (nodeLists)
    // are handled
    //
    mUniNodes->detachNode(getUniNode(GID));
    mData->transferNode(GID, DETACHEDNODES_UN);
}
//HO 

/*HV*/ void
GenericNodeClass::
/*HV*/ detachUniNode(nodeBlock_GN * detachedNodes, NodeBlockElem * nbe)
//HO ;
{
    //-----
    // Detach a UniNode from the list. Make sure that the nbe is valid
    // return a list
    detachedNodes->clear();
    mUniNodes->detachNode(detachedNodes, getUniNode(nbe->NBE_CurrentNode));
    mData->transferNode(nbe, DETACHEDNODES_UN);
}
//HO 

/*HV*/ void
GenericNodeClass::
/*HV*/ detachUniNode(NodeBlockElem * nbe)
//HO ;
{
    //-----
    // Detach a UniNode from the list. Make sure that the nbe is valid
    //
    mUniNodes->detachNode(getUniNode(nbe->NBE_CurrentNode));
    mData->transferNode(nbe, DETACHEDNODES_UN);
}
//HO 

//
//  DualNode
//

/*HV*/ void
GenericNodeClass::
/*HV*/ detachDualNode(nodeBlock_GN * detachedNodes, GenericNodeId GID)
//HO ;
{
    //-----
    // Detach a singluar DualNode from the list and make
    // sure all contained blocks (nodeLists)
    // are handled
    //
    // put it in the right list
    mData->transferNode(GID, DETACHEDNODES_DN);

    // reset the GenericNode context flags
    mData->clearCntxInfo(GID);

    // detach the node
    detachedNodes->clear();
    mDualNodes->detachNode(detachedNodes, getDualNode(GID));
}
//HO 

/*HV*/ void
GenericNodeClass::
/*HV*/ detachDualNode(GenericNodeId GID)
//HO ;
{
    //-----
    // Detach a singluar DualNode from the list and make
    // sure all contained blocks (nodeLists)
    // are handled
    //
    // put it in the right lists
    mData->transferNode(GID, DETACHEDNODES_DN);
    
    // reset the GenericNode context flags
    mData->clearCntxInfo(GID);

    // detach the node
    mDualNodes->detachNode(getDualNode(GID));
}
//HO 

/*HV*/ void
GenericNodeClass::
/*HV*/ detachDualNode(nodeBlock_GN * detachedNodes, NodeBlockElem * nbe)
//HO ;
{
    //-----
    // Detach a singluar DualNode from the list and make
    // sure all contained blocks (nodeLists)
    // are handled
    //
    // put it in the right lists
    GenericNodeId GID = nbe->NBE_CurrentNode;
    mData->transferNode(nbe, DETACHEDNODES_DN);

    // reset the GenericNode context flags
    mData->clearCntxInfo(GID);
    
    // detach the node
    detachedNodes->clear();
    mDualNodes->detachNode(detachedNodes, getDualNode(GID));
}
//HO 

/*HV*/ void
GenericNodeClass::
/*HV*/ detachDualNode(NodeBlockElem * nbe)
//HO ;
{
    //-----
    // Detach a singluar DualNode from the list and make
    // sure all contained blocks (nodeLists)
    // are handled
    //
    // put it in the right lists
    GenericNodeId GID = nbe->NBE_CurrentNode;
    mData->transferNode(nbe, DETACHEDNODES_DN);
    
    // reset the GenericNode context flags
    mData->clearCntxInfo(GID);

    // detach the node
    mDualNodes->detachNode(getDualNode(GID));
}
//HO 

/******************************************************************************
** NODE TYPE MANAGEMENT
******************************************************************************/
//HO inline int findDupes(bool deleteDupes) { return mData->findAllDupes(deleteDupes); }
//HO 

//HO inline bool banishSparseFloatingUniNodes(void) { return mData->banishSparseFloatingUniNodes(); }
//HO 

    /*
    ** Get the node type
    */
/*HV*/ uMDInt
GenericNodeClass::
/*HV*/ getNodeType(NODE_CLASS nc, GenericNodeId GID)
//HO ;
{
    switch(nc)
    {
        case UNINODE:
        {
            return mUniNodes->getNodeType(getUniNode(GID));
        }
        case DUALNODE:
        {
            return mDualNodes->getNodeType(getDualNode(GID));
        }
        default:
            return (uMDInt)-1;
    }
}
//HO 

//HO inline GenericNodeId getPair(GenericNodeId GID) { return mData->getPair(GID); }
//HO 
//HO inline GenericNodeId getHead(GenericNodeId GID) { return mUniNodes->getGn(getUniNode(GID)); }
//HO 

/******************************************************************************
** CONTIG GET AND SET
******************************************************************************/
    /*
    ** Set the contig Id for a node
    */
/*HV*/ void
GenericNodeClass::
/*HV*/ setContig(NODE_CLASS nc, ContigId CID, GenericNodeId GID)
//HO ;
{
    switch(nc)
    {
        case UNINODE:
        {
            mUniNodes->setContig(CID, getUniNode(GID));
            break;
        }
        case DUALNODE:
        {
            mDualNodes->setContig(CID, getDualNode(GID));
            break;
        }
    }
}
//HO 

    /*
    ** Set the contig Id for a node
    */
/*HV*/ ContigId
GenericNodeClass::
/*HV*/ getContig(NODE_CLASS nc, GenericNodeId GID)
//HO ;
{
    switch(nc)
    {
        case UNINODE:
        {
            return mUniNodes->getContig(getUniNode(getHead(GID)));
        }
        case DUALNODE:
        {
            return mDualNodes->getContig(getDualNode(GID));
        }
    }
    return ContigId();
}
//HO 

/******************************************************************************
** UNINODE GRAPH CLEANING
******************************************************************************/
    /*
    ** Clean DEPs and bubbles from the graph
    */
/*HV*/ void
GenericNodeClass::
/*HV*/ cleanUniNodeGraph(int shaveDepth)
//HO ;
{
    //-----
    // Go through and iteratively strip all the bubbles and fur from the graph
    // if you call this with nc == DUALNODE then you must set the current context first!
    //
    logInfo("Cleaning UniNode graph. Will clean to level: " << shaveDepth, 3);
    
    // Detach any weak nodes
    processAttachedUniNodes();

    int curr_depth = 1;
    
    while(curr_depth <= shaveDepth)
    {

        // shave off the fur (to the current depth)
        shaveUniNodeGraph(curr_depth);

        // go through and see who needs detachin...
        processAttachedUniNodes();

        curr_depth++;
    }
    logInfo("Done Cleaning", 4);

    // sort once more for good luck!
    sortUNEdges();
}
//HO 

    /*
    ** Remove non viable nodes and reclassify into caps crosses etc
    */
/*HV*/ void
GenericNodeClass::
/*HV*/ processAttachedUniNodes(void)
//HO ;
{
    //-----
    // Go through and classify every attached node
    //
    NodeBlockElem nbe;

    // get ourselves a stop block and a means to add it in
    GenericNodeId stop_node_odd = mData->newGenericNodeId();
    GenericNodeId stop_node_even = mData->newGenericNodeId();
    GenericNodeId add_node = stop_node_odd;
    GenericNodeId stop_node = stop_node_even;
    
//%%    std::cout << stop_node_odd << " : " << stop_node_even << std::endl;
    
    bool odd = true;
    bool some_detached = true;
//%%    bool first = true;
    int round_count = 0;
    
    while(some_detached)
    {
        some_detached = false;

        // we need to something a little different on
        // every second run
        if(odd)
        {
            add_node = stop_node_odd;
            stop_node = stop_node_even;
            odd = false;
        }
        else
        {
            add_node = stop_node_even;
            stop_node = stop_node_odd;
            odd = true;
        }

        NB_TYPE add_block = ATTACHEDNODES_UN;
        getAttachedBlock_UN(&nbe);
        
        // set the pointer to the first "real entry" in the block
//%%        std::cout << first << "--" << round_count << std::endl;
//%%        logInfo(first << "--" << round_count,1);
        round_count++;
        if(nextNBElem(&nbe))
        {
            // add the stop node to the front
//%%            logInfo("Adding: " << add_node << " to add_block: " << mData->sayNodeBlockTypeLikeAHuman(add_block), 1);
//%%            logInfo("(Add0): " << getPrevElemNb(add_node) << " --> ( " << add_node << " ) <-- " << getNextElemNb(add_node), 1);
            if(getPrevElemNb(add_node) != GN_NULL_ID)
                mData->removeNode(add_node);
            addNodeFront(add_node, add_block);
//%%            logInfo("(Add1): " << getPrevElemNb(add_node) << " --> ( " << add_node << " ) <-- " << getNextElemNb(add_node), 1);
            
            // now process the list
            do {
                if(nbe.NBE_CurrentNode == stop_node)
                    break;

//%%                if(first)
//%%                    logInfo("Classify: " << nbe.NBE_CurrentNode, 1);
                
                if(classifyUniNode(&nbe))
                    some_detached = true;
                
            } while(nextNBElem(&nbe));
//%%            logInfo("End round: " << round_count, 1);
        }
//%%        first = false;
//%%        logInfo("============================", 1);
    }

    // delete these two helper nodes
//%%    logInfo("PRM: (even0) - " << getPrevElemNb(stop_node_even) << " --> ( " << stop_node_even << " ) --> " << getNextElemNb(stop_node_even), 1);
//%%    logInfo("PRM: (odd0) - " << getPrevElemNb(stop_node_odd) << " --> ( " << stop_node_odd << " ) --> " << getNextElemNb(stop_node_odd), 1);
    if(getPrevElemNb(stop_node_even) != GN_NULL_ID)
        mData->removeNode(stop_node_even);
//%%    logInfo("PRM: (even1) - " << getPrevElemNb(stop_node_even) << " --> ( " << stop_node_even << " ) --> " << getNextElemNb(stop_node_even), 1);
//%%    logInfo("PRM: (odd1) - " << getPrevElemNb(stop_node_odd) << " --> ( " << stop_node_odd << " ) --> " << getNextElemNb(stop_node_odd), 1);
    mData->removeNode(stop_node_odd);
//%%    logInfo("PRM: (even2) - " << getPrevElemNb(stop_node_even) << " --> ( " << stop_node_even << " ) --> " << getNextElemNb(stop_node_even), 1);
//%%    logInfo("PRM: (odd2) - " << getPrevElemNb(stop_node_odd) << " --> ( " << stop_node_odd << " ) --> " << getNextElemNb(stop_node_odd), 1);
    mData->deleteGenericNodeId(stop_node_odd);
    mData->deleteGenericNodeId(stop_node_even);
//%%    logInfo("**============================", 1);
}
//HO 

    /*
    ** Classify a nodes type. Return a non-null GID if we need to detach this node
    ** otherwise return
    */
    static nodeBlock_GN GNG_updated_nodes;
    static nodeBlockIterator_GN GNG_current_node;
    static nodeBlockIterator_GN GNG_current_last;

/*HV*/ bool
GenericNodeClass::
/*HV*/ classifyUniNode(NodeBlockElem * nbe)
//HO ;
{
    //-----
    // Determine if this UniNode needs to be detached.
    // We test the non-Transitive rank and the "viability" 
    // related to connectivity of this node and return
    // it's ID if it should be detached or GN_NULL_ID otherwise
    //
    GenericNodeId final_GID = getPrevElemNb(nbe->NBE_CurrentNode);
    PARANOID_ASSERT_PRINT_L2((GN_NULL_ID != final_GID), "Null final");

    GenericNodeEdgeElem gnee_u(UNINODE);
    GenericNodeEdgeElem * gnee = &gnee_u;
    GenericNodeId GID = nbe->NBE_CurrentNode;
    UniNodeId UID = getUniNode(GID);

    bool should_detach = true;

    NB_TYPE add_block = ATTACHEDNODES_UN;
    NB_TYPE detach_block = DETACHEDNODES_UN;
    GenericNodeId transferred_block = GN_NULL_ID;
            
#ifdef MAKE_PARANOID
    PARANOID_ASSERT_L2((UN_NODE_TYPE_UNSET != mUniNodes->getNodeType(UID)));
    PARANOID_ASSERT_L2((UN_NODE_TYPE_DETACHED != mUniNodes->getNodeType(UID)));
    PARANOID_ASSERT_L2((UN_NODE_TYPE_BANISHED != mUniNodes->getNodeType(UID)));
#endif
    switch(mUniNodes->getNtRank(UID))
    {
        case 2:
            // UN_NODE_TYPE_PATH
            mUniNodes->setNodeType(UN_NODE_TYPE_PATH, UID);
            mData->transferNode(GID, PATHNODES_UN);
            transferred_block = mData->getNodeBlockNode(PATHNODES_UN);
            break;
        case 1:
            // UN_NODE_TYPE_CAP
            mUniNodes->setNodeType(UN_NODE_TYPE_CAP, UID);
            mData->transferNode(GID, CAPNODES_UN);
            transferred_block = mData->getNodeBlockNode(CAPNODES_UN);
            should_detach = false;
            break;
        case 0:
            // UN_NODE_TYPE_UNSET or detached? Should be banished?
            if(GN_NULL_ID == GID)
            {
                // Error! we should never try to classify a NULL node
                logError("About to go to NULL");
                return false;
            }
            if(UN_NODE_TYPE_DETACHED == mUniNodes->getNodeType(UID))
            {
                // there is no way to test whether it IS in the detached list
                // but as it has ntRank == 0 and type detached we may as well make sure...
                mData->transferNode(GID, DETACHEDNODES_UN);
                transferred_block = mData->getNodeBlockNode(DETACHEDNODES_UN);
                should_detach = false;
            }
            break;
        default:
            // UN_NODE_TYPE_CROSS
            mUniNodes->setNodeType(UN_NODE_TYPE_CROSS, UID);
            mData->transferNode(GID, CROSSNODES_UN);
            transferred_block = mData->getNodeBlockNode(CROSSNODES_UN);
            break;
    }

    // go through and check if there is a path through this node
    // this is called the "viability"
    bool has_pos = false;
    bool has_neg = false;
    if(getEdges(gnee, GN_EDGE_STATE_NTRANS, 0, GID))
    {
        do {
            if(0 > getOffset(*gnee))
            {
                has_neg = true;
                if(has_pos)
                {
                    should_detach = false;
                    break;
                }
            }
            else
            {
                has_pos = true;
                if(has_neg)
                {
                    should_detach = false;
                    break;
                }
            }
        } while(getNextEdge(gnee));
    }

    if(should_detach)
    {
        // This code detaches nodes and edges
        GNG_updated_nodes.clear();
//%%        logInfo("PN: detaching: " << GID, 1);
        detachUniNode(&GNG_updated_nodes, GID);
        transferred_block = mData->getNodeBlockNode(detach_block);

        // we will need to go through all the affected nodes and
        // re-process them
        GNG_current_node = GNG_updated_nodes.begin();
        GNG_current_last = GNG_updated_nodes.end();
        while(GNG_current_node != GNG_current_last)
        {
            // this transfer should not change the type of the node, it
            // is used to "sort" the list
            mData->transferNode(GNG_current_node->first, add_block);
//%%            logInfo("Adding in: " << GNG_current_node->first, 1);
            GNG_current_node++;
        }
    }

    // if we transferred the node we need to fix the nbe
    if(final_GID == transferred_block)
        return should_detach;
    else if(GN_NULL_ID != transferred_block)
        nbe->NBE_CurrentNode = final_GID;

    // if we should have detached then we have now.
    // tell the world about it
    return should_detach;
}
//HO 

    /*
    ** Shave all the fur off the graph
    */
    static std::vector<GenericNodeId> GNG_to_detach_holder;
    static std::vector<GenericNodeId>::iterator GNG_detach_iter;
    static std::vector<GenericNodeId>::iterator GNG_detach_last;
    static nodeBlock_GN GNG_disposable_list;
    static nodeBlockIterator_GN GNG_cap_iter;

/*HV*/ void
GenericNodeClass::
/*HV*/ shaveUniNodeGraph(int depth)
//HO ;
{
    //-----
    // remove all UniNodes which are depth or less from a cross node
    // clearly, if depth is 7 and we find a node that is 7 from a cross node, it and all nodes in
    // between it and the cross node will be removed.
    // at this point the cross node will be altered, so we must process altered nodes again
    //
    // we assume that cap nodes is a valid list of caps
    //
    logInfo("At level: " << depth << " --> start shaving", 5);

    GNG_to_detach_holder.clear();
    int count = 0;                                                  // keep count of the depth
    int num_detached = 0;

    GenericNodeEdgeElem gnee_u(UNINODE);
    GenericNodeEdgeElem * gnee = &gnee_u;

    GenericNodeWalkingElem walking_edge_u(UNINODE);
    GenericNodeWalkingElem * walking_edge = &walking_edge_u;

    NodeBlockElem nbe;
    getCapBlock_UN(&nbe);

    while(nextNBElem(&nbe))
    {
        GenericNodeId current_cap = nbe.NBE_CurrentNode;
        if(getEdges(gnee, GN_EDGE_STATE_NTRANS, 0, current_cap))
        {
            // clear this guy
            GNG_to_detach_holder.clear();
            count = 0;
            if(startWalk(walking_edge, current_cap, getOlapNode(*gnee)))
            {
                // don't put the cap on the detach list
                // but do the rest
                while(rankStep(walking_edge))
                {
                    GNG_to_detach_holder.push_back(getPrevNode(*walking_edge));
                    count++;
                    if(count > depth)
                        break;
                }
            }
            else
            {
                logError("could not walk from cap");
            }
            if(count <= depth)
            {
                // the path was too short, however, we will keep any
                // cap to cap paths that may be too short...
                uMDInt rank = mUniNodes->getNtRank(getUniNode(getCurrentNode(*walking_edge)));;
                if(rank > 1)
                {
                    // we need to detach some stuff...
                    // do the cap first and then do the remainder of the tail
                    GNG_disposable_list.clear();
//%%                    logInfo("SH: detaching cap: " << nbe.NBE_CurrentNode, 1);
                    detachUniNode(&nbe);

                    GNG_detach_iter = GNG_to_detach_holder.begin();
                    GNG_detach_last = GNG_to_detach_holder.end();
                    while(GNG_detach_iter != GNG_detach_last)
                    {
//%%                        logInfo("SH: detaching: " << *GNG_detach_iter, 1);
                        detachUniNode(*GNG_detach_iter);
                        num_detached++;
                        GNG_detach_iter++;
                    }
                }
            }
        }
        else
        {
            logError("No edges at cap: " << current_cap << " valid: " << isValidAddress(current_cap));
            printEdges(UNINODE, current_cap);
        }
    }
    logInfo("Shaved " << num_detached << " UniNodes this round", 5);
}
//HO 

/******************************************************************************
** OUTPUT AND PRINTING
******************************************************************************/
//
// these guys are used to hash a position in a context to a single uMDInt
//
//HO inline uMDInt getCntxPosHash(GenericNodeId GID) { return ( (getCntxId(GID)).getuniquehash() ^ (getCntxPos(GID)) << SIZE_OF_INT * 3 / 4); }
//HO 
//HO inline uMDInt getCntxPosHash(uMDInt pos, GenericNodeId GID) { return ( (getCntxId(GID)).getuniquehash() ^ (pos << SIZE_OF_INT * 3 / 4)); }
//HO 
//HO inline uMDInt getCntxPosHash(uMDInt pos, ContextId CTXID) { return ( CTXID.getuniquehash() ^ (pos << SIZE_OF_INT * 3 / 4)); }
//HO 

//HO inline void printEdges(NODE_CLASS nc, bool showBanished) { return mData->printAllEdges(nc, showBanished); }
//HO 
//HO inline void printAllEdges(ContextId CTXID, bool showBanished) { mData->printAllEdges(CTXID, showBanished); }
//HO 
//HO inline void printEdges(NODE_CLASS nc, GenericNodeId GID) { return mData->printEdges(nc, GID); }
//HO 
//HO inline void printLoop(bool all, GenericNodeId GID) { return mData->printLoop(all, getUniNode(GID)); }
//HO 
//HO inline void logLoop(int logLevel, GenericNodeId GID) { return mData->logLoop(logLevel, GID); }
//HO 
//HO inline void printLocalGraph(NODE_CLASS nc, GenericNodeId GID) { return mData->printLocalGraph(nc, GID); }
//HO
//HO inline std::string getSequence(GenericNodeId GID) { return mData->getSequence(GID); }
//HO 
    /*
    ** reverse compliment a sequence
    */
/*HV*/ std::string
GenericNodeClass::
/*HV*/ getReverseComplementSequence(GenericNodeId GID)
//HO ;
{
    std::string ret_string = getSequence(GID);
    mReads->revCmpSequence(&ret_string);
    return ret_string;
}
//HO 
#if __USE_RI
//HO inline std::string getReadId(GenericNodeId GID) { return mData->getReadId(GID); }
//HO 
#endif
    /*
    ** say edge state like a human
    */
/*HV*/ std::string
GenericNodeClass::
/*HV*/ sayEdgeStateLikeAHuman(NODE_CLASS nc, uMDInt state)
//HO ;
{
    switch(nc)
    {
        case UNINODE:
        {
            return mUniNodes->sayEdgeStateLikeAHuman(state);
        }
        case DUALNODE:
        {
            return mDualNodes->sayEdgeStateLikeAHuman(state);
        }
        default:
        {
            return "Random";
        }
    }
}
//HO 

    /*
    ** say edge state like a human
    */
/*HV*/ std::string
GenericNodeClass::
/*HV*/ sayNodesTypeLikeAHuman(NODE_CLASS nc, GenericNodeId GID)
//HO ;
{
    switch(nc)
    {
        case UNINODE:
        {
            return mUniNodes->sayNodesTypeLikeAHuman(getUniNode(GID));
        }
        case DUALNODE:
        {
            return mDualNodes->sayNodesTypeLikeAHuman(getDualNode(GID));
        }
        default:
        {
            return "Random";
        }
    }
}
//HO

//HO bool getCrossDistributions(std::vector<int> * distVec, ContextId * CTXID, NODE_CLASS nc) { return mData->getCrossDistributions(distVec, CTXID, nc); }
//HO 

    /*
    ** Inline wrapper for reverse complimenting a sequence
    */
//HO inline void revCmpSequence(std::string * sequence) { mReads->revCmpSequence(sequence); }
//HO 

    /*
    ** Inline wrapper for getting the readLength
    */
//HO inline int getReadLength(void) { return mReads->getReadLength(); }
//HO 

/*HV*/ void
GenericNodeClass::
/*HV*/ printContentsOfWE(GN_WALKING_ELEM gwe)
//HO ;
{
    //-----
    // more indepth printing of this element
    //
    gwe.printContents(1);
    switch(gwe.mNc) 
    { 
        case UNINODE: 
        {   
            std::cout << getSequence(mUniNodes->getGn(gwe.mUNWE.UNWE_PrevNode)) << " : " << getSequence(mUniNodes->getGn(gwe.mUNWE.UNWE_CurrentNode)) << std::endl;
            break; 
        } 
        case DUALNODE: 
        { 
            GenericNodeId prev_GID = mDualNodes->getGn(gwe.mDNWE.DNWE_PrevNode);
            GenericNodeId curr_GID = mDualNodes->getGn(gwe.mDNWE.DNWE_CurrentNode);
            UniNodeId prev_UID = getUniNode(prev_GID);
            UniNodeId curr_UID = getUniNode(curr_GID);
            
            sMDInt up2c = mUniNodes->getOffsetBetweenNodes(curr_UID, UN_EDGE_STATE_NTRANS, prev_UID);
            sMDInt uc2p = mUniNodes->getOffsetBetweenNodes(prev_UID, UN_EDGE_STATE_NTRANS , curr_UID);
            sMDInt dp2c = mDualNodes->getOffsetBetweenNodes(gwe.mDNWE.DNWE_CurrentNode, DN_EDGE_STATE_MASTER, gwe.mDNWE.DNWE_PrevNode);
            sMDInt dc2p = mDualNodes->getOffsetBetweenNodes(gwe.mDNWE.DNWE_PrevNode, DN_EDGE_STATE_MASTER, gwe.mDNWE.DNWE_CurrentNode);
            std::cout << "YYY P: " << gwe.mDNWE.DNWE_PrevNode << " C: " << gwe.mDNWE.DNWE_CurrentNode << std::endl;
            std::cout << getSequence(prev_GID) << " : " << getSequence(curr_GID) << std::endl;
            std::cout << up2c << " : " << uc2p << " : " << dp2c << " : " << dc2p << std::endl;
            std::cout << (up2c == dp2c) << " :: " << (uc2p == dc2p) << std::endl;
            std::cout << "==================" << std::endl;
            
            break; 
        }
    }
}


