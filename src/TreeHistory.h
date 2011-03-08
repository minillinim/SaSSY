//            File: TreeHistory.h
// Original Author: Michael Imelfort
// -----------------------------------------------------
//
// OVERVIEW:
// This file contains the function definitions for a class which handles
// the branching history of trees. Used mainly in the grow algorithm in Context
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

#ifndef TreeHistory_h
    #define TreeHistory_h
    
// SYSTEM INCLUDES
//
#include <iostream>
#include <map>

// LOCAL INCLUDES
//
#include "intdef.h"
#include "IdTypeDefs.h"

#define TH_HIST_BASIS  "s"
#define TH_ID_BASIS     1


typedef idInt HistoryId;

typedef map<HistoryId, std::string> treeHistory;
typedef map<HistoryId, std::string>::iterator treeHistoryIterator;

class TreeHistory
{
public:
  //Constructor/Destructor
        
    TreeHistory()
    {
        //-----
        // make sure all the history variables are A-OK
        //
        initialiseHistory();
    }
    
  // Get Methods
    bool getHistoryString(std::string * historyString, HistoryId HID); // return the history string for a guven HistoryId
    inline HistoryId getBasisId(void) { return TH_ID_BASIS; }
    
  // Operations
    HistoryId forkHistory(int num_forks, HistoryId HID);            // make a new history string from a given one and return it's reference

    bool onSameBranch(HistoryId prev_hist, HistoryId curr_hist);    // see if the two histories match up
    
    inline void initialiseHistory(HistoryId * currentHID)
    {
        //-----
        // initialise the history variables
        //
        clearHistory();
        mHistory.insert(pair<HistoryId, std::string>(mCurrentHistoryId, TH_HIST_BASIS));
        *currentHID = mCurrentHistoryId;
        initialiseTmpHistory();
    }
    
    inline void initialiseHistory()
    {
        //-----
        // initialise the history variables
        //
        clearHistory();
        mHistory.insert(pair<HistoryId, std::string>(TH_ID_BASIS, TH_HIST_BASIS));
        initialiseTmpHistory();
    }

    inline void clearHistory()
    {
        //-----
        // wipe all the maps etc...
        //
        mHistory.clear();
        mCurrentHistoryId = TH_ID_BASIS;
        initialiseTmpHistory();
    }

    inline void initialiseTmpHistory()
    {
        //-----
        // initialise the history variable for the temporary holder
        //
        mTmpHistory.clear();
        mTmpCurrentHistoryId = mCurrentHistoryId;
    }

    void merge(void);                                               // merge the tmp history with the real one
     
private:

    // this is the primary map
    treeHistory mHistory;
    HistoryId mCurrentHistoryId;

    // this is a temp map which we may "merge" into the primary
    treeHistory mTmpHistory;
    HistoryId mTmpCurrentHistoryId;
};

#endif // TreeHistory_h
