/******************************************************************************
**
** File: DualNodeMemWrapper.cpp
**
*******************************************************************************
**
** This file implments many of the low level functions for the class which
** handles paired end data. It is very similar to the UniNode class
** Both this file and the UniNode class files must present a very specific
** interface to the GenericNode class.
**
** This class is responsible for handling all the paired connections and graph
** edges when we are considering a paired-read node graph
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
/*INC*/ #include <map>
/*INC*/ 
/*INC*/ class GenericNodeClass;
/*INC*/ 

// local includes
#include "DualNodeMemWrapper.h"
#include "LoggerSimp.h"
#include "default_parameters.h"
#include "IdTypeSTL_ext.h"
#include "IdTypeDefs.h"

//INC #include "intdef.h"
//INC 
#include "intdef.h"

//INC #include "UniNodeClass.h"
//INC 
#include "UniNodeClass.h"

#if __USE_RI
//INC #include "ReadIdentifierClass.h"
//INC 
#include "ReadIdentifierClass.h"
#endif

//INC #include "IdTypeStructs.h"
//INC 
#include "IdTypeStructs.h"

//INC class DN_EDGE_ELEM {
//INC 
//INC     public:
//INC 
//INC         DN_EDGE_ELEM() { DNEE_WhichData = DualNodeDataId(); DNEE_WhichOlap = 0; }
//INC 
//INC         // to get some insight
//INC 
//INC         void printContents(void)
//INC 
//INC         {
//INC 
//INC             std::cout << "Base node: " << DNEE_BaseNode << std::dec << " Offset wanted: " << DNEE_OffsetWanted << " State Wanted: " << DNEE_StateWanted << std::endl;
//INC             std::cout << "Overlapping node: " << DNEE_OlapNode << std::dec << " Offset: " << DNEE_Offset << " ReturnOffset: " << DNEE_ReturnOffset << " State: " << DNEE_State << std::endl;
//INC 
//INC         }
//INC 
//INC         void printContents(int i)
//INC 
//INC         {
//INC 
//INC             std::cout << "Base node: " << DNEE_BaseNode << std::dec << " Offset wanted: " << DNEE_OffsetWanted << " State Wanted: " << DNEE_StateWanted << std::endl;
//INC             std::cout << "Overlapping node: " << DNEE_OlapNode << " Offset: " << DNEE_Offset << " State: " << DNEE_State << std::endl;
//INC             std::cout << "Next hit: " << DNEE_WhichData << " WhichOlap: " << DNEE_WhichOlap << std::endl;
//INC 
//INC         }
//INC 
//INC         DualNodeId DNEE_OlapNode;
//INC 
//INC         sMDInt DNEE_Offset;
//INC 
//INC         sMDInt DNEE_ReturnOffset;
//INC 
//INC         uMDInt DNEE_State;
//INC 
//INC         DualNodeId DNEE_BaseNode;
//INC 
//INC         sMDInt DNEE_OffsetWanted;
//INC 
//INC         uMDInt DNEE_StateWanted;
//INC 
//INC     protected:
//INC 
//INC         DualNodeDataId DNEE_WhichData;
//INC 
//INC         uMDInt DNEE_WhichOlap;
//INC 
//INC         friend class DualNodeMemWrapper;
//INC 
//INC         friend class DualNodeClass;
//INC 
//INC };
//INC 

// We need to have another element which is identical to the above but needs a little relabelling
// Behold! The walking element.

//INC #define DNWE_PrevNode DNEE_OlapNode
//INC 
//INC #define DNWE_CurrentNode DNEE_BaseNode
//INC 
//INC #define DNWE_OffsetWanted DNEE_OffsetWanted
//INC 
//INC #define DNWE_P2COffset DNEE_ReturnOffset
//INC 
//INC #define DNWE_C2POffset DNEE_Offset
//INC 
//INC #define DNWE_StateWanted DNEE_StateWanted
//INC 
//INC #define DNWE_State DNEE_State
//INC 
//INC #define DNWE_WhichData DNEE_WhichData
//INC 
//INC #define DNWE_WhichOlap DNEE_WhichOlap
//INC 
//INC class DN_WALKING_ELEM : public DN_EDGE_ELEM {
//INC 
//INC     public:
//INC 
//INC         void printContents(void)
//INC 
//INC         {
//INC 
//INC             std::cout << "Current node: " << DNWE_CurrentNode << std::dec << " Offset wanted: " << DNWE_OffsetWanted << " State Wanted: " << DNWE_StateWanted << std::endl;
//INC             std::cout << "Previous node: " << DNWE_PrevNode << std::dec << " P2COffset: " << DNWE_P2COffset << " State: " << DNWE_State << std::endl;
//INC 
//INC         }
//INC 
//INC         void printContents(int i)
//INC 
//INC         {
//INC 
//INC             std::cout << "Previous node: " << DNWE_PrevNode << std::dec << " C2POffset: " << DNWE_C2POffset << " P2COffset: " << DNWE_P2COffset << " State: " << DNWE_State << std::endl;
//INC             std::cout << "Current node: " << DNWE_CurrentNode << std::dec << " Offset wanted: " << DNWE_OffsetWanted << " State Wanted: " << DNWE_StateWanted << std::endl;
//INC             std::cout << "Next hit: " << DNWE_WhichData << " WhichOlap: " << DNWE_WhichOlap << std::endl;
//INC 
//INC         }
//INC 
//INC         void logContents(int i)
//INC 
//INC         {
//INC 
//INC             logInfo("Current node: " << DNWE_CurrentNode << std::dec << " Offset wanted: " << DNWE_OffsetWanted << " State Wanted: " << DNWE_StateWanted , i);
//INC             logInfo("Previous node: " << DNWE_PrevNode << std::dec << " P2COffset: " << DNWE_P2COffset << " State: " << DNWE_State , i);
//INC             logInfo("Next hit: " << DNWE_WhichData << " WhichOlap: " << DNWE_WhichOlap , i);
//INC 
//INC         }
//INC };
//INC 

    /*
    ** We need the edge states and node types to match up perfectly
    ** Generic node does the same thing.
    */ 

//HO #define DN_EDGE_STATE_DETACHED UN_EDGE_STATE_DETACHED
//HO 
//HO #define DN_EDGE_STATE_MASTER UN_EDGE_STATE_NTRANS
//HO 
//HO #define DN_EDGE_STATE_SLAVE UN_EDGE_STATE_TRANS
//HO 
//HO #define DN_EDGE_STATE_BANISHED UN_EDGE_STATE_BANISHED
//HO 
//HO #define DN_NODE_TYPE_UNSET UN_NODE_TYPE_UNSET
//HO 
//HO #define DN_NODE_TYPE_CAP UN_NODE_TYPE_CAP
//HO 
//HO #define DN_NODE_TYPE_PATH UN_NODE_TYPE_PATH
//HO 
//HO #define DN_NODE_TYPE_CROSS UN_NODE_TYPE_CROSS
//HO 
//HO #define DN_NODE_TYPE_ATTACHED UN_NODE_TYPE_ATTACHED
//HO 
//HO #define DN_NODE_TYPE_DETACHED UN_NODE_TYPE_DETACHED
//HO 
//HO #define DN_NODE_TYPE_BANISHED UN_NODE_TYPE_BANISHED
//HO 
//HO #define DN_NODE_TYPE_CORRECTED UN_NODE_TYPE_CORRECTED
//HO 

/*INC*/ typedef uMDInt EDGE_STATE;
/*INC*/
//HO #define DN_BLOCK_STATE_WIDTH UN_BLOCK_STATE_WIDTH
//HO 
//HO #define DN_BLOCK_STATE_HEIGHT UN_BLOCK_STATE_HEIGHT
//HO 
//HO #define DN_BLOCK_STATE_MASTER_COUNT UN_BLOCK_STATE_NT_COUNT
//HO 
//HO #define DN_BLOCK_STATE_SLAVE_COUNT UN_BLOCK_STATE_T_COUNT
//HO 
//HO #define DN_BLOCK_STATE_D_COUNT UN_BLOCK_STATE_D_COUNT
//HO 
//HO #define DN_BLOCK_STATE_B_COUNT UN_BLOCK_STATE_B_COUNT
//HO 
//HO #define DN_BLOCK_STATE_TOTAL_EDGE_COUNT UN_BLOCK_STATE_TOTAL_EDGE_COUNT
//HO 
//HO #define DN_BLOCK_STATE_ADD_SLAVE UN_BLOCK_STATE_ADD_T
//HO 
//HO #define DN_BLOCK_STATE_SLAVE_TO_MASTER UN_BLOCK_STATE_T_TO_NT
//HO 
//HO #define DN_BLOCK_STATE_SLAVE_TO_D UN_BLOCK_STATE_T_TO_D
//HO 
//HO #define DN_BLOCK_STATE_SLAVE_TO_B UN_BLOCK_STATE_T_TO_B
//HO 
//HO #define DN_BLOCK_STATE_MASTER_TO_D UN_BLOCK_STATE_NT_TO_D
//HO 
//HO #define DN_BLOCK_STATE_MASTER_TO_B UN_BLOCK_STATE_NT_TO_B
//HO 
//HO #define DN_BLOCK_STATE_D_TO_MASTER UN_BLOCK_STATE_D_TO_NT
//HO 
//HO #define DN_BLOCK_STATE_D_TO_B UN_BLOCK_STATE_D_TO_B
//HO 
//HO #define DN_BLOCK_STATE_RESET_ALL UN_BLOCK_STATE_RESET_ALL
//HO 
//HO #define DN_BLOCK_STATE_ERROR UN_BLOCK_STATE_ERROR
//HO 
//HO #define DN_BLOCK_STATE_HEIGHT UN_BLOCK_STATE_HEIGHT
//HO 
//HO #define DN_BLOCK_STATE_WIDTH UN_BLOCK_STATE_WIDTH
//HO 
//PV uMDInt mEdgeStates[DN_BLOCK_STATE_HEIGHT][DN_BLOCK_STATE_WIDTH];  // finite state matrix for edge states
//PV 
//PV UniNodeClass * mUniNodes;
//PV
#if __USE_RI
//PV ReadIdentifierClass * mReadIdentifiers;
//PV
#endif
/******************************************************************************
** INTIALISATION AND SAVING
******************************************************************************/

