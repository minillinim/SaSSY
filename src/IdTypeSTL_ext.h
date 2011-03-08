//            File: IdTypeSTL_ext.h
// Original Author: Michael Imelfort
// --------------------------------------------------------------------
//
// OVERVIEW:
// This file contains the definitions for STL save and load extensions for the ID TYPES
// defined in IdTypeDefs.h
//
// ...edit only at the bottom of the page
// 
// --------------------------------------------------------------------
// Copyright (C) 2009 2010 2011 Michael Imelfort and Dominic Eales
//
// This file is part of the Sassy Assembler Project.
//
// Sassy is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
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

#ifndef Id_Type_STL_H
    #define Id_Type_STL_H

// system includes
#include <iostream>
#include <fstream>
#include <map>
#include <vector>

// local includes
#include "intdef.h"
#include "paranoid.h"
#include "IdTypeDefs.h"

using namespace std;

//***********************************************
/////////////////////////////////////////////////
// Can't touch this
/////////////////////////////////////////////////

#define ADD_STL_EXT_FOR( nAME ) \
 ADD_WRITE_MULTIMAP_T_NAME_FOR( nAME ) \
 ADD_READ_MULTIMAP_T_NAME_FOR( nAME ) \
 ADD_READ_MAP_NAME_T_FOR( nAME ) \
 ADD_WRITE_MAP_NAME_T_FOR( nAME ) \
 ADD_READ_MAP_T_NAME_FOR( nAME ) \
 ADD_WRITE_MAP_T_NAME_FOR( nAME )

#define ADD_READ_MULTIMAP_T_NAME_FOR( nAME ) \
template <class T> \
void readMultiMap(multimap<T, nAME > * readmap, ifstream * inFile) { \
    readmap->clear(); \
    int size; \
    T id; \
    inFile->read(reinterpret_cast<char *>(&size), sizeof(unsigned int)); \
    readmap->resize(size); \
    while(size > 0) { \
        inFile->read(reinterpret_cast<char *>(&id), sizeof(T)); \
        readmap->insert(pair<T, nAME>(id, nAME(inFile))); \
        size--; \
    } \
}

#define ADD_WRITE_MULTIMAP_T_NAME_FOR( nAME )\
template <class T> \
void writeMultiMap(multimap<T, nAME > * writemap, ofstream * outFile) \
{ \
    int size = writemap->size(); \
    outFile->write(reinterpret_cast<char *>(&size), sizeof(unsigned int)); \
    typename multimap<T, nAME>::iterator map_iter = writemap->begin(); \
    typename multimap<T, nAME>::iterator map_last = writemap->end(); \
    while(map_iter != map_last) { \
        T id = map_iter->first; \
        outFile->write(reinterpret_cast<char *>(&id), sizeof(T)); \
        (map_iter->second).save(outFile); \
        map_iter++; \
    } \
}

#define ADD_READ_MAP_NAME_T_FOR( nAME ) \
template <class T> \
void readMap(map<nAME, T> * readmap, ifstream * inFile) { \
    readmap->clear(); \
    int size; \
    T value; \
    inFile->read(reinterpret_cast<char *>(&size), sizeof(unsigned int)); \
    readmap->resize(size); \
    while(size > 0) { \
        inFile->read(reinterpret_cast<char *>(&value), sizeof(T)); \
        readmap->insert(pair<nAME, T>(nAME(inFile), value)); \
        size--; \
    } \
}

#define ADD_WRITE_MAP_NAME_T_FOR( nAME )\
template <class T> \
void writeMap(map<nAME, T> * writemap, ofstream * outFile) \
{ \
    int size = writemap->size(); \
    outFile->write(reinterpret_cast<char *>(&size), sizeof(unsigned int)); \
    typename map<nAME, T>::iterator map_iter = writemap->begin(); \
    typename map<nAME, T>::iterator map_last = writemap->end(); \
    while(map_iter != map_last) { \
        T value = map_iter->second; \
        outFile->write(reinterpret_cast<char *>(&value), sizeof(T)); \
        (map_iter->first).save(outFile); \
        map_iter++; \
    } \
}

#define ADD_READ_MAP_T_NAME_FOR( nAME ) \
template <class T> \
void readMap(map<T, nAME> * readmap, ifstream * inFile) { \
    readmap->clear(); \
    int size; \
    T key; \
    nAME value; \
    inFile->read(reinterpret_cast<char *>(&size), sizeof(unsigned int)); \
    readmap->resize(size); \
    while(size > 0) { \
        inFile->read(reinterpret_cast<char *>(&key), sizeof(T)); \
        value = nAME(inFile); \
        readmap->insert(pair<T, nAME>(key, value)); \
        size--; \
} \
}

#define ADD_WRITE_MAP_T_NAME_FOR( nAME )\
template <class T> \
void writeMap(map<T, nAME> * writemap, ofstream * outFile) \
{ \
    int size = writemap->size(); \
    outFile->write(reinterpret_cast<char *>(&size), sizeof(unsigned int)); \
    typename map<T, nAME>::iterator map_iter = writemap->begin(); \
    typename map<T, nAME>::iterator map_last = writemap->end(); \
    while(map_iter != map_last) { \
        T key = map_iter->first; \
        outFile->write(reinterpret_cast<char *>(&key), sizeof(T)); \
        (map_iter->second).save(outFile); \
        map_iter++; \
} \
}


/////////////////////////////////////////////////
// STOP! Hammer code time
/////////////////////////////////////////////////
//***********************************************

/////////////////////////////////////////////////
// Define your list of ID TYPE classes
/////////////////////////////////////////////////

ADD_STL_EXT_FOR(UniNodeId)
ADD_STL_EXT_FOR(UniNodeDataId)
ADD_STL_EXT_FOR(ReadStoreId)
ADD_STL_EXT_FOR(DualNodeId)
ADD_STL_EXT_FOR(DualNodeDataId)
ADD_STL_EXT_FOR(GenericNodeId)
ADD_STL_EXT_FOR(ContigId)
ADD_STL_EXT_FOR(KmerMapId)
ADD_STL_EXT_FOR(KmerMapDataId)
ADD_STL_EXT_FOR(KmerMatchMapId)
ADD_STL_EXT_FOR(KmerMatchMapDataId)
ADD_STL_EXT_FOR(NodePositionMapId)
ADD_STL_EXT_FOR(ContextId)
ADD_STL_EXT_FOR(NodeCheckId)

#endif // Id_Type_STL_H
