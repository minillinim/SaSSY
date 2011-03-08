//            File: SortedTreeBranchManager.h
// Original Author: Michael Imelfort
// --------------------------------------------------------------------
//
// OVERVIEW:
// This file contains the class definitions for an object which is used during
// the pruning of extended branches. 
// 
// During this process we need to maintain a sorted vector of the type:
// 
// std::vector< std::pair<uMDInt, std::vector<GenericNodeId> * > >
// 
// Which is sorted in either direction by the uMDInt (desc, asc)
// this is very messy to do within the code so it is all wrapped up here
// so this is more of a convenience class
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

#ifndef SortedTreeBranchManager_h
 #define SortedTreeBranchManager_h

// system includes
#include <vector>
#include <iostream>
#include <map>
                 
// Local includes
#include "intdef.h"
#include "IdTypeDefs.h"
 
class SortedTreeBranchManager {

public:

  // Constructor/Destructor
    SortedTreeBranchManager(bool ascending);
    ~SortedTreeBranchManager(void);

    bool addNew(uMDInt position, GenericNodeId key_GID, GenericNodeId GID);  // when we would like to add a new element to the data block
    bool release(void);                                             // we need to call this after we call getNext to free the memory of the list
    bool getNext(std::vector<GenericNodeId> ** retVector);          // get hold of a vector of cap nodes
    uMDInt getPos(void);                                            // if we have called getNext then it may be nice to know the position it was stored to
    bool getKey(GenericNodeId * ret_GID, GenericNodeId GID);        // get the arm key for a cap
    
    inline int getNumBranches(void) { return mData.size(); }        // how many branches are being managed?
    void printAll(void);                                            // spill your guts...
    
private:
    bool mReleaseLocked;                                            // when we make a call to 'getNext' then we lock all adds or getnexts until we have called 'release'
    bool mAscendingOrdered;                                         // are we going to sort in ascending or descending order?
    std::vector< std::pair<uMDInt, std::vector<GenericNodeId> * > > mData; // this is the baby we need to maintain
    std::map<GenericNodeId, GenericNodeId> mKeyMap;        
};

#endif //SortedTreeBranchManager_h

