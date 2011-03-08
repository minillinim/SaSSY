//            File: intdef.h
// Original Author: Dominic Eales and Michael Imelfort
// --------------------------------------------------------------------
//
// OVERVIEW:
// Wrapper for defining different size ints
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

#ifndef IntDef_h
    #define IntDef_h

    //
    // how to define 32/64 bit integers on your compiler?
    // set this up so that: sizeof(uint_64t) == 8
    // and: sizeof(uint_32t) == 4
    //

    typedef unsigned long long uint_64t;
    typedef signed long long   sint_64t;

    typedef unsigned int       uint_32t;
    typedef signed int         sint_32t;

    typedef unsigned short     uint_16t;
    typedef signed short       sint_16t;

    typedef unsigned char      uint_8t;
    typedef signed char        sint_8t;

#ifdef SIZE_OF_INT
# if (SIZE_OF_INT == 64)
    typedef uint_64t uMDInt;
    typedef sint_64t sMDInt;
# elif (SIZE_OF_INT == 32)
    typedef uint_32t uMDInt;
    typedef sint_32t sMDInt;
# else
#  error SIZE_OF_INT not correct
# endif
    
    //
    // All internal memory pointers are set as either 32 or 64 bit ints
    // and then wrapped up accordingly.
    // 
    // NOTE:
    //
    // If you set this here then you MUST update the GLOBAL_DEFS.conf file
    //
    // <SIZE_OF_IDTYPE>
    //
    
# define SIZE_OF_IDTYPE 32

    // </SIZE_OF_IDTYPE>
    
# if (SIZE_OF_IDTYPE == 64)
   typedef uint_64t           idInt;
# elif (SIZE_OF_IDTYPE == 32)
   typedef uint_32t           idInt;
# else
#  error SIZE_OF_IDTYPE not correct
# endif

#else
# error SIZE_OF_INT not defined
#endif

#endif // IntDef_h

