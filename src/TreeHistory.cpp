//            File: Zobrist.cpp
// Original Author: Michael Imelfort
// -----------------------------------------------------
//
// OVERVIEW:
// Implementation the TreeHistory class
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

// SYSTEM INCLUDES
//
#include <iostream>
#include <map>
#include <sstream>
#include <string>

// LOCAL INCLUDES
//
#include "TreeHistory.h"
#include "paranoid.h"
#include "intdef.h"
#include "IdTypeDefs.h"
#include "LoggerSimp.h"

  // Get Methods
    bool TreeHistory::getHistoryString(std::string * historyString, HistoryId HID)
    {
        //-----
        // set the history string for a guven ContextId, return true or false if Ok or not
        // serach both the tmp and real histories
        //
        treeHistoryIterator finder = mHistory.find(HID);
        if(finder != mHistory.end())
        {
            // this is a valid HistoryId
            *historyString = finder->second;
            return true;
        }
        finder = mTmpHistory.find(HID);
        if(finder != mTmpHistory.end())
        {
            // this is a valid HistoryId
            *historyString = finder->second;
            return true;
        }
        return false;
    }
    
  // Operations
    HistoryId TreeHistory::forkHistory(int num_forks, HistoryId HID)
    {
        //-----
        // make a new history string from a given one and return it's reference
        // only makes sense in terms of a tmp history but we must search both
        //

        // first look in the main history section
        treeHistoryIterator finder = mHistory.find(HID);
        if(finder != mHistory.end())
        {
            // this is a valid HistoryId
            stringstream tmp_stream;
            
            // make a new suffix
            tmp_stream << num_forks << ".";
            string new_suffix = tmp_stream.str();

            // add the new historyString to the map
            string new_history_string = finder->second + new_suffix;
            mTmpCurrentHistoryId++;
            mTmpHistory[mTmpCurrentHistoryId] = new_history_string;

            // return the ID to the user
            return mTmpCurrentHistoryId;
        }

        // now look in the tmp section
        finder = mTmpHistory.find(HID);
        if(finder != mTmpHistory.end())
        {
            // this is a valid HistoryId
            stringstream tmp_stream;
            
            // make a new suffix
            tmp_stream << num_forks << ".";
            string new_suffix = tmp_stream.str();

            // add the new historyString to the map
            string new_history_string = finder->second + new_suffix;
            mTmpCurrentHistoryId++;
            mTmpHistory[mTmpCurrentHistoryId] = new_history_string;

            // return the ID to the user
            return mTmpCurrentHistoryId;
        }

        logError("Could not fork history: " << HID);
        return (idInt)0;
    }

    bool TreeHistory::onSameBranch(HistoryId prev_hist, HistoryId curr_hist)
    {
        //-----
        // see if the two histories match up
        //
        if(curr_hist == prev_hist)
            return true;

        std::string prev_string, curr_string;
        if(!(getHistoryString(&prev_string, prev_hist)))
        {
            logError("No history found for PID PID: " << prev_hist << " CID: " << curr_hist << " COUNT: " << mCurrentHistoryId);
            return false;
        }
        if(!(getHistoryString(&curr_string, curr_hist)))
        {
            logError("No history found for CID PID: " << prev_hist << " CID: " << curr_hist << " COUNT: " << mCurrentHistoryId);
            return false;
        }

        // we just check if prev_string is a substring of curr_string
        size_t found = curr_string.find(prev_string);
        if(string::npos != found)
            return true;
        return false;
    }

    void TreeHistory::merge(void)
    {
        //-----
        // merge the tmp history with the real one
        //

        // transfer the values
        treeHistoryIterator tmp_iter = mTmpHistory.begin();
        while(tmp_iter != mTmpHistory.end())
        {
            mHistory[tmp_iter->first] = tmp_iter->second;
            tmp_iter++;
        }

        // update the counter
        mCurrentHistoryId = mTmpCurrentHistoryId;

        // clear the old values
        initialiseTmpHistory();
    }