/*HV*/ void
DualNodeMemWrapper::
/*HV*/ extendedDestructor(void)
//HO ;
{
    //-----
    // Add anything you would like to happen when the destructor is called
    //
}
//HO 

/*HV*/ bool
DualNodeMemWrapper::
/*HV*/ extendedInitialise(uMDInt originalNumberOfBlocks )
//HO ;
{
    //-----
    // Add anything you would like to happen on a regular intialisation call
    //
    mUniNodes = NULL;
#if __USE_RI
    mReadIdentifiers = NULL;
#endif
    // intialise the EDGE STATE MATRIX
    initEdgeStates();
    
    return true;
}
//HO 

/*HV*/ bool
DualNodeMemWrapper::
/*HV*/ extendedInitialise(std::string fileName)
//HO ;
{
    //-----
    // Add anything you would like to happen when reloading a saved instance
    //
    mUniNodes = NULL;
#if __USE_RI
    mReadIdentifiers = NULL;
#endif
    // intialise the EDGE STATE MATRIX
    initEdgeStates();
    return true;
}
//HO 

/*HV*/ bool
DualNodeMemWrapper::
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
DualNodeMemWrapper::
/*HV*/ setObjectPointers(UniNodeClass * UNC)
//HO ;
{
    //-----
    // This object may need access to other high level objects
    // the pointers are member variables and we can set them here
    // no pointers set here can be deleted
    //
////*HV*/ setObjectPointers(UniNodeClass * UNC, ReadIdentifierClass * RIC)

    // set the main ReadStore class
    PARANOID_ASSERT_L2(mUniNodes == NULL);
    mUniNodes = UNC;

#if __USE_RI
    PARANOID_ASSERT_L2(mReadIdentifiers == NULL);
    mReadIdentifiers = RIC;
#endif
}
//HO 

/******************************************************************************
** HOUSEKEEPING
******************************************************************************/
    
/*HV*/ DualNodeId
DualNodeMemWrapper::
/*HV*/ addNode(GenericNodeId node)
//HO ;
{
    //-----
    // add a new node to DualNodeStore
    //
    // get a new address
    DualNodeId new_id = newDualNodeId();

    // add the GniNodes
    setGn(node, new_id);

    // simple!
    return new_id;
}
//HO 
#if  __USE_RI
/*HV*/ std::string
DualNodeMemWrapper::
/*HV*/ getReadIdString(DualNodeId DID)
//HO ;
{
    // now build the id string
    stringstream ss;
    char pair_id;
    if(isPairId(DID))
        pair_id = '2';
    else
        pair_id = '1';
    
    ss << (mReadIdentifiers->getReadIdString(getReadIdentifier(DID))) << pair_id;
    return ss.str();
}
//HO
#endif

/******************************************************************************
** NAVIGATION
******************************************************************************/
    /*
    ** Initialise the iterator for a DualNodes overlapping edges
    */
/*HV*/ bool
DualNodeMemWrapper::
/*HV*/ getEdges(DN_EDGE_ELEM * data, uMDInt edgeState, sMDInt offSign, DualNodeId DID)
//HO ;
{
    //-----
    // Get the first olap at address which matches the criteria, if edgeState or offSign == 0
    // then we don't care and this puppy will start us on the road to return all the overlaps
    //

    // save these guys for a rainy day
    data->DNEE_BaseNode = DID;
    data->DNEE_StateWanted = edgeState;
    data->DNEE_OffsetWanted = offSign;

    // holders for the current edge
    sMDInt offset = 0;
    uMDInt state = 0;
    
    int num_used = 0;

    PARANOID_ASSERT_L2(isValidAddress(DID));
    DualNodeDataId data_block = getNextDataBlock(DID);
    PARANOID_ASSERT_L2(isValidAddress(data_block));

    while(DND_NULL_ID != data_block)
    {
        num_used = getNumUsedInDataBlock(data_block);
        PARANOID_ASSERT_L2(isValidAddress(data_block));
        PARANOID_ASSERT_L2((num_used > 0));
        for(int focused_olap = 1; focused_olap <= num_used; focused_olap++)
        {
            state = getXthState(focused_olap, data_block);
            if(0 == edgeState || state == edgeState)
            {
                // the state was right, so we can continue...
                offset = getXthOffset(focused_olap, data_block);
                if(0 == offSign || ((offset > 0) == (offSign > 0)))
                {
                    // the offset was good too, so we have found our guy...
                    data->DNEE_OlapNode = getXthEdge(focused_olap, data_block);
                    data->DNEE_State = state;
                    data->DNEE_Offset = offset;
                    if(getXthReturnOffset(focused_olap, data_block))
                    {
                        // return offset is a match
                        data->DNEE_ReturnOffset = offset;
                    }
                    else
                    {
                        // offset is a different sign
                        data->DNEE_ReturnOffset = offset * -1;
                    }

                    // set the reserved values up correctly...
                    data->DNEE_WhichData = data_block;
                    data->DNEE_WhichOlap = focused_olap;
            
                    // done!
                    return true;
                }
            }
        }
        data_block = getNextDataBlock(data_block);
    }
    // if we are here then we didn't find what we were looking for, return false.
    return false;
}
//HO 
    
    /*
    ** Get the next edge
    */
/*HV*/ bool
DualNodeMemWrapper::
/*HV*/ getNextEdge(DN_EDGE_ELEM * data)
//HO ;
{
    //-----
    // call this AFTER getEdges to get the next olap of the same type as specified in the
    // original call.
    //
    // First we have to finish off any existing block
    //

    // holders for the current edge
    sMDInt offset = 0;
    uMDInt state = 0;

    DualNodeDataId data_block = data->DNEE_WhichData;
    sMDInt w_off = data->DNEE_OffsetWanted;
    uMDInt w_state = data->DNEE_StateWanted;
    int focused_olap = data->DNEE_WhichOlap + 1;
    int num_used = getNumUsedInDataBlock(data_block);
    
    if(focused_olap > num_used)
    {
        // we have used all the places in this data block
        data_block = getNextDataBlock(data_block);
        focused_olap = 1;
    }
    
    while(DND_NULL_ID != data_block)
    {
        num_used = getNumUsedInDataBlock(data_block);
        PARANOID_ASSERT_L2((num_used > 0));
#ifdef MAKE_PARANOID
        if(num_used <= 0)
        {
            data->printContents(1);
        }
#endif
        while(focused_olap <= num_used)
        {
            state = getXthState(focused_olap, data_block);
            if(0 ==  w_state || state == w_state)
            {
                // the state was right, so we can continue...
                offset = getXthOffset(focused_olap, data_block);
                if(0 == w_off || ((offset > 0) == (w_off > 0)))
                {
                    // the offset was good too, so we have found our guy...
                    data->DNEE_OlapNode = getXthEdge(focused_olap, data_block);
                    data->DNEE_Offset = offset;
                    data->DNEE_State = state;
                    if(getXthReturnOffset(focused_olap, data_block))
                    {
                        // return offset is a match
                        data->DNEE_ReturnOffset = offset;
                    }
                    else
                    {
                        // offset is a different sign
                        data->DNEE_ReturnOffset = offset * -1;
                    }
                    
//                    PARANOID_ASSERT_L2((offset <= SAS_DEF_UN_OFFSET_MAX_MAX));
                    
                    // set the reserved values up correctly...
                    data->DNEE_WhichData = data_block;
                    data->DNEE_WhichOlap = focused_olap;
            
                    // done!
                    return true;
                }
            }
            focused_olap++;
        }
        data_block = getNextDataBlock(data_block);
        focused_olap = 1;
    }
    // if we are here then we didn't find what we were looking for, return false.
    return false;
}
//HO 

/******************************************************************************
** EDGE STATES GET AND SET
******************************************************************************/

    /*
    ** Get overlapping node Ids from a data block
    */
/*HV*/ DualNodeId
DualNodeMemWrapper::
/*HV*/ getXthEdge(int whichOlap, DualNodeDataId DDID)
//HO ;
{
    //-----
    // Return the Xth Overlapping node
    //
    PARANOID_ASSERT_L2((whichOlap <= DN_MAX_OLAPS_IN_DATA));
    PARANOID_ASSERT_L2((whichOlap > 0));
    switch(whichOlap)
    {
        case 1:
        {
            return getDn1_olap(DDID);
        }
        case 2:
        {
            return getDn2_olap(DDID);
        }
        case 3:
        {
            return getDn3_olap(DDID);
        }
        default:
        {
            return DN_NULL_ID;
        }
    }
}
//HO 

    /*
    ** Get edge offsets from a data block
    */
/*HV*/ sMDInt
DualNodeMemWrapper::
/*HV*/ getXthOffset(int whichOlap, DualNodeDataId DDID)
//HO ;
{
    //-----
    // Return the Xth Overlapping node's offset
    //
    PARANOID_ASSERT_L2((whichOlap <= DN_MAX_OLAPS_IN_DATA));
    PARANOID_ASSERT_L2((whichOlap > 0));
    switch(whichOlap)
    {
        case 1:
        {
            return getDn1_offset(DDID);
        }
        case 2:
        {
            return getDn2_offset(DDID);
        }
        case 3:
        {
            return getDn3_offset(DDID);
        }
        default:
        {
            PARANOID_INFO_L2("Return offset is bad news");
            return 0;
        }
    }
}
//HO 

    /*
    ** Get edge states from a data block
    */
