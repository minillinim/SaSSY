//            File: paranoid.cpp
// Original Author: Dominic Eales and Michael Imelfort
// --------------------------------------------------------------------
//
// OVERVIEW:
// For debug mode. Lots of checking and printing functions (MACROS) to use
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

#ifdef MAKE_PARANOID

#include <iostream>
#include <stdlib.h>
#include "paranoid.h"

using namespace std;
void __paraAssert(const char * condition, const char * function, const char * file, int linenum)
{
    cerr << "--------------------\nparaFAIL: " << condition << "\n" << "File:     " << file << ":" << linenum << "\nFunction: " << function << endl;
    #if EXIT_ON_ASSERT==1
    exit(-1);
    #endif
}

#endif //MAKE_PARANOID
