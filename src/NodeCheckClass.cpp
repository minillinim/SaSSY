/******************************************************************************
**
** File: NodeCheckClass.cpp.template
**
*******************************************************************************
**
** Layer 3 Implementation file for use with projects including MemManager.cpp
**
** This file has been automatically generated using maike.pl.
** It serves as an interface to the main algorithm. It interfaces with the
** the wrapper class so all the nitty gritty layer 1 and 2 code is hidden away
** safely.
**
** You will need to edit this file by hand to extend the functionality
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
#include "NodeCheckMemWrapper.h"
#include "intdef.h"
#include "IdTypeDefs.h"
#include "NodeCheckClass.h"

using namespace std;


/******************************************************************************
** #INCLUDES #DEFINES AND PRIVATE VARS
******************************************************************************/

/******************************************************************************
** INTIALISATION AND SAVING
******************************************************************************/

/*HV*/ void
NodeCheckClass::
/*HV*/ extendedDestructor(void)
//HO ;
{
    //-----
    // Add anything you would like to happen when the destructor is called
    //
}
//HO 

/*HV*/ bool
NodeCheckClass::
/*HV*/ extendedInitialise(idInt originalNumberOfBlocks )
//HO ;
{
    //-----
    // Add anything you would like to happen on a regular intialisation call
    //
    return true;
}
//HO 

/*HV*/ bool
NodeCheckClass::
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
NodeCheckClass::
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
//HO inline void setObjectPointers(GenericNodeClass * GNC, contextNPMLinker * MNPM, std::string name) { mData->setObjectPointers(GNC, MNPM, name); }
//HO 

//HO inline void clearTokens(void) { /*mData->clearTokens();*/ }
//HO 

//HO inline NodeCheckId checkInNode(GenericNodeId saveNode) { return mData->checkInNode(saveNode); }
//HO 

//HO inline GenericNodeId checkOutNode(bool clear, ContextId CTXID, NodeCheckId NCID) { return mData->checkOutNode(clear, CTXID,  NCID); }
//HO 

//HO inline GenericNodeId safeGetNode(GenericNodeId savedHead, uMDInt savedPos, HistoryId savedHID, ContextId CTXID) { return mData->safeGetNode(savedHead, savedPos, savedHID, CTXID); }
//HO 