/*HV*/ uMDInt
DualNodeMemWrapper::
/*HV*/ getXthState(int whichOlap, DualNodeDataId DDID)
//HO ;
{
    //-----
    // Return the Xth Overlapping node's edge state
    //
    PARANOID_ASSERT_L2((whichOlap <= DN_MAX_OLAPS_IN_DATA));
    PARANOID_ASSERT_L2((whichOlap > 0));
    switch(whichOlap)
    {
        case 1:
        {
            return getDn1_state(DDID);
        }
        case 2:
        {
            return getDn2_state(DDID);
        }
        case 3:
        {
            return getDn3_state(DDID);
        }
        default:
        {
            PARANOID_INFO_L2("Return state is bad news");
            return 0;
        }
    }
}
//HO 

    /*
    ** Get the flag which says if the return offset is the same sign
    */
/*HV*/ bool
DualNodeMemWrapper::
/*HV*/ getXthReturnOffset(int whichOlap, DualNodeDataId DDID)
//HO ;
{
    //-----
    // Return the Xth Overlapping node's edge state
    //
    PARANOID_ASSERT_L2((whichOlap <= DN_MAX_OLAPS_IN_DATA));
    PARANOID_ASSERT_L2((whichOlap > 0));
    switch(whichOlap)
    {
        case 1:
        {
            return isDn1_returnOffset(DDID);
        }
        case 2:
        {
            return isDn2_returnOffset(DDID);
        }
        case 3:
        {
            return isDn3_returnOffset(DDID);
        }
        default:
        {
            PARANOID_INFO_L2("Return offset is bad news");
            return -1;
        }
    }
}
//HO 

    /*
    ** Set overlapping node Ids from a data block
    */
/*HV*/ void
DualNodeMemWrapper::
/*HV*/ setXthEdge(DualNodeId DID, int whichOlap, DualNodeDataId DDID)
//HO ;
{
    //-----
    // Set the Xth Overlapping node
    //
    PARANOID_ASSERT_L2((whichOlap <= DN_MAX_OLAPS_IN_DATA));
    PARANOID_ASSERT_L2((whichOlap > 0));
    switch(whichOlap)
    {
        case 1:
        {
            setDn1_olap(DID, DDID);
            break;
        }
        case 2:
        {
            setDn2_olap(DID, DDID);
            break;
        }
        case 3:
        {
            setDn3_olap(DID, DDID);
            break;
        }
    }
}
//HO 

    /*
    ** Set edge offsets from a data block
    */
/*HV*/ void
DualNodeMemWrapper::
/*HV*/ setXthOffset(sMDInt offset, int whichOlap, DualNodeDataId DDID)
//HO ;
{
    //-----
    // Set the Xth Overlapping node's offset
    //
    PARANOID_ASSERT_L2((whichOlap <= DN_MAX_OLAPS_IN_DATA));
    PARANOID_ASSERT_L2((whichOlap > 0));
    switch(whichOlap)
    {
        case 1:
        {
            setDn1_offset(offset, DDID);
            break;
        }
        case 2:
        {
            setDn2_offset(offset, DDID);
            break;
        }
        case 3:
        {
            setDn3_offset(offset, DDID);
            break;
        }
    }
}
//HO 

    /*
    ** set edge states from a data block
    */
/*HV*/ void
DualNodeMemWrapper::
/*HV*/ setXthState(EDGE_STATE state, int whichOlap, DualNodeDataId DDID)
//HO ;
{
    //-----
    // Set the Xth Overlapping node's edge state
    //
    PARANOID_ASSERT_L2((whichOlap <= DN_MAX_OLAPS_IN_DATA));
    PARANOID_ASSERT_L2((whichOlap > 0));
    switch(whichOlap)
    {
        case 1:
        {
            setDn1_state(state, DDID);
            break;
        }
        case 2:
        {
            setDn2_state(state, DDID);
            break;
        }
        case 3:
        {
            setDn3_state(state, DDID);
            break;
        }
    }
}
//HO 

    /*
    ** Set the flag which says if the return offset is the same as forward offset
    */
/*HV*/ void
DualNodeMemWrapper::
/*HV*/ setXthReturnOffset(bool sameOffset, int whichOlap, DualNodeDataId DDID)
//HO ;
{
    //-----
    // Set the Xth Overlapping node's offset
    //
    PARANOID_ASSERT_L2((whichOlap <= DN_MAX_OLAPS_IN_DATA));
    PARANOID_ASSERT_L2((whichOlap > 0));
    switch(whichOlap)
    {
        case 1:
        {
            setDn1_returnOffset(sameOffset, DDID);
            break;
        }
        case 2:
        {
            setDn2_returnOffset(sameOffset, DDID);
            break;
        }
        case 3:
        {
            setDn3_returnOffset(sameOffset, DDID);
            break;
        }
    }
}
//HO 

    /*
    ** Get the offset from node A to node B
    */
/*HV*/ sMDInt
DualNodeMemWrapper::
/*HV*/ getOffsetBetweenNodes(DualNodeId nodeB, EDGE_STATE state, DualNodeId nodeA)
//HO ;
{
    //-----
    // Sometimes we just need to get the offset between two nodes
    // this function does just that
    //
    DN_EDGE_ELEM tmp_edge;
    if(getEdges(&tmp_edge, state, 0, nodeA))
    {
        do {
            if(tmp_edge.DNEE_OlapNode == nodeB)
                return tmp_edge.DNEE_Offset;
        } while(getNextEdge(&tmp_edge));
    }
    return (sMDInt)0;
}
//HO 

/*
** Get the offset from node A to node B
*/
/*HV*/ sMDInt
DualNodeMemWrapper::
/*HV*/ getReturnOffsetBetweenNodes(DualNodeId nodeB, EDGE_STATE state, DualNodeId nodeA)
//HO ;
{
    //-----
    // Sometimes we just need to get the offset between two nodes
    // this function does just that
    //
    DN_EDGE_ELEM tmp_edge;
    if(getEdges(&tmp_edge, state, 0, nodeA))
    {
        do {
            if(tmp_edge.DNEE_OlapNode == nodeB)
                return tmp_edge.DNEE_ReturnOffset;
        } while(getNextEdge(&tmp_edge));
    }
    return (sMDInt)0;
}
//HO 

/*HV*/ uMDInt
DualNodeMemWrapper::
/*HV*/ getTRank(DualNodeId DID)
//HO ;
{
    //-----
    // get the number of non-transitive edges (slave edges)
    //
    uMDInt ret_val = 0;
    DualNodeDataId data_block = getNextDataBlock(DID);
    while(DND_NULL_ID != data_block)
    {
        ret_val += mEdgeStates[getBlockState(data_block)][DN_BLOCK_STATE_SLAVE_COUNT];
        data_block = getNextDataBlock(data_block);
    }
    return ret_val;
}
//HO 
    /*
    ** EDGE STATES
    **
    ** There are only 3 places in each data block, so there is a limit on the number of combinations we can have for
    ** overlapping, adjacent and banished nodes in a block. (the total sums to 3)
    **
    ** In fact there are only 20 combinations: ( gives 21 states including error state: ##)
    **  
    ** ---------------------------------------------------------------------------------------------------------------
    **  |  number of each -> STATE         || num filled ||                ACTIONS  -> new_state                     |
    ** ---------------------------------------------------------------------------------------------------------------
    **  |  NT |  T  |  D  |  B  |  State   ||            || +T | TN | TD | TB | NT | ND | NB | DN | DT | DB | RESET  |
    ** ------------------------------------||            ||-----------------------------------------------------------
    **  |  0  |  0  |  0  |  0  |    0     ||     0      || 10 | ## | ## | ## | ## | ## | ## | ## | ## | ## |   0    |
    **  |  0  |  0  |  0  |  1  |    1     ||     1      || 11 | ## | ## | ## | ## | ## | ## | ## | ## | ## |   1    |
    **  |  0  |  0  |  0  |  2  |    2     ||     2      || 12 | ## | ## | ## | ## | ## | ## | ## | ## | ## |   2    |
    **  |  0  |  0  |  0  |  3  |    3     ||     3      || ## | ## | ## | ## | ## | ## | ## | ## | ## | ## |   3    |
    **  |  0  |  0  |  1  |  0  |    4     ||     1      || 13 | ## | ## | ## | ## | ## | ## | 20 | 10 | 1  |   10   |
    **  |  0  |  0  |  1  |  1  |    5     ||     2      || 14 | ## | ## | ## | ## | ## | ## | 21 | 11 | 2  |   11   |
    **  |  0  |  0  |  1  |  2  |    6     ||     3      || ## | ## | ## | ## | ## | ## | ## | 22 | 12 | 3  |   12   |
    **  |  0  |  0  |  2  |  0  |    7     ||     2      || 15 | ## | ## | ## | ## | ## | ## | 23 | 13 | 5  |   16   |
    **  |  0  |  0  |  2  |  1  |    8     ||     3      || ## | ## | ## | ## | ## | ## | ## | 24 | 14 | 6  |   17   |
    **  |  0  |  0  |  3  |  0  |    9     ||     3      || ## | ## | ## | ## | ## | ## | ## | 25 | 15 | 8  |   19   |
    **  |  0  |  1  |  0  |  0  |    10    ||     1      || 16 | 20 | 4  | 1  | ## | ## | ## | ## | ## | ## |   10   |
    **  |  0  |  1  |  0  |  1  |    11    ||     2      || 17 | 21 | 5  | 2  | ## | ## | ## | ## | ## | ## |   11   |
    **  |  0  |  1  |  0  |  2  |    12    ||     3      || ## | 22 | 6  | 3  | ## | ## | ## | ## | ## | ## |   12   |
    **  |  0  |  1  |  1  |  0  |    13    ||     2      || 18 | 23 | 7  | 5  | ## | ## | ## | 26 | 16 | 11 |   16   |
    **  |  0  |  1  |  1  |  1  |    14    ||     3      || ## | 24 | 8  | 6  | ## | ## | ## | 27 | 17 | 12 |   17   |
    **  |  0  |  1  |  2  |  0  |    15    ||     3      || ## | 25 | 9  | 8  | ## | ## | ## | 28 | 18 | 14 |   19   |
    **  |  0  |  2  |  0  |  0  |    16    ||     2      || 19 | 26 | 13 | 11 | ## | ## | ## | ## | ## | ## |   16   |
    **  |  0  |  2  |  0  |  1  |    17    ||     3      || ## | 27 | 14 | 12 | ## | ## | ## | ## | ## | ## |   17   |
    **  |  0  |  2  |  1  |  0  |    18    ||     3      || ## | 28 | 15 | 14 | ## | ## | ## | 29 | 19 | 17 |   19   |
    **  |  0  |  3  |  0  |  0  |    19    ||     3      || ## | 29 | 18 | 17 | ## | ## | ## | ## | ## | ## |   19   |
    **  |  1  |  0  |  0  |  0  |    20    ||     1      || 26 | ## | ## | ## | 10 | 4  | 1  | ## | ## | ## |   10   |
    **  |  1  |  0  |  0  |  1  |    21    ||     2      || 27 | ## | ## | ## | 11 | 5  | 2  | ## | ## | ## |   11   |
    **  |  1  |  0  |  0  |  2  |    22    ||     3      || ## | ## | ## | ## | 12 | 6  | 3  | ## | ## | ## |   12   |
    **  |  1  |  0  |  1  |  0  |    23    ||     2      || 28 | ## | ## | ## | 13 | 7  | 5  | 30 | 26 | 21 |   16   |
    **  |  1  |  0  |  1  |  1  |    24    ||     3      || ## | ## | ## | ## | 14 | 8  | 6  | 31 | 27 | 22 |   17   |
    **  |  1  |  0  |  2  |  0  |    25    ||     3      || ## | ## | ## | ## | 15 | 9  | 8  | 32 | 28 | 24 |   19   |
    **  |  1  |  1  |  0  |  0  |    26    ||     2      || 29 | 30 | 23 | 21 | 16 | 13 | 11 | ## | ## | ## |   16   |
    **  |  1  |  1  |  0  |  1  |    27    ||     3      || ## | 31 | 24 | 22 | 17 | 14 | 12 | ## | ## | ## |   17   |
    **  |  1  |  1  |  1  |  0  |    28    ||     3      || ## | 32 | 25 | 24 | 18 | 15 | 14 | 33 | 29 | 27 |   19   |
    **  |  1  |  2  |  0  |  0  |    29    ||     3      || ## | 33 | 28 | 27 | 19 | 18 | 17 | ## | ## | ## |   19   |
    **  |  2  |  0  |  0  |  0  |    30    ||     2      || 33 | ## | ## | ## | 26 | 23 | 21 | ## | ## | ## |   16   |
    **  |  2  |  0  |  0  |  1  |    31    ||     3      || ## | ## | ## | ## | 27 | 24 | 22 | ## | ## | ## |   17   |
    **  |  2  |  0  |  1  |  0  |    32    ||     3      || ## | ## | ## | ## | 28 | 25 | 24 | 34 | 33 | 31 |   19   |
    **  |  2  |  1  |  0  |  0  |    33    ||     3      || ## | 34 | 32 | 31 | 29 | 28 | 27 | ## | ## | ## |   19   |
    **  |  3  |  0  |  0  |  0  |    34    ||     3      || ## | ## | ## | ## | 33 | 32 | 31 | ## | ## | ## |   19   |
    ** ---------------------------------------------------------------------------------------------------------------
    **  |  0  |  1  |  2  |  3  |          ||     4      || 5  | 6  | 7  | 8  | 9  | 10 | 11 | 12 | 13 | 14 |   15   |
    **  |                                                                                                            |
    **  |                                      ROWS OF THE MATRIX                                                    |
    ** ---------------------------------------------------------------------------------------------------------------
    **
    ** This information can be stored in a DN_DATA_BLOCK_STATE_HEIGHT * DN_DATA_BLOCK_STATE_WIDTH matrix
    ** it is called mEdgeStates, it is initialised in initEdgeStates(). The error state is (##) 35
    */

    /*
    ** Initialise the edge states matrix
    */
