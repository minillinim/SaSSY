/******************************************************************************
**
** File: ReadStoreMemWrapper.extended.h
**
*******************************************************************************
**
** The class for storing and compressing reads from the data file
** no edge information is stored here. Just a plain vanilla data wrapper
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
** INCLUDES #DEFINES AND PRIVATE VARS
******************************************************************************/

#include "IdTypeSTL_ext.h"

#include <google/sparse_hash_map>
//INC #include <google/sparse_hash_map>
//INC 

// extra includes for the h file
/*INC*/ #include <map>
/*INC*/ 
/*INC*/ #include "char_encodings.h"
/*INC*/ 
/*INC*/ #include <sstream>
/*INC*/ 
/*INC*/ #include "LoggerSimp.h"
/*INC*/ 
/*INC*/ #include "Zobrist.h"
/*INC*/ 
/*INC*/ typedef google::sparse_hash_map<idInt, ReadStoreId> StringMap;
/*INC*/ 
/*INC*/ typedef google::sparse_hash_map<idInt, ReadStoreId>::iterator StringMapIterator;
/*INC*/ 

//PV StringMap * mStringMap;                                        // map for hashing strings to ReadStoreIds
//PV 
//PV std::string mSpaceString;                                      // spaceString equal to the readlength
//PV 
//PV uMDInt mTransLU[4];                                            // used in isOverlapped for comparisons
//PV 
//PV Zobrist * mZobrist;
//PV 

static void writeMap(google::sparse_hash_map<idInt, ReadStoreId> * writemap, ofstream * outFile)
{
    //-----
    // write a map to a file stream
    //
    int size = writemap->size();
    outFile->write(reinterpret_cast<char *>(&size), sizeof(unsigned int));
    google::sparse_hash_map<idInt, ReadStoreId>::iterator map_iter = writemap->begin();
    google::sparse_hash_map<idInt, ReadStoreId>::iterator map_last = writemap->end();
    while(map_iter != map_last)
    {
        idInt id = map_iter->first;
        outFile->write(reinterpret_cast<char *>(&id), sizeof(idInt));
        (map_iter->second).save(outFile);
        map_iter++;
    }
}

static void readMap(google::sparse_hash_map<idInt, ReadStoreId> * readmap, ifstream * inFile)
{
    //-----
    // read a map from a filestream
    //
    // clear the map
    readmap->clear();
    int size;
    idInt id;
    inFile->read(reinterpret_cast<char *>(&size), sizeof(unsigned int));
    // resize once here...
    readmap->resize(size);
    while(size > 0)
    {
        inFile->read(reinterpret_cast<char *>(&id), sizeof(idInt));
        ReadStoreId tmp_RID(inFile);
        (*readmap)[id] = tmp_RID;
        size--;
    }
}

        
/******************************************************************************
** INTIALISATION AND SAVING
******************************************************************************/
//VO virtual
template <int RL>
/*HV*/ void
ReadStoreMemWrapperFixed<RL>::
/*HV*/ extendedDestructor(void)
//HO ;
//VO = 0;
{
    //-----
    // Add anything you would like to happen when the destructor is called
    //
    if(mZobrist != NULL)
        delete mZobrist;
    mZobrist = NULL;

    if(mStringMap != NULL)
        delete mStringMap;
    mStringMap = NULL;
}
//HO 
//VO 

//VO virtual
template <int RL>
/*HV*/ bool
ReadStoreMemWrapperFixed<RL>::
/*HV*/ extendedInitialise(uMDInt originalNumberOfBlocks )
//HO ;
//VO = 0;
{
    //-----
    // Add anything you would like to happen on a regular intialisation call
    //
    // make the space string
    const char* bad_food = "~BAADF00D~";
    mSpaceString = "";
    int count = 0;
    for(int i = 0; i < RL; i++)
    {
        mSpaceString += bad_food[count];
        count++;
        if(count == 10)
            count = 0;
    }
    
    // set the translation array
    mTransLU[SAS_CE_H_0] = SAS_CE_U_0;
    mTransLU[SAS_CE_H_1] = SAS_CE_U_1;
    mTransLU[SAS_CE_H_2] = SAS_CE_U_2;
    mTransLU[SAS_CE_H_3] = SAS_CE_U_3;
    
    // intialise the Zobrist
    mZobrist = new Zobrist(RL);
    PARANOID_ASSERT_L2(mZobrist != NULL);
    
    // we need a new stringmap
    mStringMap = new StringMap();
    mStringMap->set_deleted_key(0);
    
    return true;
}
//HO 
//VO 

