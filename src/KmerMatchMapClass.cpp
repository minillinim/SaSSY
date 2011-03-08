/******************************************************************************
**
** File: KmerMatchMapClass.cpp
**
*******************************************************************************
**
** Layer 3 Implementation file for use with projects including MemManager.cpp
**
** This is the public interface to the helper class KmerMatchMap. This class is
** used during UniNodes makeEdges function to store information about how sequences
** should overlap.
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
#include "KmerMatchMapMemWrapper.h"
#include "intdef.h"
#include "IdTypeDefs.h"
#include "KmerMatchMapClass.h"

using namespace std;


/******************************************************************************
** #INCLUDES #DEFINES AND PRIVATE VARS
******************************************************************************/
/*INC*/ typedef KMM_MAP_ELEM KmerMatchMapElem;
/*INC*/ 
/******************************************************************************
** INTIALISATION AND SAVING
******************************************************************************/

/*HV*/ void
KmerMatchMapClass::
/*HV*/ extendedDestructor(void)
//HO ;
{
    //-----
    // Add anything you would like to happen when the destructor is called
    //
}
//HO 

/*HV*/ bool
KmerMatchMapClass::
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
KmerMatchMapClass::
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
KmerMatchMapClass::
/*HV*/ extendedSave(std::string fileName)
//HO ;
{
    //-----
    // Add anything you would like to happen when saving (including saving private vars)
    //
    return true;
}
//HO 


/******************************************************************************
** EXTENSIONS
******************************************************************************/

//HO inline bool addElem(UniNodeId UID, KmerMatchMapElem * data) { return mData->addElem(UID, data); }
//HO 
//HO inline bool getElem(UniNodeId UID, KmerMatchMapElem * data) { return mData->getElem(UID, data); }
//HO 
//HO inline bool getNextElem(KmerMatchMapElem * data) { return mData->getNextElem(data); }
//HO 
//HO inline bool clearKmerMatchMap(void) { return mData->clearKmerMatchMap(); }
//HO 