/*HV*/ void
DualNodeMemWrapper::
/*HV*/ initEdgeStates(void)
//HO ;
{
    //-----
    // initialise the EDGE STATES matrix
    //
    
    uMDInt temp_states[DN_BLOCK_STATE_HEIGHT][DN_BLOCK_STATE_WIDTH] =
    {
        { 0, 0, 0, 0, 0, 10, 35, 35, 35, 35, 35, 35, 35, 35, 35, 0 },
        { 0, 0, 0, 1, 1, 11, 35, 35, 35, 35, 35, 35, 35, 35, 35, 1 },
        { 0, 0, 0, 2, 2, 12, 35, 35, 35, 35, 35, 35, 35, 35, 35, 2 },
        { 0, 0, 0, 3, 3, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 3 },
        { 0, 0, 1, 0, 1, 13, 35, 35, 35, 35, 35, 35, 20, 10, 1, 10 },

        { 0, 0, 1, 1, 2, 14, 35, 35, 35, 35, 35, 35, 21, 11, 2, 11 },
        { 0, 0, 1, 2, 3, 35, 35, 35, 35, 35, 35, 35, 22, 12, 3, 12 },
        { 0, 0, 2, 0, 2, 15, 35, 35, 35, 35, 35, 35, 23, 13, 5, 16 },
        { 0, 0, 2, 1, 3, 35, 35, 35, 35, 35, 35, 35, 24, 14, 6, 17 },
        { 0, 0, 3, 0, 3, 35, 35, 35, 35, 35, 35, 35, 25, 15, 8, 19 },

        { 0, 1, 0, 0, 1, 16, 20, 4, 1, 35, 35, 35, 35, 35, 35, 10 },
        { 0, 1, 0, 1, 2, 17, 21, 5, 2, 35, 35, 35, 35, 35, 35, 11 },
        { 0, 1, 0, 2, 3, 35, 22, 6, 3, 35, 35, 35, 35, 35, 35, 12 },
        { 0, 1, 1, 0, 2, 18, 23, 7, 5, 35, 35, 35, 26, 16, 11, 16 },
        { 0, 1, 1, 1, 3, 35, 24, 8, 6, 35, 35, 35, 27, 17, 12, 17 },

        { 0, 1, 2, 0, 3, 35, 25, 9, 8, 35, 35, 35, 28, 18, 14, 19 },
        { 0, 2, 0, 0, 2, 19, 26, 13, 11, 35, 35, 35, 35, 35, 35, 16 },
        { 0, 2, 0, 1, 3, 35, 27, 14, 12, 35, 35, 35, 35, 35, 35, 17 },
        { 0, 2, 1, 0, 3, 35, 28, 15, 14, 35, 35, 35, 29, 19, 17, 19 },
        { 0, 3, 0, 0, 3, 35, 29, 18, 17, 35, 35, 35, 35, 35, 35, 19 },

        { 1, 0, 0, 0, 1, 26, 35, 35, 35, 10, 4, 1, 35, 35, 35, 10 },
        { 1, 0, 0, 1, 2, 27, 35, 35, 35, 11, 5, 2, 35, 35, 35, 11 },
        { 1, 0, 0, 2, 3, 35, 35, 35, 35, 12, 6, 3, 35, 35, 35, 12 },
        { 1, 0, 1, 0, 2, 28, 35, 35, 35, 13, 7, 5, 30, 26, 21, 16 },
        { 1, 0, 1, 1, 3, 35, 35, 35, 35, 14, 8, 6, 31, 27, 22, 17 },

        { 1, 0, 2, 0, 3, 35, 35, 35, 35, 15, 9, 8, 32, 28, 24, 19 },
        { 1, 1, 0, 0, 2, 29, 30, 23, 21, 16, 13, 11, 35, 35, 35, 16 },
        { 1, 1, 0, 1, 3, 35, 31, 24, 22, 17, 14, 12, 35, 35, 35, 17 },
        { 1, 1, 1, 0, 3, 35, 32, 25, 24, 18, 15, 14, 33, 29, 27, 19 },
        { 1, 2, 0, 0, 3, 35, 33, 28, 27, 19, 18, 17, 35, 35, 35, 19 },
        
        { 2, 0, 0, 0, 2, 33, 35, 35, 35, 26, 23, 21, 35, 35, 35, 16 },
        { 2, 0, 0, 1, 3, 35, 35, 35, 35, 27, 24, 22, 35, 35, 35, 17 },
        { 2, 0, 1, 0, 3, 35, 35, 35, 35, 28, 25, 24, 34, 33, 31, 19 },
        { 2, 1, 0, 0, 3, 35, 34, 32, 31, 29, 28, 27, 35, 35, 35, 19 },
        { 3, 0, 0, 0, 3, 35, 35, 35, 35, 33, 32, 31, 35, 35, 35, 19 }
    };
    
    for(int i = 0; i < DN_BLOCK_STATE_HEIGHT; i++)
    {
        for(int j = 0; j < DN_BLOCK_STATE_WIDTH; j++)
        {
            mEdgeStates[i][j] = temp_states[i][j];
        }
    }
}
//HO 

    /*
    ** Get the number of edge spaces used in a data block
    */
/*HV*/ int
DualNodeMemWrapper::
/*HV*/ getNumUsedInDataBlock(DualNodeDataId DDID)
//HO ;
{
    //-----
    // Get the number of places used in a data block
    //
    return mEdgeStates[getBlockState(DDID)][DN_BLOCK_STATE_TOTAL_EDGE_COUNT];
}
//HO 

    /*
    ** Set the state of an edge for a UniNode
    */