//VO virtual
template <int RL>
/*HV*/ bool
ReadStoreMemWrapperFixed<RL>::
/*HV*/ extendedInitialise(std::string fileName)
//HO ;
//VO = 0;
{
    //-----
    // Add anything you would like to happen when reloading a saved instance
    //
        
    // make the space string
    const char* bad_food = "~BAADF00D~";
    mSpaceString = "";
    int count = 0;
    for(int i = 0; i < RL; i++)
    {
        mSpaceString += bad_food[count];
        count++;
        if(count == 10)
            count = 0;
    }

    // set the translation array
    mTransLU[SAS_CE_H_0] = SAS_CE_U_0;
    mTransLU[SAS_CE_H_1] = SAS_CE_U_1;
    mTransLU[SAS_CE_H_2] = SAS_CE_U_2;
    mTransLU[SAS_CE_H_3] = SAS_CE_U_3;

    // open our file
    std::ifstream myFile(fileName.c_str(), std::ifstream::binary);

    // load the stringMap
    mStringMap = new StringMap();
    mStringMap->set_deleted_key(0);
    readMap(mStringMap, &myFile);

    // close the file
    myFile.close();
    
    // load the Zobrist
    mZobrist = new Zobrist(RL);
    PARANOID_ASSERT_L2(mZobrist != NULL);
    mZobrist->loadZobrist(fileName + ".Zobrist");

    return true;
}
//HO 
//VO 

//VO virtual
template <int RL>
/*HV*/ bool
ReadStoreMemWrapperFixed<RL>::
/*HV*/ extendedSave(std::string fileName)
//HO ;
//VO = 0;
{
    //-----
    // Add anything you would like to happen when saving (including saving private vars)
    //
    // open our file
    std::ofstream myFile(fileName.c_str(), std::ofstream::binary);

    // save the stringMap
    writeMap(mStringMap, &myFile);

    // close the file
    myFile.close();

    // save the Zobrist
    mZobrist->saveZobrist(fileName + ".Zobrist");
    
    return true;
}
//HO 
//VO

/******************************************************************************
** STRING MANIPULATION AND GET/SET
******************************************************************************/

    /*
    ** get the readlength
    */
//VO virtual inline int getReadLength(void) { return 0; }
//HO inline int getReadLength(void) { return RL; }
//HO 
//VO 
 
    /*
    ** get the number of sequences stored
    */
//VO virtual inline int getNumSequences(void) { return 0; }
//HO inline int getNumSequences(void) { return mStringMap->size(); }
//HO 
//VO 
    
    /*
    ** get the spaceString
    */
//VO virtual inline std::string getSpaceString(void) { return ""; }
//HO inline std::string getSpaceString(void) { return mSpaceString; }
//HO 
//VO 

//VO virtual
template <int RL>
/*HV*/ std::string
ReadStoreMemWrapperFixed<RL>::
/*HV*/ getSequence(ReadStoreId RID)
//HO ;
//VO = 0;
{
    string ret_string = mSpaceString;
    
    // get the compressed sequence
    uMDInt tmp_array[RS_DATA_SIZE];
    getReadData(tmp_array, RID);
    
    // turn it into a string
    expandSequence(&ret_string, tmp_array);

    return ret_string;
}
//HO 
//VO 

//VO virtual
template <int RL>
/*HV*/ std::string
ReadStoreMemWrapperFixed<RL>::
/*HV*/ getSubSequence(unsigned int Kstart, unsigned int Klength, bool Korient, ReadStoreId RID)
//HO ;
//VO = 0;
{
    string ret_string = mSpaceString;
    
    // get the compressed sequence
    uMDInt tmp_array[RS_DATA_SIZE];
    getReadData(tmp_array, RID);
    
    // turn it into a string
    expandSequence(&ret_string, tmp_array);

    // reverse compliment if necessary
    if(Korient)
    {
        revCmpSequence(&ret_string);
        return ret_string.substr(RL - Klength - Kstart, Klength);
    }
    else
    {
        return ret_string.substr(Kstart, Klength);
    }
}
//HO 
//VO 

