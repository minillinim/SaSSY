/******************************************************************************
**
** File: KmerMapMemWrapper.cpp
**
*******************************************************************************
**
** This class maps kmers found during edge making to the UniNodes which 
** they were found in. As the kmers are cut thay can be added into this
** map which is used to store lists of UniNodes referenced by kmers stored
** in a ReadStore Object (this is the reference)
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

#include <google/sparse_hash_map>
//INC #include <google/sparse_hash_map>
//INC 

// local includes
//INC #include "ReadStoreClass.h"
//INC 
#include "ReadStoreClass.h"

#include "intdef.h"
#include "IdTypeDefs.h"
#include "KmerMapMemWrapper.h"
#include "IdTypeSTL_ext.h"
#include "default_parameters.h"
#include "LoggerSimp.h"

/*INC*/ #include "Zobrist.h"
/*INC*/ 

/******************************************************************************
** #INCLUDES #DEFINES AND PRIVATE VARS
******************************************************************************/
//INC class KM_MAP_ELEM {
//INC 
//INC     public:
//INC 
//INC         KM_MAP_ELEM() { KME_NextHit = KmerMapDataId(); }
//INC
//INC         // to get some insight
//INC 
//INC         void printContents(void)
//INC 
//INC         {
//INC 
//INC             std::cout << "Read: " << KME_RSID << " Position: " << KME_Position << " Orientation: " << KME_Orientation << std::endl;
//INC 
//INC         }
//INC 
//INC         ReadStoreId KME_RSID;
//INC 
//INC         int KME_Position;
//INC 
//INC         bool KME_Orientation;
//INC 
//INC     private:
//INC 
//INC         KmerMapDataId KME_NextHit;
//INC 
//INC         friend class KmerMapMemWrapper;
//INC
//INC };
//INC 

/*INC*/ typedef google::sparse_hash_map<idInt, KmerMapId> KmerLookupMap;
/*INC*/ 
/*INC*/ typedef google::sparse_hash_map<idInt, KmerMapId>::iterator KmerLookupMapIterator;
/*INC*/ 

//PV KmerLookupMap * mKmerLookupMap;
//PV 
//PV Zobrist * mZobrist;
//PV 
//PV unsigned int mKmerLength;
//PV 
//PV ReadStoreClass * mReads;
//PV 
        /*
        ** Specialized saving and loading functions just for this object...
        */

static void writeMap(KmerLookupMap * writemap, ofstream * outFile)
{
    //-----
    // write a map to a file stream
    //
    int size = writemap->size();
    outFile->write(reinterpret_cast<char *>(&size), sizeof(unsigned int));
    KmerLookupMapIterator map_iter = writemap->begin();
    KmerLookupMapIterator map_last = writemap->end();
    while(map_iter != map_last)
    {
        idInt id = map_iter->first;
        outFile->write(reinterpret_cast<char *>(&id), sizeof(idInt));
        (map_iter->second).save(outFile);
        map_iter++;
    }
}

static void readMap(KmerLookupMap * readmap, ifstream * inFile)
{
    //-----
    // read a map from a filestream
    //
    // clear the map
    readmap->clear();
    int size;
    idInt id;
    inFile->read(reinterpret_cast<char *>(&size), sizeof(unsigned int));
    readmap->resize(size);
    while(size > 0)
    {
        inFile->read(reinterpret_cast<char *>(&id), sizeof(idInt));
        KmerMapId tmp_KMID(inFile);
        (*readmap)[id] = tmp_KMID;
        size--;
    }
}

/******************************************************************************
** INTIALISATION AND SAVING
******************************************************************************/

/*HV*/ void
KmerMapMemWrapper::
/*HV*/ extendedDestructor(void)
//HO ;
{
    //-----
    // Add anything you would like to happen when the destructor is called
    //
    if(mZobrist != NULL)
        delete mZobrist;
    mZobrist = NULL;
    
    if(mKmerLookupMap != NULL)
        delete mKmerLookupMap;
    mKmerLookupMap = NULL;
}
//HO 

/*HV*/ bool
KmerMapMemWrapper::
/*HV*/ extendedInitialise(uMDInt originalNumberOfBlocks )
//HO ;
{
    //-----
    // Add anything you would like to happen on a regular intialisation call
    //
    mKmerLookupMap = new KmerLookupMap();
    mKmerLookupMap->set_deleted_key(0);

    return true;
}
//HO 

/*HV*/ bool
KmerMapMemWrapper::
/*HV*/ extendedInitialise(std::string fileName)
//HO ;
{
    //-----
    // Add anything you would like to happen when reloading a saved instance
    //
    // open our file
    std::ifstream myFile(fileName.c_str(), std::ifstream::binary);

    // get the length of the kmers
    myFile.read(reinterpret_cast<char *>(&mKmerLength), sizeof(unsigned int));

    // load the lookupmap
    mKmerLookupMap = new KmerLookupMap();
    mKmerLookupMap->set_deleted_key(0);
    readMap(mKmerLookupMap, &myFile);

    // close the file
    myFile.close();

    // load the Zobrist
    mZobrist = new Zobrist(mKmerLength);
    PARANOID_ASSERT_L2(mZobrist != NULL);
    mZobrist->loadZobrist(fileName + ".Zobrist");
    
    return true;
}
//HO 