/*HV*/ bool
DualNodeMemWrapper::
/*HV*/ setEdgeState_asymmetrical(EDGE_STATE newEdgeState, int whichOlap, DualNodeDataId DDID)
//HO ;
{
    //-----
    // set the state of the olap whichOlap in the data block DDID
    // also update the block state
    // no checking is done
    //
    PARANOID_ASSERT_L2((DDID != DND_NULL_ID));
    PARANOID_ASSERT_L2((whichOlap > 0));
    PARANOID_ASSERT_L2((whichOlap <= DN_MAX_OLAPS_IN_DATA));
    PARANOID_ASSERT_PRINT_L2(newEdgeState >= 0, newEdgeState);
    PARANOID_ASSERT_L2((newEdgeState < 4));

    uMDInt block_state = getBlockState(DDID);
    uMDInt current_edge_state = 0;
    int state_change_action = 0;

    // get the current state
    switch(whichOlap)
    {
        case 1:
        {
            current_edge_state = getDn1_state(DDID);
            break;
        }
        case 2:
        {
            current_edge_state = getDn2_state(DDID);
            break;
        }
        case 3:
        {
            current_edge_state = getDn3_state(DDID);
            break;
        }
    }

    // work out what we're doing
    switch(newEdgeState)
    {
        case DN_EDGE_STATE_SLAVE:
        {
            switch(current_edge_state)
            {
                case DN_EDGE_STATE_SLAVE:
                {
                    logError("Useless state change: " << sayEdgeStateLikeAHuman(current_edge_state) << " --> " << sayEdgeStateLikeAHuman(newEdgeState));
                    return false;
                }
                case DN_EDGE_STATE_MASTER:
                {
                    logError("Bad state change: " << sayEdgeStateLikeAHuman(current_edge_state) << " --> " << sayEdgeStateLikeAHuman(newEdgeState));
                    return false;
                }
                case DN_EDGE_STATE_DETACHED:
                {
                    logError("Bad state change: " << sayEdgeStateLikeAHuman(current_edge_state) << " --> " << sayEdgeStateLikeAHuman(newEdgeState));
                    return false;
                }
                case DN_EDGE_STATE_BANISHED:
                {
                    logError("Trying to change the state of a banished edge");
                    return false;
                }
            }
                break;
        }
        case DN_EDGE_STATE_MASTER:
        {
            switch(current_edge_state)
            {
                case DN_EDGE_STATE_SLAVE:
                {
                    state_change_action = DN_BLOCK_STATE_SLAVE_TO_MASTER;
                    break;
                }
                case DN_EDGE_STATE_MASTER:
                {
                    logError("Useless state change: " << sayEdgeStateLikeAHuman(current_edge_state) << " --> " << sayEdgeStateLikeAHuman(newEdgeState));
                    return false;
                }
                case DN_EDGE_STATE_DETACHED:
                {
                    state_change_action = DN_BLOCK_STATE_D_TO_MASTER;
                    break;
                }
                case DN_EDGE_STATE_BANISHED:
                {
                    logError("Trying to change the state of a banished edge");
                    return false;
                }
            }
            break;
        }
        case DN_EDGE_STATE_DETACHED:
        {
            switch(current_edge_state)
            {
                case DN_EDGE_STATE_SLAVE:
                {
                    state_change_action = DN_BLOCK_STATE_SLAVE_TO_D;
                    break;
                }
                case DN_EDGE_STATE_MASTER:
                {
                    state_change_action = DN_BLOCK_STATE_MASTER_TO_D;
                    break;
                }
                case DN_EDGE_STATE_DETACHED:
                {
                    logError("Useless state change: " << sayEdgeStateLikeAHuman(current_edge_state) << " --> " << sayEdgeStateLikeAHuman(newEdgeState));
                    return false;
                }
                case DN_EDGE_STATE_BANISHED:
                {
                    logError("Trying to change the state of a banished edge");
                    return false;
                }
            }
            break;
        }
        case DN_EDGE_STATE_BANISHED:
        {
            switch(current_edge_state)
            {
                case DN_EDGE_STATE_SLAVE:
                {
                    state_change_action = DN_BLOCK_STATE_SLAVE_TO_B;
                    break;
                }
                case DN_EDGE_STATE_MASTER:
                {
                    state_change_action = DN_BLOCK_STATE_MASTER_TO_B;
                    break;
                }
                case DN_EDGE_STATE_DETACHED:
                {
                    state_change_action = DN_BLOCK_STATE_D_TO_B;
                    break;
                }
                case DN_EDGE_STATE_BANISHED:
                {
                    logError("Trying to change the state of a banished edge");
                    logError("Useless state change: " << sayEdgeStateLikeAHuman(current_edge_state) << " --> " << sayEdgeStateLikeAHuman(newEdgeState));
                    return false;
                }
            }
            break;
        }
    }

    PARANOID_ASSERT_L2((0 != state_change_action));

    // calculate the new block state
    uMDInt new_block_state = mEdgeStates[block_state][state_change_action];
    PARANOID_ASSERT_L2((new_block_state != DN_BLOCK_STATE_ERROR));

    // set the new state for the overlap
    switch(whichOlap)
    {
        case 1:
        {
            setDn1_state(newEdgeState, DDID);
            break;
        }
        case 2:
        {
            setDn2_state(newEdgeState, DDID);
            break;
        }
        case 3:
        {
            setDn3_state(newEdgeState, DDID);
            break;
        }
    }

    // and for the edge
    setBlockState(new_block_state, DDID);
    return true;
}
//HO 

    /*
    ** Detach an edge from the graph (one way function)
    */
//HV bool detachEdge_asymmetrical(int whichOlap, DualNodeDataId DDID)
//HO { return setEdgeState_asymmetrical(DN_EDGE_STATE_DETACHED , whichOlap, DDID); }
//HO
    
    /*
    ** Detach the edge from node B to node A (one way function)
    */
/*HV*/ bool
DualNodeMemWrapper::
/*HV*/ detachEdge_asymmetrical(DualNodeId nodeA, DualNodeId nodeB)
//HO ;
{
    //-----
    // Detach the edge from node B to node A (one way function)
    //
    // first get the edge from A to B
    DN_EDGE_ELEM tmp_edge;
    // get the edge from B to A
    if(getEdges(&tmp_edge, 0, 0, nodeB))
    {
        do {
            if(tmp_edge.DNEE_OlapNode == nodeA)
            {
                return detachEdge_asymmetrical(tmp_edge.DNEE_WhichOlap, tmp_edge.DNEE_WhichData);
            }
        }while(getNextEdge(&tmp_edge));
    }
    PARANOID_ASSERT_PRINT_L2(false, "detachEdge_asymmetrical from " << nodeB << " to " << nodeA << " edge not found");
    return false;
}
//HO 

    /*
    ** Detach the edge betwen two nodes
    */
/*HV*/ bool
DualNodeMemWrapper::
/*HV*/ detachEdgeBetween_symmetrical(DualNodeId nodeA, DualNodeId nodeB)
//HO ;
{
    //-----
    // Detach the edge betwen two nodes
    //
    bool ret_value = true;
    ret_value &= detachEdge_asymmetrical(nodeA, nodeB);
    ret_value &= detachEdge_asymmetrical(nodeB, nodeA);
    PARANOID_ASSERT_L2((ret_value));
    return ret_value;
}
//HO 

    /*
    ** Banish an edge from the graph
    */
//HV inline bool banishEdge_asymmetrical(int whichOlap, DualNodeDataId DDID)
//HO { return setEdgeState_asymmetrical(DN_EDGE_STATE_BANISHED , whichOlap, DDID); }
//HO
    
    /*
    ** Banish the edge from node B to node A (one way function)
    */
/*HV*/ bool
DualNodeMemWrapper::
/*HV*/ banishEdge_asymmetrical(DualNodeId nodeA, DualNodeId nodeB)
//HO ;
{
    //-----
    // Detach the edge from node B to node A (one way function)
    //
    // first get the edge from A to B
    DN_EDGE_ELEM tmp_edge;
    // get the edge from B to A
    if(getEdges(&tmp_edge, 0, 0, nodeB))
    {
        do {
            if(tmp_edge.DNEE_OlapNode == nodeA)
            {
                return banishEdge_asymmetrical(tmp_edge.DNEE_WhichOlap, tmp_edge.DNEE_WhichData);
            }
        }while(getNextEdge(&tmp_edge));
    }
    PARANOID_INFO_L2("banishEdge_asymmetrical from " << nodeB << " to " << nodeA << " edge not found");
    return false;
}
//HO 

    /*
    ** Banish the edge betwen two nodes
    */
/*HV*/ bool
DualNodeMemWrapper::
/*HV*/ banishEdgeBetween_symmetrical(DualNodeId nodeA, DualNodeId nodeB)
//HO ;
{
    //-----
    // Banish the edge betwen two nodes
    //
    bool ret_value = true;
    ret_value &= banishEdge_asymmetrical(nodeA, nodeB);
    ret_value &= banishEdge_asymmetrical(nodeB, nodeA);
    PARANOID_ASSERT_L2((ret_value));
    return ret_value;
}
//HO 

/******************************************************************************
** EDGE MAKING AND MANAGEMENT
******************************************************************************/
    /*
    ** Symmetrical transitive edge addition
    */
/*HV*/ bool
DualNodeMemWrapper::
/*HV*/ addEdgeBetween(sMDInt offsetAtoB, DualNodeId nodeA, sMDInt offsetBtoA, DualNodeId nodeB)
//HO ;
{
    //-----
    // Make a master-master edge between two nodes
    //
    // nodes can not overlap with themselves
    if(nodeA == nodeB)
        return false;
    bool ret_value = true;
    bool off_matches = ( (offsetAtoB * offsetBtoA) > 0 );
    ret_value &= addEdge_asymmetrical(DN_EDGE_STATE_MASTER, nodeB, offsetAtoB, off_matches, nodeA);
    ret_value &= addEdge_asymmetrical(DN_EDGE_STATE_MASTER, nodeA, offsetBtoA, off_matches, nodeB);
    return ret_value;
}
//HO 
    
    /*
    ** Symmetrical transitive edge addition
    */
/*HV*/ bool
DualNodeMemWrapper::
/*HV*/ addSlaveEdgeBetween(DualNodeId slave, DualNodeId master)
//HO ;
{
    //-----
    // Add a transitive edge between two nodes
    //
    // nodes can not overlap with themselves
    bool ret_value = true;
    ret_value &= addEdge_asymmetrical(DN_EDGE_STATE_SLAVE, slave, 0, true, master);
    ret_value &= addEdge_asymmetrical(DN_EDGE_STATE_SLAVE, master, 0, true, slave);
    return ret_value;
}
//HO 
 
    /*
    ** Assymetrical transitive edge addition
    */
