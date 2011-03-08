//            File: Zobrist.h
// Original Author: Michael Imelfort
// --------------------------------------------------------------------
//
// OVERVIEW:
// This file contains the class definition for a zobrist hashing scheme.
// This module hashes strings into uMDInts
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

#ifndef Zobrist_h
  #define Zobrist_h

// SYSTEM INCLUDES
//
#include <map> 
  
// LOCAL INCLUDES
//
#include "intdef.h"

class Zobrist
{
public:

  // Constructor/ Destructor
    Zobrist(std::string fileName);                                  // load a Zobrist from file
    Zobrist(int stringLength);                                      // constuctor for a Zobrist for a specific length string
    ~Zobrist(void);                                                 // Destructor

  // Get methods
    idInt hashString(std::string * query);                         // Hash a string
    
  // operations
    void makeMap(void);                                             // Make the lookup map

  // file IO
    bool saveZobrist(std::string fileName);                         // Save the Zobrist to file
    bool loadZobrist(std::string fileName);                         // Load a Zobrist from file
    
private:
    
    std::map<int, std::map<char, idInt> > mLookupMap;               // This is the lookup table to go from
    int mStringLength;                                              // the number of bases in a read
    idInt * mSeed;                                                  // so we can remake the zobrist if we wish
};

#endif // Zobrist_h
