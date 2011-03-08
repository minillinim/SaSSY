/******************************************************************************
**
** File: nodeblockdef.h
**
*******************************************************************************
**
** Whole bunch of wrappers and accessories for accessing/messing with long lists
** of GenericNodeIds. (referred to as a "nodeBlock"
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

#ifndef __NODE_BLOCK_H
# define __NODE_BLOCK_H

// system includes
#include <iostream>

// local includes
#include "intdef.h"
#include "IdTypeDefs.h"

/*
** Choose from ONLY one of the options below (MAP or LIST)
** 
** For google sparse hash use the build option (one of)
** 
** make sparse_hash_map=use  or  make dense_hash_map=use
** 
*/

#define SAS_GN_BLOCK_IS_MAP        1

#if SAS_GN_BLOCK_IS_MAP
#   if(SPARH == 1)
#       include <google/sparse_hash_map>
        using google::sparse_hash_map;
#       define nodeBlock_GN             sparse_hash_map<GenericNodeId, bool>
#       define nodeBlockIterator_GN     sparse_hash_map<GenericNodeId, bool>::iterator
#   else
#       if(DENH == 1)
#           include <google/dense_hash_map>
            using google::dense_hash_map;
#           define nodeBlock_GN             dense_hash_map<GenericNodeId, bool>
#           define nodeBlockIterator_GN     dense_hash_map<GenericNodeId, bool>::iterator
#       else
#           include <map>
#           define nodeBlock_GN std::map<GenericNodeId, bool>
#           define nodeBlockIterator_GN std::map<GenericNodeId, bool>::iterator
#       endif // (DENH == 1)
#   endif // (SPARH == 1)

#   define addIn_NB(bLOCK, gN_iD) { (*bLOCK)[gN_iD] = true; }
                            
    static void takeOut_NB(nodeBlock_GN * bLOCK, GenericNodeId gN_iD)
    {
        if(bLOCK->end() != bLOCK->find(gN_iD))
            (*bLOCK)[gN_iD] = false;
    }

    static void writeNB(nodeBlock_GN * write_nb, ofstream * outFile)
    {
        //-----
        // write the nodeBlock_GN to file
        //
        int size = write_nb->size();
        outFile->write(reinterpret_cast<char *>(&size), sizeof(unsigned int));
        nodeBlockIterator_GN nb_iter = write_nb->begin();
        nodeBlockIterator_GN nb_last = write_nb->end();
        GenericNodeId id;
        bool value;
        while(nb_iter != nb_last)
        {
            id = nb_iter->first;
            value = nb_iter->second;
            outFile->write(reinterpret_cast<char *>(&id), sizeof(GenericNodeId));
            outFile->write(reinterpret_cast<char *>(&value), sizeof(bool));
            nb_iter++;
        }
    }

    static void readNB(nodeBlock_GN * read_nb, ifstream * inFile)
    {
        //-----
        // write the nodeBlock_GN to file
        //
        // clear the map
        read_nb->clear();
        int size;
        GenericNodeId id;
        bool value;
        inFile->read(reinterpret_cast<char *>(&size), sizeof(unsigned int));
        while(size > 0)
        {
            inFile->read(reinterpret_cast<char *>(&id), sizeof(GenericNodeId));
            inFile->read(reinterpret_cast<char *>(&value), sizeof(bool));
            (*read_nb)[id] = value;
            size--;
        }
    }

#endif // MAP

#endif // __NODE_BLOCK_H

