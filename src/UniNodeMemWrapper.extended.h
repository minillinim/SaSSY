/******************************************************************************
**
** File: UniNodeMemWrapper.extended.h
**
*******************************************************************************
**
** This file implements most of the low level layer 2 code for the class which
** handles unpaired read data. It is very similar to the DualNode class with
** respect to individual DualNode ends. Both this file and the DualNode class
** files must present a very specific interface to the GenericNode class.
** 
** This class is responsible for making the edges in the uninode graph
** which are partially mirroewd in the dualnode graph
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
/*INC*/ #include <vector>
/*INC*/ 
/*INC*/ #include <map>
/*INC*/ 
/*INC*/ #include <sstream>
/*INC*/ 
/*INC*/ class GenericNodeClass;
/*INC*/ 

// local includes
#include "LoggerSimp.h"
#include "default_parameters.h"
#include "IdTypeSTL_ext.h"
#include "IdTypeDefs.h"

//INC #include "ReadStoreClass.h"
//INC 
#include "ReadStoreClass.h"

//INC #include "KmerMapClass.h"
//INC 
#include "KmerMapClass.h"

//INC #include "KmerMatchMapClass.h"
//INC 
#include "KmerMatchMapClass.h"

//INC #include "IdTypeStructs.h"
//INC 
#include "IdTypeStructs.h"


//INC class UN_EDGE_ELEM {
//INC 
//INC     public:
//INC 
//INC         UN_EDGE_ELEM() { UNEE_WhichData = UniNodeDataId(); UNEE_WhichOlap = 0; }
//INC 
//INC         // to get some insight
//INC 
//INC         void printContents(void)
//INC 
//INC         {
//INC 
//INC             std::cout << "Base node: " << UNEE_BaseNode << std::dec << " Offset wanted: " << UNEE_OffsetWanted << " State Wanted: " << UNEE_StateWanted << std::endl;
//INC             std::cout << "Overlapping node: " << UNEE_OlapNode << std::dec << " Return Offset: " << UNEE_ReturnOffset << " Offset: " << UNEE_Offset << " State: " << UNEE_State << std::endl;
//INC 
//INC         }
//INC 
//INC         void printContents(int i)
//INC 
//INC         {
//INC 
//INC             std::cout << "Base node: " << UNEE_BaseNode << std::dec << " Offset wanted: " << UNEE_OffsetWanted << " State Wanted: " << UNEE_StateWanted << std::endl;
//INC             std::cout << "Overlapping node: " << UNEE_OlapNode << std::dec << " Return Offset: " << UNEE_ReturnOffset << " Offset: " << UNEE_Offset << " State: " << UNEE_State << std::endl;
//INC             std::cout << "Next hit: " << UNEE_WhichData << " WhichOlap: " << UNEE_WhichOlap << std::endl;
//INC 
//INC         }
//INC 
//INC         UniNodeId UNEE_OlapNode;
//INC 
//INC         sMDInt UNEE_Offset;
//INC 
//INC         sMDInt UNEE_ReturnOffset;
//INC 
//INC         uMDInt UNEE_State;
//INC 
//INC         UniNodeId UNEE_BaseNode;
//INC 
//INC         sMDInt UNEE_OffsetWanted;
//INC 
//INC         uMDInt UNEE_StateWanted;
//INC 
//INC     protected:
//INC 
//INC         UniNodeDataId UNEE_WhichData;
//INC 
//INC         uMDInt UNEE_WhichOlap;
//INC 
//INC         template <int RL, int OFF>
//INC 
//INC         friend class UniNodeMemWrapperFixed;
//INC 
//INC         friend class UniNodeClass;
//INC 
//INC };
//INC 

// We need to have another element which is identical to the above but needs a little relabelling
// Behold! The walking element.

//INC #define UNWE_PrevNode UNEE_OlapNode
//INC 
//INC #define UNWE_CurrentNode UNEE_BaseNode
//INC 
//INC #define UNWE_OffsetWanted UNEE_OffsetWanted
//INC 
//INC #define UNWE_P2COffset UNEE_ReturnOffset
//INC 
//INC #define UNWE_C2POffset UNEE_Offset
//INC 
//INC #define UNWE_StateWanted UNEE_StateWanted
//INC 
//INC #define UNWE_State UNEE_State
//INC 
//INC #define UNWE_WhichData UNEE_WhichData
//INC 
//INC #define UNWE_WhichOlap UNEE_WhichOlap
//INC 
//INC class UN_WALKING_ELEM : public UN_EDGE_ELEM {
//INC 
//INC     public:
//INC 
//INC         void printContents(void)
//INC 
//INC         {
//INC 
//INC             std::cout << "Current node: " << UNWE_CurrentNode << std::dec << " Offset wanted: " << UNWE_OffsetWanted << " State Wanted: " << UNWE_StateWanted << std::endl;
//INC             std::cout << "Previous node: " << UNWE_PrevNode << std::dec << " P2COffset: " << UNWE_P2COffset << " State: " << UNWE_State << std::endl;
//INC 
//INC         }
//INC 
//INC         void printContents(int i)
//INC 
//INC         {
//INC 
//INC             std::cout << "Current node: " << UNWE_CurrentNode << std::dec << " Offset wanted: " << UNWE_OffsetWanted << " State Wanted: " << UNWE_StateWanted << std::endl;
//INC             std::cout << "Previous node: " << UNWE_PrevNode << std::dec << " C2POffset: " << UNWE_C2POffset << " P2COffset: " << UNWE_P2COffset << " State: " << UNWE_State << std::endl;
//INC             std::cout << "Next hit: " << UNWE_WhichData << " WhichOlap: " << UNWE_WhichOlap << std::endl;
//INC 
//INC         }
//INC };
//INC 
/*INC*/ typedef uMDInt EDGE_STATE;
/*INC*/ 
//PV uMDInt mEdgeStates[UN_BLOCK_STATE_HEIGHT][UN_BLOCK_STATE_WIDTH];  // finite state matrix for edge states
//PV 
//PV ReadStoreClass * mReadStore;
//PV 
//PV KmerMapClass * mKmerMap;
//PV 
//PV KmerMatchMapClass * mKmerMatchMap;
//PV 
//PV uMDInt mNaiveOffset;
//PV 
//PV uMDInt mCurrentMaxOffset;
//PV 
//PV char * UMWG_ME_RAC, * UMWG_ME_RAC_saved;
//PV 
/******************************************************************************
** INTIALISATION AND SAVING
******************************************************************************/
//VO virtual
template <int RL, int OFF>
/*HV*/ void
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ extendedDestructor(void)
//HO ;
//VO = 0;
{
    //-----
    // Add anything you would like to happen when the destructor is called
    //

    // delete the kmer helper objects
    if(mKmerMap != NULL)
        delete mKmerMap;
    mKmerMap = NULL;
    
    if(mKmerMatchMap != NULL)
        delete mKmerMatchMap;
    mKmerMatchMap = NULL;
    
    if(UMWG_ME_RAC_saved != NULL)
        delete UMWG_ME_RAC_saved;
    UMWG_ME_RAC_saved = NULL;
}
//HO 
//VO 

//VO virtual
template <int RL, int OFF>
/*HV*/ bool
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ extendedInitialise(uMDInt originalNumberOfBlocks)
//HO ;
//VO = 0;
{
    //-----
    // Add anything you would like to happen on a regular intialisation call
    //
    // set the pointers to null
    mReadStore = NULL;
    mKmerMap = NULL;
    mKmerMatchMap = NULL;
    
    mNaiveOffset = SAS_DEF_UN_OFFSET_DEF_MIN;
    
    // intialise the EDGE STATE MATRIX
    initEdgeStates();

    // the char aray used to make edges
    UMWG_ME_RAC = NULL;
    UMWG_ME_RAC_saved = NULL;

    return true;
}
//HO 
//VO 

//VO virtual
template <int RL, int OFF>
/*HV*/ bool
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ extendedInitialise(std::string fileName)
//HO ;
//VO = 0;
{
    //-----
    // Add anything you would like to happen when reloading a saved instance
    //

    // set the pointers to null
    mReadStore = NULL;

    // intialise the EDGE STATE MATRIX
    initEdgeStates();

    mKmerMap = NULL;
    mKmerMatchMap = NULL;
       
    mNaiveOffset = SAS_DEF_UN_OFFSET_DEF_MIN;
    
    // the char aray used to make edges
    UMWG_ME_RAC = NULL;
    UMWG_ME_RAC_saved = NULL;
    
    // load the kmer helper object
    //mKmerMap = new KmerMapClass();
    //mKmerMap->initialise(fileName + ".kmerMap");

    //mKmerMatchMap = new KmerMatchMapClass();
    //mKmerMatchMap->initialise(SAS_DEF_UN_MM_SIZE);

    return true;
}
//HO 
//VO 

//VO virtual
template <int RL, int OFF>
/*HV*/ bool
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ extendedSave(std::string fileName)
//HO ;
//VO = 0;
{
    //-----
    // Add anything you would like to happen when saving (including saving private vars)
    //
    
    // save the kmer helper object
    //mKmerMap->save(fileName + ".kmerMap");

    return true;
}
//HO 
//VO 

//VO virtual
template <int RL, int OFF>
/*HV*/ void
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ setObjectPointers(ReadStoreClass * RSC)
//HO ;
//VO = 0;
{
    //-----
    // This object may need access to other high level objects
    // the pointers are member variables and we can set them here
    // no pointers set here can be deleted
    //

    // set the main ReadStore class
    PARANOID_ASSERT_L2(mReadStore == NULL);
    mReadStore = RSC;
    setCurrentMaxOffset( 0 );
    
    // make the char array for use in making edges
    UMWG_ME_RAC = new char [RL+1];
    UMWG_ME_RAC_saved = UMWG_ME_RAC;    
}
//HO 
//VO 

/******************************************************************************
** HOUSEKEEPING
******************************************************************************/
    /*
    ** Add a node to to UniNode object
    */
//VO virtual
template <int RL, int OFF>
/*HV*/ UniNodeId
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ addNode(ReadStoreId RID)
//HO ;
//VO = 0;
{
    //-----
    // Add a node to to UniNode object
    //
    UniNodeId new_id = newUniNodeId();
    setRs(RID, new_id);
    return new_id;
}
//HO 
//VO 

//VO virtual
template <int RL, int OFF>
/*HV*/ bool
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ shouldBeBanished(UniNodeId UID)
//HO ;
//VO = 0;
{
    //-----
    // returns true if a node should be banished
    //
    if(getNodeType(UID) == UN_NODE_TYPE_BANISHED)
        return false;
    if((getTRank(UID)) < UN_BANISH_CUT)
        return true;                                                // sparse so we banish
//    if(!isHasDualNode(UID))
  //      return true;                                                // floating so we banish
    return false;
}
//HO 
//VO 

//VO virtual
template <int RL, int OFF>
/*HV*/ bool
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ getUniNodeId(UniNodeId * UID, std::string * sequence)
//HO ;
//VO = 0;
{
    //-----
    // Get the UniNodeId for a particular sequence
    //
    ReadStoreId RID;
    if(mReadStore->getReadStoreId(&RID, sequence))
    {
        PARANOID_ASSERT_PRINT_L2((RID != mReadStore->getNullReadStoreId()), "NULL RID for sequence: " << *sequence)
        *UID = mReadStore->getUn(RID);
        // there is always the chance that there will be no UID set for this RSID.
        // it's OK and due to the way data is loaded. However we'll need to check
        // for it here and act accordingly
        return true;
    }
    // the sequence isn't in the store
    return false;
}
//HO 
//VO 

