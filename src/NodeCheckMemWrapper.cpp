/******************************************************************************
**
** File: NodeCheckMemWrapper.cpp
**
** When a list of GenericNodes is stored strange things can, and do, happen. For example, 
** there may be a dummy master in a list, say the third element. BUT when the first or
** second element is processed, the dummy may be assimilated, and the deleted. when you
** get around to processing the third node -- segfault. This class tokenises the nodes
** based on position, context, headnode and branch identifier. This gives a unique
** ID for the node. So instead of storing lists of nodes, store lists of tokens, and
** check in and out the nodes as needed...
**
*******************************************************************************
**
** There is a very specific format for editing this file
** Functions are written in this format and then make can auto generate
** the header files. The functions extendedInitialise and extendedSave are
** generated here by default. Add to these to add variables to be saved when
** save is called
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

#include <map>
//INC #include <map>
//INC 

// local includes

#include "GenericNodeClass.h"
//INC #include "GenericNodeClass.h"
//INC 

#include "NodePositionMapClass.h"
//INC #include "NodePositionMapClass.h"
//INC 

#include "TreeHistory.h"
//INC #include "TreeHistory.h"
//INC 

#include "intdef.h"
#include "IdTypeDefs.h"
#include "NodeCheckMemWrapper.h"
#include "LoggerSimp.h"

using namespace std;

/******************************************************************************
** #INCLUDES #DEFINES AND PRIVATE VARS
******************************************************************************/
// for quick retrieval of node position maps
/*INC*/ typedef map<ContextId, NodePositionMapClass *> contextNPMLinker;
/*INC*/ 
/*INC*/ typedef map<ContextId, NodePositionMapClass *>::iterator contextNPMLinkerIterator;
/*INC*/ 
//PV GenericNodeClass * mNodes;
//PV 
//PV GenericNodeId GN_NULL_ID;
//PV 
//PV contextNPMLinker * mMasterNPMs;
//PV 
//PV idInt mNumBlocks;
//PV 
//PV std::string mName;
//PV 
/******************************************************************************
** INTIALISATION AND SAVING
******************************************************************************/

/*HV*/ void
NodeCheckMemWrapper::
/*HV*/ extendedDestructor(void)
//HO ;
{
    //-----
    // Add anything you would like to happen when the destructor is called
    //
}
//HO 

/*HV*/ bool
NodeCheckMemWrapper::
/*HV*/ extendedInitialise(idInt originalNumberOfBlocks )
//HO ;
{
    //-----
    // Add anything you would like to happen on a regular intialisation call
    //
    mNumBlocks = originalNumberOfBlocks;
    return true;
}
//HO 

/*HV*/ bool
NodeCheckMemWrapper::
/*HV*/ extendedInitialise(std::string fileName)
//HO ;
{
    //-----
    // Add anything you would like to happen when reloading a saved instance
    //
    // you can't load this puppy...
    return false;
    return true;
}
//HO 

/*HV*/ bool
NodeCheckMemWrapper::
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
/*HV*/ void
NodeCheckMemWrapper::
/*HV*/ setObjectPointers(GenericNodeClass * GNC, contextNPMLinker * MNPM, std::string name)
//HO ;
{
    //-----
    // set the npm and generic node objects in sync with the Context manager
    //
    mNodes = GNC;
    GN_NULL_ID = mNodes->getNullGenericNodeId();
    mMasterNPMs = MNPM;
    mName = name;
}
//HO 

/*HV*/ void
NodeCheckMemWrapper::
/*HV*/ clearTokens(void)
//HO ;
{
    //-----
    // reset all the tokens
    //
    if (mData != NULL) { delete mData; }
    if (mNodeCheck != NULL) { delete mNodeCheck; }
    initialise(mNumBlocks);
}
//HO 

/*HV*/ NodeCheckId
NodeCheckMemWrapper::
/*HV*/ checkInNode(GenericNodeId saveNode)
//HO ;
{
    //-----
    // tokenizes a node into an uMDINt so that it doesn't get silently nuked.
    // manages the various maps
    //
    NodeCheckId ret_NCID = newNodeCheckId();
    if(mNodes->isValidAddress(saveNode))
    {
        setCntxHead(mNodes->getHead(saveNode), ret_NCID);
        setCntxPos(mNodes->getCntxPos(saveNode), ret_NCID);
        setCntxHistory(mNodes->getCntxHistory(saveNode), ret_NCID);
        return ret_NCID;
    }
    logError(mName << ": Regular node: " << saveNode << " is invalid");
    
    // delete this guy...
    deleteNodeCheckId(ret_NCID);
    return getNullNodeCheckId();
}
//HO 

/*HV*/ GenericNodeId
NodeCheckMemWrapper::
/*HV*/ checkOutNode(bool clear, ContextId CTXID, NodeCheckId NCID)
//HO ;
{
    //-----
    // tokenizes a node into an uMDINt so that it doesn't get silently nuked.
    // manages the various maps
    //
    PARANOID_ASSERT_PRINT_L4((NCID != NC_NULL_ID), "Passed a null NCID");
    GenericNodeId ret_GID = safeGetNode(getCntxHead(NCID), getCntxPos(NCID), getCntxHistory(NCID), CTXID);
    if(clear)
        deleteNodeCheckId(NCID);
    
    return ret_GID;
}
//HO 

/*HV*/ GenericNodeId
NodeCheckMemWrapper::
/*HV*/ safeGetNode(GenericNodeId savedHead, uMDInt savedPos, HistoryId savedHID, ContextId CTXID)
//HO ;
{
    //-----
    // Nodes stored in lists can get "silently" nuked.
    // BUT, the head node for a given sequence at a position in a context
    // is more or less immutable. Return the master, trust it only
    // until another delete has occurred
    //
    NodePositionMapClass * master_npm = (*mMasterNPMs)[CTXID];
    PARANOID_ASSERT_PRINT_L4((master_npm != NULL), "No masterNPM for: " << CTXID);
    NPMMasterElem tmp_master_elem;
    bool first_found = false;
    GenericNodeId ret_ID;
    if(master_npm->getMaster(&tmp_master_elem, savedPos))
    {
        // there is a master here... ... but is is the right one?
        // check all masters for if the heads match
        do {
            // explicity check if the heads match
            if(mNodes->getHead(tmp_master_elem.NPMME_GID) == savedHead)
            {
                if(mNodes->getCntxHistory(tmp_master_elem.NPMME_GID) == savedHID)
                {
                    if(!first_found)
                    {
                        ret_ID = tmp_master_elem.NPMME_GID;
                        first_found = true;
                    }
                    else
                    {
                        logError(ret_ID << " : " <<  mNodes->getSequence(ret_ID) << " : " << mNodes->getCntxId(ret_ID) << " : " << mNodes->getCntxHistory(ret_ID) << " : " << mNodes->getCntxPos(ret_ID));
                        logError(tmp_master_elem.NPMME_GID << " : " <<  mNodes->getSequence(tmp_master_elem.NPMME_GID)  << " : " << mNodes->getCntxId(tmp_master_elem.NPMME_GID) << " : " << mNodes->getCntxHistory(tmp_master_elem.NPMME_GID) << " : " << mNodes->getCntxPos(tmp_master_elem.NPMME_GID));
                    }
                }
            }
            
        } while(master_npm->getNextMaster(&tmp_master_elem));
        if(first_found)
            return ret_ID;
    }
    else
    {
        logError(mName << ": No master at: " << savedPos << " : " << savedHID << " : " << savedHead << " : " << CTXID);
    }
    return GN_NULL_ID;
}
//HO 
