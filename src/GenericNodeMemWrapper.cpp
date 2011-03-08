/******************************************************************************
**
** File: GenericNodeMemWrapper.cpp
**
*******************************************************************************
**
** This file handles all of the lower level functions for the Generic node object.
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
/*INC*/ #include <sstream>
/*INC*/ 
/*INC*/ #include <map>
/*INC*/ 
/*INC*/ class GenericNodeClass;
/*INC*/ 

#include <google/sparse_hash_map>
//INC #include <google/sparse_hash_map>
//INC 

// local includes
#include "GenericNodeMemWrapper.h"
#include "LoggerSimp.h"
#include "default_parameters.h"
#include "intdef.h"
#include "IdTypeDefs.h"
#ifdef MAKE_PARANOID
# include "MemManager.h"
#endif
#include "paranoid.h"

#include "Utils.h"
                                  
//INC #include "UniNodeClass.h"
//INC 
#include "UniNodeClass.h"

//INC #include "DualNodeClass.h"
//INC 
#include "DualNodeClass.h"

//INC #include "ReadStoreClass.h"
//INC 
#include "ReadStoreClass.h"

#if __USE_RI
//INC #include "ReadIdentifierClass.h"
//INC 
#include "ReadIdentifierClass.h"
#endif
                 
//INC #include "IdTypeStructs.h"
//INC 
#include "IdTypeStructs.h"
                 
//INC #include "nodeblockdef.h"
//INC 
#include "nodeblockdef.h"

// we need a way to discrimiate between uni and dual nodes
//INC enum NODE_CLASS { UNINODE, DUALNODE };
//INC 

/******************************************************************************
 ** We need the edge states for Dual, Uni and Generic Nodes to match up. There is a similar #define block in DualNodeMemWrapper.cpp
******************************************************************************/

//HO #define GN_EDGE_STATE_DETACHED UN_EDGE_STATE_DETACHED
//HO 
//HO #define GN_EDGE_STATE_TRANS UN_EDGE_STATE_TRANS
//HO 
//HO #define GN_EDGE_STATE_NTRANS UN_EDGE_STATE_NTRANS
//HO 
//HO #define GN_EDGE_STATE_SLAVE UN_EDGE_STATE_TRANS
//HO 
//HO #define GN_EDGE_STATE_MASTER UN_EDGE_STATE_NTRANS
//HO 
//HO #define GN_EDGE_STATE_BANISHED UN_EDGE_STATE_BANISHED
//HO 
//HO #define GN_NODE_TYPE_UNSET UN_NODE_TYPE_UNSET
//HO 
//HO #define GN_NODE_TYPE_CAP UN_NODE_TYPE_CAP
//HO 
//HO #define GN_NODE_TYPE_PATH UN_NODE_TYPE_PATH
//HO 
//HO #define GN_NODE_TYPE_CROSS UN_NODE_TYPE_CROSS
//HO 
//HO #define GN_NODE_TYPE_ATTACHED UN_NODE_TYPE_ATTACHED
//HO 
//HO #define GN_NODE_TYPE_DETACHED UN_NODE_TYPE_DETACHED
//HO 
//HO #define GN_NODE_TYPE_BANISHED UN_NODE_TYPE_BANISHED
//HO 
//HO #define GN_NODE_TYPE_CORRECTED UN_NODE_TYPE_CORRECTED
//HO 

/******************************************************************************
 ** Holder struct for iterating through the many to one uni->dual mapping
******************************************************************************/
//INC class GN_ELEM {
//INC 
//INC     public:
//INC 
//INC         GN_ELEM() { GNE_Head = GenericNodeId(); }
//INC
//INC         // to get some insight
//INC 
//INC         void printContents(void)
//INC 
//INC         {
//INC 
//INC             std::cout << GNE_GID << " : " << GNE_UID << " : " << GNE_DID << " R: " << GNE_Orientation << "\nCNTX: [" <<
//INC                       GNE_CNTX << ", " << GNE_CNTXPos << ", " << GNE_CNTXRev << "] D: " << GNE_Dummy << " M: " << GNE_Master << " H: " << GNE_isHead << "\n...." << std::endl;
//INC 
//INC         }
//INC 
//INC         void printContents(int i)
//INC 
//INC         {
//INC 
//INC             std::cout << GNE_GID << " : " << GNE_UID << " : " << GNE_DID << " R: " << GNE_Orientation << "\nCNTX: [" <<
//INC                       GNE_CNTX << ", " << GNE_CNTXPos << ", " << GNE_CNTXRev << "] D: " << GNE_Dummy << " M: " << GNE_Master << " H: " << GNE_isHead << std::endl;
//INC 
//INC             std::cout << "Next: " << GNE_NextHit << " : Head: " << GNE_Head << "\n...." << std::endl;
//INC 
//INC         }
//INC 
//INC         void printContents(std::ostringstream * SS)
//INC 
//INC         {
//INC 
//INC             *SS << GNE_GID << " : " << GNE_UID << " : " << GNE_DID << " R: " << GNE_Orientation << "\nCNTX: [" <<
//INC                       GNE_CNTX << ", " << GNE_CNTXPos << ", " << GNE_CNTXRev << "] D: " << GNE_Dummy << " M: " << GNE_Master << " H: " << GNE_isHead << "\n....";
//INC 
//INC         }
//INC 
//INC         GenericNodeId GNE_GID;
//INC 
//INC         UniNodeId GNE_UID;
//INC 
//INC         DualNodeId GNE_DID;
//INC 
//INC         bool GNE_Dummy;
//INC 
//INC         bool GNE_Master;
//INC 
//INC         bool GNE_isHead;
//INC 
//INC         bool GNE_Orientation;
//INC 
//INC         ContextId GNE_CNTX;
//INC 
//INC         uMDInt GNE_CNTXPos;
//INC 
//INC         bool GNE_CNTXRev;
//INC 
//INC     private:
//INC 
//INC         GenericNodeId GNE_NextHit;
//INC 
//INC         GenericNodeId GNE_Head;
//INC 
//INC         friend class GenericNodeMemWrapper;
//INC 
//INC         friend class Scaffolder;
//INC 
//INC };
//INC 

/******************************************************************************
 ** The generic node class wraps both dualnodes and uninodes. Thus we need to wrap alot of the calls which will
 ** be made, for example the edge elements and the calls to get edges.
 ** We define the get and next methods below in the section on navigation
******************************************************************************/

//INC class GN_EDGE_ELEM {
//INC 
//INC     public:
//INC 
//INC         GN_EDGE_ELEM(unsigned int nc) { mNc = nc; }
//INC 
//INC 
//INC         void printContents(void)
//INC 
//INC         {
//INC 
//INC               switch(mNc) { case UNINODE: { mUNEE.printContents(); break; } case DUALNODE: { mDNEE.printContents(); break; } }
//INC 
//INC         }
//INC 
//INC         void printContents(int i)
//INC 
//INC         {
//INC 
//INC               switch(mNc) { case UNINODE: { mUNEE.printContents(i); break; } case DUALNODE: { mDNEE.printContents(i) ;break; } }
//INC 
//INC         }
//INC 
//INC         unsigned int mNc;
//INC 
//INC         UN_EDGE_ELEM mUNEE;
//INC 
//INC         DN_EDGE_ELEM mDNEE;
//INC 
//INC };
//INC

//HO GenericNodeId getBaseNode(GN_EDGE_ELEM gee) { switch(gee.mNc) { case UNINODE: { return mUniNodes->getGn(gee.mUNEE.UNEE_BaseNode); break; } case DUALNODE: { break; } } return mDualNodes->getGn(gee.mDNEE.DNEE_BaseNode); return getNullGenericNodeId(); }
//HO 
//HO GenericNodeId getOlapNode(GN_EDGE_ELEM gee) { switch(gee.mNc) { case UNINODE: { return mUniNodes->getGn(gee.mUNEE.UNEE_OlapNode); break; } case DUALNODE: { break; } } return mDualNodes->getGn(gee.mDNEE.DNEE_OlapNode); return getNullGenericNodeId(); }
//HO 
//HO sMDInt getOffset(GN_EDGE_ELEM gee) { switch(gee.mNc) { case UNINODE: { return gee.mUNEE.UNEE_Offset; break; } case DUALNODE: { return gee.mDNEE.DNEE_Offset; break; } } return 0; }
//HO 
//HO sMDInt getReturnOffset(GN_EDGE_ELEM gee) { switch(gee.mNc) { case UNINODE: { return gee.mUNEE.UNEE_ReturnOffset; break; } case DUALNODE: { return gee.mDNEE.DNEE_ReturnOffset; break; } } return 0; }
//HO 
//HO uMDInt getState(GN_EDGE_ELEM gee) { switch(gee.mNc) { case UNINODE: { return gee.mUNEE.UNEE_State; break; } case DUALNODE: { return gee.mDNEE.DNEE_State; break; } } return GN_EDGE_STATE_DETACHED; }
//HO 

/******************************************************************************
 ** All about NodeBlocks
******************************************************************************/

/*INC*/ typedef unsigned int NodeBlockId;
/*INC*/ 
//INC enum NB_TYPE { UNINODES, DUALNODES, BANISHEDNODES_UN, NONBANISHEDNODES_UN, ATTACHEDNODES_UN, DETACHEDNODES_UN, CAPNODES_UN, PATHNODES_UN, CROSSNODES_UN, BANISHEDNODES_DN, NONBANISHEDNODES_DN, ATTACHEDNODES_DN, DETACHEDNODES_DN, CAPNODES_DN, PATHNODES_DN, CROSSNODES_DN };
//INC 

//HO #define GN_UniNodes         8
//HO 
//HO #define GN_U_Banished       1
//HO 
//HO #define GN_U_Non_Banished   6
//HO 
//HO #define GN_U_Detached       1
//HO 
//HO #define GN_U_Attached       4
//HO 
//HO #define GN_U_Cap            1
//HO 
//HO #define GN_U_Path           1
//HO 
//HO #define GN_U_Cross          1
//HO 

//HO #define GN_DualNodes        5
//HO 
//HO #define GN_D_Banished       1
//HO 
//HO #define GN_D_Non_Banished   3
//HO 
//HO #define GN_D_Detached       1
//HO 
//HO #define GN_D_Attached       1
//HO 
//HO #define GN_End              0
//HO 

/*INC*/ typedef std::map<ContextId, GenericNodeId> ContextBlock;
/*INC*/ 
/*INC*/ typedef std::map<ContextId, GenericNodeId>::iterator ContextBlockIterator;
/*INC*/ 

//PV ContextBlock mCaps_DN;
//PV 
//PV ContextBlock mPaths_DN;
//PV 
//PV ContextBlock mCrosses_DN;
//PV 
//PV ContextBlock mCTXS_DN;
//PV 
//PV unsigned int mNumContextsInBlock;
//PV 

// tivial wrapper which allows us to walk over (certain) multiple lists
//INC class NB_ELEM {
//INC 
//INC public:
//INC 
//INC     NB_ELEM(NodeBlockId NBID, GenericNodeId GID) { NBE_DepthRemaining = NBID; NBE_CurrentNode = GID; }
//INC 
//INC     NB_ELEM() { }
//INC 
//INC     ~NB_ELEM(void) {}
//INC 
//INC     void printContents(void)
//INC 
//INC     {
//INC 
//INC         std::cout << "Depth: " << NBE_DepthRemaining << " : " << NBE_CurrentNode << std::endl;
//INC 
//INC     }
//INC 
//INC     // this is the juicy bit
//INC 
//INC     GenericNodeId NBE_CurrentNode;
//INC 
//INC private:
//INC 
//INC     unsigned int NBE_DepthRemaining;
//INC 
//INC     friend class GenericNodeMemWrapper;
//INC 
//INC };
//INC 

//PV GenericNodeId mUniNodeBlock;
//PV 
//PV GenericNodeId mDualNodeBlock;
//PV 
//PV GenericNodeId mNonBanishedBlock_UN;
//PV 
//PV GenericNodeId mBanishedBlock_UN;
//PV 
//PV GenericNodeId mAttachedBlock_UN;
//PV 
//PV GenericNodeId mDetachedBlock_UN;
//PV 
//PV GenericNodeId mCapBlock_UN;
//PV 
//PV GenericNodeId mPathBlock_UN;
//PV 
//PV GenericNodeId mCrossBlock_UN;
//PV 
//PV GenericNodeId mNonBanishedBlock_DN;
//PV 
//PV GenericNodeId mBanishedBlock_DN;
//PV 
//PV GenericNodeId mAttachedBlock_DN;
//PV 
//PV GenericNodeId mDetachedBlock_DN;
//PV 
//PV GenericNodeId mEndBlock_DN;
//PV 

        // Main data objects used to store sequences and graph edges and nodes
//PV  UniNodeClass * mUniNodes;              /*S*/
//PV 
//PV  DualNodeClass * mDualNodes;            /*S*/
//PV 
//PV  ReadStoreClass * mReads;               /*S*/
//PV 
#if __USE_RI
//PV  ReadIdentifierClass * mReadIdentifiers;/*S*/
//PV 
#endif

        // Housekeeping vars
//PV int mReadLength;                       /*S*/
//PV 
//PV int mNumLines;                         /*S*/
//PV 
//PV int mNumReads;                         /*S*/
//PV 
//PV int mNumDummies;
//PV 
//PV int mNumDupes;
//PV 
//PV uMDInt mExtOffset;                     /*S*/
//PV 
//PV uMDInt mHighestOffset;                 /*S*/
//PV 
//PV uMDInt mNaiveOffset;                   /*S*/
//PV 

       // save the hassle of passin this guy around too much
//PV ContextId mCurrentContext;
//PV 
//HO inline void setCurrentContextId(ContextId CTXID) { mCurrentContext = CTXID; }
//HO 
//HO inline ContextId getCurrentContextId(void) { return mCurrentContext; }
//HO 

        // NOTE:
        // As we are loading reads we want to be sure that we're not loading
        // duplicates because this wastes memory. To this end we have "DupeMap"
        // A paired read looks like: ACCTTGG ... AACTAAG but internally
        // this can be represented as: RSID_1 ... RSID_2 where RSID_1 != RSID_2
        // Given that Ids are 32 bits and we are working on 64 bit machines
        // we can do the following:
        // 
        // if RSID_1 < RSID_2 then dupehash = RSID_1 * 10,000,000,000 + RSID_2
        // otherwise we just swap the numbers. This gives us a number which will
        // fit within 64 bits provided that RSID_1's guts are less than 1844674407
        // RSID_2's guts are less than 3709551616. So once we need to load more than
        // 1.8 billion reads we are boned. But we have some time before this happens
/*INC*/ typedef google::sparse_hash_map<uMDInt, bool> DupeMap;
/*INC*/ 
/*INC*/ typedef google::sparse_hash_map<uMDInt, bool>::iterator DupeMapIterator;
/*INC*/ 
//PV DupeMap * mDupeMap;
//PV 
                
/******************************************************************************
        ** INTIALISATION AND SAVING
******************************************************************************/
/*HV*/ void
GenericNodeMemWrapper::
/*HV*/ extendedDestructor(void)
//HO ;
{
    //-----
    // Add anything you would like to happen when the destructor is called
    //
    if(mReads != NULL)
        delete mReads;
    mReads = NULL;

#if __USE_RI
    if(mReadIdentifiers != NULL)
        delete mReadIdentifiers;
    mReadIdentifiers = NULL;
#endif
    
    if(mUniNodes != NULL)
        delete mUniNodes;
    mUniNodes = NULL;
    
    if(mDualNodes != NULL)
        delete mDualNodes;
    mDualNodes =NULL;

    if(mDupeMap != NULL)
        delete mDupeMap;
    mDupeMap = NULL;
}
//HO 

/*HV*/ bool
GenericNodeMemWrapper::
/*HV*/ extendedInitialise(uMDInt originalNumberOfBlocks )
//HO ;
{
    //-----
    // Add anything you would like to happen on a regular intialisation call
    //

    // set the main objects pointers to null
    mUniNodes = NULL;
    mDualNodes = NULL;
    mReads = NULL;
#if __USE_RI
    mReadIdentifiers = NULL;
#endif    
    // set uo the nodeBlocks
    intialiseNodeBlocks();
    mNumContextsInBlock = 0;
    mNumReads = 0;
    mNumDupes = 0;

    // fix up the dupe map
    mDupeMap = new DupeMap();
    mDupeMap->set_deleted_key(0);
    
    // set these defaults
    mNaiveOffset = SAS_DEF_UN_OFFSET_DEF_MIN;
    mExtOffset = SAS_DEF_UN_OFFSET_DEF_MAX;
    mHighestOffset = mExtOffset;
    
    return true;
}
//HO 

// GN_LookupMap saving and loading

void readMap(std::map<UniNodeId, GenericNodeId> * readmap, ifstream * inFile) {
    readmap->clear();
    unsigned int size;
    inFile->read(reinterpret_cast<char *>(&size), sizeof(unsigned int));
    while(size > 0) {
        UniNodeId tmp_UID(inFile);
        GenericNodeId tmp_GNID(inFile);
        (*readmap)[tmp_UID] = tmp_GNID;
        size--;
    }
}