//VO virtual
template <int RL, int OFF>
/*HV*/ void
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ setCurrentMaxOffset(uMDInt mo)
//HO ;
//VO = 0;
{ mCurrentMaxOffset = mo; }
//HO 
//VO 

//VO virtual
template <int RL, int OFF>
/*HV*/ void
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ setNaiveOffset(uMDInt no)
//HO ;
//VO = 0;
{ mNaiveOffset = no; }
//HO 
//VO 

/******************************************************************************
** NAVIGATION
******************************************************************************/
    /*
    ** Initialise the iterator for a UniNodes overlapping edges
    */
//VO virtual
template <int RL, int OFF>
/*HV*/ bool
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ getEdges(UN_EDGE_ELEM * data, uMDInt edgeState, sMDInt offSign, UniNodeId UID)
//HO ;
//VO = 0;
{
    //-----
    // Get the first olap at address which matches the criteria, if edgeState or offSign == 0
    // then we don't care and this puppy will start us on the road to return all the overlaps
    //

    // save these guys for a rainy day
    data->UNEE_BaseNode = UID;
    data->UNEE_StateWanted = edgeState;
    data->UNEE_OffsetWanted = offSign;

    // holders for the current edge
    sMDInt offset = 0;
    uMDInt state = -1;
    
    int num_used = 0;

    UniNodeDataId data_block = getNextDataBlock(UID);

    while(UND_NULL_ID != data_block)
    {
        num_used = getNumUsedInDataBlock(data_block);
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
                    data->UNEE_OlapNode = getXthEdge(focused_olap, data_block);
                    data->UNEE_State = state;
                    data->UNEE_Offset = offset;
                    if(getXthReturnOffset(focused_olap, data_block))
                    {
                        // return offset is a match
                        data->UNEE_ReturnOffset = offset;
                    }
                    else
                    {
                        // offset is a different sign
                        data->UNEE_ReturnOffset = offset * -1;
                    }

                    PARANOID_ASSERT_L2((offset <= OFF));
                    
                    // set the reserved values up correctly...
                    data->UNEE_WhichData = data_block;
                    data->UNEE_WhichOlap = focused_olap;
            
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
//VO 
    
    /*
    ** Get the next edge
    */
//VO virtual
template <int RL, int OFF>
/*HV*/ bool
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ getNextEdge(UN_EDGE_ELEM * data)
//HO ;
//VO = 0;
{
    //-----
    // call this AFTER getEdges to get the next olap of the same type as specified in the
    // original call.
    //
    // First we have to finish off any existing block
    //

    // holders for the current edge
    sMDInt offset = 0;
    uMDInt state = -1;

    UniNodeDataId data_block = data->UNEE_WhichData;
    sMDInt w_off = data->UNEE_OffsetWanted;
    uMDInt w_state = data->UNEE_StateWanted;
    int focused_olap = data->UNEE_WhichOlap + 1;
    int num_used = getNumUsedInDataBlock(data_block);
    
    if(focused_olap > num_used)
    {
        // we have used all the places in this data block
        data_block = getNextDataBlock(data_block);
        focused_olap = 1;
        num_used = getNumUsedInDataBlock(data_block);
    }
    
    while(UND_NULL_ID != data_block)
    {
        PARANOID_ASSERT_L2((num_used > 0));
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
                    data->UNEE_OlapNode = getXthEdge(focused_olap, data_block);
                    data->UNEE_Offset = offset;
                    data->UNEE_State = state;
                    if(getXthReturnOffset(focused_olap, data_block))
                    {
                        // return offset is a match
                        data->UNEE_ReturnOffset = offset;
                    }
                    else
                    {
                        // offset is a different sign
                        data->UNEE_ReturnOffset = offset * -1;
                    }

                    PARANOID_ASSERT_L2((offset <= OFF));
                    
                    // set the reserved values up correctly...
                    data->UNEE_WhichData = data_block;
                    data->UNEE_WhichOlap = focused_olap;
            
                    // done!
                    return true;
                }
            }
            focused_olap++;
        }
        data_block = getNextDataBlock(data_block);
        if(UND_NULL_ID == data_block)
            break;
        focused_olap = 1;
        num_used = getNumUsedInDataBlock(data_block);
    }
    // if we are here then we didn't find what we were looking for, return false.
    return false;
}
//HO 
//VO 
    
/******************************************************************************
** EDGE STATES -- GET AND SET
******************************************************************************/

    /*
    ** Get overlapping node Ids from a data block
    */
//VO virtual
template <int RL, int OFF>
/*HV*/ UniNodeId
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ getXthEdge(int whichOlap, UniNodeDataId UDID)
//HO ;
//VO = 0;
{
    //-----
    // Return the Xth Overlapping node
    //
    PARANOID_ASSERT_L2((whichOlap <= UN_MAX_OLAPS_IN_DATA));
    PARANOID_ASSERT_L2((whichOlap > 0));
    switch(whichOlap)
    {
        case 1:
        {
            return getUn1_olap(UDID);
        }
        case 2:
        {
            return getUn2_olap(UDID);
        }
        case 3:
        {
            return getUn3_olap(UDID);
        }
        default:
        {
            PARANOID_INFO_L2("Edge is bad news: " << whichOlap);
            logError("Edge is bad news: " << whichOlap);
            return UN_NULL_ID;
        }
    }
}
//HO 
//VO 

    /*
    ** Get edge offsets from a data block
    */
//VO virtual
template <int RL, int OFF>
/*HV*/ sMDInt
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ getXthOffset(int whichOlap, UniNodeDataId UDID)
//HO ;
//VO = 0;
{
    //-----
    // Return the Xth Overlapping node's offset
    //
    PARANOID_ASSERT_L2((whichOlap <= UN_MAX_OLAPS_IN_DATA));
    PARANOID_ASSERT_L2((whichOlap > 0));
    switch(whichOlap)
    {
        case 1:
        {
            return getUn1_offset(UDID);
        }
        case 2:
        {
            return getUn2_offset(UDID);
        }
        case 3:
        {
            return getUn3_offset(UDID);
        }
        default:
        {
            PARANOID_INFO_L2("Offset is bad news: " << whichOlap);
            logError("Offset is bad news: " << whichOlap);
            return 0;
        }
    }
}
//HO 
//VO 

    /*
    ** Get edge states from a data block
    */
//VO virtual
template <int RL, int OFF>
/*HV*/ uMDInt
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ getXthState(int whichOlap, UniNodeDataId UDID)
//HO ;
//VO = 0;
{
    //-----
    // Return the Xth Overlapping node's edge state
    //
    PARANOID_ASSERT_L2((whichOlap <= UN_MAX_OLAPS_IN_DATA));
    PARANOID_ASSERT_L2((whichOlap > 0));
    switch(whichOlap)
    {
        case 1:
        {
            return getUn1_state(UDID);
        }
        case 2:
        {
            return getUn2_state(UDID);
        }
        case 3:
        {
            return getUn3_state(UDID);
        }
        default:
        {
            PARANOID_INFO_L2("State is bad news: " << whichOlap);
            logError("State is bad news: " << whichOlap);
            return -1;
        }
    }
}
//HO 
//VO 


    /*
    ** Get the flag which says if the return offset is the same sign
    */
//VO virtual
template <int RL, int OFF>
/*HV*/ bool
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ getXthReturnOffset(int whichOlap, UniNodeDataId UDID)
//HO ;
//VO = 0;
{
    //-----
    // Return the Xth Overlapping node's edge state
    //
    PARANOID_ASSERT_L2((whichOlap <= UN_MAX_OLAPS_IN_DATA));
    PARANOID_ASSERT_L2((whichOlap > 0));
    switch(whichOlap)
    {
        case 1:
        {
            return isUn1_returnOffset(UDID);
        }
        case 2:
        {
            return isUn2_returnOffset(UDID);
        }
        case 3:
        {
            return isUn3_returnOffset(UDID);
        }
        default:
        {
            PARANOID_INFO_L2("Return offset is bad news: " << whichOlap);
            logError("Return offset is bad news: " << whichOlap);
            return false;
        }
    }
}
//HO 
//VO 

    /*
    ** Set overlapping node Ids from a data block
    */
//VO virtual
template <int RL, int OFF>
/*HV*/ void
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ setXthEdge(UniNodeId UID, int whichOlap, UniNodeDataId UDID)
//HO ;
//VO = 0;
{
    //-----
    // Set the Xth Overlapping node
    //
    PARANOID_ASSERT_L2((whichOlap <= UN_MAX_OLAPS_IN_DATA));
    PARANOID_ASSERT_L2((whichOlap > 0));
    switch(whichOlap)
    {
        case 1:
        {
            setUn1_olap(UID, UDID);
            break;
        }
        case 2:
        {
            setUn2_olap(UID, UDID);
            break;
        }
        case 3:
        {
            setUn3_olap(UID, UDID);
            break;
        }
        default:
        {
            PARANOID_INFO_L2("Edge is bad news: " << whichOlap);
            logError("Edge is bad news: " << whichOlap);
        }
    }
}
//HO 
//VO 

    /*
    ** Set edge offsets from a data block
    */
//VO virtual
template <int RL, int OFF>
/*HV*/ void
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ setXthOffset(sMDInt offset, int whichOlap, UniNodeDataId UDID)
//HO ;
//VO = 0;
{
    //-----
    // Set the Xth Overlapping node's offset
    //
    PARANOID_ASSERT_L2((whichOlap <= UN_MAX_OLAPS_IN_DATA));
    PARANOID_ASSERT_L2((whichOlap > 0));
    switch(whichOlap)
    {
        case 1:
        {
            setUn1_offset(offset, UDID);
            break;
        }
        case 2:
        {
            setUn2_offset(offset, UDID);
            break;
        }
        case 3:
        {
            setUn3_offset(offset, UDID);
            break;
        }
        default:
        {
            PARANOID_INFO_L2("Offset is bad news: " << whichOlap);
            logError("Offset is bad news: " << whichOlap);
        }
    }
}
//HO 
//VO 

    /*
    ** set edge states from a data block
    */
//VO virtual
template <int RL, int OFF>
/*HV*/ void
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ setXthState(EDGE_STATE state, int whichOlap, UniNodeDataId UDID)
//HO ;
//VO = 0;
{
    //-----
    // Set the Xth Overlapping node's edge state
    //
    PARANOID_ASSERT_L2((whichOlap <= UN_MAX_OLAPS_IN_DATA));
    PARANOID_ASSERT_L2((whichOlap > 0));
    switch(whichOlap)
    {
        case 1:
        {
            setUn1_state(state, UDID);
            break;
        }
        case 2:
        {
            setUn2_state(state, UDID);
            break;
        }
        case 3:
        {
            setUn3_state(state, UDID);
            break;
        }
        default:
        {
            PARANOID_INFO_L2("State is bad news: " << whichOlap);
            logError("State is bad news: " << whichOlap);
        }
    }
}
//HO 
//VO 

    /*
    ** Set the flag which says if the return offset is the same as forward offset
    */
//VO virtual
template <int RL, int OFF>
/*HV*/ void
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ setXthReturnOffset(bool sameOffset, int whichOlap, UniNodeDataId UDID)
//HO ;
//VO = 0;
{
    //-----
    // Set the Xth Overlapping node's offset
    //
    PARANOID_ASSERT_L2((whichOlap <= UN_MAX_OLAPS_IN_DATA));
    PARANOID_ASSERT_L2((whichOlap > 0));
    switch(whichOlap)
    {
        case 1:
        {
            setUn1_returnOffset(sameOffset, UDID);
            break;
        }
        case 2:
        {
            setUn2_returnOffset(sameOffset, UDID);
            break;
        }
        case 3:
        {
            setUn3_returnOffset(sameOffset, UDID);
            break;
        }
        default:
        {
            PARANOID_INFO_L2("Return offset is bad news: " << whichOlap);
            logError("Return offset is bad news: " << whichOlap);
        }
    }
}
//HO 
//VO 

    /*
    ** Get the offset from node A to node B
    */
