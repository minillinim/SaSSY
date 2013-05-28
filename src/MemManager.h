//            File: MemManager.h
// Original Author: Michael Imelfort
// Complete Rewrite: Dominic Eales
// Finetuning including adding this very line: Michael Imelfort
// --------------------------------------------------------------------
//
// OVERVIEW:
// This file contains the class definitions for the data management object
// this object implements a dynamic memory management system which I think is kinda kool
// --------------------------------------------------------------------
// Copyright (C) 2009 2010 2011 Michael Imelfort and Dominic Eales
//
// This file is part of the Sassy Assembler Project.
//
// Sassy is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Sassy is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Sassy.  If not, see <http://www.gnu.org/licenses/>.
// --------------------------------------------------------------------
//
//                        A
//                       A B
//                      A B R
//                     A B R A
//                    A B R A C
//                   A B R A C A
//                  A B R A C A D
//                 A B R A C A D A
//                A B R A C A D A B 
//               A B R A C A D A B R  
//              A B R A C A D A B R A
//

#ifndef MemManager_h
    #define MemManager_h

using namespace std;

// system includes
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <string.h>
                 
// local includes
#include "intdef.h"
#include "paranoid.h"
#include "vector_ext.h"

#ifdef SIZE_OF_INT
# if (SIZE_OF_INT == 64)
#  define MM_ZERO         ((uMDInt)0x0000000000000000)
#  define MM_BLOCK_UNUSED ((uMDInt)0xBBaaaaddFF0000dd)
# elif (SIZE_OF_INT == 32)
#  define MM_ZERO         ((uMDInt)0x00000000)
#  define MM_BLOCK_UNUSED ((uMDInt)0xBaadF00d)
# else
#  error SIZE_OF_INT not correct
# endif
#else
# error SIZE_OF_INT not defined
#endif
                 
#define MM_ADDR_NULL    ((uMDInt*)0)
   
template <int MMBLOCKSIZE>
class MemManager {

    public:
        // Construction and destruction
        MemManager() {
            _className = "MemManager";
            _nextNewId = 0;
            _numBlocksAvailableInLastChunk = 0;
            _initialised = false;
            _name = "";
#ifdef SHOW_MEM
            peak_used = 0;
            peak_alloc = 0;
            re_allocs = 0;
#endif

        };
        virtual ~MemManager() {
#ifdef SHOW_MEM
            idInt total_alloc, total_used;
            getUsageData(&total_alloc, &total_used);
            std::cout << "MM: " << _name << ": At destructor: - Allocated: " << total_alloc << " bytes, Used: " << total_used << " bytes" << std::endl;
            std::cout << "MM: " << _name << ": Peak - Allocated: " << (peak_alloc * MMBLOCKSIZE * sizeof(uMDInt)) << " bytes, Used: " << (peak_used * MMBLOCKSIZE * sizeof(uMDInt))<< " bytes" << std::endl;
#endif
            freeMemory();
        }

        // Init functions
        bool initialise( 
            idInt                numBlocksInInitialChunk,
            std::vector<idInt>   subsequentChunkSizeDivisors )
        {
#ifdef SHOW_MEM
            std::cout << "MM: " << _name << ": Initially allocating: " << numBlocksInInitialChunk << " blocks at: " << MMBLOCKSIZE << ", Total: " << (numBlocksInInitialChunk * MMBLOCKSIZE * sizeof(uMDInt)) << " bytes" << std::endl;
#endif
            PARANOID_ASSERT_L1(_initialised == false);
            if (_initialised == true)
                return true;
	
            // create the chunk sizes from the parameters
            unsigned int idx;
            idInt numBlocksInChunk = numBlocksInInitialChunk;
            for (idx = 0; idx < subsequentChunkSizeDivisors.size(); idx++)
            {
                _chunkSizes.push_back( numBlocksInChunk );
                numBlocksInChunk = numBlocksInInitialChunk/subsequentChunkSizeDivisors[idx];
            }
            _chunkSizes.push_back( numBlocksInChunk );
        
            // set initialised
            _initialised = true;
        
            // allocate the first chunk
            if (!allocateNewChunk()) {
                PARANOID_ASSERT_L1(!"allocateNewChunk returned 'Out of memory'");
                _initialised = false;
#ifdef SHOW_MEM
                std::cout << "MM: " << _name << " Could not allocate memory!" << std::endl;
#endif
                return false;
            }
        
            return true;
        }