void writeMap(map<UniNodeId, GenericNodeId> * writemap, ofstream * outFile)
{
    unsigned int size = writemap->size();
    outFile->write(reinterpret_cast<char *>(&size), sizeof(unsigned int));
    map<UniNodeId, GenericNodeId>::iterator map_iter = writemap->begin();
    map<UniNodeId, GenericNodeId>::iterator map_last = writemap->end();
    while(map_iter != map_last) {
        (map_iter->first).save(outFile);
        (map_iter->second).save(outFile);
        map_iter++;
    }
}

/*HV*/ bool
GenericNodeMemWrapper::
/*HV*/ extendedInitialise(std::string fileName)
//HO ;
{
    //-----
    // Add anything you would like to happen when reloading a saved instance
    //
    mUniNodes = NULL;
    mDualNodes = NULL;
    mReads = NULL;
#if __USE_RI
    mReadIdentifiers = NULL;
#endif
    // just for safety...
    mNumDupes = 0;

    mDupeMap = new DupeMap();
    mDupeMap->set_deleted_key(0);

    // open our file
    std::ifstream myFile(fileName.c_str(), std::ifstream::binary);

    // get back the important member vars
    myFile.read(reinterpret_cast<char *>(&mReadLength), sizeof(int));
    myFile.read(reinterpret_cast<char *>(&mNumLines), sizeof(int));
    myFile.read(reinterpret_cast<char *>(&mNumReads), sizeof(int));
    myFile.read(reinterpret_cast<char *>(&mNaiveOffset), sizeof(uMDInt));
    myFile.read(reinterpret_cast<char *>(&mExtOffset), sizeof(uMDInt));
    myFile.read(reinterpret_cast<char *>(&mHighestOffset), sizeof(uMDInt));
    
    // save all the nodeBlocks
    intialiseNodeBlocks(&myFile);

    // close the file
    myFile.close();

    logInfo("Loading mUniNodes", 3);
    if(mUniNodes != NULL)
        delete mUniNodes;
    mUniNodes = new UniNodeClass(mReadLength, mHighestOffset);
    PARANOID_ASSERT_L2(mUniNodes != NULL);
    mUniNodes->initialise(fileName + ".UniNodes");
    mUniNodes->setNaiveOffset(mNaiveOffset);

    logInfo("Loading mDualNodes", 3);
    if(mDualNodes != NULL)
        delete mDualNodes;
    mDualNodes = new DualNodeClass();
    PARANOID_ASSERT_L2(mDualNodes != NULL);
    mDualNodes->initialise(fileName + ".DualNodes");

    logInfo("Loading ReadStore", 3);
    if(mReads != NULL)
        delete mReads;
    mReads = new ReadStoreClass(mReadLength);
    PARANOID_ASSERT_L2(mReads != NULL);
    mReads->initialise(fileName + ".ReadStore");

#if __USE_RI    
    logInfo("Loading ReadIdentifiers", 3);
    if(mReadIdentifiers != NULL)
        delete mReadIdentifiers;
    mReadIdentifiers = new ReadIdentifierClass();
    PARANOID_ASSERT_L2(mReadIdentifiers != NULL);
    mReadIdentifiers->initialise(fileName + ".ReadIdentifiers");
#endif
    
    // fix the cross object links
    mUniNodes->setObjectPointers(mReads);
#if __USE_RI
    mDualNodes->setObjectPointers(mUniNodes, mReadIdentifiers);
#else
    mDualNodes->setObjectPointers(mUniNodes);
#endif
    // set the number of dummies to 0
    mNumDummies = 0;
    mNumContextsInBlock = 0;
    
    return true;
}
//HO 

/*HV*/ bool
GenericNodeMemWrapper::
/*HV*/ extendedSave(std::string fileName)
//HO ;
{
    //-----
    // Add anything you would like to happen when saving (including saving private vars)
    //
    // open our file
    std::ofstream myFile(fileName.c_str(), std::ofstream::binary);

    // save the important member vars
    myFile.write(reinterpret_cast<char *>(&mReadLength), sizeof(int));
    myFile.write(reinterpret_cast<char *>(&mNumLines), sizeof(int));
    myFile.write(reinterpret_cast<char *>(&mNumReads), sizeof(int));
    myFile.write(reinterpret_cast<char *>(&mNaiveOffset), sizeof(uMDInt));
    myFile.write(reinterpret_cast<char *>(&mExtOffset), sizeof(uMDInt));
    myFile.write(reinterpret_cast<char *>(&mHighestOffset), sizeof(uMDInt));
    
    // save all the nodeBlocks
    saveNodeBlocks(&myFile);

    // close the file
    myFile.close();

    logInfo("Saving ReadStore ", 3);
    if(mReads != NULL)
        mReads->save(fileName + ".ReadStore");

#if __USE_RI
    logInfo("Saving ReadIdentifiers", 3);
    if(mReadIdentifiers != NULL)
        mReadIdentifiers->save(fileName + ".ReadIdentifiers");
#endif
    
    logInfo("Saving UniNodes", 3);
    if(mUniNodes != NULL)
        mUniNodes->save(fileName + ".UniNodes");

    logInfo("Saving DualNodes", 3);
    if(mDualNodes != NULL)
        mDualNodes->save(fileName + ".DualNodes");

    return true;
}
//HO 

/*HV*/ void
GenericNodeMemWrapper::
/*HV*/ initialise2(int RL, int NL, uMDInt naiveOffset, uMDInt extOffset)
//HO ;
{
    // set the member vars first
    mReadLength = RL;
    mNumLines = NL;
    mNaiveOffset = naiveOffset;
    mExtOffset = extOffset;
    mHighestOffset = mExtOffset;
    
    // declare the read storage
    mReads = new ReadStoreClass(mReadLength);
    PARANOID_ASSERT_L2(mReads != NULL);
    mReads->initialise((uMDInt)(SAS_DEF_GN_RS_MULT * mNumLines));
    // delcare the UniNodes
    mUniNodes = new UniNodeClass(mReadLength, mHighestOffset);
    PARANOID_ASSERT_L2(mUniNodes != NULL);
    mUniNodes->initialise((uMDInt)(SAS_DEF_GN_UN_MULT * mNumLines));
    
    // declare the DualNodes
    mDualNodes = new DualNodeClass();
    PARANOID_ASSERT_L2(mDualNodes != NULL);
    mDualNodes->initialise((int)(SAS_DEF_GN_DN_MULT * mNumLines));

#if __USE_RI
    // declare the read identifiers
    mReadIdentifiers = new ReadIdentifierClass();
    PARANOID_ASSERT_L2(mReadIdentifiers != NULL);
    mReadIdentifiers->initialise((uMDInt)(SAS_DEF_GN_RI_MULT * mNumLines));
#endif
        
        // fix the cross object links
    mUniNodes->setObjectPointers(mReads);
#if __USE_RI
    mDualNodes->setObjectPointers(mUniNodes, mReadIdentifiers);
#else
    mDualNodes->setObjectPointers(mUniNodes);
#endif
    // set the number of dummies to 0
    mNumDummies = 0;
}
//HO 

/*HV*/ void
GenericNodeMemWrapper::
/*HV*/ intialiseNodeBlocks(void)
//HO ;
{
    //-----
    // set up all the chaining for the blocks nicely
    //
    // Ask the datamanager for some room to store these guys...
    mUniNodeBlock = newGenericNodeId();
    mDualNodeBlock = newGenericNodeId();
    mNonBanishedBlock_UN = newGenericNodeId();
    mBanishedBlock_UN = newGenericNodeId();
    mAttachedBlock_UN = newGenericNodeId();
    mDetachedBlock_UN = newGenericNodeId();
    mCapBlock_UN = newGenericNodeId();
    mPathBlock_UN = newGenericNodeId();
    mCrossBlock_UN = newGenericNodeId();
    mNonBanishedBlock_DN = newGenericNodeId();
    mBanishedBlock_DN = newGenericNodeId();
    mAttachedBlock_DN = newGenericNodeId();
    mDetachedBlock_DN = newGenericNodeId();
    mEndBlock_DN = newGenericNodeId();

//    std::cout << " U: " << mUniNodeBlock << " D: " << mDualNodeBlock << "\nNB: " << mNonBanishedBlock_UN << " B: " << mBanishedBlock_UN << " A: " << mAttachedBlock_UN << " D: " << mDetachedBlock_UN << " C: " << mCapBlock_UN << " P: " << mPathBlock_UN << " X: " << mCrossBlock_UN << "\nNB: " << mNonBanishedBlock_DN << " B: " << mBanishedBlock_DN << " A: " << mAttachedBlock_DN << " D: " << mDetachedBlock_DN << " C: " << mCapBlock_DN << " P: " << mPathBlock_DN << " X: " << mCrossBlock_DN << "\nEND: " << mEndBlock_DN << " : " << std::endl;
    
    // manually order the node block according to grouping
    setNodeBlockHead(true , mUniNodeBlock );
    setPrevElemNb(GN_NULL_ID , mUniNodeBlock );

    setNodeBlockHead(true , mBanishedBlock_UN );
    setPrevElemNb(mUniNodeBlock , mBanishedBlock_UN );
    setNextElemNb(mBanishedBlock_UN , mUniNodeBlock );

    setNodeBlockHead(true , mNonBanishedBlock_UN );
    setPrevElemNb(mBanishedBlock_UN , mNonBanishedBlock_UN );
    setNextElemNb(mNonBanishedBlock_UN , mBanishedBlock_UN );

    setNodeBlockHead(true , mDetachedBlock_UN );
    setPrevElemNb(mNonBanishedBlock_UN , mDetachedBlock_UN );
    setNextElemNb(mDetachedBlock_UN , mNonBanishedBlock_UN );

    setNodeBlockHead(true , mAttachedBlock_UN );
    setPrevElemNb(mDetachedBlock_UN , mAttachedBlock_UN );
    setNextElemNb(mAttachedBlock_UN , mDetachedBlock_UN );

    setNodeBlockHead(true , mCapBlock_UN );
    setPrevElemNb(mAttachedBlock_UN , mCapBlock_UN );
    setNextElemNb(mCapBlock_UN , mAttachedBlock_UN );

    setNodeBlockHead(true , mPathBlock_UN );
    setPrevElemNb(mCapBlock_UN , mPathBlock_UN );
    setNextElemNb(mPathBlock_UN , mCapBlock_UN );

    setNodeBlockHead(true , mCrossBlock_UN );
    setPrevElemNb(mPathBlock_UN , mCrossBlock_UN );
    setNextElemNb(mCrossBlock_UN , mPathBlock_UN );

    setNodeBlockHead(true , mDualNodeBlock );
    setPrevElemNb(mCrossBlock_UN , mDualNodeBlock );
    setNextElemNb(mDualNodeBlock , mCrossBlock_UN );

    setNodeBlockHead(true , mBanishedBlock_DN );
    setPrevElemNb(mDualNodeBlock , mBanishedBlock_DN );
    setNextElemNb(mBanishedBlock_DN , mDualNodeBlock );

    setNodeBlockHead(true , mNonBanishedBlock_DN );
    setPrevElemNb(mBanishedBlock_DN , mNonBanishedBlock_DN );
    setNextElemNb(mNonBanishedBlock_DN , mBanishedBlock_DN );
    
    setNodeBlockHead(true , mDetachedBlock_DN );
    setPrevElemNb(mNonBanishedBlock_DN , mDetachedBlock_DN );
    setNextElemNb(mDetachedBlock_DN , mNonBanishedBlock_DN );

    setNodeBlockHead(true , mAttachedBlock_DN );
    setPrevElemNb(mDetachedBlock_DN , mAttachedBlock_DN );
    setNextElemNb(mAttachedBlock_DN , mDetachedBlock_DN );

    setNodeBlockHead(true , mEndBlock_DN );
    setPrevElemNb(mAttachedBlock_DN, mEndBlock_DN);
    setNextElemNb(mEndBlock_DN, mAttachedBlock_DN );

    setNextElemNb(GN_NULL_ID ,  mEndBlock_DN);
}
//HO 

/*HV*/ void
GenericNodeMemWrapper::
/*HV*/ intialiseNodeBlocks(ifstream * inFile)
//HO ;
{
    //-----
    // set up all the chaining for the blocks nicely
    // using a previously saved point
    //
    mUniNodeBlock = GenericNodeId(inFile);
    mDualNodeBlock = GenericNodeId(inFile);
    mNonBanishedBlock_UN = GenericNodeId(inFile);
    mBanishedBlock_UN = GenericNodeId(inFile);
    mAttachedBlock_UN = GenericNodeId(inFile);
    mDetachedBlock_UN = GenericNodeId(inFile);
    mCapBlock_UN = GenericNodeId(inFile);
    mPathBlock_UN = GenericNodeId(inFile);
    mCrossBlock_UN = GenericNodeId(inFile);
    mNonBanishedBlock_DN = GenericNodeId(inFile);
    mBanishedBlock_DN = GenericNodeId(inFile);
    mAttachedBlock_DN = GenericNodeId(inFile);
    mDetachedBlock_DN = GenericNodeId(inFile);
    mEndBlock_DN = GenericNodeId(inFile);
    
    //std::cout << " U: " << mUniNodeBlock << " D: " << mDualNodeBlock << "\nNB: " << mNonBanishedBlock_UN << " B: " << mBanishedBlock_UN << " A: " << mAttachedBlock_UN << " D: " << mDetachedBlock_UN << " C: " << mCapBlock_UN << " P: " << mPathBlock_UN << " X: " << mCrossBlock_UN << "\nNB: " << mNonBanishedBlock_DN << " B: " << mBanishedBlock_DN << " A: " << mAttachedBlock_DN << " D: " << mDetachedBlock_DN << "\nEND: " << mEndBlock_DN << " : " << std::endl;
}
//HO 

/*HV*/ void
GenericNodeMemWrapper::
/*HV*/ saveNodeBlocks(ofstream * outFile)
//HO ;
{
    //-----
    // save up all the chaining for the blocks nicely
    //
    mUniNodeBlock.save(outFile);
    mDualNodeBlock.save(outFile);
    mNonBanishedBlock_UN.save(outFile);
    mBanishedBlock_UN.save(outFile);
    mAttachedBlock_UN.save(outFile);
    mDetachedBlock_UN.save(outFile);
    mCapBlock_UN.save(outFile);
    mPathBlock_UN.save(outFile);
    mCrossBlock_UN.save(outFile);
    mNonBanishedBlock_DN.save(outFile);
    mBanishedBlock_DN.save(outFile);
    mAttachedBlock_DN.save(outFile);
    mDetachedBlock_DN.save(outFile);
    mEndBlock_DN.save(outFile);
    
    //std::cout << " U: " << mUniNodeBlock << " D: " << mDualNodeBlock << "\nNB: " << mNonBanishedBlock_UN << " B: " << mBanishedBlock_UN << " A: " << mAttachedBlock_UN << " D: " << mDetachedBlock_UN << " C: " << mCapBlock_UN << " P: " << mPathBlock_UN << " X: " << mCrossBlock_UN << "\nNB: " << mNonBanishedBlock_DN << " B: " << mBanishedBlock_DN << " A: " << mAttachedBlock_DN << " D: " << mDetachedBlock_DN << "\nEND: " << mEndBlock_DN << " : " << std::endl;
}
//HO 

/******************************************************************************
** HOUSEKEEPING
******************************************************************************/
//HO inline void setExtOffset(uMDInt eo) { mExtOffset = eo; }
//HO 
//HO inline void setNaiveOffset(uMDInt no) { mNaiveOffset = no; mUniNodes->setNaiveOffset(no); }
//HO 
//HO inline GenericNodeId getHead(GenericNodeId GID) { return mUniNodes->getGn(getUniNode(GID)); }
//HO 

/*HV*/ void
GenericNodeMemWrapper::
/*HV*/ getNodeClasses(UniNodeClass ** UNC, DualNodeClass ** DNC, ReadStoreClass ** RSC)
//HO ;
{
    //-----
    // the class object above this guy needs access to these mofo-s
    //
    *RSC = mReads;
    *UNC = mUniNodes;
    *DNC = mDualNodes;
}
//HO 

    /*
    ** Find the paired mate of a Generic Node
    */
//HO inline GenericNodeId getPair(GenericNodeId GID) { return mDualNodes->getGn(mDualNodes->getPair(getDualNode(GID))); }
//HO 
    
    /*
    ** Return a pointer to the readStore
    */
//HO inline ReadStoreClass * getReads(void) { return mReads; }
//HO 
    
    /*
    ** return the number of nodes in the given list
    */
/*HV*/ int
GenericNodeMemWrapper::
/*HV*/ getNumNodes(NODE_CLASS nc, std::string whichNodes)
//HO ;
{
    //-----
    // return the number of nodes in the given list for the given node type
    //
    return 0;
}
//HO 

