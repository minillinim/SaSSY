//            File: ReadStoreMemWrapper.tempconf.h
// Original Author: Michael Imelfort
// --------------------------------------------------------------------
//
// OVERVIEW:
// This file stores all the possible readlengths we'll compile for use with Sassy
// To add a new readlength you need to add 1 line here. If the new
// readlength is foo bases then add:
// 
//    MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,FOO);
//
// Note that the addition is useless unless it is in sync with
// the values in UniNodeMemWrapper.tempconf.h
// Enjoy!
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
// ReadLengths supported:
// 25 => 99
//
#ifndef ReadStore_TEMPLATES
    #define ReadStore_TEMPLATES
    
#if 0
#define makeAppropriateReadStoreMemWrapper(oBJpOINTER,iNSTaNCeRL) { \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,37); \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,45); \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,49); \
}

#else
#define makeAppropriateReadStoreMemWrapper(oBJpOINTER,iNSTaNCeRL) { \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,13); \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,15); \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,17); \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,19); \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,21); \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,23); \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,25); \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,27); \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,29); \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,31); \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,33); \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,35); \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,37); \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,39); \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,41); \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,43); \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,45); \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,47); \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,49); \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,51); \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,53); \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,55); \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,57); \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,59); \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,61); \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,63); \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,65); \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,67); \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,69); \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,71); \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,73); \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,75); \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,77); \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,79); \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,81); \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,83); \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,85); \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,87); \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,89); \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,91); \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,93); \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,95); \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,97); \
MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,99); \
}
#endif
#endif // ReadStore_TEMPLATES
