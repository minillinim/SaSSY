//            File: IdTypeDefs.h
// Original Author: Dominic Eales. Modified by Michael Imelfort
// --------------------------------------------------------------------
//
// OVERVIEW:
// This file contains the definitions for generic ID types. These are implemented at uMDInt's
// but any relation to actual ints is stripped away leaving a new 64 bit ID which is only defined
// under assignment and comparison (== and !=) Think of it as an int wrapped in a class, wrapped
// in an enigma...
//
// ...edit only at the bottom of the page
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

#ifndef Id_Type_H
    #define Id_Type_H

// system includes
#include <iostream>
#include <fstream>

// local includes
#include "intdef.h"
#include "paranoid.h"

using namespace std;

/////////////////////////////////////////////////
// Do not define the base type for the ID TYPE classes here
// It is defined in intdef.h, these calculations rely on the value set there

#ifdef SIZE_OF_IDTYPE
# if (SIZE_OF_IDTYPE == 64)
#  define ID_ZERO         ((idInt)0x0000000000000000)
#  define ID_BLOCK_UNUSED ((idInt)0xBBaaaaddFF0000dd)
# elif (SIZE_OF_IDTYPE == 32)
#  define ID_ZERO         ((idInt)0x00000000)
#  define ID_BLOCK_UNUSED ((idInt)0xBaadF00d)
# else
#  error SIZE_OF_IDTYPE not correct
# endif
#else
# error SIZE_OF_IDTYPE not defined
#endif

#define IDTYPE idInt

//***********************************************
/////////////////////////////////////////////////
// Can't touch this
/////////////////////////////////////////////////
#ifndef PARANOID_ASSERT
    #define PARANOID_ASSERT(aSSERT)
#endif

// forward declarations
template<int MMBLOCKSIZE>
class MemManager;
template<int MMBLOCKSIZE>
class DeletableMemManager;

#define DEFINE_ID_TYPE( nAME , mEMmANAGERnAME, __tEMPLATEdEFINITION__ ) \
__tEMPLATEdEFINITION__ \
class mEMmANAGERnAME; /* forward declaration */\
class nAME { \
  private:\
	IDTYPE _x;\
\
	inline IDTYPE get(void)	{ \
        PARANOID_ASSERT((_x!=getnamehash())); \
        PARANOID_ASSERT((_x!=ID_BLOCK_UNUSED)); \
		return _x; }\
	inline void   set(IDTYPE a) { _x = a; } \
	inline bool   is_equal(IDTYPE b) { \
		PARANOID_ASSERT((b._x!=getnamehash())); \
		return (_x == b)? 1:0;}\
	inline bool   not_equal(IDTYPE b) { \
		PARANOID_ASSERT((b._x!=getnamehash())); \
		return (_x != b)? 1:0;}\
	inline IDTYPE add(IDTYPE a)		{ \
		PARANOID_ASSERT((_x!=getnamehash())); \
		PARANOID_ASSERT((a._x!=getnamehash())); \
		_x += a; return _x;}\
	inline IDTYPE subtract(IDTYPE a)	{ \
		PARANOID_ASSERT((_x!=getnamehash())); \
		PARANOID_ASSERT((a._x!=getnamehash())); \
		_x -= a; return _x;}\
\
    template<int MMBLOCKSIZE>\
    friend class MemManager;\
    template<int MMBLOCKSIZE>\
    friend class DeletableMemManager;\
    __tEMPLATEdEFINITION__ \
    friend class mEMmANAGERnAME;\
\
  public:\
	nAME()  { _x = getnamehash(); }\
    nAME(std::ifstream * fh) { \
        fh->read(reinterpret_cast<char *>(&_x), sizeof(IDTYPE));} \
    ~nAME() { }\
	inline void operator =(nAME a)	{ \
		PARANOID_ASSERT((a._x!=getnamehash())); \
		_x = a._x; }\
	inline bool operator ==(const nAME b) const { \
		PARANOID_ASSERT((b._x!=getnamehash())); \
		return (_x == b._x)? 1:0;}\
	inline bool operator !=(const nAME b) const { \
		PARANOID_ASSERT((b._x!=getnamehash())); \
		return (_x != b._x)? 1:0;} \
    inline bool operator <(const nAME& b) const { \
        PARANOID_ASSERT((b._x!=getnamehash())); \
        return _x < b._x;} \
    inline void save(std::ofstream * fh) const { \
        PARANOID_ASSERT((_x!=getnamehash())); \
        fh->write((char *)(&_x), sizeof(IDTYPE));} \
    inline bool isbadfood(void) const { \
        return ( ID_BLOCK_UNUSED == (IDTYPE)_x ); }\
    inline IDTYPE getnamehash(void) { \
        const char * p = #nAME; \
        return ((IDTYPE)((uMDInt)p))^((IDTYPE)ID_BLOCK_UNUSED); }\
    inline IDTYPE getuniquehash(void) { \
        const char * p = #nAME; \
        return ((IDTYPE)((uMDInt)p))^((IDTYPE)ID_BLOCK_UNUSED)^(_x << (SIZE_OF_IDTYPE/2)); }\
    inline IDTYPE getGuts(void) { \
        return _x; }\
        friend ostream& operator <<(ostream &s, nAME a) { s << #nAME << ":" << std::hex << a._x << std::dec; return s; } \
\
}; 
/////////////////////////////////////////////////
// STOP! Hammer code time
/////////////////////////////////////////////////
//***********************************************


/////////////////////////////////////////////////
// Define your list of ID TYPE classes
/////////////////////////////////////////////////
#define template_int_RL_int_OFF_ template<int RL,int OFF>
DEFINE_ID_TYPE(UniNodeId, UniNodeMemManager, template_int_RL_int_OFF_);
DEFINE_ID_TYPE(UniNodeDataId, UniNodeMemManager, template_int_RL_int_OFF_);

DEFINE_ID_TYPE(DualNodeId, DualNodeMemManager, );
DEFINE_ID_TYPE(DualNodeDataId, DualNodeMemManager, );

DEFINE_ID_TYPE(ReadStoreId, ReadStoreMemManager, template<int RL>);

DEFINE_ID_TYPE(ContigId, ContigMemManager, );

DEFINE_ID_TYPE(GenericNodeId, GenericNodeMemManager, );

DEFINE_ID_TYPE(KmerMapId, KmerMapMemManager, );
DEFINE_ID_TYPE(KmerMapDataId, KmerMapMemManager, );

DEFINE_ID_TYPE(KmerMatchMapId, KmerMatchMapMemManager, );
DEFINE_ID_TYPE(KmerMatchMapDataId, KmerMatchMapMemManager, );

DEFINE_ID_TYPE(NodePositionMapId, NodePositionMapMemManager, );

DEFINE_ID_TYPE(ContextId, ContextMemManager, );

DEFINE_ID_TYPE(NodeCheckId, NodeCheckMemManager, );

DEFINE_ID_TYPE(ReadIdentifierId, ReadIdentifierMemManager, );

// for the file types cause these guys are just uMDInts...
typedef idInt FileSetId;

#endif // Id_Type_H