/*HV*/ bool
DualNodeMemWrapper::
/*HV*/ addEdge_asymmetrical(EDGE_STATE state, DualNodeId newNode, sMDInt offsetFromBase, bool retOffsetMatches, DualNodeId baseNode)
//HO ;
{
    //-----
    // This is an assymetrical function and should not be called except by the
    // function addEdgeBetween. Part of this function's internal
    // logic will cause some bat-ass-crazy stuff to happen if you don't call the function
    // in reverse. luckily we have a function which does this so just call that one, OK? GOOD!
    // 
    // EDIT: We d call this in a few other places. Notably assimilate dummy and transfer
    // edges which are part of GenericNodeClass. But that seems to be the limit of that
    // shananegins...
    //
    // By default nodes are added with state DN_EDGE_STATE_MASTER
    // It is up to this method to decide where to add the actual data,
    // and to allocate a new block if necessary...
    //
    // We want no duplicates here WHATSOEVER! So If we are trying to add the same node with the
    // same olapOffset it is ignored and we add nothing. If we are trying to add the same node but with
    // a different olapOffset then we keep the closest one.
    // 
    // Detached nodes represent free spaces, so we can put a new edge into a detached space...
    // The only catch is that if there is more than 1 detached spot to choose from then
    // we give preference to the spot with the same overlapping node. This kills duplicates.
    //
    // We don't care about the ordering here. the amount of effort to re-write everything
    // each time we add or remove something makes this not feasible...
    //
    // where needed we can call sort olaps to get things ordered well
    //
    // Be paranoid
    PARANOID_ASSERT_L2((newNode != DN_NULL_ID));
    PARANOID_ASSERT_L2((baseNode != DN_NULL_ID));
   
    // we need to check that this node has not been added before
    // while we run through these we take the time to check for any
    // free spots to use and we mark them.
    bool free_spot_found = false;                                   // true if we've found a free (prev detached) spot to fill up
    bool free_matches_Id = false;                                   // true if that spot was previously filled by the same node
    DualNodeDataId free_block = DND_NULL_ID;                        // the actual free block
    int free_which = 0;                                             // and which entry int hat block

    DualNodeDataId next_data_block = getNextDataBlock(baseNode);
    DualNodeDataId data_block = next_data_block;
    int num_used = 0;
    int num_used_in_db = 0;
    
    // now run through every used data_block and check it all out
    while(DND_NULL_ID != next_data_block)
    {
        data_block = next_data_block;
        num_used_in_db = getNumUsedInDataBlock(data_block);
        for(num_used = 1; num_used <= num_used_in_db; num_used++)
        {
            if(DN_EDGE_STATE_DETACHED == getXthState(num_used, data_block))
            {
                // we give preference to filling detached slots with the same ID
                // once we've found such a spot there's no need to keep
                // trying to mark free spots
                if(!free_matches_Id)
                {
                    free_spot_found = true;
                    free_block = data_block;
                    free_which = num_used;
                    // check to see if the spot matches nicely (ie. is from the same node)
                    if(getXthEdge(num_used, data_block) == newNode)
                        free_matches_Id = true;
                }
            }
            else
            {
                // this is an attached ID. 
                // we need to check that this is a unique Id
                // we don't allow to add the same edge twice unless it was previously detached
                if(getXthEdge(num_used, data_block) == newNode)
                {
                    // It matches a previously added block Check the offset and state
                    // we'll just change the state to match the new state
                    if(getXthState(num_used, data_block) == state)
                    {
                        // now check the offset
                        if(getXthOffset(num_used, data_block) == offsetFromBase)
                        {
                            logWarn("ADDING THE SAME NODE TWICE: " << baseNode << " to " << newNode << " : " << offsetFromBase, 1);
                            return false;
                        }
                        else
                        {
                            logError("ADDING TWO WITH SAME EDGE, DIFF STATE AND NON-DETACHED FIRST AT " << baseNode);
                            logError("Changing old state from: " << getXthState(num_used, data_block) << " to: " << state << " at: " << baseNode);
                            return false;
                        }
                    }
                    logError("Changing old state from: " << getXthState(num_used, data_block) << " to: " << state << " at: " << baseNode);
                    return false;
                }
            }
        }
        next_data_block = getNextDataBlock(data_block);
    }

    // check to see if we can use a detached space...
    if(free_spot_found)
    {
        setXthEdge(newNode, free_which, free_block);
        setXthOffset(offsetFromBase, free_which, free_block);
        setXthReturnOffset(retOffsetMatches, free_which, free_block);
        setXthState(state, free_which, free_block);
        remakeEdgeStates(baseNode);
        return true;
    }
    
    // not been added before and there's no free spots to fill so we continue
    if(DND_NULL_ID == data_block)
    {
        // this must be ther first edge added for this node
        data_block = newDualNodeDataId();
        // link the data to the head
        setNextDataBlock(data_block, baseNode);
    }
    else if(DN_MAX_OLAPS_IN_DATA < num_used)
    {
        // this block is full we need to make another data block
        next_data_block = newDualNodeDataId();
        // link the data to the head
        setNextDataBlock(next_data_block, data_block);
        data_block = next_data_block;
        num_used = 0;
    }
    else
    {
        num_used--;
    }

    switch(num_used)
    {
        case 0:
        {
            setDn1_state(state, data_block);
            setDn1_offset(offsetFromBase, data_block);
            setDn1_returnOffset(retOffsetMatches, data_block);
            setDn1_olap(newNode, data_block);
            setBlockState(mEdgeStates[0][DN_BLOCK_STATE_ADD_SLAVE], data_block);
            break;
        }
        case 1:
        {
            setDn2_state(state, data_block);
            setDn2_offset(offsetFromBase, data_block);
            setDn2_returnOffset(retOffsetMatches, data_block);
            setDn2_olap(newNode, data_block);
            setBlockState(mEdgeStates[getBlockState(data_block)][DN_BLOCK_STATE_ADD_SLAVE], data_block);
            break;
        }
        case 2:
        {
            setDn3_state(state, data_block);
            setDn3_offset(offsetFromBase, data_block);
            setDn3_returnOffset(retOffsetMatches, data_block);
            setDn3_olap(newNode, data_block);
            setBlockState(mEdgeStates[getBlockState(data_block)][DN_BLOCK_STATE_ADD_SLAVE], data_block);
            break;
        }
    }

    // now we fix the block state...
    switch(state)
    {
        case DN_EDGE_STATE_DETACHED:
        {
            setBlockState(mEdgeStates[getBlockState(data_block)][DN_BLOCK_STATE_SLAVE_TO_D], data_block);
            break;
        }
        case DN_EDGE_STATE_MASTER:
        {
            setBlockState(mEdgeStates[getBlockState(data_block)][DN_BLOCK_STATE_SLAVE_TO_MASTER], data_block);
            incNtRank(baseNode);
            switch(getNtRank(baseNode))
            {
                case 1:
                    setNodeType(DN_NODE_TYPE_CAP, baseNode);
                    break;
                case 2:
                    setNodeType(DN_NODE_TYPE_PATH, baseNode);
                    break;
                default:
                    setNodeType(DN_NODE_TYPE_CROSS, baseNode);
                    break;
            }
            break;
        }
        case DN_EDGE_STATE_BANISHED:
        {
            setBlockState(mEdgeStates[getBlockState(data_block)][DN_BLOCK_STATE_SLAVE_TO_B], data_block);
            break;
        } // otherwise it's a slave and we can relax
        default:
        {
            uMDInt curr_node_type = getNodeType(baseNode);
            if(curr_node_type == DN_NODE_TYPE_UNSET || curr_node_type == DN_NODE_TYPE_DETACHED)
                setNodeType(DN_NODE_TYPE_ATTACHED, baseNode);
            break;
        }
    }
    return true;
}
//HO 

/*HV*/ bool
DualNodeMemWrapper::
/*HV*/ updateEdge_asymmetrical(DualNodeId oldNode, DualNodeId newNode, DualNodeId baseNode)
//HO ;
{
    //-----
    // Updates the edge DNID but leaves everything else intact
    //
    DN_EDGE_ELEM tmp_edge;
    bool subbed = false;
    PARANOID_ASSERT_L2(isValidAddress(baseNode));
    if(getEdges(&tmp_edge, 0, 0, baseNode))
    {
        do {
            if(tmp_edge.DNEE_OlapNode == newNode)
            {
                // we better hope this guy is detached or else we're adding a double edge
                if(tmp_edge.DNEE_State != DN_EDGE_STATE_DETACHED)
                {
                    logError("Trying to update to position where newnode exists: " <<oldNode << " : " << newNode << " : " << baseNode);
                }
                else
                {
                    // we will add the newNode below, for now we should just wipe this mofo
                    setXthEdge(DN_NULL_ID, tmp_edge.DNEE_WhichOlap, tmp_edge.DNEE_WhichData);
                }
            }
            if(tmp_edge.DNEE_OlapNode == oldNode)
            {
                // we have found our guy!
                // change the DNID for this olap
                if(!subbed)
                {
                    setXthEdge(newNode, tmp_edge.DNEE_WhichOlap, tmp_edge.DNEE_WhichData);
                    if(newNode == DN_NULL_ID)
                        setXthState(DN_EDGE_STATE_DETACHED, tmp_edge.DNEE_WhichOlap, tmp_edge.DNEE_WhichData);
                    subbed = true;
                }
                else
                {
                    logError("oldNode appears twice here: " << oldNode << " : " << newNode << " : " << baseNode);
                }
            }
        } while(getNextEdge(&tmp_edge));
    }
    return subbed;
}
//HO 

    /*
    ** Reset the edge states (except banished edges)
    ** Blank the ContigId
    */