/******************************************************************************
** ADDING NODES
******************************************************************************/
/*HV*/ bool
GenericNodeMemWrapper::
/*HV*/ addNode(std::string * forward, std::string * reverse, std::string * insName, std::string * tileNum, std::string * laneNum, std::string * X_Cord, std::string * Y_Cord, std::string * mPlex, FileSetId FSID)
//HO ;
{
    //-----
    // Add a generic node
    //
    // ADDS THE HEAD (UNI) NODE TO NONBANISHEDNODES_UN AND BOTH DUALNODES TO NONBANISHEDNODES_DN
    //
    bool original_1;                                                // is this the first time we've seen this sequence?
    bool reversed_1 = false;                                        // for holding whether it has been reversed
    UniNodeId UN_1;                                                 // the node we're going to add
    ReadStoreId sequence_1_ID;                                      // the sequence reference in the readStore

    bool original_2;
    bool reversed_2 = false;
    UniNodeId UN_2;
    ReadStoreId sequence_2_ID;
    
    // create the sequences in the ReadStore
    sequence_1_ID = mReads->addSequence(&original_1, &reversed_1, forward);

    if(!mReads->isValidReadStoreId(sequence_1_ID))
    {
        return false;
    }

    // NOTE A: If the call above worked and the call(s) below fail then
    // we will have a situation where original_1/2 will return false
    // and the uninode for this read has not been set
    // when we next see this guy we can fix it by checking to seen
    // if the UID is set. However, if we never see this guy
    // again then we'll have the even more sinister case
    // of a RSID with no UID set
    
    sequence_2_ID = mReads->addSequence(&original_2, &reversed_2, reverse);
    
    if(!mReads->isValidReadStoreId(sequence_2_ID))
    {
        return false;
    }

    // check to see that we don't have identical ends
    if(sequence_1_ID == sequence_2_ID)
    {
        return false;
    }

    // check to see that these guys are not dupes
    uMDInt dupe_hash = 0;
    if(sequence_1_ID < sequence_2_ID)
    {
        dupe_hash = sequence_1_ID.getGuts() * 10000000000 + sequence_2_ID.getGuts();
    }
    else
    {
        dupe_hash = sequence_2_ID.getGuts() * 10000000000 + sequence_1_ID.getGuts();
    }

    DupeMapIterator dm_iter = mDupeMap->find(dupe_hash);
    if(dm_iter == mDupeMap->end())
    {
        // first time customer, we can continue but we need to add this
        // guy to the map
        (*mDupeMap)[dupe_hash] = true;
    }
    else
    {
        // we have seen this combo before, so this guy is a dupe!
        mNumDupes++;
        return false;
    }

    if(original_1)
    {
        // make the UniNodeIds
        GenericNodeId head_GID_1 =  newGenericNodeId();
        UN_1 = mUniNodes->addNode(sequence_1_ID);
        
        // add a link from the ReadStore to the UniNode object
        mReads->setUn(UN_1, sequence_1_ID);

        // add a link from the UniNode to the GenericNode
        mUniNodes->setGn(head_GID_1, UN_1);
        setUniNode(UN_1, head_GID_1);

        // insert the Head into the list
        insertHeadNode(UN_1, head_GID_1);
        addNodeFront(head_GID_1, mNonBanishedBlock_UN);

        mNumReads++;
        // now make the edges
        // mUniNodes->makeEdges(forward, UN_1);
    }
    else
    {
        UN_1 = mReads->getUn(sequence_1_ID);
        // NOTE A: We handle this here
        if(UN_1 == mUniNodes->getNullUniNodeId())
        {
            // make the UniNodeIds
            GenericNodeId head_GID_1 =  newGenericNodeId();
            UN_1 = mUniNodes->addNode(sequence_1_ID);
        
            // add a link from the ReadStore to the UniNode object
            mReads->setUn(UN_1, sequence_1_ID);
        
            // add a link from the UniNode to the GenericNode
            mUniNodes->setGn(head_GID_1, UN_1);
            setUniNode(UN_1, head_GID_1);

            // insert the Head into the list
            insertHeadNode(UN_1, head_GID_1);    // THIS CALL WILL UPDATE THE mNonBanishedNodes_UN LIST
            addNodeFront(head_GID_1, mNonBanishedBlock_UN);

            mNumReads++;
            // now make the edges
            // mUniNodes->makeEdges(forward, UN_1);
        }
    }

    if(original_2)
    {
        // make the UniNodeIds
        GenericNodeId head_GID_2 =  newGenericNodeId();
        UN_2 = mUniNodes->addNode(sequence_2_ID);
        
        // add a link from the ReadStore to the UniNode object
        mReads->setUn(UN_2, sequence_2_ID);
        
        // add a link from the UniNode to the GenericNode
        mUniNodes->setGn(head_GID_2, UN_2);
        setUniNode(UN_2, head_GID_2);

        // insert the Head into the list
        insertHeadNode(UN_2, head_GID_2);
        addNodeFront(head_GID_2, mNonBanishedBlock_UN);

        mNumReads++;
        // now make the edges
        // mUniNodes->makeEdges(reverse, UN_2);
    }
    else
    {
        UN_2 = mReads->getUn(sequence_2_ID);
        // NOTE A: We handle this here        
        if(UN_2 == mUniNodes->getNullUniNodeId())
        {
            // make the UniNodeId
            GenericNodeId head_GID_2 =  newGenericNodeId();
            UN_2 = mUniNodes->addNode(sequence_2_ID);
            
            // add a link from the ReadStore to the UniNode object
            mReads->setUn(UN_2, sequence_2_ID);

            // add a link from the UniNode to the GenericNode
            mUniNodes->setGn(head_GID_2, UN_2);
            setUniNode(UN_2, head_GID_2);

            // insert the Head into the list
            insertHeadNode(UN_2, head_GID_2);    // THIS CALL WILL UPDATE THE mNonBanishedNodes_UN LIST
            addNodeFront(head_GID_2, mNonBanishedBlock_UN);

            mNumReads++;
            // now make the edges
            // mUniNodes->makeEdges(reverse, UN_2);
        }
    }

    // now it's safe to increment the read depth on the  UniNodes
    mUniNodes->incReadDepth(UN_1);
    mUniNodes->incReadDepth(UN_2);

    // for each read we'll need a new GID
    GenericNodeId GID_1 = newGenericNodeId();
    GenericNodeId GID_2 = newGenericNodeId();
    
    // set the DualNodes and make them point to eachother
    // and let them know which one of the pair they were
    DualNodeId DN_1 = mDualNodes->addNode(GID_1);
    DualNodeId DN_2 = mDualNodes->addNode(GID_2);
    mDualNodes->setPair(DN_2, DN_1);
    mDualNodes->setPairId(false, DN_1);
    mDualNodes->setPair(DN_1, DN_2);
    mDualNodes->setPairId(true, DN_2);

    // set the fields for the generic nodes
    setFileReversed(reversed_1, GID_1);
    setDualNode(DN_1, GID_1);
    setUniNode(UN_1, GID_1);

    setFileReversed(reversed_2, GID_2);
    setDualNode(DN_2, GID_2);
    setUniNode(UN_2, GID_2);

    // we add them into the data structure properly
    insertNode(GID_1);
    insertNode(GID_2);

    // set the file ID
    setFileId(FSID, GID_1);
    setFileId(FSID, GID_2);
    
    addNodeFront(GID_1, mDetachedBlock_DN);
    mDualNodes->setNodeType(DN_NODE_TYPE_DETACHED, DN_1);
    addNodeFront(GID_2, mDetachedBlock_DN);
    mDualNodes->setNodeType(DN_NODE_TYPE_DETACHED, DN_2);

    // now store the readIds for the dual nodes
    uMDInt lane, tile, x_Cord, y_Cord;
    from_string<uMDInt>(tile,*tileNum,std::dec);
    from_string<uMDInt>(lane,*laneNum,std::dec);
    from_string<uMDInt>(x_Cord,*X_Cord,std::dec);
    from_string<uMDInt>(y_Cord,*Y_Cord,std::dec);
#if __USE_RI
    ReadIdentifierId RiID_1 = mReadIdentifiers->addReadIdentifier(DN_1, insName, mPlex, tile, lane, x_Cord, y_Cord);
    ReadIdentifierId RiID_2 = mReadIdentifiers->addReadIdentifier(DN_2, insName, mPlex, tile, lane, x_Cord, y_Cord);

    mDualNodes->setReadIdentifier(RiID_1, DN_1);
    mDualNodes->setReadIdentifier(RiID_2, DN_2);
#endif
    
    // and we're done!
    return true;
}
//HO 

/*HV*/ GenericNodeId
GenericNodeMemWrapper::
/*HV*/ addDummyNode(UniNodeId UID)
//HO ;
{
    //-----
    // add a dummy node and ensure all the flags are set right
    //
    // make the node and set the flags
    GenericNodeId new_dummy = newGenericNodeId();
    setDummy(true, new_dummy);
    setFileReversed(false, new_dummy);
    setUniNode(UID, new_dummy);

    // set a dualnode so we can have edges
    DualNodeId new_DID = mDualNodes->addNode(new_dummy);
    setDualNode(new_DID, new_dummy);

    // add this guy to the non-banished lists
    mDualNodes->setNodeType(DN_NODE_TYPE_DETACHED, new_DID);
    addNodeFront(new_dummy, mDetachedBlock_DN);
    
    // put this guy in the right lists
    insertNode(new_dummy);
    mUniNodes->incReadDepth(UID);

    mNumDummies++;
    
    return new_dummy;
}
//HO 

/*HV*/ bool
GenericNodeMemWrapper::
/*HV*/ insertHeadNode(UniNodeId UID, GenericNodeId GID)
//HO ;
{
    //-----
    // Insert a node into the list the right way
    //
    // new head !
    setHead(true, GID);

    setNextElem(GID, GID);
    setPrevElem(GID, GID);
    
    // done
    return true;
}
//HO 

/*HV*/ bool
GenericNodeMemWrapper::
/*HV*/ insertNode(GenericNodeId GID)
//HO ;
{
    //-----
    // Insert a node into the list the right way
    //
    // check and see if we added this guy before
    // otherwise we'll make a new head.
    GenericNodeId head = getHead(GID);
    setNextElem(getNextElem(head), GID);
    setPrevElem(GID, getNextElem(head));
    // point the head to the new block
    // and the new block back to the head
    setNextElem(GID, head);
    setPrevElem(head, GID);

    // done
    return true;
}
//HO 

/******************************************************************************
** UPDATING NODES
******************************************************************************/
/*HV*/ bool
GenericNodeMemWrapper::
/*HV*/ assimilateDummyNode(GenericNodeId dummy, GenericNodeId newId)
//HO ;
{

    DualNodeId DID_dummy = getDualNode(dummy);
    DualNodeId DID_new = getDualNode(newId);
    
    // set the node specific flags
    setCntxMaster(true, newId);
    ContextId CTXID = getCntxId(dummy);
    setCntxId(CTXID, newId);
    setCntxPos(getCntxPos(dummy), newId);
    setCntxReversed(isCntxReversed(dummy), newId);
    setCntxHistory(getCntxHistory(dummy), newId);
    setCntxReadDepth(getCntxReadDepth(dummy), newId);

#ifdef MAKE_PARANOID
    bool edges_exist = false;
#endif

    // now we need to update all the master master edges
    DN_EDGE_ELEM tmp_dnee;
    if(mDualNodes->getEdges(&tmp_dnee, GN_EDGE_STATE_MASTER, 0, DID_dummy))
    {
#ifdef MAKE_PARANOID
        edges_exist = true;
#endif
        do {
            // add the edge from the new node to the olapnode (only master edges)
            mDualNodes->addEdge_asymmetrical(DN_EDGE_STATE_MASTER, tmp_dnee.DNEE_OlapNode, tmp_dnee.DNEE_Offset, ((tmp_dnee.DNEE_Offset * tmp_dnee.DNEE_ReturnOffset) > 0), DID_new);
            // update the information at the olapnode
            mDualNodes->updateEdge_asymmetrical(DID_dummy, DID_new, tmp_dnee.DNEE_OlapNode);
        } while(mDualNodes->getNextEdge(&tmp_dnee));
    }
#ifdef MAKE_PARANOID
    if(!edges_exist)
    {
        PARANOID_INFO_L2("No edges for " << dummy);
    }
#endif

    // finally we can delete the dummy
    // there is no dualNode for a dummy so we need only to deal with the
    // generic node
    deleteDummyNode(dummy);

    return true;
}

/******************************************************************************
** DELETING NODES
******************************************************************************/
/*HV*/ bool
GenericNodeMemWrapper::
/*HV*/ deleteDualNode(GenericNodeId GID)
//HO ;
{
    //-----
    // Delete one link in a loop
    //
    // THIS IS THE MAIN ENTRY FOR DELETING!
    // THIS DECIDES THE FATE OF DUMMIES AND PAIRS
    //
    // Does not do nice things to the validity of edges...
    // It deletes the data blocks and resets all edge related data 
    // and then just deletes the node itself.
    // 
    // We need to call reset so that the data blocks will be deleted
    //
    DualNodeId first_DID = getDualNode(GID);
    if(!isDummy(GID))
    {
        // we need to delete the pair too
        DualNodeId pair_DID = mDualNodes->getPair(first_DID);
        GenericNodeId pair_GID = mDualNodes->getGn(pair_DID);

        mDualNodes->resetEdges(pair_DID);
        deleteNode(pair_GID);
        mDualNodes->deleteDualNodeId(pair_DID);
    }
    mDualNodes->resetEdges(first_DID);
    deleteNode(GID);
    mDualNodes->deleteDualNodeId(first_DID);

    return true;
}
//HO 

/*HV*/ bool
GenericNodeMemWrapper::
/*HV*/ deleteDualNode(NB_ELEM * nbe)
//HO ;
{
    //-----
    // Delete one link in a loop
    //
    GenericNodeId final_GID = getPrevElemNb(nbe->NBE_CurrentNode);
    PARANOID_ASSERT_PRINT_L2((GN_NULL_ID != final_GID),"Null final");
    if(deleteDualNode(nbe->NBE_CurrentNode))
    {
        nbe->NBE_CurrentNode = final_GID;
        return true; 
    }
    return false;
}
//HO 

//HO inline bool deleteDualNode(DualNodeId DID) { return deleteDualNode(mDualNodes->getGn(DID)); }
//HO 
//HO inline bool deleteDummyNode(GenericNodeId dummy) { if(deleteDualNode(dummy)) { mNumDummies--; return true; } return false; }
//HO 
//HO inline bool deleteDummyNode(NB_ELEM * nbe) { if(deleteDualNode(nbe)) { mNumDummies--; return true; } return false; }
//HO 

/*HV*/ bool
GenericNodeMemWrapper::
/*HV*/ deleteNode(GenericNodeId GID)
//HO ;
{
    //-----
    // Delete a Generic node and remove it from the chain
    // 
    // **** This MUST be the address of a "DualNode" GN! ****
    // 
    
    // make sure that we are not deleting a "UniNode" GN (Head)
    GenericNodeId head = getHead(GID);
    if(!isHead(GID))
    {
        // we are deleting some random link
        // so re-link the loop
        setNextElem(getNextElem(GID), getPrevElem(GID));
        setPrevElem(getPrevElem(GID), getNextElem(GID));
    }
    else
    {
        // we are trying to delete the head ("UniNode")
        logError("Deleting head for: " << GID << " in wrong function call");
        return false;
    }
    
    UniNodeId UID = getUniNode(GID);

    // update the node lists
    // the GID is useless so we need to strike it from any blocks
    removeNode(GID);

    // delete the actual generic node ("DualNode")
    deleteGenericNodeId(GID);

    // there is one less member in this loop
    // check to see if this was the last dualnode for this loop
    // and if so then delete the head GN ("UniNode")
    mUniNodes->decReadDepth(UID);
    if(0 == mUniNodes->getReadDepth(UID))
        deleteHead(mUniNodes->getGn(UID));

    return true;
}
//HO 

/*HV*/ bool
GenericNodeMemWrapper::
/*HV*/ deleteNode(NB_ELEM * nbe)
//HO ;
{
    //-----
    // Delete a Generic node and remove it from the chain
    //
    // TECHNICALLY YOU SHOULD NEVER CALL THIS FUNCTION
    //
    // first we need to get the head of this loop
    GenericNodeId final_GID = getPrevElemNb(nbe->NBE_CurrentNode);
    PARANOID_ASSERT_PRINT_L2((GN_NULL_ID != final_GID),"Null final");
    GenericNodeId GID = nbe->NBE_CurrentNode;
    if(deleteNode(GID))
    {
        nbe->NBE_CurrentNode = final_GID;
        return true;
    }
    return false;
}
//HO 

/*HV*/ bool
GenericNodeMemWrapper::
/*HV*/ deleteHead(GenericNodeId GID)
//HO ;
{
    //-----
    // Delete the head of a GenericNode loop and banish the uninode
    //
    // first we need to re-link the loop
    GenericNodeId head = getHead(GID);
    UniNodeId UID = getUniNode(GID);

    if(head == GID)
    {
        if(mUniNodes->getReadDepth(UID) > 0)
        {
            logError("Deleting head " << UID << " : " << GID << " when loop has " << mUniNodes->getReadDepth(UID) << " elements");
            return false;
        }
        else
        {
            // we can't delete this guy but we can banish it
            // it will be done in the following call
            mUniNodes->banishNode(UID);

            // the GID is useless so we need to strike it from any blocks
            removeNode(GID);
            deleteGenericNodeId(GID);

            return true;
        }
    }
    else
    {
        logError(GID << " is not a head node");
        return false;
    }
}
//HO 

/******************************************************************************
** VALIDATING NODES
******************************************************************************/