/*HV*/ bool
KmerMapMemWrapper::
/*HV*/ extendedSave(std::string fileName)
//HO ;
{
    //-----
    // Add anything you would like to happen when saving (including saving private vars)
    //
    // open our file
    std::ofstream myFile(fileName.c_str(), std::ofstream::binary);

    // save the length of the kmers
    myFile.write(reinterpret_cast<char *>(&mKmerLength), sizeof(unsigned int));
    
    // save our node lists
    writeMap(mKmerLookupMap, &myFile);

    // save our node lists
    writeMap(mKmerLookupMap, &myFile);

    // close the file
    myFile.close();

    // save the Zobrist
    mZobrist->saveZobrist(fileName + ".Zobrist");
    
    return true;
}
//HO 

/*HV*/ void
KmerMapMemWrapper::
/*HV*/ initialise2(unsigned int KL, ReadStoreClass * RSC)
//HO ;
{
    //-----
    // Extra initialisation stuff...
    //
    mKmerLength = KL;
    mReads = RSC;
    
    // intialise the Zobrist    
    mZobrist = new Zobrist(KL);
    PARANOID_ASSERT_L2(mZobrist != NULL);
}
//HO 

/******************************************************************************
** EXTENSIONS
******************************************************************************/

/*HV*/ bool
KmerMapMemWrapper::
/*HV*/ addElem(std::string * sequence, bool * reversed, unsigned int position, ReadStoreId RSID, KM_MAP_ELEM * data)
//HO ;
{
    //-----
    // Add a kmer to the map. We check to see if the kmer was already added
    // and return false only if it is the first time we've seen this kmer...
    // ie. data is invalid.
    // 
    // Thus this function doubles as a "getElem" type function.
    //
    KmerMapId chain_KMID = KM_NULL_ID;
    
    // first work out if the sequence is in the correct orientation...
    orientateSequence(reversed, sequence);
    
    idInt hashed = mZobrist->hashString(sequence);
    KmerLookupMapIterator kmer_map_iter = mKmerLookupMap->find(hashed);

    if(kmer_map_iter == mKmerLookupMap->end())
    {
        // nothing has hashed this way previously
        // get a new element
        KmerMapId new_id = newKmerMapId();

        // set its values
        setRs(RSID, new_id);
        setPosition(position, new_id);
        setOrientation(*reversed, new_id);
        setNextKmer(KM_NULL_ID, new_id);
        setNextElem(KMD_NULL_ID, new_id);

        // add it to the map
        (*mKmerLookupMap)[hashed] = new_id;

        // nothing to do to data
        return false;       
    }
    else
    {
        // something has hashed here previously. Was it the same kmer
        // or just a side effect of the hash? Go through all the "head" kmers
        // and see...
        KmerMapId curr_KMID = (*kmer_map_iter).second;
        do {
            unsigned int L_position = getPosition(curr_KMID);
            bool L_orient = isOrientation(curr_KMID);
            ReadStoreId L_RSID = getRs(curr_KMID);
            if(mReads->getSubSequence(L_position, mKmerLength, L_orient, L_RSID) == *sequence)
            {
                // seen this guy before
                // make a new data block. This will hold the info from the current
                // "head" so there is a little shuffling to do...
                KmerMapDataId new_tail = newKmerMapDataId();

                // set its values (shuffle)
                setRs(L_RSID, new_tail);
                setPosition(L_position, new_tail);
                setOrientation(L_orient, new_tail);

                // we are going to return this guy, so we may as well set "data" here now
                data->KME_RSID = L_RSID;
                data->KME_Position = L_position;
                data->KME_Orientation = L_orient;

                // now we can update the "head"
                setRs(RSID, curr_KMID);
                setPosition(position, curr_KMID);
                setOrientation(*reversed, curr_KMID);
                
                // we need to tack this new data entry just behind the head
                // we will join the rest of the chain onto the end
                // of the new_tail
                KmerMapDataId current_tail = getNextElem(curr_KMID);
                if(KMD_NULL_ID != current_tail)
                {
                    // some kind of tail exists
                    // tack it onto the end of the "new" guy
                    setNextElem(current_tail, new_tail);
                    data->KME_NextHit = current_tail;
                }
                else
                {
                    data->KME_NextHit = KMD_NULL_ID;
                }

                // in any case the "new" guy sits just behind the head...
                // add this to the head
                setNextElem(new_tail, curr_KMID);

                // data is updated, return true...
                return true;
            }
            chain_KMID = curr_KMID;
            curr_KMID = getNextKmer(curr_KMID);            
        } while(KM_NULL_ID != curr_KMID);
    }
    
    // if we're here then the kmer was not found
    // we can add this guy as the first entry at the end of the "head" chain

    // get a new element
    KmerMapId new_id = newKmerMapId();

    // set its values
    setRs(RSID, new_id);
    setPosition(position, new_id);
    setOrientation(*reversed, new_id);
    setNextKmer(KM_NULL_ID, new_id);
    setNextElem(KMD_NULL_ID, new_id);
    
    // add this mo-fo to the end of the chain...
    setNextKmer(new_id, chain_KMID);

    // nothing to do to data
    return false;
}
//HO 