//VO virtual
template <int RL, int OFF>
/*HV*/ sMDInt
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ getOffsetBetweenNodes(UniNodeId nodeB, EDGE_STATE state, UniNodeId nodeA)
//HO ;
//VO = 0;
{
    //-----
    // Sometimes we just need to get the offset between two nodes
    // this function does just that From A to B
    //
    UN_EDGE_ELEM tmp_edge;
    if(getEdges(&tmp_edge, state, 0, nodeA))
    {
        do {
            if(tmp_edge.UNEE_OlapNode == nodeB)
                return tmp_edge.UNEE_Offset;
        } while(getNextEdge(&tmp_edge));
    }
    return (sMDInt)0;
}
//HO 
//VO 

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
    ** This information can be stored in a UN_DATA_BLOCK_STATE_HEIGHT * UN_DATA_BLOCK_STATE_WIDTH matrix
    ** it is called mEdgeStates, it is initialised in initEdgeStates(). The error state is (##) 35
    */

    /*
    ** Initialise the edge states matrix
    */
//VO virtual
template <int RL, int OFF>
/*HV*/ void
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ initEdgeStates(void)
//HO ;
//VO = 0;
{
    //-----
    // initialise the EDGE STATES matrix
    //
    
    uMDInt temp_states[UN_BLOCK_STATE_HEIGHT][UN_BLOCK_STATE_WIDTH] =
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
    
    for(int i = 0; i < UN_BLOCK_STATE_HEIGHT; i++)
    {
        for(int j = 0; j < UN_BLOCK_STATE_WIDTH; j++)
        {
            mEdgeStates[i][j] = temp_states[i][j];
        }
    }
}
//HO 
//VO 

    /*
    ** Get the number of edge spaces used in a data block
    */
//VO virtual
template <int RL, int OFF>
/*HV*/ int
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ getNumUsedInDataBlock(UniNodeDataId UDID)
//HO ;
//VO = 0;
{
    //-----
    // Get the number of places used in a data block
    //
    return mEdgeStates[getBlockState(UDID)][UN_BLOCK_STATE_TOTAL_EDGE_COUNT];
}
//HO 
//VO 

    /*
    ** Set the state of an edge for a UniNode
    */
//VO virtual
template <int RL, int OFF>
/*HV*/ bool
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ setEdgeState_asymmetrical(EDGE_STATE newEdgeState, int whichOlap, UniNodeDataId UDID)
//HO ;
//VO = 0;
{
    //-----
    // set the state of the olap whichOlap in the data block UDID
    // also update the block state
    // no checking is done
    //
    PARANOID_ASSERT_L2((UDID != UND_NULL_ID));
    PARANOID_ASSERT_L2((whichOlap > 0));
    PARANOID_ASSERT_L2((whichOlap <= UN_MAX_OLAPS_IN_DATA));
    PARANOID_ASSERT_PRINT_L2(newEdgeState >= 0, newEdgeState);
    PARANOID_ASSERT_L2((newEdgeState < 4));

    uMDInt block_state = getBlockState(UDID);
    uMDInt current_edge_state = 0;
    int state_change_action = 0;

    // get the current state
    switch(whichOlap)
    {
        case 1:
        {
            current_edge_state = getUn1_state(UDID);
            break;
        }
        case 2:
        {
            current_edge_state = getUn2_state(UDID);
            break;
        }
        case 3:
        {
            current_edge_state = getUn3_state(UDID);
            break;
        }
    }

    // work out what we're doing
    switch(newEdgeState)
    {
        case UN_EDGE_STATE_NTRANS:
        {
            switch(current_edge_state)
            {
                case UN_EDGE_STATE_NTRANS:
                {
                    logError("Useless state change: " << sayEdgeStateLikeAHuman(current_edge_state) << " --> " << sayEdgeStateLikeAHuman(newEdgeState));
                    return false;
                }
                case UN_EDGE_STATE_TRANS:
                {
                    state_change_action = UN_BLOCK_STATE_T_TO_NT;
                    break;
                }
                case UN_EDGE_STATE_DETACHED:
                {
                    state_change_action = UN_BLOCK_STATE_D_TO_NT;
                    break;
                }
                case UN_EDGE_STATE_BANISHED:
                {
                    logError("Trying to change the state of a banished edge");
                    return false;
                }
            }
            break;
        } 
        case UN_EDGE_STATE_TRANS:
        {
            switch(current_edge_state)
            {
                case UN_EDGE_STATE_NTRANS:
                {
                    state_change_action = UN_BLOCK_STATE_NT_TO_T;
                    break;
                }
                case UN_EDGE_STATE_TRANS:
                {
                    logError("Useless state change: " << sayEdgeStateLikeAHuman(current_edge_state) << " --> " << sayEdgeStateLikeAHuman(newEdgeState));
                    return false;
                }
                case UN_EDGE_STATE_DETACHED:
                {
                    state_change_action = UN_BLOCK_STATE_D_TO_T;
                    break;
                }
                case UN_EDGE_STATE_BANISHED:
                {
                    logError("Trying to change the state of a banished edge");
                    return false;
                }
            }
            break;
        }
        case UN_EDGE_STATE_DETACHED:
        {
            switch(current_edge_state)
            {
                case UN_EDGE_STATE_NTRANS:
                {
                    state_change_action = UN_BLOCK_STATE_NT_TO_D;
                    break;
                }
                case UN_EDGE_STATE_TRANS:
                {
                    state_change_action = UN_BLOCK_STATE_T_TO_D;
                    break;
                }
                case UN_EDGE_STATE_DETACHED:
                {
                    logError("Useless state change: " << sayEdgeStateLikeAHuman(current_edge_state) << " --> " << sayEdgeStateLikeAHuman(newEdgeState));
                    return false;
                }
                case UN_EDGE_STATE_BANISHED:
                {
                    logError("Trying to change the state of a banished edge");
                    return false;
                }
            }
            break;
        }
        case UN_EDGE_STATE_BANISHED:
        {
            switch(current_edge_state)
            {
                case UN_EDGE_STATE_NTRANS:
                {
                    state_change_action = UN_BLOCK_STATE_NT_TO_B;
                    break;
                }
                case UN_EDGE_STATE_TRANS:
                {
                    state_change_action = UN_BLOCK_STATE_T_TO_B;
                    break;
                }
                case UN_EDGE_STATE_DETACHED:
                {
                    state_change_action = UN_BLOCK_STATE_D_TO_B;
                    break;
                }
                case UN_EDGE_STATE_BANISHED:
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
    PARANOID_ASSERT_L2((new_block_state != UN_BLOCK_STATE_ERROR));

    // set the new state for the overlap
    switch(whichOlap)
    {
        case 1:
        {
            setUn1_state(newEdgeState, UDID);
            break;
        }
        case 2:
        {
            setUn2_state(newEdgeState, UDID);
            break;
        }
        case 3:
        {
            setUn3_state(newEdgeState, UDID);
            break;
        }
    }

    // and for the edge
    setBlockState(new_block_state, UDID);
    return true;
}
//HO 
//VO 

    /*
    ** Mark a transitive edge as non-transitive (one way function)
    */
//VO virtual
//HV inline bool promoteEdge_asymmetrical(int whichOlap, UniNodeDataId UDID)
//VO = 0;
//HO { return setEdgeState_asymmetrical(UN_EDGE_STATE_NTRANS , whichOlap, UDID); }
//HO 
//VO 
    
    /*
    ** Mark the edge from node B to node A as non-transitive (one way function)
    */
//VO virtual
template <int RL, int OFF>
/*HV*/ bool
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ promoteEdge_asymmetrical(UniNodeId nodeA, UniNodeId nodeB)
//HO ;
//VO = 0;
{
    //-----
    // Mark the edge from node B to node A as non-transitive (one way function)
    //
    // first get the edge from A to B
    UN_EDGE_ELEM tmp_edge;
    // get the edge from B to A
    if(getEdges(&tmp_edge, 0, 0, nodeB))
    {
        do {
            if(tmp_edge.UNEE_OlapNode == nodeA)
            {
                return promoteEdge_asymmetrical(tmp_edge.UNEE_WhichOlap, tmp_edge.UNEE_WhichData);
            }
        }while(getNextEdge(&tmp_edge));
    }
    PARANOID_INFO_L2("promoteEdge_asymmetrical from " << nodeB << " to " << nodeA << " edge not found");
    return false;
}
//HO 
//VO 
    
    /*
    ** Mark the edge betwen two nodes as non-transitive
    */
//VO virtual
template <int RL, int OFF>
/*HV*/ bool
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ promoteEdgeBetween_symmetrical(UniNodeId nodeA, UniNodeId nodeB)
//HO ;
//VO = 0;
{
    //-----
    // Mark the edge betwen two nodes as non-transitive
    //
    bool ret_value = true;
    ret_value &= promoteEdge_asymmetrical(nodeA, nodeB);
    PARANOID_ASSERT_L2((ret_value));
    ret_value &= promoteEdge_asymmetrical(nodeB, nodeA);
    PARANOID_ASSERT_L2((ret_value));
    incNtRank(nodeA);
    incNtRank(nodeB);
    decTRank(nodeA);
    decTRank(nodeB);
    return ret_value;
}
//HO 
//VO 
    
    /*
    ** Mark a non-transitive edge as transitive (one way function)
    */
//VO virtual
//HV inline bool demoteEdge_asymmetrical(int whichOlap, UniNodeDataId UDID)
//VO = 0;
//HO { return setEdgeState_asymmetrical(UN_EDGE_STATE_TRANS , whichOlap, UDID); }
//HO
//VO 
    
    /*
    ** Mark the edge from node B to node A as transitive (one way function)
    */
//VO virtual
template <int RL, int OFF>
/*HV*/ bool
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ demoteEdge_asymmetrical(UniNodeId nodeA, UniNodeId nodeB)
//HO ;
//VO = 0;
{
    //-----
    // Mark the edge from node B to node A as transitive (one way function)
    //
    // first get the edge from A to B
    UN_EDGE_ELEM tmp_edge;
    // get the edge from B to A
    if(getEdges(&tmp_edge, 0, 0, nodeB))
    {
        do {
            if(tmp_edge.UNEE_OlapNode == nodeA)
            {
                return demoteEdge_asymmetrical(tmp_edge.UNEE_WhichOlap, tmp_edge.UNEE_WhichData);
            }
        }while(getNextEdge(&tmp_edge));
    }
    PARANOID_INFO_L2("demoteEdge_asymmetrical from " << nodeB << " to " << nodeA << " edge not found");
    return false;
}
//HO 
//VO 
        
    /*
    ** Mark the edge betwen two nodes as transitive
    */
//VO virtual
        template <int RL, int OFF>
/*HV*/ bool
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ demoteEdgeBetween_symmetrical(UniNodeId nodeA, UniNodeId nodeB)
//HO ;
//VO = 0;
{
    //-----
    // Mark the edge betwen two nodes as transitive
    //
    bool ret_value = true;
    ret_value &= demoteEdge_asymmetrical(nodeA, nodeB);
    ret_value &= demoteEdge_asymmetrical(nodeB, nodeA);
    PARANOID_ASSERT_L2((ret_value));
    decNtRank(nodeA);
    decNtRank(nodeB);
    incTRank(nodeA);
    incTRank(nodeB);
    return ret_value;
}
//HO 
//VO 

    /*
    ** Mark the edge from baseNode to searchNode as transitive if it is non-transitive
    */
//VO virtual
template <int RL, int OFF>
/*HV*/ bool
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ demoteEdgeIfNodeFound_symmetrical(UniNodeId searchNode, UniNodeId baseNode)
//HO ;
//VO = 0;
{
    //-----
    // check out all the non-transitive edges at the baseNode
    // if searchNode is in the list, then demote the
    // edge between them back to a regular transitive edge
    //
    // called in labelNonTransitiveEdges
    //
    // returns true if we banished the baseNode
    //
    //printEdges(baseNode);
    UN_EDGE_ELEM tmp_edge;
    if(getEdges(&tmp_edge, UN_EDGE_STATE_NTRANS, 0, baseNode))
    {
        do {
            if(tmp_edge.UNEE_OlapNode == searchNode)
            {
                if(1 != fAbs(tmp_edge.UNEE_Offset))
                {
                    demoteEdge_asymmetrical(tmp_edge.UNEE_WhichOlap, tmp_edge.UNEE_WhichData);
                    demoteEdge_asymmetrical(baseNode, searchNode);
                    decNtRank(baseNode);
                    decNtRank(searchNode);
                    incTRank(baseNode);
                    incTRank(searchNode);
                    return false;
                }
                else
                {
                    // there is no real way to get around this kind of situation
                    // we just need to banish the base
                    banishNode(baseNode);
                    return true;
                }
            }
        } while(getNextEdge(&tmp_edge));
    }
    return false;
}
//HO 
//VO 
     
    /*
    ** Detach an edge from the graph (one way function)
    */
//VO virtual
//HV bool detachEdge_asymmetrical(int whichOlap, UniNodeDataId UDID)
//VO = 0;
//HO { return setEdgeState_asymmetrical(UN_EDGE_STATE_DETACHED , whichOlap, UDID); }
//HO
//VO 
    
    /*
    ** Detach the edge from node B to node A (one way function)
    */
//VO virtual
template <int RL, int OFF>
/*HV*/ bool
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ detachEdge_asymmetrical(UniNodeId nodeA, UniNodeId nodeB)
//HO ;
//VO = 0;
{
    //-----
    // Detach the edge from node B to node A (one way function)
    //
    // first get the edge from A to B
    UN_EDGE_ELEM tmp_edge;
    // get the edge from B to A
    if(getEdges(&tmp_edge, 0, 0, nodeB))
    {
        do {
            if(tmp_edge.UNEE_OlapNode == nodeA)
            {
                return detachEdge_asymmetrical(tmp_edge.UNEE_WhichOlap, tmp_edge.UNEE_WhichData);
            }
        }while(getNextEdge(&tmp_edge));
    }
    PARANOID_INFO_L2("detachEdge_asymmetrical from " << nodeB << " to " << nodeA << " edge not found");
    return false;
}
//HO 
//VO 

    /*
    ** Detach the edge betwen two nodes
    */
//VO virtual
        template <int RL, int OFF>
/*HV*/ bool
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ detachEdgeBetween_symmetrical(UniNodeId nodeA, UniNodeId nodeB)
//HO ;
//VO = 0;
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
//VO 

    /*
    ** Banish an edge from the graph
    */
//VO virtual
//HV inline bool banishEdge_asymmetrical(int whichOlap, UniNodeDataId UDID)
//VO = 0;
//HO { return setEdgeState_asymmetrical(UN_EDGE_STATE_BANISHED , whichOlap, UDID); }
//HO
//VO 
    
    /*
    ** Banish the edge from node B to node A (one way function)
    */
//VO virtual
template <int RL, int OFF>
/*HV*/ bool
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ banishEdge_asymmetrical(UniNodeId nodeA, UniNodeId nodeB)
//HO ;
//VO = 0;
{
    //-----
    // Detach the edge from node B to node A (one way function)
    //
    // first get the edge from A to B
    UN_EDGE_ELEM tmp_edge;
    // get the edge from B to A
    if(getEdges(&tmp_edge, 0, 0, nodeB))
    {
        do {
            if(tmp_edge.UNEE_OlapNode == nodeA)
            {
                return banishEdge_asymmetrical(tmp_edge.UNEE_WhichOlap, tmp_edge.UNEE_WhichData);
            }
        }while(getNextEdge(&tmp_edge));
    }
    PARANOID_INFO_L2("banishEdge_asymmetrical from " << nodeB << " to " << nodeA << " edge not found");
    return false;
}
//HO 
//VO 

    /*
    ** Banish the edge betwen two nodes
    */
//VO virtual
template <int RL, int OFF>
/*HV*/ bool
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ banishEdgeBetween_symmetrical(UniNodeId nodeA, UniNodeId nodeB)
//HO ;
//VO = 0;
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
//VO 

/******************************************************************************
** EDGE MAKING AND MANAGEMENT
******************************************************************************/

    /*
    ** Make the edges for a UniNode
    */

//VO virtual
template <int RL, int OFF>
/*HV*/ void
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ prepForEdgeMaking(uMDInt kmerMapSize)
//HO ;
//VO = 0;
{
    //-----
    // go through all the non-banished nodes and make some edges
    //
    // make the helper classes
    if(mKmerMatchMap != NULL)
        delete mKmerMatchMap;
    mKmerMatchMap = new KmerMatchMapClass();
    PARANOID_ASSERT_L2(mKmerMatchMap != NULL);
    mKmerMatchMap->initialise(SAS_DEF_UN_KMM_SIZE);

    if(mKmerMap != NULL)
        delete mKmerMap;
    mKmerMap = new KmerMapClass();
    PARANOID_ASSERT_L2(mKmerMap != NULL);
    mKmerMap->initialise(kmerMapSize);
    mKmerMap->initialise2(UN_KMER_LENGTH, mReadStore);
}
//HO 

//VO virtual
template <int RL, int OFF>
/*HV*/ bool
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ makeEdges(std::vector<UniNodeId> * banishVec, UniNodeId UID)
//HO ;
//VO = 0;
{
    //-----
    // This function performs probably the most important part of the algorithm
    // All the functions to the left and right of this guy have caused me no end
    // of grief over the past couple of years. It's late Nov. 2009 and here I am 
    // once more having just found a pretty major bug in this code. Expect more.
    // Ever more subtle...
    //
    // May 2010. realised a way to cut down the memory 8-fold in this code...
    // june 2010. Yeeha, here I am again...
    // ... added the banishVec cause the node lists above in Generic node need to know that a node has been banished
    //
    // There are two parts to this function, the first cuts Kmers from the read
    // and the second uses both these new kmers and stored information about 
    // previously stored kmers to make all the edges. Two main objects are used.
    // The first is called a kmer map, it stores kmers in the same way as a readstore
    // does however it does not store the reads, just links into the readstore; This guy persists.
    // The second is called a kmer match map. It is made new for every new UID. 
    // The match map is activated when we see a similarity with another read. We
    // know whent this happens cause they will share similar kmers...
    // What we store in the match map is information about what kmer we need
    // to seal the deal. 
    //
    // For example with the overlapping reads:
    //   AATAATAATAATAATAATAATAACGAAAACA        -- A
    //  AAATAATAATAATAATAATAATAACGAAAAC         -- B
    //
    // once we see the first kmer : AATAATAATAATAATAA at position 0 in A and position 1 in B
    // we can say that if we find another matching kmer at the last position in B which exactly
    // matches ther kmer at the second last position in A then they must overlap. 
    // Orientation information is also used and stored BUt using this simple method
    // we can work out overlaps, orienatations and offsets with EASE*
    //          *provided you are not the developer
    //
    // First we cut kmers from the sequence then we use these to
    // determine overlaps, finally we make edges
    //
    // When we cut kmers from a read it is like this...
    //
    //  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
    //  ------------------------------
    //  XXXXXXXXXXXXXXXXXXXXXXXXXX
    //   XXXXXXXXXXXXXXXXXXXXXXXXXX
    //    XXXXXXXXXXXXXXXXXXXXXXXXXX
    //     XXXXXXXXXXXXXXXXXXXXXXXXXX
    //      XXXXXXXXXXXXXXXXXXXXXXXXXX    
    //
    // so we break the job into three parts
    //
    //  XXXX|XXXXXXXXXXXXXXXXXXXXXX|XXXX
    //  ----|----------------------|----
    //  XXXX|XXXXXXXXXXXXXXXXXXXXXX|
    //   XXX|XXXXXXXXXXXXXXXXXXXXXX|X
    //    XX|XXXXXXXXXXXXXXXXXXXXXX|XX
    //     X|XXXXXXXXXXXXXXXXXXXXXX|XXX
    //      |XXXXXXXXXXXXXXXXXXXXXX|XXXX
    //
    // the first and last part may be a little slow but the middle part can fly through...
    //
    banishVec->clear();
   
    int kmer_offsets[UN_NUM_CUTS];                                  // use these offsets when we cut kmers, they are a component of the algorithm
    char kmers[UN_NUM_CUTS][UN_KMER_LENGTH + 1];                    // array for building kmers
    bool kmer_orients[UN_NUM_CUTS];                                 // stores the orientations of the cut kmers

    // get hold of the global char array...
    UMWG_ME_RAC = UMWG_ME_RAC_saved;
    strcpy (UMWG_ME_RAC, (mReadStore->getSequence(getRs(UID))).c_str());

    //const char * rr1 = (mReadStore->getSequence(getRs(UID))).c_str();
    int read_counter = 0;
    
    for(int i = 0; i < UN_NUM_CUTS; i++)
    {
        kmer_offsets[i] = i * -1;                                   // Starts at [0, -1, -2, -3, -4]
    }

    // a slow-ish first part
    while(read_counter < OFF)
    {
        for(int j = 0; j < UN_NUM_CUTS; j++)
        {
            if(read_counter >= j)
            {
                kmers[j][kmer_offsets[j]] = *UMWG_ME_RAC;
            }
            kmer_offsets[j]++;
        }
        UMWG_ME_RAC++;
        read_counter++;
    }

    // this is the fast part of the loop
    while(read_counter < UN_KMER_LENGTH)
    {
        for(int j = 0; j < UN_NUM_CUTS; j++)
        {
            kmers[j][kmer_offsets[j]] = *UMWG_ME_RAC;
            kmer_offsets[j]++;
        }
        UMWG_ME_RAC++;
        read_counter++;
    }

    // an even slower ending
    while(read_counter < RL)
    {
        for(int j = 0; j < UN_NUM_CUTS; j++)
        {
            if(kmer_offsets[j] < UN_KMER_LENGTH)
            {
                kmers[j][kmer_offsets[j]] = *UMWG_ME_RAC;
            }
            kmer_offsets[j]++;
        }
        UMWG_ME_RAC++;
        read_counter++;
    }
    
    //
    // Now the fun stuff begins:
    //
    
    // clear the match map
    mKmerMatchMap->clearKmerMatchMap();

    // for handling the maps
    KmerMapElem km_elem;
    KmerMatchMapElem kmm_elem;

    // for holding query node info
    UniNodeId Q_UID;
    uMDInt Q_Position;
    bool Q_Orientation;
    
    for(uMDInt kmer_count = 0; kmer_count < UN_NUM_CUTS; kmer_count++)
    {
        // make the kmer into a string and then add it to the kmer store
        kmers[kmer_count][UN_KMER_LENGTH] = '\0';
        std::string kmer = kmers[kmer_count];
        
        PARANOID_ASSERT_PRINT_L4(((unsigned int)(kmer.length()) == UN_KMER_LENGTH), kmer << " : " << kmer_count << "\n" << (mReadStore->getSequence(getRs(UID)).length()) << " : " << mReadStore->getSequence(getRs(UID)) << " : " << ((unsigned int)(kmer.length())) << " == " << UN_KMER_LENGTH);
        
        if(mKmerMap->addElem(&kmer, &kmer_orients[kmer_count], kmer_count, getRs(UID), &km_elem))
        {
            int loop_watcher = 0;

            // this kmer is already in the map
            // get the first guy in the chain and process...
            do {
                loop_watcher++;
                // get the information stored about this kmer
                Q_UID = mReadStore->getUn(km_elem.KME_RSID);
                Q_Position = km_elem.KME_Position;
                Q_Orientation = km_elem.KME_Orientation;
                bool edge_added = false;

                // we can have multiple identical kmers from the same read
                // check we're not comparing to ourselves
                if(Q_UID != UID)
                {
                    // if we have a previous shared kmer with another uninode then we would have set
                    // up a kmm entry so we could see when we get to the end.
                    int other_loop_watcher = 0;
                    if(mKmerMatchMap->getElem(Q_UID, &kmm_elem))
                    {
                        do {
                            other_loop_watcher++;
                            // we have seen this uninode before, see if we are at the
                            // adding criteria set the first time we saw a shared kmer
                            if(kmer_count == kmm_elem.KMME_BPosition)
                            {
                                // the base is in the right position
                                if(Q_Position == kmm_elem.KMME_QPosition)
                                {
                                    // the query is in the right position
                                    if(Q_Orientation ^ kmer_orients[kmer_count] ^ kmm_elem.KMME_OrientMatch)
                                    {
                                        // the orientations match (or not) as they should
                                        // so we can add the edge!
                                        if(!addEdgeBetween(kmm_elem.KMME_BOffset, UID, kmm_elem.KMME_QOffset, Q_UID))
                                        {
                                            banishVec->push_back(Q_UID);
                                            banishNode(Q_UID);
                                        }
                                        edge_added = true;
                                        break;
                                    }
                                }
                            }
                        } while(mKmerMatchMap->getNextElem(&kmm_elem));
                    }

                    // if we didn't add an edge then we will need to set a new match elem
                    if(!edge_added)
                    {
                        // there are a number of cases to consider here...
                        switch(kmer_count)
                        {
                            case 0:
                            {
                                // If this is the first kmer then we can only overlap with nodes
                                // which share their very last kmer...
                                //
                                //        |XXXXX|XXX|XXXXX|XXXXXX
                                //  XXXXXX|XXXXX|XXX|XXXXX|
                                //
                                // we need to check where this kmer lies in the query
                                switch(Q_Position)
                                {
                                    case 0:                         // case 0, 0
                                    {
                                        // at the start
                                        // we can make an edge iff the orientations don't agree
                                        // The overlap is minimal, ie. the kmer length and the offset is maximal
                                        if(Q_Orientation != kmer_orients[kmer_count])
                                        {
                                            // add the edge!!!
                                            if(!addEdgeBetween(-1 * (sMDInt)OFF, UID, -1 * (sMDInt)OFF, Q_UID))
                                            {
                                                banishVec->push_back(Q_UID);
                                                banishNode(Q_UID);
                                            }
                                        }
                                        break;
                                    }
                                    case OFF:                       // case 0, M
                                    {
                                        // at the end
                                        // we can add this edge iff the orientations agree
                                        // The overlap is minimal, ie. the kmer length and the offset is maximal
                                        if(Q_Orientation == kmer_orients[kmer_count])
                                        {
                                            // add the edge
                                            if(!addEdgeBetween(-1 * (sMDInt)OFF, UID, (sMDInt)OFF, Q_UID))
                                            {
                                                banishVec->push_back(Q_UID);
                                                banishNode(Q_UID);
                                            }
                                        }
                                        break;
                                    }
                                    default:                        // case 0, ?
                                    {
                                        // somewhere in the middle
                                        // add the overlapping criteria for this node
                                        // we have hit the base at kmer 0, check if the orientations match
                                        if(Q_Orientation == kmer_orients[kmer_count])
                                        {
                                            // the orientations match so we need to conserve the difference
                                            // between the two positions.
                                            kmm_elem.KMME_BPosition = (uMDInt)OFF - Q_Position;
                                            kmm_elem.KMME_QPosition = (uMDInt)OFF;
                                            kmm_elem.KMME_OrientMatch  = 1;
                                            kmm_elem.KMME_BOffset = -1 * (sMDInt)Q_Position;
                                            kmm_elem.KMME_QOffset = (sMDInt)Q_Position;
                                            mKmerMatchMap->addElem(Q_UID, &kmm_elem);
                                        }
                                        else
                                        {
                                        // the orientations do not match so we need to conserve
                                        // the sum of the two positions. because one of them is 0 this is easy
                                        // to calculate.
                                            kmm_elem.KMME_BPosition = Q_Position;
                                            kmm_elem.KMME_QPosition = 0;
                                            kmm_elem.KMME_OrientMatch  = 0;
                                            kmm_elem.KMME_BOffset = (sMDInt)Q_Position - (sMDInt)OFF;
                                            kmm_elem.KMME_QOffset = (sMDInt)Q_Position - (sMDInt)OFF;
                                            mKmerMatchMap->addElem(Q_UID, &kmm_elem);
                                        }
                                        break;
                                    }
                                }
                                break;
                            }
                            case OFF:
                            {
                                switch(Q_Position)
                                {
                                    case 0:                         // case M, 0
                                    {
                                        // we can add this edge iff the orientations agree
                                        if(Q_Orientation == kmer_orients[kmer_count])
                                        {
                                            // add the edge
                                            if(!addEdgeBetween((sMDInt)OFF, UID, -1 * (sMDInt)OFF, Q_UID))
                                            {
                                                banishVec->push_back(Q_UID);
                                                banishNode(Q_UID);
                                            }
                                        }
                                        break;
                                    }
                                    case OFF:                       // case M, M
                                    {
                                        // we can add this edge iff the orientations don't agree
                                        if(Q_Orientation != kmer_orients[kmer_count])
                                        {
                                            // add the edge
                                            if(!addEdgeBetween((sMDInt)OFF, UID, (sMDInt)OFF, Q_UID))
                                            {
                                                banishVec->push_back(Q_UID);
                                                banishNode(Q_UID);
                                            }
                                        }

                                        break;
                                    }
                                    // else there is some kind of error
                                }
                                break;
                            }
                            default:                                // case ?, ?
                            {
                                // add the overlapping criteria for this node
                                // this is easy because we know from the structure of this
                                // code that Q_Position must equal 0 or OFF for there
                                // to be any chance of an overlap
                                switch(Q_Position)
                                {
                                    case 0:
                                    {
                                        kmm_elem.KMME_BPosition = (uMDInt)OFF;
                                        kmm_elem.KMME_QPosition = (uMDInt)OFF - kmer_count;
                                        kmm_elem.KMME_OrientMatch  = 1;
                                        kmm_elem.KMME_BOffset = (sMDInt)kmer_count;
                                        kmm_elem.KMME_QOffset = -1 * (sMDInt)kmer_count;
                                        mKmerMatchMap->addElem(Q_UID, &kmm_elem);
                                        break;
                                    }
                                    case OFF:
                                    {
                                        kmm_elem.KMME_BPosition = (uMDInt)OFF;
                                        kmm_elem.KMME_QPosition = kmer_count;
                                        kmm_elem.KMME_OrientMatch  = 0;
                                        kmm_elem.KMME_BOffset = (sMDInt)kmer_count;
                                        kmm_elem.KMME_QOffset = (sMDInt)kmer_count;
                                        mKmerMatchMap->addElem(Q_UID, &kmm_elem);
                                        break;
                                    }
                                }
                                break;
                            }
                        }
                    }
                }
            } while(mKmerMap->getNextElem(&km_elem));
        }
    }
    return true;
}
//HO 
//VO 

//VO virtual
template <int RL, int OFF>
/*HV*/ void
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ killKmerLists(void)
//HO ;
//VO = 0;
{
    //-----
    // delete the kmer lists so we can conserve memory
    //
    if(mKmerMap != NULL)
        delete mKmerMap;
    mKmerMap = NULL;

    if(mKmerMatchMap != NULL)
        delete mKmerMatchMap;
    mKmerMatchMap = NULL;
}
//HO 
//VO 

    /*
    ** Symmetrical transitive edge addition
    */
//VO virtual 
template <int RL, int OFF>
/*HV*/ bool
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ addEdgeBetween(sMDInt offsetAtoB, UniNodeId nodeA, sMDInt offsetBtoA, UniNodeId nodeB)
//HO ;
//VO = 0;
{
    //-----
    // Add a transitive edge between two nodes
    // Sometimes when this is attempted is it seen that the node
    // should be banished. Return "false" is the UID should be banished
    // true otherwise...
    //
    // can't make edges between banished nodes
    if(getNodeType(nodeA) == UN_NODE_TYPE_BANISHED || getNodeType(nodeB) == UN_NODE_TYPE_BANISHED)
        return true;
    // nodes can not overlap with themselves
    if(nodeA == nodeB)
        return true;
    bool off_matches = (!((offsetBtoA + offsetAtoB) == 0));
    if(!addEdge_asymmetrical(nodeB, offsetAtoB, off_matches, nodeA))
    {
        return false;
    }
    else
    {
        return addEdge_asymmetrical(nodeA, offsetBtoA, off_matches, nodeB);
    }
}
//HO 
//VO 

    /*
    ** Assymetrical transitive edge addition
    */
//VO virtual 
template <int RL, int OFF>
/*HV*/ bool
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ addEdge_asymmetrical(UniNodeId newNode, sMDInt offsetFromBase, bool retOffsetMatches, UniNodeId baseNode)
//HO ;
//VO = 0;
{
    //-----
    // This is an assymetrical function and should not be called except by the
    // function addEdgeBetween. Part of this function's internal
    // logic will cause some bat-ass-crazy stuff to happen if you don't call the function
    // in reverse. luckily we have a function which does this so just call that one, OK? GOOD!
    //
    // By default nodes are added with state UN_EDGE_STATE_TRANS
    // It is up to this method to decide where to add the actual data,
    // and to allocate a new block if necessary...
    //
    // We want no duplicates here, so If we are trying to add the same node with the
    // same olapOffset it is ignored and we add nothing, if we are trying to add the same node but with
    // a different olapOffset then we banish the existing guy and we don't do anything either
    // we don't care about the ordering here. the amount of effort to re-write everything
    // each time we add or remove something makes this not feasible...
    //
    // where needed we can call sort olaps to get things ordered well
    //
    // Be paranoid
    PARANOID_ASSERT_L2((newNode != UN_NULL_ID));
    PARANOID_ASSERT_L2((baseNode != UN_NULL_ID));
    
    // we need to check that this node has not been added before
    UniNodeDataId next_data_block = getNextDataBlock(baseNode);
    UniNodeDataId data_block = next_data_block;
    int num_used = 0;
    while(UND_NULL_ID != next_data_block)
    {
        data_block = next_data_block;
        for(num_used = 1; num_used <= getNumUsedInDataBlock(data_block); num_used++)
        {
            // check that this is a unique Id
            if(getXthEdge(num_used, data_block) == newNode)
            {
                // we have a match, this is bad hoodoo, we'll need to so some banishing
                return false;
            }
        }
        next_data_block = getNextDataBlock(data_block);
    }

    // not been added before so we continue
    if(UND_NULL_ID == data_block)
    {
        // this must be ther first edge added for this node
        data_block = newUniNodeDataId();
        // link the data to the head
        setNextDataBlock(data_block, baseNode);
    }
    else if(UN_MAX_OLAPS_IN_DATA < num_used)
    {
        // this block is full we need to make another data block
        next_data_block = newUniNodeDataId();
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
            setUn1_state(UN_EDGE_STATE_TRANS, data_block);
            setUn1_offset(offsetFromBase, data_block);
            setUn1_returnOffset(retOffsetMatches, data_block);
            setUn1_olap(newNode, data_block);
            setBlockState(mEdgeStates[0][UN_BLOCK_STATE_ADD_T], data_block);
            break;
        }
        case 1:
        {
            setUn2_state(UN_EDGE_STATE_TRANS, data_block);
            setUn2_offset(offsetFromBase, data_block);
            setUn2_returnOffset(retOffsetMatches, data_block);
            setUn2_olap(newNode, data_block);
            setBlockState(mEdgeStates[getBlockState(data_block)][UN_BLOCK_STATE_ADD_T], data_block);
            break;
        }
        case 2:
        {
            setUn3_state(UN_EDGE_STATE_TRANS, data_block);
            setUn3_offset(offsetFromBase, data_block);
            setUn3_returnOffset(retOffsetMatches, data_block);
            setUn3_olap(newNode, data_block);
            setBlockState(mEdgeStates[getBlockState(data_block)][UN_BLOCK_STATE_ADD_T], data_block);
            break;
        }
    }
    incTRank(baseNode);
    return true;
}
//HO 
//VO 

    /*
    ** Makes some helper lists used when labelling non-transitive edges
    */
    static std::map<UniNodeId, bool> UNG_non_trans_store;
    static std::vector<olap_pair>::iterator UNG_olap_iter;
    static std::vector<olap_pair>::iterator UNG_olap_last;

//VO virtual 
template <int RL, int OFF>
/*HV*/ void
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ getMakingLists(int maxOffset, std::vector<int> * posNts, std::vector<int> * negNts, std::vector<bool> * Nts, std::vector<olap_pair> * olaps, UniNodeId UID)
//HO ;
//VO = 0;
{
    //-----
    // This function updates three vectors. `olaps` holds all overlaps and the offsets (inc. non-transitives),
    // the other two vectors hold the addresses of all non-transitive nodes to address.
    // We decide which to add it to depending on the sign of the offset
    //
    // the vector<bool> is the same size as olaps and tells us whether the corresp. value in olaps is adjacent
    //
    
    // first, clear the vectors we've been given. just to make sure...
    olaps->clear();
    posNts->clear();
    negNts->clear();
    Nts->clear();
    UNG_non_trans_store.clear();
    
    // go through all the edges and add them to the right lists
    // we only care about transitive and non-transitive edges
    UN_EDGE_ELEM tmp_edge;
    if(getEdges(&tmp_edge, 0, 0, UID))
    {
        do {
            if(fAbs(tmp_edge.UNEE_Offset) <= maxOffset)
            {
                switch(tmp_edge.UNEE_State)
                {
                    case UN_EDGE_STATE_NTRANS:
                        olaps->push_back(make_pair(tmp_edge.UNEE_OlapNode, tmp_edge.UNEE_Offset));
                        UNG_non_trans_store[tmp_edge.UNEE_OlapNode] = true;
                        break;
                    case UN_EDGE_STATE_TRANS:
                        olaps->push_back(make_pair(tmp_edge.UNEE_OlapNode, tmp_edge.UNEE_Offset));
                        UNG_non_trans_store[tmp_edge.UNEE_OlapNode] = false;
                        break;
                }
            }
        } while(getNextEdge(&tmp_edge));
    }

    // Sort the list so that the closest values are to the top
    std::sort(olaps->begin(), olaps->end(), sort_olaps_abs);

    // finally make the pos and neg non-transitive lists of UniNodeIds
    UNG_olap_iter = olaps->begin();
    UNG_olap_last = olaps->end();
    int indexer = 0;                                                // holds the index in the list
    while(UNG_olap_iter != UNG_olap_last)
    {
        if(UNG_non_trans_store[UNG_olap_iter->first])
        {
            // this is a non-transitive node
            // look at the offset to see which vector to push it on
            if(UNG_olap_iter->second > 0)
                posNts->push_back(indexer);
            else
                negNts->push_back(indexer);
            Nts->push_back(true);
        }
        else
            Nts->push_back(false);
        indexer++;
        UNG_olap_iter++;
    }
    // and we're done!
}
//HO 
//VO 

    /*
    ** Label non-transitive edges for a UniNode
    */

    // again we have that bug about making stl classes within the function call
    static std::vector<olap_pair> UNG_olaps;
    static std::vector<int> UNG_ntPos, UNG_ntNeg;
    static std::vector<bool> UNG_nts;
    static std::vector<olap_pair>::iterator UNG_base_olap_start;                    // we use index counters to find particular nodes, so we need to know where to start from
    static std::vector<olap_pair>::iterator UNG_base_olap_iter;                     // the usual suspects...
    static std::vector<olap_pair>::iterator UNG_base_olap_last;
    static std::vector<bool>::iterator UNG_nts_iter;                                // and the iterator
    static std::vector<int> * UNG_posNeg;                                           // use this to save coding lines. could point to either of the above...
    static std::vector<int>::iterator UNG_posNeg_iter;                              // iterators to suit
    static std::vector<int>::iterator UNG_posNeg_last;

//VO virtual
template <int RL, int OFF>
/*HV*/ bool
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ labelNonTransitiveEdges(std::vector<UniNodeId> * banishVec, int maxOffset, UniNodeId UID)
//HO ;
//VO = 0;
{
    //-----
    // Do non-transitive labeling for one uninode
    //
    // we are only going to add edges to the non-transitive list here if
    // the offset is less than or equal to maxOffset
    //
    // we use a std::vector of: 
    // typedef pair<UniNodeId, sMDInt> olap_pair for the base node
    // this vector will hold the overlapping pairs of ints.
    // These are: <olap address, olap offset>, for every overlapping node with the base node
    // we make this once because while we are not going to add or remove anything from this list
    // once it's made we traverse it a few times and getting individual nodes is expensive
    // we aim to change the state of the nodes within

    banishVec->clear();
    bool done_banished = false;
    
    UNG_olaps.clear();                                                   // all the olaps at this node all sorted nicely
    UNG_nts.clear();                                                     // vector of bools state whether an entry in Olaps is non-transitive
    // these vectors hold indexes to the adjacent nodes in the above vector
    UNG_ntPos.clear();                                                   // std::vector of indexes to the positive offset non-transitive nodes in olaps
    UNG_ntNeg.clear();                                                   // and the negative

    UniNodeId query_node;                                           // the current node we're querying
    int counter = 0;
    bool got_attached = false;                                      // is this node attached to the graph?
    
    // get the overlapping nodes
    getMakingLists(maxOffset, &UNG_ntPos, &UNG_ntNeg, &UNG_nts, &UNG_olaps, UID);   // for the base node, get all overlaps (reg + adj) and indexes to the pos and neg ones
                                                                    // a node may already be adjacent by the time we get here
                                                                    // due to a call from a previous node, nts holds bools which we use for this
    UNG_base_olap_iter = UNG_olaps.begin();                             // set iterators
    UNG_base_olap_start = UNG_olaps.begin();
    UNG_base_olap_last = UNG_olaps.end();
    UNG_nts_iter = UNG_nts.begin();
    
    // for each olap in the list
    while(UNG_base_olap_iter != UNG_base_olap_last)
    {
        // it makes no sense to process a guy who's already adjacent
        if(!(*UNG_nts_iter))
        {
            // we wish to try join this guy
            query_node = UNG_base_olap_iter->first;

            // we need the adjacent nodes in the same direction as the query
            // so we use posNeg to point to the correct one
            if(UNG_base_olap_iter->second < 0)
                UNG_posNeg = &UNG_ntNeg;
            else
                UNG_posNeg = &UNG_ntPos;

            // go through the posNeg list, if any of these nodes can "see" the query
            // then adding the edge would cause a triangle to be formed, so we won't add the edge
            //
            //  [B] -->_[Q]
            //   \     /|       Take any node X in the same direction from B as Q
            //    \   /         If X overlaps with Q then making an edge from B to Q would cause
            //    -||/_         the formation of a triangle
            //     [X]
            //

            bool seen = false;
            UNG_posNeg_iter = UNG_posNeg->begin();
            UNG_posNeg_last = UNG_posNeg->end();
            while(UNG_posNeg_iter != UNG_posNeg_last)
            {
                // find the index in the olap vector with the node we are testing
                // we lever posNeg so we consider only the correct nodes
                // see if there is an overlap there and if the offset is limited by the maxOffset
                int tmp_off = fAbs(getOffsetBetweenNodes(query_node, 0, (UNG_base_olap_start + *UNG_posNeg_iter)->first));
                if(tmp_off != 0 && tmp_off <= maxOffset)
                {
                    seen = true;
                    break;
                }
                UNG_posNeg_iter++;
            }

            // if it's been seen we'll go onto the next query node
            if(!seen)
            {
                // otherwise promote the node!!!!
                if(promoteEdgeBetween_symmetrical(query_node, UID))
                    got_attached = true;

                // push the new index on the appropriate pos neg vector
                UNG_posNeg->push_back(counter);

                // now we may have broken something (ie, created some kind of loop, EX a triangle)
                // and we may need to delete a link.
                // we need to run through these nodes again, this time
                // we check all nodes joined by edges in the opposite direction
                // so switch the values of pos_neg
                if(UNG_base_olap_iter->second > 0)
                    UNG_posNeg = &UNG_ntNeg;
                else
                    UNG_posNeg = &UNG_ntPos;
                //
                //  [X] <->_[Q]
                //   \     /|       Take any node X in the opposite direction from B as Q
                //    \|  /         If X is adjacent with Q then this edge needs to be demoted to a
                //    -  /          regular overlap to break this triangle
                //     [B]
                //
                //
                UNG_posNeg_iter = UNG_posNeg->begin();
                UNG_posNeg_last = UNG_posNeg->end();

                while(UNG_posNeg_iter != UNG_posNeg_last)
                {
                    // if this node is adjacent to the query then
                    // break the link! (this is all tested in the following function call)
                    //
                    // There is a problem with this situation:
                    //
                    //  AAGAGAGAGAGAGAGAGAGAGAGAGAGAGAGAGAGAGAGAGAGAGAGAG
                    //  AGAGAGAGAGAGAGAGAGAGAGAGAGAGAGAGAGAGAGAGAGAGAGAGA
                    //  CTCTCTCTCTCTCTCTCTCTCTCTCTCTCTCTCTCTCTCTCTCTCTCTC
                    //
                    // For example the above reads will cause a natural loop which we cannot 
                    // fix via any of these methods. Removing one of the reads fixes this problem
                    // and this is handled in the following call too. (we look for demotions od offset 1)
                    if(demoteEdgeIfNodeFound_symmetrical(query_node, (UNG_base_olap_start + *UNG_posNeg_iter)->first))
                    {
                        banishVec->push_back((UNG_base_olap_start + *UNG_posNeg_iter)->first);
                        done_banished = true;
                    }
                    UNG_posNeg_iter++;
                }
            }
        }
        counter++;
        UNG_base_olap_iter++;
        UNG_nts_iter++;
    }

    return done_banished;
}
//HO 
//VO 

    /*
    ** Reset the edge states (except banished edges)
    ** Blank the UN_ContigId
    */
//VO virtual
template <int RL, int OFF>
/*HV*/ void
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ resetEdges(bool keepContigId, UniNodeId UID)
//HO ;
//VO = 0;
{
    //-----
    // Go trough all the edges at this node and reset all non-transitive
    // and detached edges back to transitive ones
    // call remakeEdgeStates to fix the edge states
    //
    UN_EDGE_ELEM tmp_edge;
    PARANOID_ASSERT_L2(getNodeType(UID) != UN_NODE_TYPE_BANISHED);
    if(getEdges(&tmp_edge, 0, 0, UID))
    {
        do {
            switch(tmp_edge.UNEE_State)
            {
                case UN_EDGE_STATE_NTRANS:
                    setXthState(UN_EDGE_STATE_TRANS, tmp_edge.UNEE_WhichOlap, tmp_edge.UNEE_WhichData);
                    decNtRank(UID);
                    incTRank(UID);
                    break;
                case UN_EDGE_STATE_DETACHED:
                    setXthState(UN_EDGE_STATE_TRANS, tmp_edge.UNEE_WhichOlap, tmp_edge.UNEE_WhichData);
                    incTRank(UID);
                    break;
            }
        } while(getNextEdge(&tmp_edge));
        remakeEdgeStates(UID);
    }

    setNodeType(UN_NODE_TYPE_UNSET, UID);
    
    // clear the contig
    if(!keepContigId)
        clearContig(UID);
}
//HO 
//VO 

    /*
    ** Sort the edges for faster retrieval of non-transitive nodes
    */

    static std::map<int, UniNodeDataId> UNG_swap_into_block_map;
    static std::map<int, unsigned int> UNG_swap_into_position_map;
    
//VO virtual
    template <int RL, int OFF>
/*HV*/ bool
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ shallowSortEdges(UniNodeId UID)
//HO ;
//VO = 0;
{
    //-----
    // make sure the gross order is:
    // non-transitive, transitive, the we don't care
    // local ordering does not matter too much
    //

    // clear the static maps
    UNG_swap_into_block_map.clear();
    UNG_swap_into_position_map.clear();
    
    int swap_read_index = 0;
    int swap_write_index = 0;
    
    int num_used = 0;
    bool changes_made = false;

    // we need to go through all the data blocks
    // first get all the non-transitive guys to the top
    UniNodeDataId data_block = getNextDataBlock(UID);
    while(UND_NULL_ID != data_block)
    {
        num_used = getNumUsedInDataBlock(data_block);
        PARANOID_ASSERT_L2((num_used > 0));
        for(int focused_olap = 1; focused_olap <= num_used; focused_olap++)
        {
            switch(getXthState(focused_olap, data_block))
            {
                case UN_EDGE_STATE_NTRANS:
                    if(0 != swap_write_index)
                    {
                        if((UNG_swap_into_position_map[swap_read_index] > 3) || ((UNG_swap_into_position_map[swap_read_index] == 0)))
                        {
                            logError("about to swap with a baddie here: Swap Into overlap: " << UNG_swap_into_position_map[swap_read_index] << " at: " << UNG_swap_into_block_map[swap_read_index] << " From overlap: " << focused_olap << " at: " << data_block);
                        }
                        swapEdgeInfo(UNG_swap_into_position_map[swap_read_index], UNG_swap_into_block_map[swap_read_index], focused_olap, data_block);
                        UNG_swap_into_block_map[swap_write_index] = data_block;
                        UNG_swap_into_position_map[swap_write_index] = focused_olap;
                        swap_read_index++;
                        swap_write_index++;
                        changes_made = true;
                    }
                    break;
                case UN_EDGE_STATE_TRANS:
                case UN_EDGE_STATE_DETACHED:
                case UN_EDGE_STATE_BANISHED:
                    // add a swap to the block
                    UNG_swap_into_block_map[swap_write_index] = data_block;
                    UNG_swap_into_position_map[swap_write_index] = focused_olap;
                    swap_write_index++;
                    break;
            }
        }
        data_block = getNextDataBlock(data_block);
    }

    // reset
    UNG_swap_into_block_map.clear();
    UNG_swap_into_position_map.clear();
    swap_read_index = 0;
    swap_write_index = 0;
    
    // then get all the transitive guys up behind them
    data_block = getNextDataBlock(UID);
    while(UND_NULL_ID != data_block)
    {
        num_used = getNumUsedInDataBlock(data_block);
        PARANOID_ASSERT_L2((num_used > 0));
        for(int focused_olap = 1; focused_olap <= num_used; focused_olap++)
        {
            switch(getXthState(focused_olap, data_block))
            {
                case UN_EDGE_STATE_NTRANS:
                    break;
                case UN_EDGE_STATE_TRANS:
                    if(0 != swap_write_index)
                    {
                        if((UNG_swap_into_position_map[swap_read_index] > 3) || ((UNG_swap_into_position_map[swap_read_index] == 0)))
                        {
                            logError("about to swap with a baddie here: Swap Into overlap: " << UNG_swap_into_position_map[swap_read_index] << " at: " << UNG_swap_into_block_map[swap_read_index] << " From overlap: " << focused_olap << " at: " << data_block);
                        }
                        swapEdgeInfo(UNG_swap_into_position_map[swap_read_index], UNG_swap_into_block_map[swap_read_index], focused_olap, data_block);
                        UNG_swap_into_block_map[swap_write_index] = data_block;
                        UNG_swap_into_position_map[swap_write_index] = focused_olap;
                        swap_read_index++;
                        swap_write_index++;
                        changes_made = true;
                    }
                    break;
                case UN_EDGE_STATE_DETACHED:
                case UN_EDGE_STATE_BANISHED:
                    UNG_swap_into_block_map[swap_write_index] = data_block;
                    UNG_swap_into_position_map[swap_write_index] = focused_olap;
                    swap_write_index++;
                    break;
            }
        }
        data_block = getNextDataBlock(data_block);
    }

    // the data states will be fubar so we need to redo them
    if(changes_made)
        remakeEdgeStates(UID);
    return true;
}
//HO 
//VO 

    /*
    ** Swap information between two containers in two data blocks
    */
//VO virtual
template <int RL, int OFF>
/*HV*/ void
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ swapEdgeInfo(int whichOlapA, UniNodeDataId UDIDA, int whichOlapB, UniNodeDataId UDIDB)
//HO ;
//VO = 0;
{
    EDGE_STATE tmp_state = getXthState(whichOlapA, UDIDA);
    UniNodeId tmp_edge = getXthEdge(whichOlapA, UDIDA);
    sMDInt tmp_offset = getXthOffset(whichOlapA, UDIDA);
    bool tmp_return = getXthReturnOffset(whichOlapA, UDIDA);
    
    setXthState(getXthState(whichOlapB, UDIDB), whichOlapA, UDIDA);
    setXthEdge(getXthEdge(whichOlapB, UDIDB), whichOlapA, UDIDA);
    setXthOffset(getXthOffset(whichOlapB, UDIDB), whichOlapA, UDIDA);
    setXthReturnOffset(getXthReturnOffset(whichOlapB, UDIDB), whichOlapA, UDIDA);

    setXthState(tmp_state, whichOlapB, UDIDB);
    setXthEdge(tmp_edge, whichOlapB, UDIDB);
    setXthOffset(tmp_offset, whichOlapB, UDIDB);
    setXthReturnOffset(tmp_return, whichOlapB, UDIDB);
}
//HO 
//VO 

    /*
    ** Remake the information in a data block
    */
//VO virtual
template <int RL, int OFF>
/*HV*/ void
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ remakeEdgeStates(UniNodeId UID)
//HO ;
//VO = 0;
{
    //-----
    // After we reset the nodes or we sort them the data states make no more sense
    // we need to go through and rebuild them.
    //
    UN_EDGE_ELEM tmp_edge;
    EDGE_STATE tmp_state = 0;
    UniNodeDataId prev_data_block;
    if(getEdges(&tmp_edge, 0, 0, UID))
    {
        // initialise this guy to the first data block
        prev_data_block = tmp_edge.UNEE_WhichData;
        do {
            // tmp_edge.UNEE_WhichData holds the id of the data block
            // when this changes from what is stored in prev_data_block
            // then we know that we need to store the state
            if(tmp_edge.UNEE_WhichData != prev_data_block)
            {
                // time to write the previous data block in
                setBlockState(tmp_state, prev_data_block);
                prev_data_block = tmp_edge.UNEE_WhichData;
                tmp_state = 0;
            }
            
            // otherwise we can update the state
            switch(tmp_edge.UNEE_State)
            {
                case UN_EDGE_STATE_NTRANS:
                    tmp_state = mEdgeStates[tmp_state][UN_BLOCK_STATE_ADD_T];
                    tmp_state = mEdgeStates[tmp_state][UN_BLOCK_STATE_T_TO_NT];
                    break;
                case UN_EDGE_STATE_TRANS:
                    tmp_state = mEdgeStates[tmp_state][UN_BLOCK_STATE_ADD_T];
                    break;
                case UN_EDGE_STATE_BANISHED:
                    tmp_state = mEdgeStates[tmp_state][UN_BLOCK_STATE_ADD_T];
                    tmp_state = mEdgeStates[tmp_state][UN_BLOCK_STATE_T_TO_B];
                    break;
                case UN_EDGE_STATE_DETACHED:
                    tmp_state = mEdgeStates[tmp_state][UN_BLOCK_STATE_ADD_T];
                    tmp_state = mEdgeStates[tmp_state][UN_BLOCK_STATE_T_TO_D];
                    break;
            }
        } while(getNextEdge(&tmp_edge));

        // write the last block
        setBlockState(tmp_state, prev_data_block);
    }
}
//HO 
//VO 
    
/******************************************************************************
** NODE TYPE MANAGEMENT
******************************************************************************/

    /*
    ** Banish a node and all its edges
    */
        
    // again we have that bug about making stl classes within the function call
    static std::vector<UniNodeId> UNG_pos_edge_ids;             // for holding the non-transitive edges (positive offset)
    static std::vector<sMDInt> UNG_pos_edge_offsets_to;         // the offsets to the edge
    static std::vector<UniNodeId> UNG_neg_edge_ids;
    static std::vector<sMDInt> UNG_neg_edge_offsets_to;
    static std::vector<sMDInt>::iterator UNG_pos_to_offs_iter;
    static std::vector<UniNodeId>::iterator UNG_pos_edges_iter;
    static std::vector<sMDInt>::iterator UNG_neg_to_offs_iter;
    static std::vector<UniNodeId>::iterator UNG_neg_edges_iter;

//VO virtual
template <int RL, int OFF>
/*HV*/ bool
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ banishNode(UniNodeId UID)
//HO ;
//VO = 0;
{
    //-----
    // Banish a node and all edges coming to and from it
    //

    // first we need to make sure we're not going to break the graph
    // consider this situation:
    //
    //  X: catcatgcactgactgactgactgactgactgcat
    //  Y:    catgcactgactgactgactgactgactgcattag            <--- to banish
    //  Z:       gcactgactgactgactgactgactgcattaggat
    //
    // when we banish node Y we need to check if the edges between X and Z should e promoted
    //
    if(getNtRank(UID) > 1)
    {
        // at least two edges so it's worth checking
        UNG_pos_edge_ids.clear();             // for holding the non-transitive edges (positive offset)
        UNG_pos_edge_offsets_to.clear();         // the offsets to the edge
        UNG_neg_edge_ids.clear();
        UNG_neg_edge_offsets_to.clear();
        bool has_pos = false;
        bool has_neg = false;
        // first we make a list of the edges
        UN_EDGE_ELEM check_edge;
        if(getEdges(&check_edge, UN_EDGE_STATE_NTRANS, 0, UID))
        {
            do {
                if(check_edge.UNEE_Offset > 0)
                {
                    UNG_pos_edge_ids.push_back(check_edge.UNEE_OlapNode);
                    UNG_pos_edge_offsets_to.push_back(check_edge.UNEE_Offset);
                    has_pos = true;
                }
                else
                {
                    UNG_neg_edge_ids.push_back(check_edge.UNEE_OlapNode);
                    UNG_neg_edge_offsets_to.push_back(check_edge.UNEE_Offset);
                    has_neg = true;
                }
            } while(getNextEdge(&check_edge));
        }
        
        // now we can promote the edges between the pos and the neg nodes provided the offsets are within limits
        if(has_pos && has_neg)
        {
            UNG_pos_to_offs_iter = UNG_pos_edge_offsets_to.begin();
            UNG_pos_edges_iter = UNG_pos_edge_ids.begin();
            while(UNG_pos_edges_iter != UNG_pos_edge_ids.end())
            {
                UNG_neg_to_offs_iter = UNG_neg_edge_offsets_to.begin();
                UNG_neg_edges_iter = UNG_neg_edge_ids.begin();
                while(UNG_neg_edges_iter != UNG_neg_edge_ids.end())
                {
                    if((uMDInt)fAbs(*UNG_neg_to_offs_iter - *UNG_pos_to_offs_iter) <= mCurrentMaxOffset)
                    {
                        // we can promote these edges
                        promoteEdgeBetween_symmetrical(*UNG_pos_edges_iter, *UNG_neg_edges_iter);
                    } 
                    UNG_neg_edges_iter++;
                    UNG_neg_to_offs_iter++;
                }
                UNG_pos_edges_iter++;
                UNG_pos_to_offs_iter++;
            }
        }
    }    

    // set the new node type
    setNodeType(UN_NODE_TYPE_BANISHED, UID);

    // then fix the edges
    UN_EDGE_ELEM tmp_edge;
    if(getEdges(&tmp_edge, 0, 0, UID))
    {
        do {
            if(UN_EDGE_STATE_BANISHED != tmp_edge.UNEE_State)
            {
                // banish the edge to the overlapping node
                banishEdge_asymmetrical(tmp_edge.UNEE_WhichOlap, tmp_edge.UNEE_WhichData);
                // and the returning edge
                banishEdge_asymmetrical(UID, tmp_edge.UNEE_OlapNode);
                switch(tmp_edge.UNEE_State)
                {
                    case UN_EDGE_STATE_TRANS:
                        decTRank(tmp_edge.UNEE_OlapNode);
                        break;
                    case UN_EDGE_STATE_NTRANS:
                        decNtRank(tmp_edge.UNEE_OlapNode);
                        break;
                }
            }
        } while(getNextEdge(&tmp_edge));
    }
    
    setTRank(0, UID);
    setNtRank(0, UID);

    return true;
}
//HO 
//VO 

    /*
    ** Detach a node and all its edges
    */
//VO virtual
template <int RL, int OFF>
/*HV*/ bool
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ detachNode(map<GenericNodeId, bool> * detMap, UniNodeId UID)
//HO ;
//VO = 0;
{
    //-----
    // Detach a node and make all Nt edges T
    //
    // This code detaches nodes and edges
    // set this guy as detached
    setNodeType(UN_NODE_TYPE_DETACHED, UID);

    // detach all the edges
    UN_EDGE_ELEM tmp_edge;
    if(getEdges(&tmp_edge, 0, 0, UID))
    {
        do {
            switch(tmp_edge.UNEE_State)
            {
                case UN_EDGE_STATE_NTRANS:
                    detachEdge_asymmetrical(tmp_edge.UNEE_WhichOlap, tmp_edge.UNEE_WhichData);
                    detachEdge_asymmetrical(UID, tmp_edge.UNEE_OlapNode);
                    decNtRank(UID);
                    decNtRank(tmp_edge.UNEE_OlapNode);
                    (*detMap)[getGn(tmp_edge.UNEE_OlapNode)] = true;
                    break;
                case UN_EDGE_STATE_TRANS:
                    detachEdge_asymmetrical(tmp_edge.UNEE_WhichOlap, tmp_edge.UNEE_WhichData);
                    detachEdge_asymmetrical(UID, tmp_edge.UNEE_OlapNode);
                    decTRank(UID);
                    decTRank(tmp_edge.UNEE_OlapNode);
                    //(*detMap)[getGn(tmp_edge.UNEE_OlapNode)] = true;
                    break;
            }
        } while(getNextEdge(&tmp_edge));
    }
    return true;
}
//HO 
//VO 


//VO virtual
template <int RL, int OFF>
/*HV*/ bool
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ detachNode(UniNodeId UID)
//HO ;
//VO = 0;
{
    //-----
    // Detach a node and make all Nt edges T
    //
    // This code detaches nodes and edges
    // set this guy as detached
    setNodeType(UN_NODE_TYPE_DETACHED, UID);

    // detach all the edges
    UN_EDGE_ELEM tmp_edge;
    if(getEdges(&tmp_edge, 0, 0, UID))
    {
        do {
            switch(tmp_edge.UNEE_State)
            {
                case UN_EDGE_STATE_NTRANS:
                    detachEdge_asymmetrical(tmp_edge.UNEE_WhichOlap, tmp_edge.UNEE_WhichData);
                    detachEdge_asymmetrical(UID, tmp_edge.UNEE_OlapNode);
                    decNtRank(UID);
                    decNtRank(tmp_edge.UNEE_OlapNode);
                    break;
                case UN_EDGE_STATE_TRANS:
                    detachEdge_asymmetrical(tmp_edge.UNEE_WhichOlap, tmp_edge.UNEE_WhichData);
                    detachEdge_asymmetrical(UID, tmp_edge.UNEE_OlapNode);
                    decTRank(UID);
                    decTRank(tmp_edge.UNEE_OlapNode);
                    break;
            }
        } while(getNextEdge(&tmp_edge));
    }
    return true;
}
//HO 
//VO 

/******************************************************************************
** OUTPUT AND PRINTING
******************************************************************************/

    /*
    ** Get a sequence for a UniNodeId
    */
//VO virtual
template <int RL, int OFF>
/*HV*/ std::string
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ getSequence(UniNodeId UID)
//HO ;
//VO = 0;
{
    //-----
    // Get the sequence from the main sequence store
    //
    PARANOID_ASSERT_L2((mReadStore != NULL));
    return mReadStore->getSequence(getRs(UID));
}
//HO 
//VO 

    /*
    ** Print edges at a uninode
    */
//VO virtual
template <int RL, int OFF>
/*HV*/ void
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ printEdges(UniNodeId UID)
//HO ;
//VO = 0;
{
    //-----
    // print a list of all the edges, their offsets and the state...
    //
    std::cout << "Edge info for " << sayNodeTypeLikeAHuman(getNodeType(UID)) <<
                 " node: " << getSequence(UID) <<
                 " : " << getRs(UID) <<
                 " : " << UID <<
                 " : " << getContig(UID);
    UN_EDGE_ELEM tmp_edge;
    UniNodeDataId prev_data_block;
    if(getEdges(&tmp_edge, 0, 0, UID))
    {
        prev_data_block = tmp_edge.UNEE_WhichData;
        do {
            if(prev_data_block != tmp_edge.UNEE_WhichData)
            {
                EDGE_STATE tmp_state = getBlockState(prev_data_block);
                std::cout << " [Nt: " << mEdgeStates[tmp_state][UN_BLOCK_STATE_NT_COUNT] <<
                        ", Tr: " << mEdgeStates[tmp_state][UN_BLOCK_STATE_T_COUNT] <<
                        ", De: " << mEdgeStates[tmp_state][UN_BLOCK_STATE_D_COUNT] <<
                        ", Ba: " << mEdgeStates[tmp_state][UN_BLOCK_STATE_B_COUNT] <<
                        ", To: " << mEdgeStates[tmp_state][UN_BLOCK_STATE_TOTAL_EDGE_COUNT] << "] " << tmp_state << std::endl;
                prev_data_block = tmp_edge.UNEE_WhichData;
            }
            else
            {
                std::cout << std::endl;
            }
            std::cout << sayNodeTypeLikeAHuman(getNodeType(tmp_edge.UNEE_OlapNode)) <<
                         " : " << tmp_edge.UNEE_OlapNode <<
                         " : " << getRs(tmp_edge.UNEE_OlapNode) <<
                         " : " << getGn(tmp_edge.UNEE_OlapNode) <<
                         " : " << getContig(tmp_edge.UNEE_OlapNode) <<
                         " : " << getSequence(tmp_edge.UNEE_OlapNode) <<
                         " with offsets: [" << tmp_edge.UNEE_Offset << ", " << tmp_edge.UNEE_ReturnOffset << "]" <<
                         " and state: " << sayEdgeStateLikeAHuman(tmp_edge.UNEE_State);
        } while(getNextEdge(&tmp_edge));
        EDGE_STATE tmp_state = getBlockState(prev_data_block);
        std::cout << " [Nt: " << mEdgeStates[tmp_state][UN_BLOCK_STATE_NT_COUNT] <<
                ", Tr: " << mEdgeStates[tmp_state][UN_BLOCK_STATE_T_COUNT] <<
                ", De: " << mEdgeStates[tmp_state][UN_BLOCK_STATE_D_COUNT] <<
                ", Ba: " << mEdgeStates[tmp_state][UN_BLOCK_STATE_B_COUNT] <<
                ", To: " << mEdgeStates[tmp_state][UN_BLOCK_STATE_TOTAL_EDGE_COUNT] << "] " << tmp_state << std::endl;
        std::cout << "T: " << getTRank(UID) << " Nt: " << getNtRank(UID) << "\n++++" << std::endl;
        return;
    }
    else
    {
        std::cout << " No edges found\n++++" << std::endl;
        return;
    }
}
//HO 
//VO 

    /*
    ** A nicer way to see the edge state
    */
//VO virtual
template <int RL, int OFF>
/*HV*/ std::string
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ sayEdgeStateLikeAHuman(uMDInt state)
//HO ;
//VO = 0;
{
    //-----
    // A nicer way to see the edge state
    //
    switch(state)
    {
        case UN_EDGE_STATE_NTRANS:
            return "Non-Transitive";
        case UN_EDGE_STATE_TRANS:
            return "Transitive";
        case UN_EDGE_STATE_DETACHED:
            return "Detached";
        case UN_EDGE_STATE_BANISHED:
            return "Banished";
        default:
            return "***Random, fubar, take your pick***";
    }
}
//HO 
//VO 

    /*
    ** A nicer way to see the edge state
    */
//VO virtual
template <int RL, int OFF>
/*HV*/ std::string
UniNodeMemWrapperFixed<RL, OFF>::
/*HV*/ sayNodeTypeLikeAHuman(uMDInt type)
//HO ;
//VO = 0;
{
    //-----
    // A nicer way to see the edge state
    //
    switch(type)
    {
        case UN_NODE_TYPE_UNSET:
           return "Unset";
        case UN_NODE_TYPE_CAP:
            return "Cap";
        case UN_NODE_TYPE_PATH:
            return "Path";
        case UN_NODE_TYPE_CROSS:
            return "Cross";
        case UN_NODE_TYPE_DETACHED:
            return "Detached";
        case UN_NODE_TYPE_ATTACHED:
            return "Attached";
        case UN_NODE_TYPE_BANISHED:
            return "Banished";
        case UN_NODE_TYPE_CORRECTED:
            return "Corrected";
        default:
            return "***Random, fubar, take your pick***";
    }
}
//HO 
//VO 

