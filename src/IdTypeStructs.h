//            File: IdTypeStructs.h
// Original Author: Michael Imelfort
// --------------------------------------------------------------------
//
// OVERVIEW:
// When we need weird STL objects that need sorting and such we define them here
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

#ifndef Id_Type_Structs_h
    #define Id_Type_Structs_h
    
#include "IdTypeDefs.h"
#include "math_ext.h"

typedef std::pair<UniNodeId, sMDInt> olap_pair;
typedef std::pair<ContextId, uMDInt> conLengthPair;

static bool sort_olaps_abs(const olap_pair & left, const olap_pair & right)
{
    //-----
    //
    // < UniNodeId, offset >
    //
    // we use this when we're sorting a list of olap pairs in ascending order
    // we use fAbs because we only care about the magnitude
    //
    return fAbs(left.second) < fAbs(right.second);
}

#endif // Id_Type_Structs_h