#if 0
/*HV*/ bool
KmerMapMemWrapper::
/*HV*/ addElem(ReadStoreId RID, KM_MAP_ELEM * data)
//HO ;
{
    //-----
    // Add a kmer to the map
    //
    // check and see if we added this guy before
    KmerLookupIter finder = mKmerLookupMap->find(RID.getGuts());
    if(finder != mKmerLookupMap->end())
    {
        // this guy exists!
        // make a new data block
        KmerMapDataId new_tail = newKmerMapDataId();
        // set its values
        setUn(data->KME_UID, new_tail);
        setPosition(data->KME_Position, new_tail);
        setOrientation(data->KME_Orientation, new_tail);

        // we need to tack the new entry just behind the head
        // we will join the rest of the chain onto the end
        // of the new_tail
        KmerMapDataId current_tail = getNextElem(finder->second);
        if(KMD_NULL_ID != current_tail)
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
    KmerMapId new_id = newKmerMapId();
    // set its values
    setUn(data->KME_UID, new_id);
    setPosition(data->KME_Position, new_id);
    setOrientation(data->KME_Orientation, new_id);
    
    // add it to the map
    (*mKmerLookupMap)[RID.getGuts()] = new_id;
    return true;
}
//HO 
#endif

/*HV*/ bool
KmerMapMemWrapper::
/*HV*/ getNextElem(KM_MAP_ELEM * data)
//HO ;
{
    //-----
    // get the next matching kmer from the map
    //
    if(data->KME_NextHit == KMD_NULL_ID || data->KME_NextHit == KmerMapDataId())
    {
        return false;
    }
    else
    {
        data->KME_RSID = getRs(data->KME_NextHit);
        data->KME_Position = getPosition(data->KME_NextHit);
        data->KME_Orientation = isOrientation(data->KME_NextHit);
        data->KME_NextHit = getNextElem(data->KME_NextHit);
        return true;
    }
}
//HO 

/*HV*/ void
KmerMapMemWrapper::
/*HV*/ orientateSequence(bool * reversed, std::string * sequence)
//HO ;
{
    //-----
    // lexicographically orientate reads. (works for DNA space and color space)
    // this only works is the reads are odd length. The decision on which
    // orientation is lower can be made by the time we are half way through the readlength
    // so we do this in two loops to save some time
    //
    
#ifdef TRANSCRIPTOMIC_
    // orientation for transcriptomic data sets
    *reversed = false;
    return;
#endif
    
    const char * in_buff_start = sequence->c_str();
    const char * in_buff_end = in_buff_start + mKmerLength - 1;
    char out_buff[mKmerLength];
    char *pOut_buff = out_buff;
    int i = mKmerLength;
    while(1)
    {
        switch(*in_buff_end)
        {
            case SAS_CE_D_0:
                *pOut_buff = SAS_CE_R_0;
                break;
            case SAS_CE_D_1:
                *pOut_buff = SAS_CE_R_1;
                break;
            case SAS_CE_D_2:
                *pOut_buff = SAS_CE_R_2;
                break;
            case SAS_CE_D_3:
                *pOut_buff = SAS_CE_R_3;
                break;
        }
        
        if(*pOut_buff != *in_buff_start)
            break;
        
        pOut_buff++;
        in_buff_start++;
        in_buff_end--;
        i--;
    }
    
    if(*in_buff_start < *pOut_buff)
    {
        // the original was lower
        // so stop here...
        *reversed = false;
        return;
    }
    
    // now we must always hit the break statement in the above loop
    // if the input sequences are odd length
    // so we need to do the last ++/-- now
    pOut_buff++;
    in_buff_end--;
    i--;
    
    // in any case, if we are here then the reversed must be the lower
    *reversed = true;
    
    // now finish the reversed string
    while(i)
    {
        switch(*in_buff_end)
        {
            case SAS_CE_D_0:
                *pOut_buff = SAS_CE_R_0;
                break;
            case SAS_CE_D_1:
                *pOut_buff = SAS_CE_R_1;
                break;
            case SAS_CE_D_2:
                *pOut_buff = SAS_CE_R_2;
                break;
            case SAS_CE_D_3:
                *pOut_buff = SAS_CE_R_3;
                break;
        }
        i--;
        pOut_buff++;
        in_buff_end--;
    }
    
    sequence->assign(out_buff, mKmerLength);
}
//HO 
