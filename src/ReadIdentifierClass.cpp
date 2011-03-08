/******************************************************************************
**
** File: ReadIdentifierClass.cpp
**
*******************************************************************************
**
** Layer 3 Implementation file for use with projects including MemManager.cpp
**
** public interface for the readidentifier class
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
#include "ReadIdentifierMemWrapper.h"
#include "intdef.h"
#include "IdTypeDefs.h"
#include "ReadIdentifierClass.h"

using namespace std;


/******************************************************************************
** #INCLUDES #DEFINES AND PRIVATE VARS
******************************************************************************/

/******************************************************************************
** INTIALISATION AND SAVING
******************************************************************************/

/*HV*/ void
ReadIdentifierClass::
/*HV*/ extendedDestructor(void)
//HO ;
{
    //-----
    // Add anything you would like to happen when the destructor is called
    //
}
//HO 

/*HV*/ bool
ReadIdentifierClass::
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
ReadIdentifierClass::
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
ReadIdentifierClass::
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
// ADDING A NEW ID
//HO inline ReadIdentifierId addReadIdentifier(DualNodeId DID, std::string * insName, std::string * mPlex, uMDInt tileNum, uMDInt laneNum, uMDInt X_Cord, uMDInt Y_Cord) { return mData-> addReadIdentifier(DID, insName, mPlex, tileNum, laneNum, X_Cord, Y_Cord); }
//HO 

// getting the actual read id
//HO inline std::string getReadIdString(ReadIdentifierId RiID) { return mData->getReadIdString(RiID); }
//HO 

// MULTIPLEX CALLS
//HO inline uMDInt addMultString(std::string multStr) { return mData->addMultString(multStr); }
//HO 

//HO inline std::string getMultString(uMDInt ref) { return mData->getMultString(ref); }
//HO 

//HO inline bool isMultStringPresent(std::string multStr) { return mData->isMultStringPresent(multStr); }
//HO 

// COMMONSTRING CALLS
//HO inline uMDInt addCommonString(std::string commonStr) { return mData->addCommonString(commonStr); }
//HO 

//HO inline std::string getCommonString(uMDInt ref) { return mData->getCommonString(ref); }
//HO 

//HO inline bool isCommonStringPresent(std::string commonStr) { return mData->isCommonStringPresent(commonStr); }
//HO 

//HO 
//HO 