/*HV*/ bool
GenericNodeMemWrapper::
/*HV*/ validateGenericNodes(void)
//HO ;
{
    //-----
    // Make sure all the info about all the generic nodes is kosher
    //
    logInfo("Start validating the generic node structure", 2);
    bool valid = true;

    NB_ELEM nbe;
    getNonBanishedBlock_DN(&nbe);
    
    while(nextNBElem(&nbe))
    {
        GenericNodeId current_GN = nbe.NBE_CurrentNode;
        UniNodeId current_UN = getUniNode(current_GN);
        DualNodeId current_DN = getDualNode(current_GN);
        // check that the nodes listed as before and after have the same UniNodeId listed
        if(getUniNode(getNextElem(current_GN)) != current_UN)
        {
            logError("Invalid 1" << current_GN << " - " << current_UN <<
                    " - " << getNextElem(current_GN) << " - " << getUniNode(getNextElem(current_GN)));
            valid = false;
        }
        if(getUniNode(getPrevElem(current_GN)) != current_UN)
        {
            logError("Invalid 2" << current_GN << " - " << current_UN << " - " <<
                    getPrevElem(current_GN) << " - " << getUniNode(getPrevElem(current_GN)));
            valid = false;
        }

        // check that the dualnode points to this guy
        if(mDualNodes->getGn(getDualNode(current_GN)) != current_GN)
        {
            logError("Invalid 3" << current_GN << " - " << current_DN << " - " << mDualNodes->getGn(current_DN));
            valid = false;
        }
    }

    if(valid)
    {
        logInfo("GenericNode structure is valid", 2);
    } else {
        logInfo("Invalid GenericNode structure", 2);
    }

    return valid;
}
//HO 

/*HV*/ bool
GenericNodeMemWrapper::
/*HV*/ validateUniNodes(void)
//HO ;
{
     //-----
    // we need to check that the UniNode graph is valid
    // that is all edge links must be symmetrical
    // nodes of certain ranks must have the correct labels
    //
    logInfo("Validating UniNode graph", 1);
    
    stringstream ss;
    
    map<string, uMDInt> check_map_1;
    // this is a map consisting of a string key "ID,ID" which represents the two IDs in an edge and the value is
    // the type of the edge: O, A or B. We add or remove values from this map (add on the first time we see
    // an edge and remove on the second time.)
    // if it''s not empty at the end then something's wrong
    string key;                                                 // the key for the map
    map<string, uMDInt>::iterator finder;
    
    map<string, int> check_map_2;
    // this map has the same key but keeps a counter of how many times we have seen an edge between these
    // nodes. This should be no greater than 2 and no less than 2

    UniNodeClass::UniNodeEdgeElem tmp_edge;

    // first do the banished nodes
    logInfo("Validating banished UniNodes", 1);
    NB_ELEM nbe;
    getBanishedBlock_UN(&nbe);

    while(nextNBElem(&nbe))
    {
        // go through all the nodes in the data set...
        // take down the info we have stored
        UniNodeId current_uninode = getUniNode(nbe.NBE_CurrentNode);
        if(mUniNodes->getNodeType(current_uninode) != UN_NODE_TYPE_BANISHED)
            logError("Banished node " << current_uninode << " with type not banished, type is: " << mUniNodes->sayNodesTypeLikeAHuman(current_uninode));
        string this_id_string;
        string that_id_string;

        // stringify this ID
        stringstream this_ss;
        this_ss << current_uninode;
        this_id_string = this_ss.str();

        if(mUniNodes->getEdges(&tmp_edge, 0, 0, current_uninode))
        {
            // we have some olaps, so process them
            do {
                if(GN_EDGE_STATE_BANISHED  != tmp_edge.UNEE_State)
                    logError("Banished uninode with non-banished edges: " << current_uninode);

                // stringify that ID
                stringstream that_ss;
                that_ss << tmp_edge.UNEE_OlapNode;
                that_id_string = that_ss.str();

                if(current_uninode < tmp_edge.UNEE_OlapNode)
                {
                    key = this_id_string + that_id_string;
                }
                else if(tmp_edge.UNEE_OlapNode < current_uninode)
                {
                    key = that_id_string + this_id_string;
                }
                else
                {
                    logError("node " << current_uninode << " links to self");
                }

                // remove this guy if he's in the map, otherwise add it...
                finder = check_map_1.find(key);
                if(finder == check_map_1.end())
                {
                    // add this guy in
                    check_map_1[key] = tmp_edge.UNEE_State;
                    check_map_2[key] = 1;
                }
                else
                {
                    // found it, we'll check the symmetry

                    if(tmp_edge.UNEE_State != finder->second)
                    {
                        logError("Symmetry broken between nodes: " << current_uninode << " and " << tmp_edge.UNEE_OlapNode);
                    }
                    check_map_1.erase(finder);
                    check_map_2[key]++;
                    if(check_map_2[key] > 2)
                    {
                        logError("Multiple edges between nodes: " << current_uninode << " and " << tmp_edge.UNEE_OlapNode);
                    }
                }

                // now check that the offset is within bounds
                if((uMDInt)(fAbs(tmp_edge.UNEE_Offset)) > mExtOffset)
                {
                    logError("Offset to large: " << tmp_edge.UNEE_Offset <<  " > " << mExtOffset << " between: " << current_uninode << " and " << tmp_edge.UNEE_OlapNode);
                }

            } while (mUniNodes->getNextEdge(&tmp_edge));
        }
    }
    
    // then the non-banished ones
    logInfo("Validating non-banished UniNodes", 1);
    getNonBanishedBlock_UN(&nbe);

    while(nextNBElem(&nbe))
    {
        UniNodeId current_uninode = getUniNode(nbe.NBE_CurrentNode);
        if(mUniNodes->getNullUniNodeId() == current_uninode)
        {
            logError("'0' UniNode! -- " << nbe.NBE_CurrentNode << " : " << isValidAddress(nbe.NBE_CurrentNode));
        }

        // go through all the nodes in the data set...
        // take down the info we have stored
        int rec_Nt_rank = mUniNodes->getNtRank(current_uninode);
        int rec_T_rank = mUniNodes->getTRank(current_uninode);

        // check that the node type is set correctly
        switch(rec_Nt_rank)
        {
            case 1:
            {
                if(mUniNodes->getNodeType(current_uninode) != UN_NODE_TYPE_CAP)
                    logError("Rank 1 node not a cap, type is: " << mUniNodes->sayNodesTypeLikeAHuman(current_uninode));
                break;
            }
            case 2:
                if(mUniNodes->getNodeType(current_uninode) != UN_NODE_TYPE_PATH)
                    logError("Rank 2 node not a path, type is: " << mUniNodes->sayNodesTypeLikeAHuman(current_uninode));
                break;
            case 0:
                if(mUniNodes->getNodeType(current_uninode) != UN_NODE_TYPE_DETACHED)
                    logError("Rank 0 node " << current_uninode << " not detached, type is: " << mUniNodes->sayNodesTypeLikeAHuman(current_uninode));
                break;
            default:
            {
                if(mUniNodes->getNodeType(current_uninode) != UN_NODE_TYPE_CROSS)
                {
                    logError("Rank 3 node not a cross, type is: " << mUniNodes->sayNodesTypeLikeAHuman(current_uninode));
                }
                break;
            }
        }


        int true_Nt_rank = 0;
        int true_T_rank = 0;

        string this_id_string;
        string that_id_string;

        // stringify this ID
        stringstream this_ss;
        this_ss << current_uninode;
        this_ss >> this_id_string;

        if(mUniNodes->getEdges(&tmp_edge, 0, 0, current_uninode))
        {
            // we have some olaps, so process them
            do {
                switch(tmp_edge.UNEE_State)
                {
                    case GN_EDGE_STATE_TRANS:
                        true_T_rank++;
                        break;
                    case GN_EDGE_STATE_NTRANS:
                        true_Nt_rank++;
                        break;
                }

                // stringify that ID
                stringstream that_ss;
                that_ss << tmp_edge.UNEE_OlapNode;
                that_ss >> that_id_string;

                if(current_uninode < tmp_edge.UNEE_OlapNode)
                {
                    key = this_id_string + that_id_string;
                }
                else if(tmp_edge.UNEE_OlapNode < current_uninode)
                {
                    key = that_id_string + this_id_string;
                }
                else
                {
                    logError("node " << current_uninode << " links to self");
                }

                // remove this guy if he's in the map, otherwise add it...
                finder = check_map_1.find(key);
                if(finder == check_map_1.end())
                {
                    // add this guy in
                    check_map_1[key] = tmp_edge.UNEE_State;
                    check_map_2[key] = 1;
                }
                else
                {
                    // found it, we'll check the symmetry
                    if(tmp_edge.UNEE_State != finder->second)
                    {
                        logError("Symmetry broken between nodes: " << current_uninode << " and " << tmp_edge.UNEE_OlapNode);
                        logError(tmp_edge.UNEE_State << " != " << finder->second);
                    }
                    check_map_1.erase(finder);
                    check_map_2[key]++;
                    if(check_map_2[key] > 2)
                    {
                        logError("Multiple edges between nodes: " << current_uninode << " and " << tmp_edge.UNEE_OlapNode);
                    }
                }

                // now check that the offset is within bounds
                if((uMDInt)(fAbs(tmp_edge.UNEE_Offset)) > mExtOffset)
                {
                    logError("Offset to large: " << tmp_edge.UNEE_Offset <<  " > " << mExtOffset << " between: " << current_uninode << " and " << tmp_edge.UNEE_OlapNode);
                }

            } while (mUniNodes->getNextEdge(&tmp_edge));

            // check to see if the ranks line up...

            if(rec_Nt_rank != true_Nt_rank)
            {
                logError("Non-transitive ranks do not match for node: " << current_uninode << " T:[" << rec_T_rank << ", " << true_T_rank << "] Nt:[" << rec_Nt_rank << ", " << true_Nt_rank << "]");
            }
            if(rec_T_rank != true_T_rank)
            {
                logError("Transitive ranks do not match for node: " << current_uninode << " T:[" << rec_T_rank << ", " << true_T_rank << "] Nt:[" << rec_Nt_rank << ", " << true_Nt_rank << "]");
            }
        }
    }

    if(!check_map_1.empty())
    {
        // something is very wrong
        logError("Invalid UniNode graph, check_map_1 is not empty");
        map<string, uMDInt>::iterator check_iter = check_map_1.begin();
        map<string, uMDInt>::iterator check_last = check_map_1.end();
        while(check_iter != check_last)
        {
            logWarn(check_iter->first << "  " << mUniNodes->sayEdgeStateLikeAHuman(check_iter->second), 1);
            check_iter++;
        }
        return false;
    }
    
    logInfo("done.", 2);
    return true;
}
//HO 

/*HV*/ bool
GenericNodeMemWrapper::
/*HV*/ validateDualNodes(ContextId CTXID)
//HO ;
{
    bool valid_context = true;
    
    // get the right node list and get the iterators ready
    GenericNodeId current_GID;
    DualNodeId current_DID, olap_DID;

    // for traversing through edges
    DualNodeEdgeElem dnee;
    string this_id_string;
    string that_id_string;
    string key;                                                 // the key for the map
    
    map<string, uMDInt> check_map_1;
    map<string, uMDInt>::iterator finder;
    // this is a map consisting of a string key "ID,ID" which represents the two IDs in an edge and the value is
    // the type of the edge: We add or remove values from this map (add on the first time we see
    // an edge and remove on the second time.)
    // if it''s not empty at the end then something's wrong
    // also we wanyt to make sure that edges are not added twice so we need this next check map too    
    map<string, int> check_map_2;
    
    NB_ELEM nbe;
    getNonBanishedBlock_DN(&nbe);
    
    while(nextNBElem(&nbe))
    {
            current_GID = nbe.NBE_CurrentNode;

            if(getCntxId(current_GID) == CTXID)
            {
                // we have a guy in this context
                current_DID = getDualNode(current_GID);
                stringstream this_ss;
                this_ss << current_DID;
                this_ss >> this_id_string;
                
                int rec_Nt_rank = mDualNodes->getNtRank(current_DID);

                // check that the node type is set correctly
                switch(rec_Nt_rank)
                {
                    case 1:
                    {
                        if(mDualNodes->getNodeType(current_DID) != DN_NODE_TYPE_CAP)
                        {
                            logError("Rank 1 node not a cap");
                            valid_context = false;
                        }
                        break;
                    }
                    case 2:
                        if(mDualNodes->getNodeType(current_DID) != DN_NODE_TYPE_PATH)
                        {
                            logError("Rank 2 node not a path");
                            valid_context = false;
                        }
                        break;
                    case 0:
                        break;
                    default:
                    {
                        if(mDualNodes->getNodeType(current_DID) != DN_NODE_TYPE_CROSS)
                        {
                            logError("Rank 3 node not a cross");
                            valid_context = false;
                        }
                        break;
                    }
                }

                int true_Nt_rank = 0;

                if(mDualNodes->getEdges(&dnee, 0, 0, current_DID))
                {
                    do {
                        olap_DID = dnee.DNEE_OlapNode;

                        if(dnee.DNEE_State == DN_EDGE_STATE_MASTER)
                            true_Nt_rank++;
                        
                        stringstream that_ss;
                        that_ss << olap_DID;
                        that_ss >> that_id_string;
                        
                        if(current_DID < olap_DID)
                        {
                            key = this_id_string + that_id_string;
                        }
                        else if(olap_DID < current_DID)
                        {
                            key = that_id_string + this_id_string;
                        }
                        else
                        {
                            logError("node " << current_DID  << " : " << mDualNodes->getGn(current_DID) << " links to self");
                            valid_context = false;
                        }

                        // remove this guy if he's in the map, otherwise add it...
                        finder = check_map_1.find(key);
                        if(finder == check_map_1.end())
                        {
                            // add this guy in
                            check_map_1[key] = dnee.DNEE_State;
                            check_map_2[key] = 1;
                        }
                        else
                        {
                            // found it, we'll check the symmetry
                            if(dnee.DNEE_State != finder->second)
                            {
                                logError("Symmetry broken between nodes: " << current_DID << " : " << mDualNodes->getGn(current_DID) <<  " and " << olap_DID << " : " << mDualNodes->getGn(olap_DID));
#ifdef MAKE_PARANOID
                                std::cout << "SYM_ERROR: " << __LINE__ << " : " << __FILE__ << std::endl;
                                printEdges(DUALNODE, mDualNodes->getGn(current_DID));
                                printEdges(DUALNODE, mDualNodes->getGn(olap_DID));
                                std::cout << "SYM_ERROR: " << __LINE__ << " : " << __FILE__ << std::endl;
#endif
                                valid_context = false;
                            }
                            check_map_1.erase(finder);
                            check_map_2[key]++;
                            if(check_map_2[key] > 2)
                            {
                                logError("Multiple edges between nodes: " << current_DID << " and " << olap_DID);
                                valid_context = false;
                            }
                        }

                        // now check that the offset is within bounds
                        if((uMDInt)(fAbs(dnee.DNEE_Offset)) > mExtOffset)
                        {
                            logError("Offset to large: " << dnee.DNEE_Offset <<  " > " << mExtOffset << " between: " << current_DID << " and " << olap_DID);
                            valid_context = false;
                        }
                    } while(mDualNodes->getNextEdge(&dnee));
                }
                // finally check to see if the recorded rank matched the true rank
                if(true_Nt_rank != rec_Nt_rank)
                {
                    logError("true: " << true_Nt_rank <<  " != rec: " << rec_Nt_rank << " for " << current_DID << " : " << mDualNodes->getGn(current_DID));
                    valid_context = false;
                }
            }
    }
    if(valid_context)
    {
        logInfo("Valid context", 3);
        return true;
    }
    logInfo("Context contains errors", 3);
    return false;

}
//HO 

/******************************************************************************
*******************************************************************************
** NAVIGATION
*******************************************************************************
*******************************************************************************
** LOOPS
******************************************************************************/
/*HV*/ bool
GenericNodeMemWrapper::
/*HV*/ getElem(GN_ELEM * data, UniNodeId UID)
//HO ;
{
    //-----
    // Get the first element in the loop
    //
    GenericNodeId head = mUniNodes->getGn(UID);
    data->GNE_UID = UID;
    data->GNE_GID = head;
    data->GNE_DID = getDualNode(head);
    data->GNE_Orientation = isFileReversed(head);
    data->GNE_Master = isCntxMaster(head);
    data->GNE_Dummy = isDummy(head);
    data->GNE_isHead = isHead(head);
    data->GNE_CNTX = getCntxId(head);
    data->GNE_CNTXPos = getCntxPos(head);
    data->GNE_CNTXRev = isCntxReversed(head);
    data->GNE_Head = head;
    data->GNE_NextHit = getNextElem(head);
    return true;
}
//HO 