        // Init functions
        bool initialise( std::string fileName )
        {
            PARANOID_ASSERT_L1(_initialised == false);
            if (_initialised == true)
                return true;

            //open the file
            ifstream myFile(fileName.c_str(), ifstream::binary);

            // load class name
            std::string tempString;
            myFile >> tempString;

            // check the class name is good
            if (_className != tempString)
            {
                PARANOID_INFO_L1( "init-with-file: wrong class: class=" << _className << ", file=" << tempString << " (" << fileName << ")");
                // close the file
                myFile.close();
                return false;
            }

            // clear "\n" out...
            char k;
            myFile.get(k);

            // load block size
            unsigned int blockSize;
            myFile.read(reinterpret_cast<char *>(&blockSize), sizeof(unsigned int));

            // check block size
            if (blockSize != MMBLOCKSIZE)
            {
                PARANOID_INFO_L1( "init-with-file: wrong block size: class=" << MMBLOCKSIZE << ", file=" << blockSize << " (" << fileName << ")");
                // close the file
                myFile.close();
                return false;
            }

            // read some basic variables
            myFile.read(reinterpret_cast<char *>(&_initialised), sizeof(bool));
            myFile.read(reinterpret_cast<char *>(&_nextNewId), sizeof(idInt));
            myFile.read(reinterpret_cast<char *>(&_numBlocksAvailableInLastChunk), sizeof(idInt));

            // read in vectors
            readVector( &_chunkSizes,       &myFile);
            readVector( &_chunkFirstId,     &myFile);
            readVector( &_chunkTotalNumIds, &myFile);
            
            // now the tricky part -- read in the chunks
            PARANOID_ASSERT_L1(_chunkList.size() == 0);

            // for each allocated chunk...
            for (unsigned int chunkIdx = 0; chunkIdx < _chunkFirstId.size(); chunkIdx++) {

                // get the chunk size by (the first ID after chunk) minus (first ID in chunk)
                idInt newChunkSize = _chunkTotalNumIds[chunkIdx] - _chunkFirstId[chunkIdx];

                // get memory chunk
                uMDInt * pNewChunk = new uMDInt[ newChunkSize ];
                if ( pNewChunk == NULL )
                {
                    PARANOID_INFO_L1("init-with-file: out of memory, alloc (" << newChunkSize << "x" << sizeof(uMDInt) << " bytes");
                    PARANOID_ASSERT_L1(!"init-with-file 'Out of memory'");
                    // free allocated memory
                    freeMemory();
                    // close the file
                    myFile.close();
                    return false;
                }
            
                // read in data
                myFile.read(reinterpret_cast<char *>(pNewChunk), newChunkSize * sizeof(uMDInt));

                // add to chunk list
                _chunkList.push_back( pNewChunk );
            }
            
            // close the file
            myFile.close();

            // update initialised
            _initialised = true;

            #ifdef MAKE_PARANOID
                PARANOID_INFO_L1("Load complete");
                debugvars();
            #endif
#ifdef SHOW_MEM
                idInt total_alloc, total_used;
                getUsageData(&total_alloc, &total_used);
                std::cout << _name << " On load - Allocating: " << total_alloc << " Used: " << total_used << std::endl;
                peak_used = total_used;
                peak_alloc = total_alloc;
#endif
            return true;
        }

        // Save function
        bool save( std::string fileName )
        {
            PARANOID_ASSERT_L1(_initialised == true);
            if (_initialised == false)
                return false;

            //open the file
            ofstream myFile(fileName.c_str(), ofstream::binary);

            // save class name
            myFile << _className << "\n";

            // save block size
            unsigned int blockSize = MMBLOCKSIZE;
            myFile.write(reinterpret_cast<char *>(&blockSize), sizeof(unsigned int));

            // save some basic variables
            myFile.write(reinterpret_cast<char *>(&_initialised), sizeof(bool));
            myFile.write(reinterpret_cast<char *>(&_nextNewId), sizeof(idInt));
            myFile.write(reinterpret_cast<char *>(&_numBlocksAvailableInLastChunk), sizeof(idInt));

            // save vectors
            writeVector( &_chunkSizes,       &myFile);
            writeVector( &_chunkFirstId,     &myFile);
            writeVector( &_chunkTotalNumIds, &myFile);
            
            // now the tricky part -- save the chunks
            PARANOID_ASSERT_L1(_chunkList.size() != 0);

            // for each allocated chunk...
            for (unsigned int chunkIdx = 0; chunkIdx < _chunkFirstId.size(); chunkIdx++) {

                // get the chunk size by (the first ID after chunk) minus (first ID in chunk)
                idInt chunkSize = _chunkTotalNumIds[chunkIdx] - _chunkFirstId[chunkIdx];
            
                // read in data
                myFile.write(reinterpret_cast<char *>(_chunkList[chunkIdx]), chunkSize * sizeof(uMDInt));
            }
            
            // close the file
            myFile.close();

            #ifdef MAKE_PARANOID
                PARANOID_INFO_L1("Save complete");
                debugvars();
            #endif

            return true;
        }

