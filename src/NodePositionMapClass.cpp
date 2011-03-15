/******************************************************************************
**
** File: NodePositionMapClass.cpp
**
*******************************************************************************
**
** Public interface for the nodeposition map
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
#include "NodePositionMapMemWrapper.h"
#include "intdef.h"
#include "IdTypeDefs.h"
#include "NodePositionMapClass.h"

using namespace std;


/******************************************************************************
** #INCLUDES #DEFINES AND PRIVATE VARS
******************************************************************************/
/*INC*/ typedef NPM_HEAD_ELEM NPMHeadElem;
/*INC*/ 
/*INC*/ typedef NPM_MASTER_ELEM NPMMasterElem;
/*INC*/ 
/******************************************************************************
** INTIALISATION AND SAVING
******************************************************************************/

/*HV*/ void
NodePositionMapClass::
/*HV*/ extendedDestructor(void)
//HO ;
{
    //-----
    // Add anything you would like to happen when the destructor is called
    //
}
//HO 

/*HV*/ bool
NodePositionMapClass::
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
NodePositionMapClass::
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
NodePositionMapClass::
/*HV*/ extendedSave(std::string fileName)
//HO ;
{
    //-----
    // Add anything you would like to happen when saving (including saving private vars)
    //
    return true;
}
//HO 

//HO inline void wipeAll(void) { mData->wipeAll(); }
//HO 

/******************************************************************************
** EXTENSIONS
******************************************************************************/
//HO inline bool addElem(NPMHeadElem * data, GenericNodeId GID) { return mData->addElem(data, GID); }
//HO 

/*HV*/ bool
NodePositionMapClass::
/*HV*/ addElem(GenericNodeId GID, uMDInt position, bool reversed)
//HO ;
{
    //-----
    // Add a node to the map
    //
    NPMHeadElem tmp_npm_elem;
    tmp_npm_elem.NPMHE_GID = GID;
    tmp_npm_elem.NPMHE_Position = position;
    tmp_npm_elem.NPMHE_Reversed = reversed;
    addElem(&tmp_npm_elem, GID);
    return true;
}
//HO 

//HO inline bool updateElem(GenericNodeId oldGID, GenericNodeId newGID) { return mData->updateElem(oldGID, newGID); }
//HO 
//HO inline bool deleteElem(GenericNodeId GID) { return mData->deleteElem(GID); }
//HO 
//HO inline void deleteElem(GenericNodeId GID, bool orientation, uMDInt position) { mData->deleteElem(GID, orientation, position); }
//HO 
//HO inline bool getHead(NPMHeadElem * data, GenericNodeId GID) { return mData->getHead(data, GID); }
//HO 
//HO inline bool getNextHead(NPMHeadElem * data) { return mData->getNextHead(data); }
//HO 
/*HV*/ bool
NodePositionMapClass::
/*HV*/ getMaster(NPMMasterElem * data, uMDInt position)
//HO ;
{
    return mData->getMaster(data, position);
}
//HO 
//HO inline bool getNextMaster(NPMMasterElem * data) { return mData->getNextMaster(data); }
//HO 
//HO inline void printEverything(GenericNodeId GID) { return mData->printEverything(GID); }
//HO 
//HO inline void printEverything(uMDInt pos) { return mData->printEverything(pos); }
//HO 
