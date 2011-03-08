/******************************************************************************
**
** File: KmerMatchMapMemWrapper.cpp
**
*******************************************************************************
**
** A throwaway class used whenw e try to amke edges in the uninode graph
** Simply stored Kmers vs their position and orientation within a read
** and methods to access this info
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
#include "intdef.h"
#include "IdTypeDefs.h"
#include "KmerMatchMapMemWrapper.h"

using namespace std;

/******************************************************************************
** #INCLUDES #DEFINES AND PRIVATE VARS
******************************************************************************/
//INC class KMM_MAP_ELEM {
//INC 
//INC     public:
//INC 
//INC         KMM_MAP_ELEM() { KMME_NextHit = KmerMatchMapDataId(); }
//INC
//INC         // to get some insight
//INC 
//INC         void printContents(void)
//INC 
//INC         {
//INC 
//INC             std::cout << " QPosition: " << KMME_QPosition << " QOffset: " << KMME_QOffset << " BPosition: " << KMME_BPosition << " BOffset: " << KMME_BOffset << " OrientsMatch: " << KMME_OrientMatch << std::endl;
//INC 
//INC         }
//INC 
//INC         sMDInt KMME_QOffset;
//INC 
//INC         uMDInt KMME_QPosition;
//INC 
//INC         sMDInt KMME_BOffset;
//INC 
//INC         uMDInt KMME_BPosition;
//INC 
//INC         bool KMME_OrientMatch;
//INC 
//INC     private:
//INC 
//INC         KmerMatchMapDataId KMME_NextHit;
//INC 
//INC         friend class KmerMatchMapMemWrapper;
//INC
//INC };
//INC 
/*INC*/ #include <map>
/*INC*/ 
/*INC*/ typedef map<UniNodeId, KmerMatchMapId>::iterator KmerMatchLookupIter;
/*INC*/ 
//PV map<UniNodeId, KmerMatchMapId> * mKmerMatchLookupMap;
//PV 
//PV uMDInt mNumBlocks;
//PV 
/******************************************************************************
** INTIALISATION AND SAVING
******************************************************************************/

/*HV*/ void
KmerMatchMapMemWrapper::
/*HV*/ extendedDestructor(void)
//HO ;
{
    //-----
    // Add anything you would like to happen when the destructor is called
    //
    if(  mKmerMatchLookupMap != NULL )
        delete mKmerMatchLookupMap;
    mKmerMatchLookupMap = NULL;    
}
//HO 

/*HV*/ bool
KmerMatchMapMemWrapper::
/*HV*/ extendedInitialise(uMDInt originalNumberOfBlocks )
//HO ;
{
    //-----
    // Add anything you would like to happen on a regular intialisation call
    //
    mNumBlocks = originalNumberOfBlocks;
    mKmerMatchLookupMap = new map<UniNodeId, KmerMatchMapId>;
    return true;
}
//HO 

/*HV*/ bool
KmerMatchMapMemWrapper::
/*HV*/ extendedInitialise(std::string fileName)
//HO ;
{
    //-----
    // Add anything you would like to happen when reloading a saved instance
    //
    mKmerMatchLookupMap = new map<UniNodeId, KmerMatchMapId>;
    return true;
}
//HO 

/*HV*/ bool
KmerMatchMapMemWrapper::
/*HV*/ extendedSave(std::string fileName)
//HO ;
{
    //-----
    // Add anything you would like to happen when saving (including saving private vars)
    //
    return true;
}
//HO 

/******************************************************************************
** EXTENSIONS
******************************************************************************/