/*HV*/ bool
GenericNodeMemWrapper::
/*HV*/ getNextElem(GN_ELEM * data)
//HO ;
{
    //-----
    // Get the next element in the loop
    //
    if(data->GNE_Head != data->GNE_NextHit && GenericNodeId() != data->GNE_Head)
    {
        data->GNE_GID = data->GNE_NextHit;
        data->GNE_DID = getDualNode(data->GNE_NextHit);
        data->GNE_Orientation = isFileReversed(data->GNE_NextHit);
        data->GNE_Master = isCntxMaster(data->GNE_NextHit);
        data->GNE_Dummy = isDummy(data->GNE_NextHit);
        data->GNE_isHead = isHead(data->GNE_NextHit);
        data->GNE_CNTX = getCntxId(data->GNE_NextHit);
        data->GNE_CNTXPos = getCntxPos(data->GNE_NextHit);
        data->GNE_CNTXRev = isCntxReversed(data->GNE_NextHit);
        data->GNE_NextHit = getNextElem(data->GNE_NextHit);
        return true;
    }
    return false;
}
//HO 

/*HV*/ bool
GenericNodeMemWrapper::
/*HV*/ getNextElem(GN_ELEM * data, bool yoohoo)
//HO ;
{
    //-----
    // Get the next element in the loop
    //
    if(data->GNE_Head != data->GNE_NextHit && GenericNodeId() != data->GNE_Head)
    {
        data->GNE_GID = data->GNE_NextHit;
        data->GNE_DID = getDualNode(data->GNE_NextHit);
        data->GNE_Orientation = isFileReversed(data->GNE_NextHit);
        data->GNE_Master = isCntxMaster(data->GNE_NextHit);
        data->GNE_Dummy = isDummy(data->GNE_NextHit);
        data->GNE_isHead = isHead(data->GNE_NextHit);
        data->GNE_CNTX = getCntxId(data->GNE_NextHit);
        data->GNE_CNTXPos = getCntxPos(data->GNE_NextHit);
        data->GNE_CNTXRev = isCntxReversed(data->GNE_NextHit);
        data->GNE_NextHit = getNextElem(data->GNE_NextHit);
        return true;
    }
    return false;
}
//HO 

/******************************************************************************
** EDGES
******************************************************************************/
    /*
    ** For getting edges
    */