//VO virtual
template <int RL>
/*HV*/ ReadStoreId
ReadStoreMemWrapperFixed<RL>::
/*HV*/ getReadStoreId(std::string * sequence)
//HO ;
//VO = 0;
{
    //-----
    // get the ReadStoreId for a sequence
    //
    ReadStoreId RID = RS_NULL_ID;
    std::string copy_seq = *sequence;
    bool reversed;
    orientateSequence(&reversed, &copy_seq);

    idInt hashed = mZobrist->hashString(&copy_seq);
    StringMapIterator string_map_iter = mStringMap->find(hashed);

    if(string_map_iter != mStringMap->end())
    {
        ReadStoreId curr_RID = (*string_map_iter).second;
        do {
            if(getSequence(curr_RID) == copy_seq)
            {
                // got our guy!
                return curr_RID;
            }
            curr_RID = getRsc(curr_RID);
        } while(RS_NULL_ID != curr_RID);
    }
    return RS_NULL_ID;
}
//HO 
//VO 

//VO virtual
template <int RL>
/*HV*/ bool
ReadStoreMemWrapperFixed<RL>::
/*HV*/ getReadStoreId(ReadStoreId * RID, std::string * sequence)
//HO ;
//VO = 0;
{
    //-----
    // get the ReadStoreId for a sequence
    //
    ReadStoreId ret_RID = getReadStoreId(sequence);
    if(RS_NULL_ID != ret_RID)
    {
        *RID = ret_RID;
        return true;
    }
    return false;
}
//HO 
//VO 

//VO virtual
template <int RL>
/*HV*/ ReadStoreId
ReadStoreMemWrapperFixed<RL>::
/*HV*/ addSequence(bool * originalSequence, bool * reversed, std::string * sequence)
//HO ;
//VO = 0;
{
    //-----
    // Add a sequence to the ReadStore.
    //
    orientateSequence(reversed, sequence);
    
    ReadStoreId chain_RSID = RS_NULL_ID;

    idInt hashed = mZobrist->hashString(sequence);
    StringMapIterator string_map_iter = mStringMap->find(hashed);
    if(string_map_iter != mStringMap->end())
    {
        ReadStoreId curr_RSID = (*string_map_iter).second;
        do {
            if(getSequence(curr_RSID) == *sequence)
            {
                // seen this guy before
                *originalSequence = false;
                return curr_RSID;
            }
            chain_RSID = curr_RSID;
            curr_RSID = getRsc(curr_RSID);
        } while(RS_NULL_ID != curr_RSID);
    }
    // else must be a new sequence

    // try to compress the sequence
    uMDInt tmp_array[RS_DATA_SIZE];
    if(!compressSequence(tmp_array, sequence)) { return RS_NULL_ID; }

    // compression worked!
    *originalSequence = true;    

    // get a new ID
    ReadStoreId new_RSID = newReadStoreId();
    
    // write it into the store
    setReadData(tmp_array, new_RSID);

    // update the chain if need be
    if(RS_NULL_ID != chain_RSID)
    {
        // chain it on
        setRsc(new_RSID, chain_RSID);
    }
    else
    {
        // only if string_map_iter == mStringMap->end()
        // *brand new*, so put it in the map
        (*mStringMap)[hashed] = new_RSID;
    }

    // done
    return new_RSID;
}
//HO 
//VO 

    /*
    ** Check to see if we have stored this sequence
    */
//VO virtual inline ReadStoreId addSequence(bool * reversed, std::string * sequence) { ReadStoreId foo; return foo; }
//HO inline ReadStoreId addSequence(bool * reversed, std::string * sequence) { bool orig_seq; return addSequence(&orig_seq, reversed, sequence); }
//HO 
//VO 

    /*
    ** Check to see if we have stored this sequence
    */
//VO virtual inline bool isSequenceStored(std::string * sequence) { return false; }
//HO inline bool isSequenceStored(std::string * sequence) { return RS_NULL_ID != getReadStoreId(sequence); }
//HO 
//VO 

    /*
    ** Check to see if we have stored this sequence
    */
