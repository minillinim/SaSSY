/******************************************************************************
**
** File: KmerMapClass.cpp
**
*******************************************************************************
**
** Layer 3 Implementation file for use with projects including MemManager.cpp
**
** This is the public interface to the helper class KmerMap. This Class stores
** Information about which UniNodes contain Which Kmers. Given a Kmer It is
** possible to get a list of all UniNodes which contain that kmer as well as the
** relative position and orientation of the kmer in the UniNodes sequence
**
**
*******************************************************************************
**
**                        A
**                       A B
**                      A B R
**                     A B R A
**                    A B R A C
**                   A B R A C A
**                  A B R A C A D
**                 A B R A C A D A
**                A B R A C A D A B
**               A B R A C A D A B R
**              A B R A C A D A B R A
**
*******************************************************************************
**
** Copyright (C) 2009 2010 2011 Michael Imelfort and Dominic Eales
**
** This file is part of the Sassy Assembler Project.
**
** Sassy is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** Sassy is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Sassy.  If not, see <http://www.gnu.org/licenses/>.
**
******************************************************************************/

// system includes
#include <iostream>

// local includes
#include "KmerMapMemWrapper.h"
#include "intdef.h"
#include "IdTypeDefs.h"
#include "KmerMapClass.h"

using namespace std;


/******************************************************************************
** #INCLUDES #DEFINES AND PRIVATE VARS
******************************************************************************/
/*INC*/ typedef KM_MAP_ELEM KmerMapElem;
/*INC*/
/******************************************************************************
** INTIALISATION AND SAVING
******************************************************************************/

/*HV*/ void
KmerMapClass::
/*HV*/ extendedDestructor(void)
//HO ;
{
    //-----
    // Add anything you would like to happen when the destructor is called
    //
}
//HO 

/*HV*/ bool
KmerMapClass::
/*HV*/ extendedInitialise(uMDInt originalNumberOfBlocks )
//HO ;
{
    //-----
    // Add anything you would like to happen on a regular intialisation call
    //
    return true;
}
//HO 

/*HV*/ bool
KmerMapClass::
/*HV*/ extendedInitialise(std::string fileName)
//HO ;
{
    //-----
    // Add anything you would like to happen when reloading a saved instance
    //
    return true;
}
//HO 

/*HV*/ bool
KmerMapClass::
/*HV*/ extendedSave(std::string fileName)
//HO ;
{
    //-----
    // Add anything you would like to happen when saving (including saving private vars)
    //
    return true;
}
//HO 

//HO inline void initialise2(unsigned int KL, ReadStoreClass * RSC) { mData->initialise2(KL, RSC); }
//HO 

/******************************************************************************
** EXTENSIONS
******************************************************************************/
    
//HO inline bool addElem(std::string * sequence, bool * reversed, unsigned int position, ReadStoreId RSID, KM_MAP_ELEM * data) { return mData->addElem(sequence, reversed, position, RSID, data); }
//HO 
//HO inline bool getNextElem(KmerMapElem * data) { return mData->getNextElem(data); }
//HO 