/*HV*/ void
DualNodeMemWrapper::
/*HV*/ resetEdges(DualNodeId DID)
//HO ;
{
    //-----
    // delete all the data blocks
    //
    DualNodeDataId data_block = getNextDataBlock(DID);
    DualNodeDataId delete_block;
    while(DND_NULL_ID != data_block)
    {
        delete_block = data_block;
        data_block = getNextDataBlock(data_block);
        deleteDualNodeDataId(delete_block);
    }

    // reset the data fields
    clearNextDataBlock(DID);
    clearNodeType(DID);
    resetNtRank(DID);
    clearContig(DID);
}
//HO 

    /*
    ** Sort the edges for faster retrieval of non-transitive nodes
    */

    static DualNodeDataId DNG_swap_into_block_array[(SAS_DEF_MAX_READLENGTH -SAS_DEF_UN_OFFSET_MAX_MAX) * 2 * 4 + 1];
    static int DNG_swap_into_position_array[(SAS_DEF_MAX_READLENGTH -SAS_DEF_UN_OFFSET_MAX_MAX) * 2 * 4 + 1];
    
/*HV*/ bool
DualNodeMemWrapper::
/*HV*/ shallowSortEdges(DualNodeId DID)
//HO ;
{
    //-----
    // make sure the gross order is:
    // non-transitive, transitive, the we don't care
    // local ordering does not matter too much
    //
    int num_used = 0;
    
    int swap_read_index = 0;
    int swap_write_index = 0;
    bool changes_made = false;

    // first bubble up the masters
    DualNodeDataId data_block = getNextDataBlock(DID);
    
    while(DND_NULL_ID != data_block)
    {
        num_used = getNumUsedInDataBlock(data_block);
        PARANOID_ASSERT_L2((num_used > 0));
        for(int focused_olap = 1; focused_olap <= num_used; focused_olap++)
        {
            switch(getXthState(focused_olap, data_block))
            {
                case DN_EDGE_STATE_MASTER:
                    if(swap_read_index != swap_write_index)
                    {
                        swapEdgeInfo(DNG_swap_into_position_array[swap_read_index], DNG_swap_into_block_array[swap_read_index], focused_olap, data_block);
                        DNG_swap_into_block_array[swap_write_index] = data_block;
                        DNG_swap_into_position_array[swap_write_index] = focused_olap;
                        swap_read_index++;
                        swap_write_index++;
                        changes_made = true;
                    }
                    break;
                case DN_EDGE_STATE_SLAVE:
                case DN_EDGE_STATE_DETACHED:
                case DN_EDGE_STATE_BANISHED:
                    DNG_swap_into_block_array[swap_write_index] = data_block;
                    DNG_swap_into_position_array[swap_write_index] = focused_olap;
                    swap_write_index++;
                    break;
            }
        }
        data_block = getNextDataBlock(data_block);
    }

    // reset
    swap_write_index = 0;
    swap_read_index = 0;

    // then get all the transitive guys up behind them
    data_block = getNextDataBlock(DID);
    while(DND_NULL_ID != data_block)
    {
        num_used = getNumUsedInDataBlock(data_block);
        PARANOID_ASSERT_L2((num_used > 0));
        for(int focused_olap = 1; focused_olap <= num_used; focused_olap++)
        {
            switch(getXthState(focused_olap, data_block))
            {
                case DN_EDGE_STATE_MASTER:
                    break;
                case DN_EDGE_STATE_SLAVE:
                    if(swap_read_index != swap_write_index)
                    {
                        swapEdgeInfo(DNG_swap_into_position_array[swap_read_index], DNG_swap_into_block_array[swap_read_index], focused_olap, data_block);
                        DNG_swap_into_block_array[swap_write_index] = data_block;
                        DNG_swap_into_position_array[swap_write_index] = focused_olap;
                        swap_read_index++;
                        swap_write_index++;
                        changes_made = true;
                    }
                    break;
                case DN_EDGE_STATE_DETACHED:
                case DN_EDGE_STATE_BANISHED:
                    DNG_swap_into_block_array[swap_write_index] = data_block;
                    DNG_swap_into_position_array[swap_write_index] = focused_olap;
                    swap_write_index++;
                    break;
            }
        }
        data_block = getNextDataBlock(data_block);
    }

    // the data states will be fubar so we need to redo them
    if(changes_made)
        remakeEdgeStates(DID);
    return true;
}
//HO 

    /*
    ** Swap information between two containers in two data blocks
    */
/*HV*/ void
DualNodeMemWrapper::
/*HV*/ swapEdgeInfo(int whichOlapA, DualNodeDataId DDIDA, int whichOlapB, DualNodeDataId DDIDB)
//HO ;
{
    EDGE_STATE tmp_state = getXthState(whichOlapA, DDIDA);
    DualNodeId tmp_edge = getXthEdge(whichOlapA, DDIDA);
    sMDInt tmp_offset = getXthOffset(whichOlapA, DDIDA);
    bool tmp_return = getXthReturnOffset(whichOlapA, DDIDA);
    
    setXthState(getXthState(whichOlapB, DDIDB), whichOlapA, DDIDA);
    setXthEdge(getXthEdge(whichOlapB, DDIDB), whichOlapA, DDIDA);
    setXthOffset(getXthOffset(whichOlapB, DDIDB), whichOlapA, DDIDA);
    setXthReturnOffset(getXthReturnOffset(whichOlapB, DDIDB), whichOlapA, DDIDA);

    setXthState(tmp_state, whichOlapB, DDIDB);
    setXthEdge(tmp_edge, whichOlapB, DDIDB);
    setXthOffset(tmp_offset, whichOlapB, DDIDB);
    setXthReturnOffset(tmp_return, whichOlapB, DDIDB);
}
//HO 

    /*
    ** Remake the information in a data block
    */
/*HV*/ void
DualNodeMemWrapper::
/*HV*/ remakeEdgeStates(DualNodeId DID)
//HO ;
{
    //-----
    // After we reset the nodes or we sort them the data states make no more sense
    // we need to go through and rebuild them.
    //
    DN_EDGE_ELEM tmp_edge;
    EDGE_STATE tmp_state = 0;
    DualNodeDataId prev_data_block;
    int num_mas_mas = 0;
    if(getEdges(&tmp_edge, 0, 0, DID))
    {
        // initialise this guy to the first data block
        prev_data_block = tmp_edge.DNEE_WhichData;
        do {
            // tmp_edge.DNEE_WhichData holds the id of the data block
            // when this changes from what is stored in prev_data_block
            // then we know that we need to store the state
            if(tmp_edge.DNEE_WhichData != prev_data_block)
            {
                // time to write the previous data block in
                setBlockState(tmp_state, prev_data_block);
                prev_data_block = tmp_edge.DNEE_WhichData;
                tmp_state = 0;
            }
            
            // otherwise we can update the state
            switch(tmp_edge.DNEE_State)
            {
                case DN_EDGE_STATE_MASTER:
                    tmp_state = mEdgeStates[tmp_state][DN_BLOCK_STATE_ADD_SLAVE];
                    tmp_state = mEdgeStates[tmp_state][DN_BLOCK_STATE_SLAVE_TO_MASTER];
                    num_mas_mas++;
                    break;
                case DN_EDGE_STATE_SLAVE:
                    tmp_state = mEdgeStates[tmp_state][DN_BLOCK_STATE_ADD_SLAVE];
                    break;
                case DN_EDGE_STATE_BANISHED:
                    tmp_state = mEdgeStates[tmp_state][DN_BLOCK_STATE_ADD_SLAVE];
                    tmp_state = mEdgeStates[tmp_state][DN_BLOCK_STATE_SLAVE_TO_B];
                    break;
                case DN_EDGE_STATE_DETACHED:
                    tmp_state = mEdgeStates[tmp_state][DN_BLOCK_STATE_ADD_SLAVE];
                    tmp_state = mEdgeStates[tmp_state][DN_BLOCK_STATE_SLAVE_TO_D];
                    break;
            }
        } while(getNextEdge(&tmp_edge));

        // write the last block
        setBlockState(tmp_state, prev_data_block);

        // fix up rank and type
        switch(num_mas_mas)
        {
            case 0:
                setNodeType(DN_NODE_TYPE_ATTACHED, DID);
                setNtRank(0, DID);
                break;
            case 1:
                setNodeType(DN_NODE_TYPE_CAP, DID);
                setNtRank(1, DID);
                break;
            case 2:
                setNodeType(DN_NODE_TYPE_PATH, DID);
                setNtRank(2, DID);
                break;
            default:
                setNodeType(DN_NODE_TYPE_CROSS, DID);
                setNtRank(num_mas_mas, DID);
                break;
        }
    }
    else
    {
        setNodeType(DN_NODE_TYPE_UNSET, DID);
        setNtRank(0, DID);
    }
}
//HO 

/******************************************************************************
** NODE TYPE MANAGEMENT
******************************************************************************/
    /*
    ** Banish a node and all its edges
    */
/*HV*/ bool
DualNodeMemWrapper::
/*HV*/ banishNode(DualNodeId DID)
//HO ;
{
    //-----
    // Banish a node and all edges coming to and from it
    //
    // set the new node type
    setNodeType(DN_NODE_TYPE_BANISHED, DID);

    // then fix the edges
    DN_EDGE_ELEM tmp_edge;
    if(getEdges(&tmp_edge, 0, 0, DID))
    {
        do {
            if(DN_EDGE_STATE_BANISHED != tmp_edge.DNEE_State)
            {
                // banish the edge to the overlapping node
                banishEdge_asymmetrical(tmp_edge.DNEE_WhichOlap, tmp_edge.DNEE_WhichData);
                // and the returning edge
                banishEdge_asymmetrical(DID, tmp_edge.DNEE_OlapNode);
                switch(tmp_edge.DNEE_State)
                {
                    case DN_EDGE_STATE_MASTER:
                        decNtRank(tmp_edge.DNEE_OlapNode);
                        break;
                }
            }
        } while(getNextEdge(&tmp_edge));
    }
    
    setNtRank(0, DID);

    return true;
}
//HO

    /*
    ** Detach a node and all its edges
    */