        // Access functions
        inline bool isValidAddress( idInt id )
        {
            // we can be quite paranoid here
            PARANOID_ASSERT_L1((id%MMBLOCKSIZE)==0);
            PARANOID_ASSERT_L1((_initialised));
            PARANOID_ASSERT_L1((id<_chunkTotalNumIds[_chunkList.size()-1]));

            uMDInt * addr;

            // do something finally
            if ( _chunkList.size() == 1 ) {
                addr = &((_chunkList[0])[id]);
            } else {
                unsigned int chunkIdx = 0;
                while (id >= _chunkTotalNumIds[chunkIdx]) {
                    chunkIdx++;
                    if (chunkIdx == _chunkList.size()) { return false; };
                }
                addr = &((_chunkList[chunkIdx])[id-_chunkFirstId[chunkIdx]]);
            }

            return (addr != MM_ADDR_NULL && addr[0] != MM_BLOCK_UNUSED);
        }

        inline uMDInt * getAddress( idInt id ) {
            //-----
            // Return a pointer to the start int of the block.
            //
            // we can be quite paranoid here
            PARANOID_ASSERT_PRINT_L1(((id%MMBLOCKSIZE)==0), _name << ": ID: "<< id << " BS: " << MMBLOCKSIZE );
            PARANOID_ASSERT_L1((_initialised));
            PARANOID_ASSERT_PRINT_L1((id<_chunkTotalNumIds[_chunkList.size()-1]), _name << ": ID: " << id << " < " << ( _chunkTotalNumIds[_chunkList.size()-1] ));

            uMDInt * addr;

            // do something finally
            if ( _chunkList.size() == 1 ) {
                addr = &((_chunkList[0])[id]);
            } else {
                unsigned int chunkIdx = 0;
                while (id >= _chunkTotalNumIds[chunkIdx]) {
                    chunkIdx++;
                    if (chunkIdx == _chunkList.size()) { return MM_ADDR_NULL; };
                }
                addr = &((_chunkList[chunkIdx])[id-_chunkFirstId[chunkIdx]]);
            }

            PARANOID_ASSERT_L1(addr != MM_ADDR_NULL);
            PARANOID_ASSERT_PRINT_L1((addr[0] != MM_BLOCK_UNUSED), _name << " Id:" << id << " addr[0]: " << std::hex << (addr[0]) << std::dec);

            return addr;
        }

        inline void setAddressUsed( idInt id ) {
            // reset any MM_BLOCK_UNUSED flags
            // we can be quite paranoid here
            PARANOID_ASSERT_L1((id%MMBLOCKSIZE)==0);
            PARANOID_ASSERT_L1((_initialised));
            PARANOID_ASSERT_L1((id<_chunkTotalNumIds[_chunkList.size()-1]));

            uMDInt * addr;

            // do something finally
            if ( _chunkList.size() == 1 ) {
                addr = &((_chunkList[0])[id]);
            } else {
                unsigned int chunkIdx = 0;
                while (id >= _chunkTotalNumIds[chunkIdx]) {
                    chunkIdx++;
                    if (chunkIdx == _chunkList.size()) { return; };
                }
                addr = &((_chunkList[chunkIdx])[id-_chunkFirstId[chunkIdx]]);
            }

            PARANOID_ASSERT_L1(addr != MM_ADDR_NULL);
            addr[0] = MM_ZERO;
        }

        // Wrapping and unwrapping IdTypes
        template <class T>
        inline void wrapId(idInt rawId, T * idType)
        {
            // wrap the value in rawId in an IdType of type T
            idType->set(rawId);
        }
        