/*HV*/ bool
GenericNodeMemWrapper::
/*HV*/ getEdges(GN_EDGE_ELEM * data, uMDInt edgeState, sMDInt offSign, GenericNodeId GID)
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
            return mUniNodes->getEdges(&(data->mUNEE), edgeState, offSign, getUniNode(GID));
        }
        case DUALNODE:
        {
            return mDualNodes->getEdges(&(data->mDNEE), edgeState, offSign, getDualNode(GID));
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
GenericNodeMemWrapper::
/*HV*/ getNextEdge(GN_EDGE_ELEM * data)
//HO ;
{
    switch(data->mNc)
    {
        case UNINODE:
        {
            return mUniNodes->getNextEdge(&(data->mUNEE));
        }
        case DUALNODE:
        {
            return mDualNodes->getNextEdge(&(data->mDNEE));
        }
        default:
            return false;
    }
}
//HO 

/******************************************************************************
** NODE BLOCKS
******************************************************************************/
//HO inline void getUniNodeBlock(NB_ELEM * nbe)        { nbe->NBE_DepthRemaining = GN_UniNodes; nbe->NBE_CurrentNode = mUniNodeBlock; }
//HO 
//HO inline void getUniNodeBlockOnly(NB_ELEM * nbe)        { nbe->NBE_DepthRemaining = 1; nbe->NBE_CurrentNode = mUniNodeBlock; }
//HO 
//HO inline void getNonBanishedBlock_UN(NB_ELEM * nbe) { nbe->NBE_DepthRemaining = GN_U_Non_Banished; nbe->NBE_CurrentNode = mNonBanishedBlock_UN; }
//HO 
//HO inline void getNonBanishedBlockOnly_UN(NB_ELEM * nbe) { nbe->NBE_DepthRemaining = 1; nbe->NBE_CurrentNode = mNonBanishedBlock_UN; }
//HO 
//HO inline void getBanishedBlock_UN(NB_ELEM * nbe)    { nbe->NBE_DepthRemaining = GN_U_Banished; nbe->NBE_CurrentNode = mBanishedBlock_UN; }
//HO 
//HO inline void getAttachedBlock_UN(NB_ELEM * nbe)    { nbe->NBE_DepthRemaining = GN_U_Attached; nbe->NBE_CurrentNode = mAttachedBlock_UN; }
//HO 
//HO inline void getAttachedBlockOnly_UN(NB_ELEM * nbe)    { nbe->NBE_DepthRemaining = 1; nbe->NBE_CurrentNode = mAttachedBlock_UN; }
//HO 
//HO inline void getDetachedBlock_UN(NB_ELEM * nbe)    { nbe->NBE_DepthRemaining = GN_U_Detached; nbe->NBE_CurrentNode = mDetachedBlock_UN; }
//HO 
//HO inline void getCapBlock_UN(NB_ELEM * nbe)         { nbe->NBE_DepthRemaining = GN_U_Cap; nbe->NBE_CurrentNode = mCapBlock_UN; }
//HO 
//HO inline void getPathBlock_UN(NB_ELEM * nbe)        { nbe->NBE_DepthRemaining = GN_U_Path; nbe->NBE_CurrentNode = mPathBlock_UN; }
//HO 
//HO inline void getCrossBlock_UN(NB_ELEM * nbe)       { nbe->NBE_DepthRemaining = GN_U_Cross; nbe->NBE_CurrentNode = mCrossBlock_UN; }
//HO 

//HO void clearUniNodeBlock(void)               { NB_ELEM nbe; getUniNodeBlock(&nbe); clearBlock(&nbe); }
//HO 
//HO void clearNonBanishedBlock_UN(void)        { NB_ELEM nbe; getNonBanishedBlock_UN(&nbe); clearBlock(&nbe); }
//HO 
//HO void clearBanishedBlock_UN(void)           { NB_ELEM nbe; getBanishedBlock_UN(&nbe); clearBlock(&nbe); }
//HO 
//HO void clearAttachedBlock_UN(void)           { NB_ELEM nbe; getAttachedBlock_UN(&nbe); clearBlock(&nbe); }
//HO 
//HO void clearDetachedBlock_UN(void)           { NB_ELEM nbe; getDetachedBlock_UN(&nbe); clearBlock(&nbe); }
//HO 
//HO void clearCapBlock_UN(void)                { NB_ELEM nbe; getCapBlock_UN(&nbe); clearBlock(&nbe); }
//HO 
//HO void clearPathBlock_UN(void)               { NB_ELEM nbe; getPathBlock_UN(&nbe); clearBlock(&nbe); }
//HO 
//HO void clearCrossBlock_UN(void)              { NB_ELEM nbe; getCrossBlock_UN(&nbe); clearBlock(&nbe); }
//HO 

/*HV*/ void
GenericNodeMemWrapper::
/*HV*/ addContextBlocks(ContextId CTXID)
//HO ;
{
    //-----
    // for every new context we make we need to inser a number of place holders into the lists
    // this is handled here:
    //
    // make sure we haven't already added this guy before
    ContextBlockIterator ctx_iter = mCTXS_DN.find(CTXID);
    if(ctx_iter != mCTXS_DN.end())
    {
        logError("Trying to re-add: context: " << CTXID);
        return;
    }

    // make the nodeblockheads...
    GenericNodeId tmp_CTX = newGenericNodeId();
    GenericNodeId tmp_CAP = newGenericNodeId();
    GenericNodeId tmp_PATH = newGenericNodeId();
    GenericNodeId tmp_CROSS = newGenericNodeId();

    // set them up as heads...
    setNodeBlockHead(true, tmp_CTX);
    setNodeBlockHead(true, tmp_CAP);
    setNodeBlockHead(true, tmp_PATH);
    setNodeBlockHead(true, tmp_CROSS);
    
    // link them up
    addNodeFront(tmp_CROSS, mAttachedBlock_DN);
    addNodeFront(tmp_PATH, mAttachedBlock_DN);
    addNodeFront(tmp_CAP, mAttachedBlock_DN);
    addNodeFront(tmp_CTX, mAttachedBlock_DN);

    // this will leave the lists like:
    // Before: Att_DN --> etc...
    // After:  Att_DN --> tmp_CTX --> tmp_CAP --> tmp_PATH --> tmp_CROSS --> etc...
    
    // add them in to the lookup map
    mCaps_DN[CTXID] = tmp_CAP;
    mPaths_DN[CTXID] = tmp_PATH;
    mCrosses_DN[CTXID] = tmp_CROSS;
    mCTXS_DN[CTXID] = tmp_CTX;

    // add one more to the list
    mNumContextsInBlock++;

//    std::cout << "New CTX: " << CTXID << " S: " << tmp_CTX << " C: " << tmp_CAP << " P: " << tmp_PATH << " X: " << tmp_CROSS << std::endl;
}
//HO 

/*HV*/ void
GenericNodeMemWrapper::
/*HV*/ removeContextBlocks(ContextId CTXID)
//HO ;
{
    //-----
    // remove all the node blocks for a contextID.
    // The context MUST be deviod of all nodes when this is called
    // get the nodeblockheads...

    // have we even added this context?
    ContextBlockIterator ctx_iter = mCTXS_DN.find(CTXID);
    if(ctx_iter == mCTXS_DN.end())
    {
        logError("Trying to remove non-existant context: " << CTXID);
        return;
    }
    
    // we have to ensure that all the nodes in the main list have been detached
    GenericNodeId tmp_CTX = mCTXS_DN[CTXID];
    GenericNodeId tmp_CAP = mCaps_DN[CTXID];
    GenericNodeId tmp_PATH = mPaths_DN[CTXID];
    GenericNodeId tmp_CROSS = mCrosses_DN[CTXID];

    GenericNodeId next = getNextElemNb(tmp_CTX);
    if(next != tmp_CAP)
    {
        logError("Next from: " << tmp_CTX << " : " << CTXID << " not the cap: " << tmp_CAP);
        return;
    }
    next = getNextElemNb(tmp_CAP);
    if(!isNodeBlockHead(next))
    {
        logError("Next from: " << tmp_CAP << " : " << CTXID << " not the path: " << tmp_PATH);
        return;
    }
    next = getNextElemNb(tmp_PATH);
    if(!isNodeBlockHead(next))
    {
        logError("Next from: " << tmp_PATH << " : " << CTXID << " not the cross: " << tmp_CROSS);
        return;
    }
    next = getNextElemNb(tmp_CROSS);
    if(!isNodeBlockHead(next))
    {
        logError("Next from: " << tmp_CROSS << " : " << CTXID << " not a nodeBlockHead");
        return;
    }

    // if we are here then the context must be empty of nodes...
    // relink the nodeblocks
    GenericNodeId prev = getPrevElemNb(tmp_CTX);
    setNextElemNb(next, prev);
    setPrevElemNb(prev, next);


    // now we remove all the nodeHeadBlocks
    ctx_iter = mCTXS_DN.find(CTXID);
    if(ctx_iter != mCTXS_DN.end())
    {
        mCTXS_DN.erase(ctx_iter);
        deleteGenericNodeId(tmp_CTX);
    }
    ctx_iter = mCaps_DN.find(CTXID);
    if(ctx_iter != mCaps_DN.end())
    {
        mCaps_DN.erase(ctx_iter);
        deleteGenericNodeId(tmp_CAP);
    }
    ctx_iter = mPaths_DN.find(CTXID);
    if(ctx_iter != mPaths_DN.end())
    {
        mPaths_DN.erase(ctx_iter);
        deleteGenericNodeId(tmp_PATH);
    }
    ctx_iter = mCrosses_DN.find(CTXID);
    if(ctx_iter != mCrosses_DN.end())
    {
        mCrosses_DN.erase(ctx_iter);
        deleteGenericNodeId(tmp_CROSS);
    }
            
    // take this one off the list
    mNumContextsInBlock--;
}
//HO 

//HO inline void getDualNodeBlock(NB_ELEM * nbe)       { nbe->NBE_DepthRemaining = GN_DualNodes; nbe->NBE_CurrentNode = mDualNodeBlock; }
//HO 
//HO inline void getDualNodeBlockOnly(NB_ELEM * nbe)       { nbe->NBE_DepthRemaining = 1; nbe->NBE_CurrentNode = mDualNodeBlock; }
//HO 
//HO inline void getNonBanishedBlock_DN(NB_ELEM * nbe) { nbe->NBE_DepthRemaining = GN_D_Non_Banished; nbe->NBE_CurrentNode = mNonBanishedBlock_DN; }
//HO 
//HO inline void getNonBanishedBlockOnly_DN(NB_ELEM * nbe) { nbe->NBE_DepthRemaining = 1; nbe->NBE_CurrentNode = mNonBanishedBlock_DN; }
//HO 
//HO inline void getBanishedBlock_DN(NB_ELEM * nbe)    { nbe->NBE_DepthRemaining = GN_D_Banished; nbe->NBE_CurrentNode = mBanishedBlock_DN; }
//HO 
//HO inline void getBanishedBlockOnly_DN(NB_ELEM * nbe)    { nbe->NBE_DepthRemaining = 1; nbe->NBE_CurrentNode = mBanishedBlock_DN; }
//HO 
//HO inline void getAttachedBlock_DN(NB_ELEM * nbe)    { nbe->NBE_DepthRemaining = (GN_D_Attached + 4 * mNumContextsInBlock); nbe->NBE_CurrentNode = mAttachedBlock_DN; }
//HO 
//HO inline void getAttachedBlockOnly_DN(NB_ELEM * nbe)    { nbe->NBE_DepthRemaining = 1; nbe->NBE_CurrentNode = mAttachedBlock_DN; }
//HO 
//HO inline void getDetachedBlock_DN(NB_ELEM * nbe)    { nbe->NBE_DepthRemaining = GN_D_Detached; nbe->NBE_CurrentNode = mDetachedBlock_DN; }
//HO 
/*HV*/ void
GenericNodeMemWrapper::
/*HV*/ getCTXBlock_DN(NB_ELEM * nbe, ContextId CTXID)
//HO ;
{
    //-----
    // Get all the attached nodes within a context
    //
    ContextBlockIterator ctx_iter = mCTXS_DN.find(CTXID);
    if(ctx_iter != mCTXS_DN.end())
    {
        nbe->NBE_CurrentNode = ctx_iter->second;
        nbe->NBE_DepthRemaining = 4;
    }
    else
    {
        logError("No such context: " << CTXID);
    }
}
//HO 
/*HV*/ GenericNodeId
GenericNodeMemWrapper::
/*HV*/ getCTXBlockNode_DN(ContextId CTXID)
//HO ;
{
    //-----
    // Get all the attached nodes within a context
    //
    ContextBlockIterator ctx_iter = mCTXS_DN.find(CTXID);
    if(ctx_iter != mCTXS_DN.end())
    {
        return ctx_iter->second;
    }
    else
    {
        logError("No such context: " << CTXID);
        return GN_NULL_ID;
    }
}
//HO 

/*HV*/ void
GenericNodeMemWrapper::
/*HV*/ getCapBlock_DN(NB_ELEM * nbe, ContextId CTXID)
//HO ;
{
    //-----
    // like the call for uniNodes but we need to specify which context
    // we are working within
    //
    ContextBlockIterator ctx_iter = mCaps_DN.find(CTXID);
    if(ctx_iter != mCaps_DN.end())
    {
        nbe->NBE_CurrentNode = ctx_iter->second;
        nbe->NBE_DepthRemaining = 1;
    }
    else
    {
        logError("No such context: " << CTXID);
    }
}
//HO 

//HO void shuffleCapForward_DN(GenericNodeId cap_GID, ContextId CTXID) { transferNode(cap_GID, getCapBlockNode_DN(CTXID)); }
//HO 

/*HV*/ GenericNodeId
GenericNodeMemWrapper::
/*HV*/ getCapBlockNode_DN(ContextId CTXID)
//HO ;
{
    //-----
    // like the call for uniNodes but we need to specify which context
    // we are working within
    //
    ContextBlockIterator ctx_iter = mCaps_DN.find(CTXID);
    if(ctx_iter != mCaps_DN.end())
    {
        return ctx_iter->second;
    }
    else
    {
        logError("No such context: " << CTXID);
        return GN_NULL_ID;
    }
}
//HO 

/*HV*/ void
GenericNodeMemWrapper::
/*HV*/ getPathBlock_DN(NB_ELEM * nbe, ContextId CTXID)
//HO ;
{
    //-----
    // like the call for uniNodes but we need to specify which context
    // we are working within
    //
    ContextBlockIterator ctx_iter = mPaths_DN.find(CTXID);
    if(ctx_iter != mPaths_DN.end())
    {
        nbe->NBE_CurrentNode = ctx_iter->second;
        nbe->NBE_DepthRemaining = 1;
    }
    else
    {
        logError("No such context: " << CTXID);
    }
}
//HO 
/*HV*/ GenericNodeId
GenericNodeMemWrapper::
/*HV*/ getPathBlockNode_DN(ContextId CTXID)
//HO ;
{
    //-----
    // like the call for uniNodes but we need to specify which context
    // we are working within
    //
    ContextBlockIterator ctx_iter = mPaths_DN.find(CTXID);
    if(ctx_iter != mPaths_DN.end())
    {
        return ctx_iter->second;
    }
    else
    {
        logError("No such context: " << CTXID);
        return GN_NULL_ID;
    }
}
//HO 

/*HV*/ void
GenericNodeMemWrapper::
/*HV*/ getCrossBlock_DN(NB_ELEM * nbe, ContextId CTXID)
//HO ;
{
    //-----
    // like the call for uniNodes but we need to specify which context
    // we are working within
    //
    ContextBlockIterator ctx_iter = mCrosses_DN.find(CTXID);
    if(ctx_iter != mCrosses_DN.end())
    {
        nbe->NBE_CurrentNode = ctx_iter->second;
        nbe->NBE_DepthRemaining = 1;
    }
    else
    {
        logError("No such context: " << CTXID);
    }
}
//HO 
/*HV*/ GenericNodeId
GenericNodeMemWrapper::
/*HV*/ getCrossBlockNode_DN(ContextId CTXID)
//HO ;
{
    //-----
    // like the call for uniNodes but we need to specify which context
    // we are working within
    //
    ContextBlockIterator ctx_iter = mCrosses_DN.find(CTXID);
    if(ctx_iter != mCrosses_DN.end())
    {
        return ctx_iter->second;
    }
    else
    {
        logError("No such context: " << CTXID);
        return GN_NULL_ID;
    }
}
//HO 

//HO void clearDualNodeBlock(void)              { NB_ELEM nbe; getDualNodeBlock(&nbe); clearBlock(&nbe); }
//HO 
//HO void clearNonBanishedBlock_DN(void)        { NB_ELEM nbe; getNonBanishedBlock_DN(&nbe); clearBlock(&nbe); }
//HO 
//HO void clearBanishedBlock_DN(void)           { NB_ELEM nbe; getBanishedBlock_DN(&nbe); clearBlock(&nbe); }
//HO 
//HO void clearAttachedBlock_DN(void)           { NB_ELEM nbe; getAttachedBlock_DN(&nbe); clearBlock(&nbe); }
//HO 
//HO void clearDetachedBlock_DN(void)           { NB_ELEM nbe; getDetachedBlock_DN(&nbe); clearBlock(&nbe); }
//HO 

/*HV*/ bool
GenericNodeMemWrapper::
/*HV*/ nextNBElem(NB_ELEM * nbe)
//HO ;
{
    //-----
    // Use this when trvaersing through a node block
    // The "get" statement is inline above, This will allow
    // you to walk through the list, NBE_CurrentNode cannot
    //  be deleted during this run.
    //
    // Note there is no block type specified, you need to
    // do this in the "get" statement
    //
    GenericNodeId next_node = getNextElemNb(nbe->NBE_CurrentNode);
    PARANOID_ASSERT_PRINT_L2((GN_NULL_ID != getNextElemNb(nbe->NBE_CurrentNode)), nbe->NBE_CurrentNode << " points to NULL");
    // we need to skip over any other heads we have
    while(isNodeBlockHead(next_node))
    {
        if(1 >= nbe->NBE_DepthRemaining)
        {
            // this is the end of the list we were asked for
            return false;
        }
        else
        {
            nbe->NBE_DepthRemaining--;
            next_node = getNextElemNb(next_node);
        }
    }
    nbe->NBE_CurrentNode = next_node;
    return true;
}
//HO 

/*HV*/ GenericNodeId
GenericNodeMemWrapper::
/*HV*/ getNodeBlockNode(NB_TYPE nbt)
//HO ;
{
    //-----
    // Get the ID of the most common types
    //
    switch(nbt)
    {
        case UNINODES:
        {
            return mUniNodeBlock;
        }
        case DUALNODES:
        {
            return mDualNodeBlock;
        }
        case BANISHEDNODES_UN:
        {
            return mBanishedBlock_UN;
        }
        case NONBANISHEDNODES_UN:
        {
            return mNonBanishedBlock_UN;
        }
        case ATTACHEDNODES_UN:
        {
            return mAttachedBlock_UN;
        }
        case DETACHEDNODES_UN:
        {
            return mDetachedBlock_UN;
        }
        case CAPNODES_UN:
        {
            return mCapBlock_UN;
        }
        case PATHNODES_UN:
        {
            return mPathBlock_UN;
        }
        case CROSSNODES_UN:
        {
            return mCrossBlock_UN;
        }
        case BANISHEDNODES_DN:
        {
            return mBanishedBlock_DN;
        }
        case NONBANISHEDNODES_DN:
        {
            return mNonBanishedBlock_DN;
        }
        case ATTACHEDNODES_DN:
        {
            return mAttachedBlock_DN;
        }
        case DETACHEDNODES_DN:
        {
            return mDetachedBlock_DN;
        }
        default:
        {
            logError("Invalid NB_TYPE");
        }
    }
    return GN_NULL_ID;
}
//HO 

/*HV*/ GenericNodeId
GenericNodeMemWrapper::
/*HV*/ getNodeBlockNode(NB_TYPE nbt, ContextId CTXID)
//HO ;
{
    //-----
    // Get the ID of the most common types
    //
    switch(nbt)
    {
        case ATTACHEDNODES_DN:
        {
            return mCTXS_DN[CTXID];
        }
        case CAPNODES_DN:
        {
            return mCaps_DN[CTXID];
        }
        case PATHNODES_DN:
        {
            return mPaths_DN[CTXID];
        }
        case CROSSNODES_DN:
        {
            return mCrosses_DN[CTXID];
        }
        default:
        {
            logError("Invalid NB_TYPE");
        }
    }
    return GN_NULL_ID;
}
//HO 

/*HV*/ bool
GenericNodeMemWrapper::
/*HV*/ addNodeFront(GenericNodeId GID, GenericNodeId block_GID)
//HO ;
{
    //-----
    // add a node to the front of the list.
    // This is a cheap operation. Should only really be called
    // when we first make the list of all nodes
    //
    PARANOID_ASSERT_PRINT_L2((GN_NULL_ID != GID), "Trying to add NULL to: " << block_GID);
    PARANOID_ASSERT_PRINT_L2((GN_NULL_ID != block_GID), "Block ID is NULL for: " << GID);
    PARANOID_ASSERT_PRINT_L2((isValidAddress(getNextElemNb(block_GID))), "Next (" << getNextElemNb(block_GID) << ") at " << block_GID << " is invalid");
    PARANOID_ASSERT_PRINT_L2((GN_NULL_ID != getNextElemNb(block_GID)), block_GID << " - is not a node block head for add: " << GID);

    GenericNodeId next = getNextElemNb(block_GID);
    setPrevElemNb(GID, next);
    setNextElemNb(next, GID);

    setNextElemNb(GID, block_GID);
    setPrevElemNb(block_GID, GID);
    return true;
}
//HO 

/*HV*/ bool
GenericNodeMemWrapper::
/*HV*/ addNodeFront(GenericNodeId GID, NB_TYPE nbt)
//HO ;
{
    //-----
    // add a node to the front of the list.
    // This is a cheap operation. Should only really be called
    // when we first make the list of all nodes
    //
    return addNodeFront(GID, getNodeBlockNode(nbt));
}
//HO 

/*HV*/ bool
GenericNodeMemWrapper::
/*HV*/ removeNode(GenericNodeId GID)
//HO ;
{
    //-----
    // strips a node from any lists it may be in
    // useful for dumies but be careful...
    //
    // take out of list
    PARANOID_ASSERT_PRINT_L2((GN_NULL_ID != GID), "Trying to remove NULL");
    PARANOID_ASSERT_PRINT_L2((!isNodeBlockHead(GID)), "Trying to remove a nodeblockhead; " << GID);

    GenericNodeId next = getNextElemNb(GID);
    GenericNodeId prev = getPrevElemNb(GID);

    PARANOID_ASSERT_PRINT_L2((GN_NULL_ID != next && GN_NULL_ID != prev), "Rem failed: " << prev << " --> ( " << GID << " ) --> " << next);

    setPrevElemNb(prev, next);
    setNextElemNb(next, prev);
    // set pointers to null
    setPrevElemNb(GN_NULL_ID, GID);
    setNextElemNb(GN_NULL_ID, GID);
    return true;
}
//HO 

/*HV*/ bool
GenericNodeMemWrapper::
/*HV*/ removeNode(NB_ELEM * activeElem)
//HO ;
{
    //-----
    // strips a node from any lists it may be in
    // This is a safe way to remove nodes
    // the activeElem is set back one place to the previous node
    //
    // keep this guy safe
    GenericNodeId final_GID = getPrevElemNb(activeElem->NBE_CurrentNode);
    PARANOID_ASSERT_PRINT_L2((GN_NULL_ID != final_GID),"Null final");

    // remove the node
    if(removeNode(activeElem->NBE_CurrentNode))
    {
        // fix up the activeElem
        activeElem->NBE_CurrentNode = final_GID;
        return true;
    }
    return false;
}
//HO 

/*HV*/ bool
GenericNodeMemWrapper::
/*HV*/ transferNode(GenericNodeId GID, GenericNodeId block_GID)
//HO ;
{
    //-----
    // DO NOT CALL THIS GUY EXPLICITLY. It is for internal use only
    // If you want to transfer a node between lists and you are sure it's safe to
    // do then call the transferNode(GenericNodeId GID, NB_TYPE nbt) call instead!
    //

    PARANOID_ASSERT_PRINT_L2(!(isNodeBlockHead(GID)), "Trying to transferNode a nodeblockhead; " << GID);
    PARANOID_ASSERT_PRINT_L2((isNodeBlockHead(block_GID)), "Trying to transferNode to a non-nodeblockhead; " << block_GID);
    PARANOID_ASSERT_PRINT_L2((GN_NULL_ID != GID), "Transferring NULL to: " << block_GID);
    
    // remove the node
    if(removeNode(GID))
    {
        // add it to the correct list
        addNodeFront(GID, block_GID);
    }
    else
    {
        logError("could not remove: " << GID);
        return false;
    }
    
    return true;
}
//HO 

/*HV*/ bool
GenericNodeMemWrapper::
/*HV*/ transferNode(GenericNodeId GID, NB_TYPE nbt)
//HO ;
{
    //-----
    // DO NOT CALL THIS GUY EXPLICITLY IF YOU ARE CURRENTLY WALKING THROUGH
    // A LIST ( ie using nextNBElem() ) 
    // In this case call the transferNode(NB_ELEM * activeElem, NB_TYPE nbt)
    //
    GenericNodeId block_GID = getNodeBlockNode(nbt);
    if(getNextElemNb(block_GID) != GID)
        return transferNode(GID, block_GID);
    return false;
}
//HO 

/*HV*/ bool
GenericNodeMemWrapper::
/*HV*/ transferNode(NB_ELEM * activeElem, NB_TYPE nbt)
//HO ;
{
    //-----
    // In nearly all cases you should call this guy. Then everything will be taken care
    // of for you. This is the SAFE call...
    //
    // activeElem will still be valid and will point to the guy directly
    // before the guy you are trying to transfer. Thus a follow-up call to 
    // nextNBElem will put it one step forward and make it point to the guy
    // WHO WAS directly after this guy before the transfer.
    //
    GenericNodeId block_GID = getNodeBlockNode(nbt);
    if(getNextElemNb(block_GID) != activeElem->NBE_CurrentNode)
        return transferNode(activeElem, block_GID);
    return false;
}
//HO 

/*HV*/ bool
GenericNodeMemWrapper::
/*HV*/ transferNode(NB_ELEM * activeElem, GenericNodeId block_GID)
//HO ;
{
    //-----
    // if you feel some overwhelming need to call this guy then make sure you 
    // have good reason to. In nearly all cases you should call the 
    // transferNode(NB_ELEM * activeElem, NB_TYPE nbt) call above instead...
    //
    GenericNodeId final_GID = getPrevElemNb(activeElem->NBE_CurrentNode);
    PARANOID_ASSERT_PRINT_L2((GN_NULL_ID != final_GID),"Null final");
    PARANOID_ASSERT_PRINT_L2((GN_NULL_ID != block_GID), "Null Block");

    if(transferNode(activeElem->NBE_CurrentNode, block_GID))
    {
        // we need to make sure that we don't point 
        // the nbe to the block we just transferred to...
        // If this could happen then it's because we are moving this
        // node to the same list it's in and it was the first node in the rank
        if(final_GID != block_GID)
            activeElem->NBE_CurrentNode = final_GID;
        return true;
    }
    return false;
}
//HO 

/*HV*/ void
GenericNodeMemWrapper::
/*HV*/ clearBlock(NB_ELEM * nbe)
//HO ;
{
    //-----
    // We assume that the nbe given is a nodeBlockHead
    // IE. someone has called get_XXX_Block(nbe) immediately before this call
    // Try not to call this guy directly
    //
    // SHOULD ONLY BE CALLED FROM THE MACROS ABOVE DIRECTLY AFTER THE CALL
    //
    while(nextNBElem(nbe))
    {
        if(!removeNode(nbe))
            break;
    }
}
//HO 
/******************************************************************************
** EDGE MAKING AND MANAGEMENT
******************************************************************************/
  //----------------------------------
  // ADDING

/*HV*/ bool
GenericNodeMemWrapper::
/*HV*/ makeBaseUniNodeEdges(void)
//HO ;
{
    //-----
    // Make all the base edges for the uninodes.
    // 
    // We assume we arrive here directly after we have done loading the data
    //

    // Kill the dupemap cause we don't need it no more...
    logInfo("Datset(s) contained: " << mNumDupes << " duplicate paired reads", 2);
    if(mDupeMap != NULL)
        delete mDupeMap;
    mDupeMap = NULL;

    logInfo("Making base UniNode edges   ...1", 2);
    // prep the helper objects
    mUniNodes->prepForEdgeMaking(mNumReads * SAS_DEF_UN_KM_MULT);

    logInfo("Prepped", 5);

    // get all the UniNodes and make edges...
    NB_ELEM nbe;
    getNonBanishedBlock_UN(&nbe);
    int counter = 0;
    int made_counter = 0;
    int tot_counter = 0;
    int tot_made_counter = 0;
    while(nextNBElem(&nbe))
    {
	counter++;
        std::vector<UniNodeId> banish_vector;

        if(mUniNodes->makeEdges(&banish_vector, getUniNode(nbe.NBE_CurrentNode)))
        {
            // something got banished. so we need to transfer the node to the banished
            // pile
            std::vector<UniNodeId>::iterator to_banish_iter = banish_vector.begin();
            std::vector<UniNodeId>::iterator to_banish_last = banish_vector.end();
            while(to_banish_iter != to_banish_last)
            {
                transferNode(mUniNodes->getGn(*to_banish_iter), BANISHEDNODES_UN);
                to_banish_iter++;
            }
        }
        else
        {
            made_counter++;
            if(counter == 500000)
            {
                tot_made_counter += made_counter;
                logInfo("... " << tot_made_counter << " UniNode edges made", 4);
                made_counter = 0;
            }
        }
        if(counter == 100000)
        {
            tot_counter += counter;
            logInfo("..." << tot_counter << " UniNodes processed", 6);
            counter = 0;
        }
    }

    // clean up the memory we used
    mUniNodes->killKmerLists();

    logInfo("Done", 1);
    return true;
}
//HO 

/*HV*/ bool
GenericNodeMemWrapper::
/*HV*/ buildUniNodeGraph(int max_offset)
//HO ;
{
    //-----
    // wrapper for the graph building functions
    //
    logInfo("Start building naive UniNodeGraph with maximum offset: " << max_offset, 1);

    mUniNodes->setCurrentMaxOffset(max_offset);
    NB_ELEM nbe;
    std::vector<UniNodeId> banish_vector;

    // we should only call this on a clean dataset
    getNonBanishedBlock_UN(&nbe);

    int total_parsed = 0;
    while(nextNBElem(&nbe))
    {
        total_parsed++;
        if(mUniNodes->labelNonTransitiveEdges(&banish_vector, max_offset, getUniNode(nbe.NBE_CurrentNode)))
        {
            // something got banished. so we need to transfer the node to the banished
            // pile
            std::vector<UniNodeId>::iterator to_banish_iter = banish_vector.begin();
            std::vector<UniNodeId>::iterator to_banish_last = banish_vector.end();
            while(to_banish_iter != to_banish_last)
            {
                transferNode(mUniNodes->getGn(*to_banish_iter), BANISHEDNODES_UN);
                to_banish_iter++;
            }
        }
    }
    
    logInfo("Done building naive UniNodeGraph - total nodes parsed: " << total_parsed, 2);

    // work out all the caps and crosses etc...
    curateUniNodeBlocks();
    return true;
}
//HO 

  //----------------------------------
  // UPDATING
    /*
    ** return the number of nodes in the given list
    */
/*HV*/ void
GenericNodeMemWrapper::
/*HV*/ sortUNEdges(void)
//HO ;
{
    //-----
    // sort the edges for the uninodes
    //
    logInfo("Shallow sorting UniNode edges", 1);

    NB_ELEM nbe;
    getAttachedBlock_UN(&nbe);

    int num_sorted = 0;
    while(nextNBElem(&nbe))
    {
        num_sorted++;
        mUniNodes->sortEdges(getUniNode(nbe.NBE_CurrentNode));
    }
    logInfo("Sorted " << num_sorted << " UniNodes", 2);
    return;
}
//HO 

/*HV*/ void
GenericNodeMemWrapper::
/*HV*/ sortDNEdges(ContextId CTXID)
//HO ;
{
    //-----
    // return the number of nodes in the given list for the given node type
    //
    logInfo("Shallow sorting DualNode edges", 1);

    NB_ELEM nbe;
    getCTXBlock_DN(&nbe, CTXID);

    int num_sorted = 0;
    while(nextNBElem(&nbe))
    {
        num_sorted++;
        mDualNodes->sortEdges(getDualNode(nbe.NBE_CurrentNode));
    }
    logInfo("Sorted " << num_sorted << " DualNodes", 2);
    return;
}
//HO 

/*HV*/ void
GenericNodeMemWrapper::
/*HV*/ sortEdges(GenericNodeId GID)
//HO ;
{
    //-----
    // return the number of nodes in the given list for the given node type
    //
    if(isHead(GID))
    {
        mUniNodes->sortEdges(getUniNode(GID));
    }
    else
    {
        mDualNodes->sortEdges(getDualNode(GID));
    }
}
//HO 

/*HV*/ void
GenericNodeMemWrapper::
/*HV*/ transferEdges(GenericNodeId oldGID, GenericNodeId replacementGID)
//HO ;
{
    //-----
    // transfer the dualnode edges from old to new.
    // make sure that it is a symmetrical transfer
    //
    // get all the edges
    PARANOID_ASSERT_PRINT_L2(isValidAddress(oldGID), oldGID);
    PARANOID_ASSERT_PRINT_L2(isValidAddress(replacementGID), replacementGID);
    DN_EDGE_ELEM tmp_dnee;
    DualNodeId old_DID = getDualNode(oldGID);
    PARANOID_ASSERT_PRINT_L2(mDualNodes->isValidAddress(old_DID), old_DID);
    DualNodeId rep_DID = getDualNode(replacementGID);
    PARANOID_ASSERT_PRINT_L2(mDualNodes->isValidAddress(rep_DID), rep_DID);
    // we are going to go through all the edges here.
    // because this is called during a detachment, if we are calling thsi on the
    // start of a branch then it will not have 100% valid edges.
    if(mDualNodes->getEdges(&tmp_dnee, 0, 0, old_DID))
    {
        do {
            if(mDualNodes->isValidAddress(tmp_dnee.DNEE_OlapNode))
            {
                // no use adding detached nodes...
                if(tmp_dnee.DNEE_OlapNode != rep_DID)
                {
                    // we don't want to include an edge from rep to rep!
                    // nor a detached node
                    // add the edge from the rep node to the olapnode
                    if(DN_EDGE_STATE_DETACHED != tmp_dnee.DNEE_State)
                    {
                        PARANOID_ASSERT_PRINT_L2(mDualNodes->isValidAddress(tmp_dnee.DNEE_OlapNode), tmp_dnee.DNEE_OlapNode);
                        mDualNodes->addEdge_asymmetrical(tmp_dnee.DNEE_State, tmp_dnee.DNEE_OlapNode, tmp_dnee.DNEE_Offset, ((tmp_dnee.DNEE_Offset * tmp_dnee.DNEE_ReturnOffset) > 0), rep_DID);
                        // update the information at the olapnode
                        PARANOID_ASSERT_PRINT_L2(mDualNodes->isValidAddress(tmp_dnee.DNEE_OlapNode), tmp_dnee.DNEE_OlapNode);
                        mDualNodes->updateEdge_asymmetrical(old_DID, rep_DID, tmp_dnee.DNEE_OlapNode);
                    }
                }
            }
        } while(mDualNodes->getNextEdge(&tmp_dnee));
    }
    mDualNodes->sortEdges(rep_DID);
}
//HO 

  //----------------------------------
 // RESETTING
    /*
    ** Reset all Detached and non transitive labels on edges
    */
/*HV*/ bool
GenericNodeMemWrapper::
/*HV*/ resetUniNodeEdges(bool keepContigIds)
//HO ;
{
    //-----
    // reset all the edges in a graph
    //
    logInfo("Reset UniNode edges", 1);

    NB_ELEM nbe;
    getNonBanishedBlock_UN(&nbe);

    int num_reset = 0;
    while(nextNBElem(&nbe))
    {
        num_reset++;
        mUniNodes->resetEdges(keepContigIds, getUniNode(nbe.NBE_CurrentNode));
        // now this node will be in the wrong list
        // we must fix that below
    }

    logInfo("Reset " << num_reset << " UniNodes", 2);

    // now we need to fix up the node lists
    curateUniNodeBlocks();
    
    return true;
}
//HO 

/*HV*/ void
GenericNodeMemWrapper::
/*HV*/ clearCntxInfo(GenericNodeId GID)
//HO ;
{
    //-----
    // clear away all the context based info for a DualNode
    //
    clearCntxId(GID);
    resetCntxPos(GID);
    clearCntxMaster(GID);
    clearCntxReversed(GID);
    resetCntxHistory(GID);
    resetCntxReadDepth(GID);

    GenericNodeId head = mUniNodes->getGn(getUniNode(GID));
    clearSaturated(head);
}
//HO 

    /*
    ** Reset all Detached and non transitive labels on edges
    */
/*HV*/ bool
GenericNodeMemWrapper::
/*HV*/ resetDualNodeEdges(bool doHeads, ContextId CTXID)
//HO ;
{
    //-----
    // reset all the edges in a graph.
    //
    // Its a little more difficult for DualNodes because we have to deal with dummies
    // Also Generic Nodes need to be reset.
    //
    logInfo("Reset DualNode edges for: " << CTXID, 6);
    int num_reset = 0;
    int num_dummies_reset = 0;

    NB_ELEM nbe;

    // do the heads first if need be
    if(doHeads)
    {
        GenericNodeId head;
        getCTXBlock_DN(&nbe, CTXID);
        while(nextNBElem(&nbe))
        {
            head = getHead(nbe.NBE_CurrentNode);
            clearCntxInfo(head);
            num_reset++;
        }
        logInfo("Reset " << num_reset << " HeadNodes", 7);
    }
    
    // now do the regular nodes
    num_reset = 0;
    getCTXBlock_DN(&nbe, CTXID);
    while(nextNBElem(&nbe))
    {
        if(isDummy(nbe.NBE_CurrentNode))
        {
            num_dummies_reset++;
            // next call handles removal from lists
            deleteDummyNode(&nbe);
        }
        else
        {
            resetDualNode(&nbe);
        }
        num_reset++;
    }
    logInfo("Reset " << num_reset << " DualNodes, including: " << num_dummies_reset << " dummy nodes. Leaves: " << mNumDummies << " dummies.", 7);

    return true;
}
//HO

/*HV*/ void
GenericNodeMemWrapper::
/*HV*/ resetDualNode(GenericNodeId GID)
//HO ;
{
    //-----
    // reset the edges for a dualnode
    //
    // Its a little more difficult for DualNodes because we have to deal with dummies
    // Also Generic Nodes need to be reset.
    //
    PARANOID_ASSERT_L2(!isDummy(GID));

    // reset the DualNode edges
    mDualNodes->resetEdges(getDualNode(GID));

    // set the type to detached
    mDualNodes->setNodeType(DN_NODE_TYPE_DETACHED, getDualNode(GID));
    transferNode(GID, DETACHEDNODES_DN);
    
    // reset the GenericNode context flags
    clearCntxInfo(GID);
}
//HO 

/*HV*/ void
GenericNodeMemWrapper::
/*HV*/ resetDualNode(NB_ELEM * nbe)
//HO ;
{
    //-----
    // reset the edges for a dualnode
    //
    // Its a little more difficult for DualNodes because we have to deal with dummies
    // Also Generic Nodes need to be reset.
    //
    GenericNodeId GID = nbe->NBE_CurrentNode;
    PARANOID_ASSERT_L2(!isDummy(GID));

    // reset the DualNode edges
    mDualNodes->resetEdges(getDualNode(GID));
    if(0 != mDualNodes->getNtRank(getDualNode(GID)))
        logError("Non-zero rank after reset: " << mDualNodes->getNtRank(getDualNode(GID)));

    // set the type to detached
    mDualNodes->setNodeType(DN_NODE_TYPE_DETACHED, getDualNode(GID));
    transferNode(nbe, DETACHEDNODES_DN);

    // reset the GenericNode context flags
    clearCntxInfo(GID);
}
//HO 

    /*
    ** Reset the contigId for a contexts' nodes
    */
/*HV*/ bool
GenericNodeMemWrapper::
/*HV*/ resetDNContigs(ContextId CTXID)
//HO ;
{
    //-----
    // reset all the contigIds
    //
    int num_reset = 0;
    NB_ELEM nbe;
    getCTXBlock_DN(&nbe, CTXID);
    while(nextNBElem(&nbe))
    {
        num_reset++;
        mDualNodes->clearContig(getDualNode(nbe.NBE_CurrentNode));
    }
    logInfo("Reset " << num_reset << " DualNodes' ContigIds", 6);
    return true;
}
//HO 

/******************************************************************************
** MOVE NODES BETWEEN CONTEXTS
******************************************************************************/
    static std::vector<GenericNodeId> GNG_slaves;
    static vector<GenericNodeId>::iterator GNG_slave_iter;
/*HV*/ GenericNodeId
GenericNodeMemWrapper::
/*HV*/ moveNode(GenericNodeId masterGID, GenericNodeId GID)
//HO ;
{
    //-----
    // Called when we are merging contexts
    // Set the ContextId and position and make a slave edge
    // between this node and the given master.
    //
    // return the node which is master at this position now
    //
    GenericNodeId ret_GID = masterGID;
    if(isDummy(GID))
    {
        // we won't move a dummy.
        deleteDummyNode(GID);
    }
    else
    {
        ContextId CTXID = getCntxId(masterGID);
        uMDInt pos = getCntxPos(masterGID);
        bool reversed = isCntxReversed(masterGID);
        uMDInt hist = getCntxHistory(masterGID);

        // get a list of all the slaves
        GN_EDGE_ELEM gnee(DUALNODE);
        GenericNodeId tmp_slave;
        GNG_slaves.clear();
        if(getEdges(&gnee, GN_EDGE_STATE_TRANS, 0, GID))
        {
            do {
                tmp_slave = getOlapNode(gnee);
                transferNode(tmp_slave, DETACHEDNODES_DN);
                mDualNodes->resetEdges(getDualNode(tmp_slave));
                GNG_slaves.push_back(tmp_slave);
            } while(getNextEdge(&gnee));
        }

        // remove this guy from any context lists it may be in
        // but keep it in the detached list
        transferNode(GID, DETACHEDNODES_DN);
        
        // reset the DualNode edges
        mDualNodes->resetEdges(getDualNode(GID));

        // set all the new positonal markers
        setCntxId(CTXID, GID);
        setCntxPos(pos, GID);
        setCntxMaster(false, GID);
        setCntxReversed(reversed, GID);
        setCntxHistory(hist, GID);

        // make the slave edge
        if(isDummy(masterGID))
        {
            ret_GID = GID;
            assimilateDummyNode(masterGID, GID);
            curateMasterState(GID);
            
            GNG_slave_iter = GNG_slaves.begin();
            while(GNG_slave_iter != GNG_slaves.end())
            {
                mDualNodes->addSlaveEdgeBetween(getDualNode(*GNG_slave_iter), getDualNode(GID));
                setCntxId(CTXID, *GNG_slave_iter);
                setCntxPos(pos, *GNG_slave_iter);
                setCntxMaster(false, *GNG_slave_iter);
                setCntxReversed(reversed, *GNG_slave_iter);
                setCntxHistory(hist, *GNG_slave_iter);
                curateSlaveState(*GNG_slave_iter);
                incCntxReadDepth(GID);
                GNG_slave_iter++;
            }
        }
        else
        {
            mDualNodes->addSlaveEdgeBetween(getDualNode(masterGID), getDualNode(GID));
            curateSlaveState(GID);
            incCntxReadDepth(masterGID);

            GNG_slave_iter = GNG_slaves.begin();
            while(GNG_slave_iter != GNG_slaves.end())
            {
                mDualNodes->addSlaveEdgeBetween(getDualNode(*GNG_slave_iter), getDualNode(masterGID));
                setCntxId(CTXID, *GNG_slave_iter);
                setCntxPos(pos, *GNG_slave_iter);
                setCntxMaster(false, *GNG_slave_iter);
                setCntxReversed(reversed, *GNG_slave_iter);
                setCntxHistory(hist, *GNG_slave_iter);
                curateSlaveState(*GNG_slave_iter);
                incCntxReadDepth(masterGID);
                GNG_slave_iter++;
            }
        }
    }
    return ret_GID;
}
//HO 

    static std::vector<GenericNodeId> GNG_update_list;
    static std::vector<GenericNodeId>::iterator GNG_update_iter;
/*HV*/ void
GenericNodeMemWrapper::
/*HV*/ translateNode(ContextId CTXID, uMDInt pos, uMDInt history, bool revRev, GenericNodeId GID)
//HO ;
{
    //-----
    // Called when we are merging contexts
    // We don;t need to change anything except for the 
    // ContextId, the position, orientation and the history
    //
    
    
    GNG_update_list.clear();
    GNG_update_list.push_back(GID);

    // add in all the slaves
    GN_EDGE_ELEM gnee(DUALNODE);
    if(getEdges(&gnee, GN_EDGE_STATE_TRANS, 0, GID))
    {
        do {
            GNG_update_list.push_back(getOlapNode(gnee));
        } while(getNextEdge(&gnee));
    }

    // now update the nodes
    GNG_update_iter = GNG_update_list.begin();
    while(GNG_update_iter != GNG_update_list.end())
    {
        // remove it from any context lists
        transferNode(*GNG_update_iter, DETACHEDNODES_DN);

        setCntxId(CTXID, *GNG_update_iter);
        setCntxPos(pos, *GNG_update_iter);
        setCntxHistory(history, *GNG_update_iter);
        if(revRev)
            setCntxReversed(!isCntxReversed(*GNG_update_iter), *GNG_update_iter);

        // update the lists
        if(isCntxMaster(*GNG_update_iter))
        {
            curateMasterState(*GNG_update_iter);
        }
        else
        {
            curateSlaveState(*GNG_update_iter);
        }
        GNG_update_iter++;
    }
}
//HO 

/******************************************************************************
** MANAGE NODE BLOCKS
******************************************************************************/
/*HV*/ void
GenericNodeMemWrapper::
/*HV*/ curateUniNodeBlocks(void)
//HO ;
{
    //-----
    // You should only need to call this once for each node type
    // From there all operations are responsible for taking care of these
    // lists themselves
    //
    NB_ELEM nbe;
    getNonBanishedBlock_UN(&nbe);
    while(nextNBElem(&nbe))
    {
        UniNodeId current_UN = getUniNode(nbe.NBE_CurrentNode);
        switch(mUniNodes->getNtRank(current_UN))
        {
            case 2:
                mUniNodes->setNodeType(UN_NODE_TYPE_PATH, current_UN);
                transferNode(&nbe, PATHNODES_UN);
                break;
            case 0:
                mUniNodes->setNodeType(UN_NODE_TYPE_DETACHED, current_UN);
                transferNode(&nbe, DETACHEDNODES_UN);
                break;
            case 1:
                mUniNodes->setNodeType(UN_NODE_TYPE_CAP, current_UN);
                transferNode(&nbe, CAPNODES_UN);
                break;
            default:
                mUniNodes->setNodeType(UN_NODE_TYPE_CROSS, current_UN);
                transferNode(&nbe, CROSSNODES_UN);
                break;
        }
    }
}
//HO 

/*HV*/ void
GenericNodeMemWrapper::
/*HV*/ curateMasterState(GenericNodeId GID)
//HO ;
{
    //-----
    // Fixup which list this guy is in. Call only on a DualNode
    //
    DualNodeId DID = getDualNode(GID);
    ContextId CTXID = getCntxId(GID);
    GenericNodeId block_GID;
    switch(mDualNodes->getNtRank(DID))
    {
        case 0:
            mDualNodes->setNodeType(DN_NODE_TYPE_DETACHED, DID);
            block_GID = getCTXBlockNode_DN(CTXID);
            transferNode(GID, getCTXBlockNode_DN(getCntxId(GID)));
            break;
        case 1:
            mDualNodes->setNodeType(DN_NODE_TYPE_CAP, DID);
            block_GID = getCapBlockNode_DN(CTXID);
            transferNode(GID, block_GID);
            break;
        case 2:
            mDualNodes->setNodeType(DN_NODE_TYPE_PATH, DID);
            block_GID = getPathBlockNode_DN(CTXID);
            transferNode(GID, block_GID);
            break;
        default:
            mDualNodes->setNodeType(DN_NODE_TYPE_CROSS, DID);
            block_GID = getCrossBlockNode_DN(CTXID);
            transferNode(GID, block_GID);
            break;
    }
}
//HO 

/*HV*/ void
GenericNodeMemWrapper::
/*HV*/ curateSlaveState(GenericNodeId GID)
//HO ;
{
    //-----
    // Fixup which list this guy is in. Call only on a DualNode
    //
    mDualNodes->setNodeType(DN_NODE_TYPE_ATTACHED, getDualNode(GID));
    transferNode(GID, getCTXBlockNode_DN(getCntxId(GID)));
}
//HO 

/*HV*/ void
GenericNodeMemWrapper::
/*HV*/ addMasterToContext(GenericNodeId GID)
//HO ;
{
    //-----
    // Fixup which list this guy is in. Call only on a DualNode
    //
    transferNode(GID, getCTXBlockNode_DN(getCntxId(GID)));
}
//HO 

    /*
    ** Return a pointer to the capNodes
    */
//HV GenericNodeId getCapBlock(NODE_CLASS nc)
//HO { return mCapBlock_UN; switch(nc) { case UNINODE: { return mCapBlock_UN; break; } case DUALNODE: { /* return mCapBlock_DN; */ break; } } return GN_NULL_ID; }
//HO 

    /*
    ** Return a pointer to the crossNodes
    */
//HV GenericNodeId getCrossBlock(NODE_CLASS nc)
//HO { return mCrossBlock_UN; switch(nc) { case UNINODE: { return mCrossBlock_UN; break; }  case DUALNODE: { /* return mCrossBlock_DN; */ break; } } return GN_NULL_ID; }
//HO 

    /*
    ** Return a pointer to the attachedNodes
    */
//HV GenericNodeId getAttachedBlock(NODE_CLASS nc)
//HO { switch(nc) { case UNINODE: { return mAttachedBlock_UN; break; } case DUALNODE: { return mAttachedBlock_DN; break; } } return GN_NULL_ID; }
//HO 

    /*
    ** Return a pointer to the detachedNodes
    */
//HV GenericNodeId getDetachedBlock(NODE_CLASS nc)
//HO { switch(nc) { case UNINODE: { return mDetachedBlock_UN; break; } case DUALNODE: { return mDetachedBlock_DN; break; } } return GN_NULL_ID; }
//HO 

    /*
    ** Return a pointer to the nonBanishedNodes
    */
//HV GenericNodeId getNonBanishedBlock(NODE_CLASS nc)
//HO { switch(nc) { case UNINODE: { return mNonBanishedBlock_UN; break; } case DUALNODE: { return mNonBanishedBlock_DN; break; } } return GN_NULL_ID; }
//HO 

    /*
    ** Return a pointer to the nonBanishedNodes
    */
//HV GenericNodeId getBanishedBlock(NODE_CLASS nc)
//HO { switch(nc) { case UNINODE: { return mBanishedBlock_UN; break; } case DUALNODE: { return mBanishedBlock_DN; break; } } return GN_NULL_ID; }
//HO 

/******************************************************************************
** BANISHING
******************************************************************************/
/*
    ** Banish a node and all its edges
    */
/*HV*/ bool
GenericNodeMemWrapper::
/*HV*/ banishHead(NB_ELEM * nbe)
//HO ;
//VO = 0;
{
    //-----
    // Banish a node and all edges coming to and from it
    // We are banishig via the uninode so we need to delete all the GIDs in the chain
    //
    GenericNodeId GID = nbe->NBE_CurrentNode;
    UniNodeId UID = getUniNode(GID);

    GN_ELEM tmp_elem;
    if(getElem(&tmp_elem, UID))
    {
        // tmp_elem holds the "HEAD"
        while(getNextElem(&tmp_elem))
        {
            // now tmp_elem holds a Regular DualNode
            DualNodeId first_DID = getDualNode(tmp_elem.GNE_GID);
            GenericNodeId first_GID = tmp_elem.GNE_GID;
            PARANOID_ASSERT_PRINT_L2((GN_NULL_ID != first_GID), "About to banish NULL: " << first_DID << " : " << tmp_elem.GNE_GID << " : " << mUniNodes->getGn(UID) << " : " << isHead(mUniNodes->getGn(UID)));
            if(!isDummy(first_GID))
            {
                // we need to delete the pair too
                DualNodeId pair_DID = mDualNodes->getPair(first_DID);
                GenericNodeId pair_GID = mDualNodes->getGn(pair_DID);

                setBanished(true, pair_GID);
                mDualNodes->banishNode(pair_DID);
                transferNode(pair_GID, BANISHEDNODES_DN);
                mDualNodes->setNodeType(DN_NODE_TYPE_BANISHED, pair_DID);
            }
            setBanished(true, first_GID);
            mDualNodes->banishNode(first_DID);
            transferNode(first_GID, BANISHEDNODES_DN);
            mDualNodes->setNodeType(DN_NODE_TYPE_BANISHED, first_DID);
        }
    }
    else
    {
        logError(UID << " not found");
        return false;
    }

    // first transfer this guy to the banished list
    transferNode(nbe, BANISHEDNODES_UN);
    
    // finally banish this guy in the uninode store
    mUniNodes->banishNode(UID);

    // done!
    return true;
}
//HO 

/*HV*/ bool
GenericNodeMemWrapper::
/*HV*/ banishUniNode(UniNodeId UID)
//HO ;
//VO = 0;
{
    //-----
    // Banish the uninode and fix up the uninode specific lists and holders
    //

    // banish it!
    // node-type is set within this call
    mUniNodes->banishNode(UID);

    // fix all the lists up
    transferNode(mUniNodes->getGn(UID), BANISHEDNODES_UN);
    return true;
}
//HO 

/******************************************************************************
** SIMPLE FILTERING
******************************************************************************/
/*HV*/ int
GenericNodeMemWrapper::
/*HV*/ findAllDupes(bool deleteDupes)
//HO ;
{
    //-----
    // go through all the blocks and mark any duplicates
    //
    // first mark them
    logInfo("Marking duplicate GenericNode pairs", 1);
    int num_dupes = 0;

    NB_ELEM nbe;

    getNonBanishedBlockOnly_UN(&nbe);
    
    while(nextNBElem(&nbe))
    {
        UniNodeId current_UID = getUniNode(nbe.NBE_CurrentNode);
        if(mUniNodes->getReadDepth(current_UID) > 1)
        {
            // get the head node, call it the stop node
            GenericNodeId stop_node_GN = getHead(nbe.NBE_CurrentNode);
            // we start comparing from the node just after this node
            GenericNodeId current_base_compare_GN = getNextElem(stop_node_GN);
            GenericNodeId current_query_GN;
            do {
                if(!isDupe(current_base_compare_GN)) // no need to compare dupes
                {
                    // this is the paired UniNode of the base generic node
                    // not that one exists but you get the picture
                    UniNodeId current_base_pair_UN = getUniNode(getPair(current_base_compare_GN));
                    current_query_GN = getNextElem(current_base_compare_GN);
                    while(current_query_GN != stop_node_GN) {
                        if(!isDupe(current_query_GN))
                        {
                            UniNodeId current_query_pair_UN = getUniNode(getPair(current_query_GN));
                            if(current_base_pair_UN == current_query_pair_UN)
                            {
                                // then we have found a dupe
                                setDupe(true, current_query_GN);
                                setDupe(true, getPair(current_query_GN));
                                num_dupes++;
                            }

                        }
                        // move the query node along one place
                        current_query_GN = getNextElem(current_query_GN);
                    }
                }
                // move the base comparison node along one place
                current_base_compare_GN = getNextElem(current_base_compare_GN);
            } while(current_base_compare_GN != stop_node_GN);
        }
    }

    // then delete them (if necessary)
    if(deleteDupes && 0 != num_dupes) {
        logInfo("Deleting GenericNode pairs", 1);
        logInfo("Need to Delete " << num_dupes << " GenericNodes", 2);

        getNonBanishedBlock_DN(&nbe);
        while(nextNBElem(&nbe))
        {
            if(isDupe(nbe.NBE_CurrentNode))
            {
                // delete this guy first
                deleteDualNode(&nbe);
            }
        }
    }
    logInfo("Done.", 2);
    return num_dupes;
}

/*HV*/ bool
GenericNodeMemWrapper::
/*HV*/ shouldBeBanished(UniNodeId UID)
//HO ;
{
    //-----
    // returns true if a node should be banished
    //
    if(mUniNodes->getNodeType(UID) == UN_NODE_TYPE_BANISHED)
        return false;
    if((mUniNodes->getTRank(UID)) < UN_BANISH_CUT)
        return true;                                                // sparse so we banish
    return false;
}
//HO 

/*HV*/ bool
GenericNodeMemWrapper::
/*HV*/ banishSparseFloatingUniNodes(void)
//HO ;
{
    //-----
    // Any node with only one neighbour (or even only two)
    // really must be some kind of error. so we'll just banish it...
    //
    // also, any node with less than 2 banished neighbours should also be banished
    //
    // run through all the nodes and check for this...
    //

    logInfo("Start banishing sparse and floating UniNodes", 2);

    NB_ELEM nbe;

    int banned_counter = 0;                                         // the number we have banished
    int keeping_counter = 0;                                        // the number we'll keep
    int tot_counter = 0;
    bool some_banished = true;
    while(some_banished)                                            // banishing begets banishing, keep going until it all stabilises
    {
        some_banished = false;
        getNonBanishedBlockOnly_UN(&nbe);
        
        while(nextNBElem(&nbe))
        {
            if(shouldBeBanished(getUniNode(nbe.NBE_CurrentNode)))
            {
                some_banished = true;
                banishHead(&nbe);
                banned_counter++;
            }
            else
            {
                keeping_counter++;
            }
            tot_counter++;
        }
        logInfo("Banished: " << banned_counter << " UniNodes this round", 4);
        logInfo("Keeping: " << keeping_counter << + " UniNodes", 4);
        logInfo("Total: " << tot_counter << + " UniNodes", 4);

        banned_counter = 0;
        keeping_counter = 0;
        tot_counter = 0;
    }

    logInfo("Done banishing UniNodes", 3);

    return true;
}
//HO 

/******************************************************************************
** OUTPUT AND PRINTING
******************************************************************************/
/*HV*/ void
GenericNodeMemWrapper::
/*HV*/ printAllEdges(NODE_CLASS nc, bool showBanished)
//HO ;
{
    //-----
    // Print the edges for a particular node type
    //
    switch(nc)
    {
        case UNINODE:
        {
            NB_ELEM nbe;
            getNonBanishedBlock_UN(&nbe);
            while(nextNBElem(&nbe))
            {
                printEdges(UNINODE, nbe.NBE_CurrentNode);
            }
            break;
        }
        case DUALNODE:
        {
            logWarn("call has no effect", 1);
            break;
        }
        default:
            logError("Invalid node class specified");
    }
}
//HO 

/*HV*/ void
GenericNodeMemWrapper::
/*HV*/ printAllEdges(ContextId CTXID, bool showBanished)
//HO ;
{
    //-----
    // Print all the edges for a particular context
    //
    NB_ELEM nbe;
    getCTXBlock_DN(&nbe, CTXID);
    while(nextNBElem(&nbe))
    {
        printEdges(DUALNODE, nbe.NBE_CurrentNode);
    }
}
//HO 


/*HV*/ void
GenericNodeMemWrapper::
/*HV*/ printEdges(NODE_CLASS nc, GenericNodeId GID)
//HO ;
{
    //-----
    // Print the edges for a particular node type
    //
    switch(nc)
    {
        case UNINODE:
        {
            std::cout << "UN PERSPECTIVE - " << GID << " ReadDepth: " << mUniNodes->getReadDepth(getUniNode(GID)) << std::endl;
            mUniNodes->printEdges(getUniNode(GID));
            printLoop(true, getUniNode(GID));
            break;
        }
        case DUALNODE:
        {
            string ms = "Unset ";
            if(mDualNodes->getNtRank(getDualNode(GID)) != 0)
            {
                if(isCntxMaster(GID))
                    ms = "Master ";
                else
                    ms = "Slave ";
            }
            string du = "Regular ";
            if(isDummy(GID))
                du = "Dummy ";

            std::cout << "DN PERSPECTIVE - " << du << ms << GID << " -- ReadDepth: " << mDualNodes->getReadDepth(getDualNode(GID)) << " -- [" << getCntxId(GID) << " : " << getCntxPos(GID) << " : " << isCntxReversed(GID) << "]" << std::endl;
            std::cout << getSequence(GID) << std::endl;
            mDualNodes->printEdges(getDualNode(GID));
            break;
        }
        default:
            logError("Invalid node class specified");
    }
}
//HO 

/*HV*/ void
GenericNodeMemWrapper::
/*HV*/ printLocalGraph(NODE_CLASS nc, GenericNodeId GID)
//HO ;
{
    //-----
    // Print the edges for a particular node type
    //
    switch(nc)
    {
        case UNINODE:
        {
            break;
        }
        case DUALNODE:
        {
            mDualNodes->printLocalGraph(getDualNode(GID));
            break;
        }
        default:
            logError("Invalid node class specified");
    }
}
//HO 

/*HV*/ void
GenericNodeMemWrapper::
/*HV*/ printLoop(bool all, UniNodeId UID)
//HO ;
{
    //-----
    // Print the entire Loop for a UniNodeId
    //
    GN_ELEM data;
    if(getElem(&data, UID))
    {
        do {
            if(all)
                data.printContents(1);
            else
                data.printContents();
        } while(getNextElem(&data));
        std::cout << "---" << std::endl;
    }
    else
    {
        std::cout << "Empty Loop for " << UID << "\n---" << std::endl;
    }
}
//HO 

/*HV*/ void
GenericNodeMemWrapper::
/*HV*/ logLoop(int logLevel, GenericNodeId GID)
//HO ;
{
    //-----
    // Print the entire Loop for a UniNodeId
    //
    GN_ELEM data;
    UniNodeId UID = getUniNode(GID);
    if(getElem(&data, UID))
    {
        do {
            std::ostringstream SS;
            data.printContents(&SS);
            logInfo(SS.str(), logLevel);
        } while(getNextElem(&data));
        logInfo("---" << mUniNodes->getContig(UID), logLevel);
    }
    else
    {
        logInfo("Empty Loop for " << UID << "\n---", logLevel);
    }
}
//HO 

/*HV*/ std::string
GenericNodeMemWrapper::
/*HV*/ getSequence(GenericNodeId GID)
//HO ;
{
    //-----
    // Return the uninodes sequence
    //
    PARANOID_ASSERT_PRINT_L2((mReads->isValidReadStoreId(mUniNodes->getRs(getUniNode(GID)))), "Invalid RS for : " << GID)
            return mUniNodes->getSequence(getUniNode(GID));
}
//HO 

#if __USE_RI
/*HV*/ std::string
GenericNodeMemWrapper::
/*HV*/ getReadId(GenericNodeId GID)
//HO ;
{
    //-----
    // Return the dualnodes readID
    //
    return mDualNodes->getReadIdString(getDualNode(GID));
}
//HO 
#endif

/*HV*/ std::string
GenericNodeMemWrapper::
/*HV*/ sayNodeBlockTypeLikeAHuman(NB_TYPE nbt)
//HO ;
{
    //-----
    // 
    //
    stringstream ss;
    switch(nbt)
    {
        case UNINODES:
        {
            ss << mUniNodeBlock;
            break;
        }
        case DUALNODES:
        {
            ss << mDualNodeBlock;
            break;
        }
        case BANISHEDNODES_UN:
        {
            ss << mBanishedBlock_UN;
            break;
        }
        case NONBANISHEDNODES_UN:
        {
            ss << mNonBanishedBlock_UN;
            break;
        }
        case ATTACHEDNODES_UN:
        {
            ss << mAttachedBlock_UN;
            break;
        }
        case DETACHEDNODES_UN:
        {
            ss << mDetachedBlock_UN;
            break;
        }
        case CAPNODES_UN:
        {
            ss << mCapBlock_UN;
            break;
        }
        case PATHNODES_UN:
        {
            ss << mPathBlock_UN;
            break;
        }
        case CROSSNODES_UN:
        {
            ss << mCrossBlock_UN;
            break;
        }
        case BANISHEDNODES_DN:
        {
            ss << mBanishedBlock_DN;
            break;
        }
        case NONBANISHEDNODES_DN:
        {
            ss << mNonBanishedBlock_DN;
            break;
        }
        case ATTACHEDNODES_DN:
        {
            ss << mAttachedBlock_DN;
            break;
        }
        case DETACHEDNODES_DN:
        {
            ss << mDetachedBlock_DN;
            break;
        }
        default:
        {
            logError("Invalid NB_TYPE");
            return "";
        }
    }
    std::string ret_str = ss.str();
    return ret_str;
}
//HO 

/*HV*/ bool
GenericNodeMemWrapper::
/*HV*/ getCrossDistributions(std::vector<int> * distVec, ContextId * CTXID, NODE_CLASS nc)
//HO ;
{
    //-----
    // for examining how the corss node ranks are distributed...
    //
    distVec->clear();
    for(uMDInt i = 0; i < 4 * mExtOffset; i++)
    {
        distVec->push_back(0);
    }
    NB_ELEM nbe;
    std::vector<int>::iterator dist_vec_iter = distVec->begin();
    switch(nc)
    {
        case UNINODE:
            getCrossBlock_UN(&nbe);
            while(nextNBElem(&nbe))
            {
                (*(dist_vec_iter + mUniNodes->getNtRank(getUniNode(nbe.NBE_CurrentNode))))++;
            }
            break;
        case DUALNODE:
            getCrossBlock_DN(&nbe, *CTXID);
            while(nextNBElem(&nbe))
            {
                (*(dist_vec_iter + mDualNodes->getNtRank(getDualNode(nbe.NBE_CurrentNode))))++;
            }
            break;
    }
    return true;
}
//HO 