//VO virtual inline bool isSequenceAt(std::string * sequence, ReadStoreId RID) { return false; }
//HO inline bool isSequenceAt(std::string * sequence, ReadStoreId RID) { return RID == getReadStoreId(sequence); }
//HO 
//VO 

//VO virtual
template <int RL>
/*HV*/ void
ReadStoreMemWrapperFixed<RL>::
/*HV*/ orientateSequence(bool * reversed, std::string * sequence)
//HO ;
//VO = 0;
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
    const char * in_buff_end = in_buff_start + RL - 1;
    char out_buff[RL];
    char *pOut_buff = out_buff;
    int i = RL;
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
    
    sequence->assign(out_buff, RL);
}
//HO 
//VO 

//VO virtual
template <int RL>
/*HV*/ void
ReadStoreMemWrapperFixed<RL>::
/*HV*/ revCmpSequence(std::string * sequence)
//HO ;
//VO = 0;
{
    //-----
    // get the reverse complement of a sequence
    //
    const char * in_buff_end = sequence->c_str() + RL - 1;
    char out_buff[RL];
    char *pOut_buff = out_buff;
    int i = RL;
    
    // just do it!!!
    while(0 != i)
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
    sequence->assign(out_buff, RL);
}
//HO 
//VO 

//VO virtual
template <int RL>
/*HV*/ bool
ReadStoreMemWrapperFixed<RL>::
/*HV*/ compressSequence(uMDInt * compressed, std::string * sequence)
//HO ;
//VO = 0;
{
    //----
    // compress a sequence ready for storing
    //
    int read_counter = 0;
    uMDInt current_word = 0;
    
    compressed--;
    
    for(int word_counter = 0; word_counter < RS_DATA_SIZE - 1; word_counter++)
    {
        compressed++;
        current_word = 0;

        for(int base_counter = 0; base_counter < RS_BASES_PER_INT; base_counter++)
        {
            current_word = current_word << 2;
            switch((*sequence)[read_counter])
            {
                case SAS_CE_D_0:
                    current_word |= SAS_CE_H_0;
                    break;
                case SAS_CE_D_1:
                    current_word |= SAS_CE_H_1;
                    break;
                case SAS_CE_D_2:
                    current_word |= SAS_CE_H_2;
                    break;
                case SAS_CE_D_3:
                    current_word |= SAS_CE_H_3;
                    break;
                default:
                    return false;
            }
            read_counter++;
        }
        *compressed = current_word;
    }
    
    compressed++;
    current_word = 0;

    int remaining = RL - read_counter;
        
    for(int base_counter = 0; base_counter < remaining; base_counter++)
    {
        current_word = current_word << 2;
        switch((*sequence)[read_counter])
        {
            case SAS_CE_D_0:
                current_word |= SAS_CE_H_0;
                break;
            case SAS_CE_D_1:
                current_word |= SAS_CE_H_1;
                break;
            case SAS_CE_D_2:
                current_word |= SAS_CE_H_2;
                break;
            case SAS_CE_D_3:
                current_word |= SAS_CE_H_3;
                break;
            default:
                return false;
        }
        read_counter++;
    }
    
    //
    // At this stage we will need to do some bitshifting left
    // Say read length is 19 bases and we can fit only 16 bases to an int
    // then we will have the lower 6 bits in the int full and the upper 26 bits
    // will be zeroes. so we need to shift up 26 bits either here or when we read
    // and seeing as we only write once and read many times it makes sense
    // to do this now.
    //
    // RL / BASES_PER_INT gives us the number of whole ints used. (1)
    // due to the "wonders" of integer division, ( RL / BASES_PER_INT ) * BASES_PER_INT 
    // gives us the number of bases stored in whole ints (16) and if we subtract this from RL
    // we get the number of bases left in the last word (3)
    // multiply this by 2 to get the number of bits (6) and then subtract all of this
    // from the SIZE_OF_INT (32) gives us the amount to shift left. (26)
    // the preprocessor will calculate this number pronto for us
    //
    
    current_word = current_word << ( SIZE_OF_INT - (RL - ( (RL / RS_BASES_PER_INT) * RS_BASES_PER_INT ) ) * 2 );
    *compressed = current_word;
    return true;
}
//HO 
//VO 