        template <class T>
        inline idInt unWrapId(T idType)
        {
            // get the guts of the idType and return as an idInt
            return idType.get();
        }

        // Add functions
        idInt getNewId(void)
        {
            idInt thisId;
            if ( _numBlocksAvailableInLastChunk == 0 ) {
                if (!allocateNewChunk()) {
                    PARANOID_ASSERT_L1(!"allocateNewChunk returned 'Out of memory'");
                }
            }
            thisId = _nextNewId;
#ifdef SHOW_MEM
            peak_used++;
#endif
            _nextNewId += (idInt)MMBLOCKSIZE;
            _numBlocksAvailableInLastChunk--;

            // get rid of any MM_BLOCK_UNUSED flags
            MemManager<MMBLOCKSIZE>::setAddressUsed( thisId );
            return thisId;
        }

        void getUsageData( idInt * totalBytesAllocated, idInt * maxBytesUsed ) {
            if (totalBytesAllocated != NULL ) {
                *totalBytesAllocated = _chunkTotalNumIds.back() * MMBLOCKSIZE * sizeof(uMDInt);
            }
            if (maxBytesUsed != NULL ) {
                *maxBytesUsed = _nextNewId * MMBLOCKSIZE * sizeof(uMDInt);
            }
        }

        #ifdef MAKE_PARANOID
        void debugvars(void)
        {
            unsigned int i;
            #define DO_VAR(vAR) PARANOID_INFO_L1( #vAR "=" << vAR)
            DO_VAR(_initialised);
            DO_VAR(_nextNewId);
            DO_VAR(_numBlocksAvailableInLastChunk);
            #define DO_LIST(vAR) for (i=0;i<vAR.size();i++) PARANOID_INFO_L1( #vAR "[" << i << "]=" << vAR[i])
            DO_LIST(_chunkList);
            DO_LIST(_chunkSizes);
            DO_LIST(_chunkFirstId);
            DO_LIST(_chunkTotalNumIds);
        }
        #endif

        std::string           _name;
        
    protected:
        // housekeeping
        bool   _initialised;
        const char * _className;
    
        // Memory chunk vars
        idInt                 _nextNewId;
        idInt                 _numBlocksAvailableInLastChunk;
        std::vector<uMDInt *> _chunkList;
        std::vector<idInt>    _chunkSizes;
        std::vector<idInt>    _chunkFirstId;
        std::vector<idInt>    _chunkTotalNumIds;

#ifdef SHOW_MEM
        idInt peak_alloc, peak_used, re_allocs;  
#endif
        // Chunk management
        inline bool allocateNewChunk() {
            PARANOID_ASSERT_L1(_initialised);
            PARANOID_ASSERT_L1(_numBlocksAvailableInLastChunk == 0);

            unsigned int chunkIdx;
            idInt newChunkSize;
            uMDInt *pNewChunk;

            // find the appropriate size
            chunkIdx = _chunkList.size();
            // if we can't find a size for this chunk, just use the last available chunk size
            if ( chunkIdx > _chunkSizes.size()-1 ) {
                chunkIdx = _chunkSizes.size()-1;
            }
            newChunkSize = _chunkSizes[chunkIdx] * MMBLOCKSIZE;

#ifdef SHOW_MEM
            std::cout << "MM: " << _name << ": New chunk - Adding: " << _chunkSizes[chunkIdx] << " blocks at: " << MMBLOCKSIZE << ", Total: " << (newChunkSize * sizeof(uMDInt)) << " bytes" << std::endl;
            peak_alloc += _chunkSizes[chunkIdx];
            re_allocs++;
#endif

            // allocate memory
            pNewChunk = new uMDInt[newChunkSize];
            if ( pNewChunk != NULL ) {

                uMDInt * addr;
                idInt   counter;

                PARANOID_INFO_L1(_name << ": Allocating chunk #" << _chunkList.size() << ", Size: " << _chunkSizes[chunkIdx] << "*sizeof(uMDInt)");

                // empty it
                memset( pNewChunk, 0, newChunkSize * sizeof(uMDInt) );

                // put in MM_BLOCK_UNUSED flags
                addr = pNewChunk;
                counter = 0;
                while (counter < _chunkSizes[chunkIdx]) {
                    addr[0] = MM_BLOCK_UNUSED;
                    addr += MMBLOCKSIZE;
                    counter++;
                }

                // update lists
                _chunkList.push_back( pNewChunk );
                _chunkFirstId.push_back( _nextNewId );
                _chunkTotalNumIds.push_back( (_nextNewId + newChunkSize) );
                _numBlocksAvailableInLastChunk = _chunkSizes[chunkIdx];
            }
            return ( pNewChunk != NULL );
        }