/*HV*/ bool
KmerMatchMapMemWrapper::
/*HV*/ addElem(UniNodeId UID, KMM_MAP_ELEM * data)
//HO ;
{
    //-----
    // Add a kmerMatch to the map
    //
    // check and see if we added this guy before
    KmerMatchLookupIter finder = mKmerMatchLookupMap->find(UID);
    if(finder != mKmerMatchLookupMap->end())
    {
        // this guy exists!
        // make a new data block
        KmerMatchMapDataId new_tail = newKmerMatchMapDataId();
        // set its values
        setQPosition(data->KMME_QPosition, new_tail);
        setQOffset(data->KMME_QOffset, new_tail);
        setBPosition(data->KMME_BPosition, new_tail);
        setBOffset(data->KMME_BOffset, new_tail);
        setOrientMatch(data->KMME_OrientMatch, new_tail);

        // we need to tack the new entry just behind the head
        // we will join the rest of the chain onto the end
        // of the new_tail
        KmerMatchMapDataId current_tail = getNextElem(finder->second);
        if(KMMD_NULL_ID != current_tail)
        {
            // first data block
            setNextElem(current_tail, new_tail);
        }

        // add this to the head
        setNextElem(new_tail, finder->second);

        // done
        return true;
    }
    // get a new element
    KmerMatchMapId new_id = newKmerMatchMapId();
    // set its values
    setQPosition(data->KMME_QPosition, new_id);
    setQOffset(data->KMME_QOffset, new_id);
    setBPosition(data->KMME_BPosition, new_id);
    setBOffset(data->KMME_BOffset, new_id);
    setOrientMatch(data->KMME_OrientMatch, new_id);

    // add it to the map
    mKmerMatchLookupMap->insert(pair<UniNodeId, KmerMatchMapId>(UID, new_id));
    return true;
}
//HO 

/*HV*/ bool
KmerMatchMapMemWrapper::
/*HV*/ getElem(UniNodeId UID, KMM_MAP_ELEM * data)
//HO ;
{
    //-----
    // Get the first kmerMatch from the map
    //
    KmerMatchLookupIter finder = mKmerMatchLookupMap->find(UID);
    if(finder != mKmerMatchLookupMap->end())
    {
        data->KMME_QPosition = getQPosition(finder->second);
        data->KMME_QOffset = getQOffset(finder->second);
        data->KMME_BPosition = getBPosition(finder->second);
        data->KMME_BOffset = getBOffset(finder->second);
        data->KMME_OrientMatch = isOrientMatch(finder->second);
        
        KmerMatchMapDataId next_hit = getNextElem(finder->second);
        if(KMMD_NULL_ID == next_hit)
        {
            data->KMME_NextHit = KMMD_NULL_ID;
        }
        else
        {
            data->KMME_NextHit = next_hit;
        }
        // done
        return true;
    }
    // we don't have this fella
    return false;
}
//HO 

/*HV*/ bool
KmerMatchMapMemWrapper::
/*HV*/ getNextElem(KMM_MAP_ELEM * data)
//HO ;
{
    //-----
    // get the next kmerMatch from the map
    //
    if(data->KMME_NextHit == KMMD_NULL_ID || data->KMME_NextHit == KmerMatchMapDataId())
    {
        return false;
    }
    else
    {
        data->KMME_QPosition = getQPosition(data->KMME_NextHit);
        data->KMME_QOffset = getQOffset(data->KMME_NextHit);
        data->KMME_BPosition = getBPosition(data->KMME_NextHit);
        data->KMME_BOffset = getBOffset(data->KMME_NextHit);
        data->KMME_OrientMatch = isOrientMatch(data->KMME_NextHit);

        KmerMatchMapDataId next_hit = getNextElem(data->KMME_NextHit);
        if(KMMD_NULL_ID == next_hit)
        {
            data->KMME_NextHit = KMMD_NULL_ID;
        }
        else
        {
            data->KMME_NextHit = next_hit;
        }
        // done
        return true;
    }
}
//HO 

/*HV*/ bool
KmerMatchMapMemWrapper::
/*HV*/ clearKmerMatchMap(void)
//HO ;
{
    //-----
    // remove all entries from the map, we could just delete the map but it would be smarter
    // to just clear the values
    //

    KmerMatchLookupIter kmm_iter = mKmerMatchLookupMap->begin();
    KmerMatchLookupIter kmm_last = mKmerMatchLookupMap->end();
    KmerMatchMapDataId tmp_KMMDID, delay_KMMDID;
    while(kmm_iter != kmm_last)
    {
        delay_KMMDID = getNextElem(kmm_iter->second);
        // delete the head node
        deleteKmerMatchMapId(kmm_iter->second);
        while(delay_KMMDID != KMMD_NULL_ID)
        {
            tmp_KMMDID = getNextElem(delay_KMMDID);
            deleteKmerMatchMapDataId(delay_KMMDID);
            delay_KMMDID = tmp_KMMDID;
        }
        kmm_iter++;
    }

    // clear the map...
    if(  mKmerMatchLookupMap != NULL )
        delete mKmerMatchLookupMap;
    mKmerMatchLookupMap = new map<UniNodeId, KmerMatchMapId>;
    return true;
}
//HO 
 