//VO virtual
template <int RL>
/*HV*/ void
ReadStoreMemWrapperFixed<RL>::
/*HV*/ expandSequence(std::string * sequence, uMDInt * compressed)
//HO ;
//VO = 0;
{
    //-----
    // convert a set of ints back into a string
    // we assume always that sequence is exactly RL chars long!
    //
    int read_counter = 0;
    uMDInt current_word = *compressed;

    // run out all the whole data words first
    for(int word_counter = 0; word_counter < RS_DATA_SIZE - 1; word_counter++)
    {
        for(int base_counter = 0; base_counter < RS_BASES_PER_INT; base_counter++)
        {
            switch(current_word & SAS_CE_U_3)
            {
                case SAS_CE_U_0:
                    (*sequence)[read_counter] = SAS_CE_D_0;
                    break;
                case SAS_CE_U_1:
                    (*sequence)[read_counter] = SAS_CE_D_1;
                    break;
                case SAS_CE_U_2:
                    (*sequence)[read_counter] = SAS_CE_D_2;
                    break;
                case SAS_CE_U_3:
                    (*sequence)[read_counter] = SAS_CE_D_3;
                    break;
            }
            current_word = current_word << 2;
            read_counter++;
        }
        compressed++;
        current_word = *compressed;
    }
    
    int remaining = RL - read_counter;

    // now do the final partial data word
    for(int base_counter = 0; base_counter < remaining; base_counter++)
    {
        switch(current_word & SAS_CE_U_3)
        {
            case SAS_CE_U_0:
                (*sequence)[read_counter] = SAS_CE_D_0;
                break;
            case SAS_CE_U_1:
                (*sequence)[read_counter] = SAS_CE_D_1;
                break;
            case SAS_CE_U_2:
                (*sequence)[read_counter] = SAS_CE_D_2;
                break;
            case SAS_CE_U_3:
                (*sequence)[read_counter] = SAS_CE_D_3;
                break;
        }
        current_word = current_word << 2;
        read_counter++;
    }
}
//HO 
//VO