        void freeMemory(void) {
            for (unsigned int chunkIdx = 0; chunkIdx < _chunkList.size(); chunkIdx++) {
                delete[] _chunkList[chunkIdx];
            }
            _nextNewId = 0;
            _numBlocksAvailableInLastChunk = 0;
            _initialised = false;
            _chunkList.clear();
            _chunkSizes.clear();
            _chunkFirstId.clear();
            _chunkTotalNumIds.clear();
        }
};

#ifdef PARANOID_LAYER_1
class DeletedIdDebugInfo {
public:
    const char *fname;
    int linenum;
};
#endif

template <int MMBLOCKSIZE>
class DeletableMemManager : public MemManager<MMBLOCKSIZE> {

    public:
        // Construction and destruction
        DeletableMemManager() {
            MemManager<MMBLOCKSIZE>::_className = "DeletableMemManager";
        }
        virtual ~DeletableMemManager() {
            MemManager<MMBLOCKSIZE>::freeMemory();
        }

        // Access functions
        inline uMDInt * getAddress( idInt id ) {

            uMDInt * addr = MemManager<MMBLOCKSIZE>::getAddress(id);

            // check if empty block
            if (  ( addr != MM_ADDR_NULL )
                &&( addr[0] == MM_BLOCK_UNUSED ) ) {

                #ifdef PARANOID_LAYER_1
                unsigned int pos = std::find(_deletedIdList.begin(), _deletedIdList.end(), id) - _deletedIdList.begin();
                if ( pos != _deletedIdList.size() ) {
                    PARANOID_INFO_L1("Requested deleted block, index: " << id << " deleted on line " << _deletedIdInfoList[pos].linenum << " of " << _deletedIdInfoList[pos].fname);
                } else
                #endif

                PARANOID_INFO_L1("Requested unused block, index: " << id << " : " << std::hex << addr[0] << std::dec);

                return MM_ADDR_NULL;
            }

            return addr;
        }

        // Add functions
        idInt getNewId(void) {

            // if there is something on the list then give that index back
            if ( _deletedIdList.size() != 0 ) {

                idInt retIndex = _deletedIdList[_deletedIdList.size()-1];
                _deletedIdList.pop_back();
                #ifdef PARANOID_LAYER_1
                _deletedIdInfoList.pop_back();
                #endif

                // unset the unused flags
                MemManager<MMBLOCKSIZE>::setAddressUsed( retIndex );

                // done!
                return retIndex;
            }

            return MemManager<MMBLOCKSIZE>::getNewId();
        }

        #ifdef PARANOID_LAYER_1
        # define freeId(iDeNiFiEr) _freeId(iDeNiFiEr,__FILE__,__LINE__)
        #else
        # define freeId(iDeNiFiEr) _freeId(iDeNiFiEr)
        #endif

        // Delete function
        inline bool _freeId( 
               idInt id
        #ifdef PARANOID_LAYER_1
               , const char *fname, int linenum
        #endif
        ) {

            // get the address
            uMDInt * addr = MemManager<MMBLOCKSIZE>::getAddress( id );

            // if success then delete the block
            if ( addr != MM_ADDR_NULL ) {
                // empty it
                memset( addr, 0, MMBLOCKSIZE * sizeof(uMDInt));

                // set block as unused
                addr[0] = MM_BLOCK_UNUSED;

                // add the address to the deleted list
                _deletedIdList.push_back( id );

                #ifdef PARANOID_LAYER_1
                DeletedIdDebugInfo dbgInfo;
                dbgInfo.fname = fname;
                dbgInfo.linenum = linenum;
                _deletedIdInfoList.push_back( dbgInfo );
                #endif

                return true;
            }
            return false;
        }

    private:
        std::vector<idInt> _deletedIdList;

        #ifdef PARANOID_LAYER_1
        std::vector<DeletedIdDebugInfo> _deletedIdInfoList;
        #endif
};


#endif // MemManager_h