/*HV*/ bool
DualNodeMemWrapper::
/*HV*/ detachNode(map<GenericNodeId, bool> * detMap, DualNodeId DID)
//HO ;
{
    //-----
    // Detach a node and make all Nt edges T
    //
    // This code detaches nodes and edges
    // set this guy as detached
    setNodeType(DN_NODE_TYPE_DETACHED, DID);

    // detach all the edges
    DN_EDGE_ELEM tmp_edge;
    if(getEdges(&tmp_edge, 0, 0, DID))
    {
        do {
            switch(tmp_edge.DNEE_State)
            {
                case DN_EDGE_STATE_MASTER:
                {
                    // we must be a master, and the overlapping node must be too
                    detachEdge_asymmetrical(tmp_edge.DNEE_WhichOlap, tmp_edge.DNEE_WhichData);
                    detachEdge_asymmetrical(DID, tmp_edge.DNEE_OlapNode);
                    decNtRank(DID);
                    decNtRank(tmp_edge.DNEE_OlapNode);
                    (*detMap)[getGn(tmp_edge.DNEE_OlapNode)] = true;
                    break;
                }
                case DN_EDGE_STATE_SLAVE:
                {
                    // the overlapping node must be a master. We must be a slave. This is OK.
                    detachEdge_asymmetrical(tmp_edge.DNEE_WhichOlap, tmp_edge.DNEE_WhichData);
                    detachEdge_asymmetrical(DID, tmp_edge.DNEE_OlapNode);
                    break;
                }
            }
        } while(getNextEdge(&tmp_edge));
    }
    return true;
}
//HO 

/*HV*/ bool
DualNodeMemWrapper::
/*HV*/ detachNode(DualNodeId DID)
//HO ;
{
    //-----
    // Detach a node and make all Nt edges T
    //
    // This code detaches nodes and edges
    // set this guy as detached
    setNodeType(DN_NODE_TYPE_DETACHED, DID);

    // detach all the edges
    DN_EDGE_ELEM tmp_edge;
    if(getEdges(&tmp_edge, 0, 0, DID))
    {
        do {
            switch(tmp_edge.DNEE_State)
            {
                case DN_EDGE_STATE_MASTER:
                {
                    // we must be a master, and the overlapping node must be too
                    detachEdge_asymmetrical(tmp_edge.DNEE_WhichOlap, tmp_edge.DNEE_WhichData);
                    detachEdge_asymmetrical(DID, tmp_edge.DNEE_OlapNode);
                    decNtRank(DID);
                    decNtRank(tmp_edge.DNEE_OlapNode);
                    break;
                }
                case DN_EDGE_STATE_SLAVE:
                {
                    // the overlapping node must be a master. We must be a slave. This is OK.
                    detachEdge_asymmetrical(tmp_edge.DNEE_WhichOlap, tmp_edge.DNEE_WhichData);
                    detachEdge_asymmetrical(DID, tmp_edge.DNEE_OlapNode);
                    break;
                }
            }
        } while(getNextEdge(&tmp_edge));
    }
    return true;
}
//HO 

/******************************************************************************
** OUTPUT AND PRINTING
******************************************************************************/
    /*
    ** A nicer way to see the edge state
    */
/*HV*/ std::string
DualNodeMemWrapper::
/*HV*/ sayEdgeStateLikeAHuman(uMDInt state)
//HO ;
{
    //-----
    // A nicer way to see the edge state
    //
    switch(state)
    {
        case DN_EDGE_STATE_MASTER:
            return "MASTER_MASTER";
        case DN_EDGE_STATE_SLAVE:
            return "SLAVE";
        case DN_EDGE_STATE_DETACHED:
            return "Detached";
        case DN_EDGE_STATE_BANISHED:
            return "Banished";
        default:
            return "***Random, fubar, take your pick***";
    }
}
//HO 

    /*
    ** A nicer way to see the edge state
    */
/*HV*/ std::string
DualNodeMemWrapper::
/*HV*/ sayNodeTypeLikeAHuman(uMDInt type)
//HO ;
{
    //-----
    // A nicer way to see the edge state
    //
    switch(type)
    {
        case DN_NODE_TYPE_UNSET:
            return "Unset";
        case DN_NODE_TYPE_CAP:
            return "Master Cap";
        case DN_NODE_TYPE_PATH:
            return "Master Path";
        case DN_NODE_TYPE_CROSS:
            return "Master Cross";
        case DN_NODE_TYPE_DETACHED:
            return "Detached";
        case DN_NODE_TYPE_ATTACHED:
            return "Slave/Attached";
        case DN_NODE_TYPE_BANISHED:
            return "Banished";
        case DN_NODE_TYPE_CORRECTED:
            return "Corrected";
        default:
            return "***Random, fubar, take your pick***";
    }
}
//HO 

    /*
    ** Print edges at a uninode
    */
/*HV*/ void
DualNodeMemWrapper::
/*HV*/ printEdges(DualNodeId DID)
//HO ;
{
    //-----
    // print a list of all the edges, their offsets and the state...
    //
    std::cout << "Edge info for " << sayNodeTypeLikeAHuman(getNodeType(DID)) <<
            " : " << DID <<
            " : " << getContig(DID) <<
            " : rank: " << getNtRank(DID);
    DN_EDGE_ELEM tmp_edge;
    DualNodeDataId prev_data_block;
    if(getEdges(&tmp_edge, 0, 0, DID))
    {
        prev_data_block = tmp_edge.DNEE_WhichData;
        do {
            if(prev_data_block != tmp_edge.DNEE_WhichData)
            {
                EDGE_STATE tmp_state = getBlockState(prev_data_block);
                std::cout << " [Ma: " << mEdgeStates[tmp_state][DN_BLOCK_STATE_MASTER_COUNT] <<
                        ", Sl: " << mEdgeStates[tmp_state][DN_BLOCK_STATE_SLAVE_COUNT] <<
                        ", De: " << mEdgeStates[tmp_state][DN_BLOCK_STATE_D_COUNT] <<
                        ", Ba: " << mEdgeStates[tmp_state][DN_BLOCK_STATE_B_COUNT] <<
                        ", To: " << mEdgeStates[tmp_state][DN_BLOCK_STATE_TOTAL_EDGE_COUNT] << "] " << tmp_state << " : " << prev_data_block << " : " << tmp_edge.DNEE_WhichData << std::endl;
                prev_data_block = tmp_edge.DNEE_WhichData;
            }
            else
            {
                std::cout << std::endl;
            }
            if(!isValidAddress(tmp_edge.DNEE_OlapNode))
            {
                std::cout << "INVALID" << 
                        " : " << tmp_edge.DNEE_OlapNode <<
                        " with offsets: [" << tmp_edge.DNEE_Offset << ", " << tmp_edge.DNEE_ReturnOffset << "]" <<
                        " and state: " << sayEdgeStateLikeAHuman(tmp_edge.DNEE_State);
            }
            else if(DN_EDGE_STATE_DETACHED == tmp_edge.DNEE_State)
            {
                std::cout << sayNodeTypeLikeAHuman(DN_NODE_TYPE_DETACHED) <<
                        " : " << tmp_edge.DNEE_OlapNode <<
                        " with offsets: [" << tmp_edge.DNEE_Offset << ", " << tmp_edge.DNEE_ReturnOffset << "]" <<
                        " and state: " << sayEdgeStateLikeAHuman(tmp_edge.DNEE_State);
            }
            else
            {
                std::cout << sayNodeTypeLikeAHuman(getNodeType(tmp_edge.DNEE_OlapNode)) <<
                    " : " << tmp_edge.DNEE_OlapNode <<
                    " : " << getGn(tmp_edge.DNEE_OlapNode) <<
                    " : " << getContig(tmp_edge.DNEE_OlapNode) <<
                    " with offsets: [" << tmp_edge.DNEE_Offset << ", " << tmp_edge.DNEE_ReturnOffset << "]" <<
                    " and state: " << sayEdgeStateLikeAHuman(tmp_edge.DNEE_State);
            }
        } while(getNextEdge(&tmp_edge));
        EDGE_STATE tmp_state = getBlockState(prev_data_block);
        std::cout << " [Ma: " << mEdgeStates[tmp_state][DN_BLOCK_STATE_MASTER_COUNT] <<
                ", Sl: " << mEdgeStates[tmp_state][DN_BLOCK_STATE_SLAVE_COUNT] <<
                ", De: " << mEdgeStates[tmp_state][DN_BLOCK_STATE_D_COUNT] <<
                ", Ba: " << mEdgeStates[tmp_state][DN_BLOCK_STATE_B_COUNT] <<
                ", To: " << mEdgeStates[tmp_state][DN_BLOCK_STATE_TOTAL_EDGE_COUNT] << "] " << tmp_state << " : " << prev_data_block << " : " << tmp_edge.DNEE_WhichData << std::endl;
        std::cout << " Nt: " << getNtRank(DID) << "\n++++" << std::endl;
        return;
    }
    else
    {
        std::cout << " No edges found\n++++" << std::endl;
        return;
    }
}
//HO 

    /*
    ** Print edges at a uninode
    */
/*HV*/ void
DualNodeMemWrapper::
/*HV*/ printLocalGraph(DualNodeId DID)
//HO ;
{
    //-----
    // print the graphviz type graph for this node
    //
    DN_EDGE_ELEM tmp_edge;
    if(getEdges(&tmp_edge, 0, 0, DID))
    {
        do {
            if(tmp_edge.DNEE_State == DN_EDGE_STATE_SLAVE)
                std::cout << "\t" << DID << " -- " << tmp_edge.DNEE_OlapNode << " [style=dashed];" << std::endl;
            else
                std::cout << "\t" << DID << " -- " << tmp_edge.DNEE_OlapNode << ";" << std::endl;
        } while(getNextEdge(&tmp_edge));
    }
}
//HO 

    /*
    ** Print information about a dualnode
    */
/*HV*/ void
DualNodeMemWrapper::
/*HV*/ printNodeInfo(DualNodeId DID)
//HO ;
{
    //-----
    // print the ID of this node and it's pair...
    //
}
//HO 