//VO virtual
template <int RL>
/*HV*/ bool
ReadStoreMemWrapperFixed<RL>::
/*HV*/ isOverlapped(ReadStoreId bSeq, int bTrim, ReadStoreId qSeq, int qTrim, int len, bool revComp)
//HO ;
//VO = 0;
{
    //-----
    // do these two guys share an overlap?
    //
    // olap operator
    // these are used in the function makeEdges in Assembler.cpp
    // they probably do not belong here, but they are here for speed reasons
    // the main goal is given a string of length RL (for the assembly)
    // to determine if we should make an edge between the mo-fos, ie. do they overlap correctly at the string level?
    // this requires some hacking here but I will try to limit the carnage to this specific area
    // of the code. Try to understand the rest of the module before looking at this.
    //
    // Explanation of the input variables
    //
    // bSeq is an element in the readStore. as is qSeq
    // bTrim and qTrim indicate how many bases we should trim off the start of
    // reads AS THEY ARE WRITTEN. We will always compare b from the start of the untrimmed
    // bases to the end. len is the number of bases we will compare. 
    // revComp indicates if we should compare bases in q starting from the
    // start of the untrimmed sequence or (len + qTrim) bases along the sequence working towards the start
    //
    // example input: (these sequences overlap)
    //
    // b: GAAAAGTGTGTAAGCTGAATAGATC q: AAAAGTGTGTAAGCTGAATAGATCT
    // bTrim: 1 qTrim: 0 len: 24 revC: 0
    //
    // b: GAGTCTTTGCTTGTTACTTTGCCAC q: GGCAAAGTAACAAGCAAAGACTCAA
    // bTrim: 0 qTrim: 0 len: 23 revC: 1
    //
    // one EXCELLENT property. the reverse compliment is just the read from the other end
    // XORED with 0xFFF...for Illumina data or 0x0000 for SOLiD data
    
#if (SIZE_OF_INT == 64)
# ifdef SOLID_DATA_
    uMDInt complimentoR = 0x0000000000000000;
# else
    uMDInt complimentoR = 0xFFFFFFFFFFFFFFFF;
# endif
#elif (SIZE_OF_INT == 32)
# ifdef SOLID_DATA_
    uMDInt complimentoR = 0x00000000;
# else
    uMDInt complimentoR = 0xFFFFFFFF;
# endif
#else
    uMDInt complimentoR = 0;
    cout << "ReadStoreMemWrapperFixed<RL>::isOverlapped ***_ERROR _*** Unexpected integer size: " << SIZE_OF_INT << endl;
#endif

    //
    // first we have to shift down the base so that it starts at the right position
    // we can just use the resident int* for the base
    //
    uMDInt b_compressed[RS_DATA_SIZE];
    getReadData(b_compressed, bSeq);
    
    int b_char_counter = 0;
    int b_word_counter = 0;
    uMDInt b_current_word = *b_compressed;

    // shift it down by the right amount
    while(bTrim > 0)
    {
        // should we move down whole words?
        while(bTrim > RS_BASES_PER_INT)
        {
            bTrim -= RS_BASES_PER_INT;
            b_word_counter++;
            b_current_word = b_compressed[b_word_counter];
            PARANOID_ASSERT_L2(bTrim > 0);
        }
        // now there is only part of a word to shift
        b_current_word = b_current_word << 2;
        b_char_counter++;
        bTrim--;
    }

    // load the query string
    uMDInt q_compressed[RS_DATA_SIZE];
    getReadData(q_compressed, qSeq);
    
    uMDInt q_current_word;
    int q_char_counter;
    int q_word_counter;

    // next we have to make sure the query is shifted down right too
    if(revComp)
    {
        // we have to count from the back, back in...
        q_word_counter = RS_DATA_SIZE - 1;
        q_current_word = q_compressed[q_word_counter] ^ complimentoR;

        // in any case, we'll have to shift the last bits right up to the end of the int
        // this is the opposite of what we had to do in compress string
        q_current_word = q_current_word >> ( SIZE_OF_INT - (RL - ( (RL / RS_BASES_PER_INT) * RS_BASES_PER_INT ) ) * 2 );
        q_char_counter = RS_BASES_PER_INT - (RL - ( (RL / RS_BASES_PER_INT) * RS_BASES_PER_INT ) );

        
        // now we shift further right for the offset
        int shifter = RL - len - qTrim;
        while(shifter > 0)
        {
            // shift down whole words
            while(shifter > RS_BASES_PER_INT)
            {
                shifter -= RS_BASES_PER_INT;
                q_word_counter--;
                q_current_word = q_compressed[q_word_counter] ^ complimentoR;
                PARANOID_ASSERT_L2(shifter > 0);
                q_char_counter = 0;
            }
            q_current_word = q_current_word >> 2;
            q_char_counter++;
            shifter--;
        }
    }
    else
    {
        q_word_counter = 0;
        q_char_counter = 0;
        q_current_word = *q_compressed;

        while(qTrim > 0)
        {
            while(qTrim > RS_BASES_PER_INT)
            {
                qTrim -= RS_BASES_PER_INT;
                q_word_counter++;
                q_current_word = q_compressed[q_word_counter];
                PARANOID_ASSERT_L2(qTrim > 0);
            }
            q_current_word = q_current_word << 2;
            q_char_counter++;
            qTrim--;
        }

    }

    // now everything is buffered and all the counters should be ok
    // do a straight comparison
    while(len > 0)
    {
        if(b_char_counter == RS_BASES_PER_INT)
        {
            // next word
            b_word_counter++;
            b_current_word = b_compressed[b_word_counter];
            b_char_counter = 0;
        }

        if(revComp)
        {
            if(q_char_counter == RS_BASES_PER_INT)
            {
                // next word
                q_word_counter--;
                q_current_word = q_compressed[q_word_counter] ^ complimentoR;
                q_char_counter = 0;
            }

            // here is the comparison
            if((b_current_word ^ mTransLU[q_current_word & SAS_CE_H_3]) & SAS_CE_U_3)
                return false;
            q_current_word = q_current_word >> 2;
        }
        else
        {
            if(q_char_counter == RS_BASES_PER_INT)
            {
            // next word
                q_word_counter++;
                q_current_word = q_compressed[q_word_counter];
                q_char_counter = 0;
            }

            // here is the comparison
            if((b_current_word ^ q_current_word) & SAS_CE_U_3)
                return false;
            q_current_word = q_current_word << 2;
        }
        b_current_word = b_current_word << 2;
        
        b_char_counter++;
        q_char_counter++;
        len--;
    }
    return true;
}
//HO 
//VO 
