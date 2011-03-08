//            File: paranoid.h
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

# ifndef PARANOID_H
#  define PARANOID_H

// define all paranoid statements to do nothing
#  define PARANOID_ASSERT_L1(cONDITION) 
#  define PARANOID_ASSERT_L2(cONDITION) 
#  define PARANOID_ASSERT_L3(cONDITION) 
#  define PARANOID_ASSERT_L4(cONDITION) 
#  define PARANOID_ASSERT_PRINT_L1(cONDITION, pRINTiNFO)
#  define PARANOID_ASSERT_PRINT_L2(cONDITION, pRINTiNFO)
#  define PARANOID_ASSERT_PRINT_L3(cONDITION, pRINTiNFO)
#  define PARANOID_ASSERT_PRINT_L4(cONDITION, pRINTiNFO)
#  define PARANOID_INFO_L1(cOUTsTRING)
#  define PARANOID_INFO_L2(cOUTsTRING)
#  define PARANOID_INFO_L3(cOUTsTRING)
#  define PARANOID_INFO_L4(cOUTsTRING)

// Uncomment to enable logging
#  define PARANOID_INFO_1
#  define PARANOID_INFO_2
#  define PARANOID_INFO_3
#  define PARANOID_INFO_4
# endif

#ifdef MAKE_PARANOID
// some paranoid parameters
#define EXIT_ON_ASSERT 0

// forward declaration of assert function
void __paraAssert(const char * condition, const char * function, const char * file, int linenum);

// ASSERT Macros
# ifdef PARANOID_LAYER_1
#  undef  PARANOID_ASSERT_L1
#  define PARANOID_ASSERT_L1(cOND) {if(!(cOND)){__paraAssert("(L1) : " #cOND,__PRETTY_FUNCTION__,__FILE__,__LINE__);}}
#  undef  PARANOID_ASSERT_PRINT_L1
#  define PARANOID_ASSERT_PRINT_L1(cOND,pRINT) {if(!(cOND)){__paraAssert("(L1) : " #cOND,__PRETTY_FUNCTION__,__FILE__,__LINE__); { std::cerr << "Info:     " << pRINT << std::endl; } }}
#  ifdef PARANOID_INFO_1
#   undef  PARANOID_INFO_L1
#   define PARANOID_INFO_L1(cOUTsTRING) {std::cout << "--------------------\nparaINFO: (L1) : " __FILE__ << ":" << __LINE__ << "\nMessage:  " << cOUTsTRING << std::endl;}
#  endif
# endif
# ifdef PARANOID_LAYER_2
#  undef  PARANOID_ASSERT_L2
#  define PARANOID_ASSERT_L2(cOND) {if(!(cOND)){__paraAssert("(L2) : " #cOND,__PRETTY_FUNCTION__,__FILE__,__LINE__);}}
#  undef  PARANOID_ASSERT_PRINT_L2
#  define PARANOID_ASSERT_PRINT_L2(cOND,pRINT) {if(!(cOND)){__paraAssert("(L2) : " #cOND,__PRETTY_FUNCTION__,__FILE__,__LINE__); std::cerr << "Info:     " << pRINT << std::endl; }}
#  ifdef PARANOID_INFO_2
#   undef  PARANOID_INFO_L2
#   define PARANOID_INFO_L2(cOUTsTRING) {std::cout << "--------------------\nparaINFO: (L2) : " __FILE__ << ":" << __LINE__ << "\nMessage:  " << cOUTsTRING << std::endl;}
#  endif
# endif
# ifdef PARANOID_LAYER_3
#  undef  PARANOID_ASSERT_L3
#  define PARANOID_ASSERT_L3(cOND) {if(!(cOND)){__paraAssert("(L3) : " #cOND,__PRETTY_FUNCTION__,__FILE__,__LINE__);}}
#  undef  PARANOID_ASSERT_PRINT_L3
#  define PARANOID_ASSERT_PRINT_L3(cOND,pRINT) {if(!(cOND)){__paraAssert("(L3) : " #cOND,__PRETTY_FUNCTION__,__FILE__,__LINE__); std::cerr << "Info:     " << pRINT << std::endl; }}
#  ifdef PARANOID_INFO_3
#   undef  PARANOID_INFO_L3
#   define PARANOID_INFO_L3(cOUTsTRING) {std::cout << "--------------------\nparaINFO: (L3) : " __FILE__ << ":" << __LINE__ << "\nMessage:  " << cOUTsTRING << std::endl;}
#  endif
# endif
# ifdef PARANOID_LAYER_4
#  undef  PARANOID_ASSERT_L4
#  define PARANOID_ASSERT_L4(cOND) {if(!(cOND)){__paraAssert("(L4) : " #cOND,__PRETTY_FUNCTION__,__FILE__,__LINE__);}}
#  undef  PARANOID_ASSERT_PRINT_L4
#  define PARANOID_ASSERT_PRINT_L4(cOND,pRINT) {if(!(cOND)){__paraAssert("(L4) : " #cOND,__PRETTY_FUNCTION__,__FILE__,__LINE__); std::cerr << "Info:     " << pRINT << std::endl; }}
#  ifdef PARANOID_INFO_4
#   undef  PARANOID_INFO_L4
#   define PARANOID_INFO_L4(cOUTsTRING) {std::cout << "--------------------\nparaINFO: (L4) : " __FILE__ << ":" << __LINE__ << "\nMessage:  " << cOUTsTRING << std::endl;}
#  endif
# endif

#endif // MAKE_PARANOID
