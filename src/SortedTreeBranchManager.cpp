//            File: SortedTreeBranchManager.cpp
// Original Author: Michael Imelfort
// --------------------------------------------------------------------
//
// OVERVIEW:
// Implementation of SortedTreeBranchManager methods.
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

// system includes
#include <vector>
#include <map>
#include <iostream>
 
// local includes
#include "SortedTreeBranchManager.h"
#include "LoggerSimp.h"
#include "intdef.h"
#include "IdTypeDefs.h"

using namespace std;

  // Constructor/Destructor
SortedTreeBranchManager::SortedTreeBranchManager(bool ascending)
{
    //-----
    // default constructor
    //
    mReleaseLocked = false;
    mAscendingOrdered = ascending;
}

SortedTreeBranchManager::~SortedTreeBranchManager(void)
{
    //-----
    // default destructor
    //
    std::vector<GenericNodeId> * disposable_vector = NULL;
    while(mData.size() > 0)
    {
        getNext(&disposable_vector);
        release();
    }
}

bool SortedTreeBranchManager::addNew(uMDInt position, GenericNodeId key_GID, GenericNodeId GID)
{
    //-----
    // when we would like to add a new element to the data block
    //
    if(mReleaseLocked)
    {
        logError("Trying to add a new element while the lock is active");
        return false;
    }
    std::vector< std::pair<uMDInt, std::vector<GenericNodeId> * > >::iterator vec_head_iter = mData.begin();
    mKeyMap[GID] = key_GID;
    while(vec_head_iter != mData.end())
    {
        uMDInt current_pos = vec_head_iter->first;
        if(current_pos == position)
        {
            // this position is already in the data block
            // we only need to add it to the vector
            std::vector<GenericNodeId> * adding_vec = vec_head_iter->second;
            adding_vec->push_back(GID);
            return true;
        }
        else
        {
            if(mAscendingOrdered)
            {
                if(current_pos < position)
                {
                    // we can insert here!
                    // however we will need to make a new pair.
                    std::vector<GenericNodeId> * new_vec = new std::vector<GenericNodeId>();
                    new_vec->push_back(GID);
                    mData.insert(vec_head_iter, std::make_pair(position, new_vec));
                    return true;
                }
            }
            else
            {
                if(current_pos > position)
                {
                    // we can insert here!
                    // however we will need to make a new pair.
                    std::vector<GenericNodeId> * new_vec = new std::vector<GenericNodeId>();
                    new_vec->push_back(GID);
                    mData.insert(vec_head_iter, std::make_pair(position, new_vec));
                    return true;
                }
            }
        }
        vec_head_iter++;
    }
    // if we made it here then we haven't met the criteria to add it in yet
    // we will have to make a new inner vector and push this guy onto the end
    std::vector<GenericNodeId> * new_vec = new std::vector<GenericNodeId>();
    new_vec->push_back(GID);
    mData.push_back(std::make_pair(position, new_vec));
    return true;
}

bool SortedTreeBranchManager::release(void)
{
    //-----
    // we need to call this after we call getNext to free the memory of the list
    //
    if(!mReleaseLocked)
    {
        logError("Trying to release while the lock is inactive");
        return false;
    }
    std::vector< std::pair<uMDInt, std::vector<GenericNodeId> * > >::iterator vec_head_iter = mData.begin();
    if(vec_head_iter != mData.end())
    {
        std::vector<GenericNodeId> * del_pointer = vec_head_iter->second;
        // who hates seg faults?
        if(del_pointer != NULL)
        {
            // delete the inner vector
            delete del_pointer;
            del_pointer = NULL;
        }
        // erase the entire element
        mData.erase(vec_head_iter);
        // unlock the release lock
        mReleaseLocked = false;
        // done!
        return true;
    }
    return false;
}

bool SortedTreeBranchManager::getNext(std::vector<GenericNodeId> ** retVector)
{
    //-----
    // get hold of a vector of cap nodes
    //
    if(mReleaseLocked)
    {
        logError("Trying to get another element while the lock is active");
        return false;
    }
    std::vector< std::pair<uMDInt, std::vector<GenericNodeId> * > >::iterator vec_head_iter = mData.begin();
    if(vec_head_iter != mData.end())
    {
        // point the return vector at the right guy
        *retVector = (vec_head_iter->second);
        // lock the release vector
        mReleaseLocked = true;
        // done!
        return true;
    }
    return false;
}

uMDInt SortedTreeBranchManager::getPos(void)
{
    //-----
    // if we have called getNext then it may be nice to know the position it was stored to
    //
    if(!mReleaseLocked)
    {
        logError("Trying to get position while the lock is inactive");
        return 0;
    }
    std::vector< std::pair<uMDInt, std::vector<GenericNodeId> * > >::iterator vec_head_iter = mData.begin();
    if(vec_head_iter != mData.end())
    {
        // point the return vector at the right guy
        return vec_head_iter->first;
    }
    return 0;
}

bool SortedTreeBranchManager::getKey(GenericNodeId * ret_GID, GenericNodeId GID)
{
    //-----
    // get the arm key for a cap
    //
    std::map<GenericNodeId, GenericNodeId>::iterator key_map_iter = mKeyMap.find(GID);
    if(key_map_iter != mKeyMap.end())
    {
        *ret_GID = key_map_iter->second;
        return true;
    }
    return false;
}

void SortedTreeBranchManager::printAll(void)
{
    //-----
    // print out all the contents of the tree manager...
    //
    std::vector<GenericNodeId> * inner_vec;
    std::vector<GenericNodeId>::iterator inner_vec_iter;
    std::vector<GenericNodeId>::iterator inner_vec_last;
    std::vector< std::pair<uMDInt, std::vector<GenericNodeId> * > >::iterator vec_head_iter = mData.begin();
    std::cout << "-------------------------------\nSTBM is " << std::flush;
    if(mAscendingOrdered)
        std::cout << "Ascending" << std::endl;
    else
        std::cout << "Descending" << std::endl;
        
    while(vec_head_iter != mData.end())
    {
        inner_vec = vec_head_iter->second;
        if(inner_vec != NULL)
        {
            std::cout << "At pos: " << vec_head_iter->first << " : " << std::flush;
            inner_vec_iter = inner_vec->begin();
            inner_vec_last = inner_vec->end();
            while(inner_vec_iter != inner_vec_last)
            {
                std::cout << *inner_vec_iter << ", " << std::flush;
                inner_vec_iter++;
            }
            std::cout << std::endl;            
        }
        vec_head_iter++;
    }
}

