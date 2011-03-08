/******************************************************************************
**
** File: NodePositionMapMemWrapper.cpp
**
*******************************************************************************
** 
** Layer 2 file for implementing a node position map class.
** This guy can be used in two ways.
** 
** First it maps GenericNodeIds to positions in the context.
** These are head GNs so it is a one to many relationship.
** 
** Also it can be used to map master nodes in the same way
** 
** Essentially one is a mapper for UniNodes and one for DualNodes.
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
#include "intdef.h"
#include "IdTypeDefs.h"
#include "NodePositionMapMemWrapper.h"
#include "IdTypeSTL_ext.h"
#include "LoggerSimp.h"
#include "paranoid.h"

using namespace std;

/******************************************************************************
** #INCLUDES #DEFINES AND PRIVATE VARS
******************************************************************************/
/*INC*/ #include <map>
/*INC*/ 
/*INC*/ typedef std::map<GenericNodeId, NodePositionMapId>::iterator nodePosRefIter;
/*INC*/ 
/*INC*/ typedef std::multimap<uMDInt, GenericNodeId>::iterator posNodeRefIter;
/*INC*/ 
/*INC*/ typedef std::pair<posNodeRefIter, posNodeRefIter> posNodeRefMultiIter;
/*INC*/ 
//INC class NPM_HEAD_ELEM {
//INC 
//INC     public:
//INC 
//INC         NPM_HEAD_ELEM() { NPMHE_NextHit = NodePositionMapId(); }
//INC
//INC         // to get some insight
//INC 
//INC         void printContents(void)
//INC 
//INC         {
//INC 
//INC             std::cout << "Node: " << NPMHE_GID << " Position: " << NPMHE_Position << " Orientation: " << NPMHE_Reversed << std::endl;
//INC 
//INC         }
//INC 
//INC         GenericNodeId NPMHE_GID;
//INC 
//INC         uMDInt NPMHE_Position;
//INC 
//INC         bool NPMHE_Reversed;            // is this guy orientated AS WRITTEN or reversed
//INC 
//INC     private:
//INC 
//INC         NodePositionMapId NPMHE_NextHit;
//INC 
//INC         friend class NodePositionMapMemWrapper;
//INC
//INC };
//INC 
//INC class NPM_MASTER_ELEM {
//INC 
//INC     public:
//INC 
//INC         NPM_MASTER_ELEM() { }
//INC
//INC         // to get some insight
//INC 
//INC         void printContents(void)
//INC 
//INC         {
//INC 
//INC             std::cout << "Node: " << NPMME_GID << std::endl;
//INC 
//INC         }
//INC 
//INC         GenericNodeId NPMME_GID;
//INC 
//INC     private:
//INC 
//INC         posNodeRefIter NPMME_ThisHit;
//INC 
//INC         posNodeRefIter NPMME_LastHit;
//INC 
//INC         friend class NodePositionMapMemWrapper;
//INC
//INC };
//INC 
//PV map<GenericNodeId, NodePositionMapId> mNodePosRef;
//PV 
//PV multimap<uMDInt, GenericNodeId> mPosNodeRef;
//PV 
/******************************************************************************
** INITIALISATION AND SAVING
******************************************************************************/

/*HV*/ void
NodePositionMapMemWrapper::
/*HV*/ extendedDestructor(void)
//HO ;
{
    //-----
    // Add anything you would like to happen when the destructor is called
    //
}
//HO 

/*HV*/ bool
NodePositionMapMemWrapper::
/*HV*/ extendedInitialise(uMDInt originalNumberOfBlocks )
//HO ;
{
    //-----
    // Add anything you would like to happen on a regular initialisation call
    //
    return true;
}
//HO 

    /*
    ** Specialized saving and loading functions just for this object...
    */
        
void readMap(std::map<GenericNodeId, NodePositionMapId> * readmap, ifstream * inFile) 
{
    readmap->clear();
    unsigned int size;
    inFile->read(reinterpret_cast<char *>(&size), sizeof(unsigned int));
    while(size > 0) {
        GenericNodeId tmp_GID(inFile);
        NodePositionMapId tmp_NPMID(inFile);
        (*readmap)[tmp_GID] = tmp_NPMID;
        size--;
    }
}

void writeMap(std::map<GenericNodeId, NodePositionMapId> * writemap, ofstream * outFile)
{
    unsigned int size = writemap->size();
    outFile->write(reinterpret_cast<char *>(&size), sizeof(unsigned int));
    map<GenericNodeId, NodePositionMapId>::iterator map_iter = writemap->begin();
    map<GenericNodeId, NodePositionMapId>::iterator map_last = writemap->end();
    while(map_iter != map_last) {
        (map_iter->first).save(outFile);
        (map_iter->second).save(outFile);
        map_iter++;
    }
}

/*HV*/ bool
NodePositionMapMemWrapper::
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
NodePositionMapMemWrapper::
/*HV*/ extendedSave(std::string fileName)
//HO ;
{
    //-----
    // Add anything you would like to happen when saving (including saving private vars)
    //
    return true;
}
//HO 

/*HV*/ void
NodePositionMapMemWrapper::
/*HV*/ wipeAll(void)
//HO ;
{
    //-----
    // wipe all the data from this guy
    //
    nodePosRefIter all_NPMs_iter = mNodePosRef.begin();
    while(all_NPMs_iter != mNodePosRef.end())
    {
        NodePositionMapId head_NPM = all_NPMs_iter->second;

        // first delete all the data blocks
        NodePositionMapId next_hit = getNextElem(head_NPM);
        while(NPM_NULL_ID != next_hit) {
            NodePositionMapId this_hit = next_hit;
            next_hit = getNextElem(next_hit);
            deleteNodePositionMapId(this_hit);
        }

        // delete the head elem itself
        deleteNodePositionMapId(head_NPM);
        all_NPMs_iter++;
    }
    // finally clear these maps too
    mNodePosRef.clear();
    mPosNodeRef.clear();
}
//HO 
/******************************************************************************
** EXTENSIONS
******************************************************************************/

/*HV*/ bool
NodePositionMapMemWrapper::
/*HV*/ addElem(NPM_HEAD_ELEM * data, GenericNodeId GID)
//HO ;
{
    //-----
    // Add a sequence to the map
    //
    // check and see if we added this guy before
    nodePosRefIter finder = mNodePosRef.find(GID);
    if(finder != mNodePosRef.end())
    {
        // this guy exists!
        // make a new data block
        NodePositionMapId new_tail = newNodePositionMapId();
        // set its values
        setGn(data->NPMHE_GID, new_tail);
        setPosition(data->NPMHE_Position, new_tail);
        setOrientation(data->NPMHE_Reversed, new_tail);

        // we need to tack the new entry just behind the head
        // we will join the rest of the chain onto the end
        // of the new_tail
        NodePositionMapId current_tail = getNextElem(finder->second);
        if(NPM_NULL_ID != current_tail)
        {
            // first data block
            setNextElem(current_tail, new_tail);
        }

        // add this to the head
        setNextElem(new_tail, finder->second);

        // add the reference to the posNodeRef
        mPosNodeRef.insert(std::pair<uMDInt, GenericNodeId>(data->NPMHE_Position, GID));
        
        // done
        return true;
    }
    // get a new element
    NodePositionMapId new_id = newNodePositionMapId();
    // set its values
    setGn(data->NPMHE_GID, new_id);
    setPosition(data->NPMHE_Position, new_id);
    setOrientation(data->NPMHE_Reversed, new_id);
    
    // add it to the nodePosRef
    mNodePosRef[GID] = new_id;
     
    // add the reference to the posNodeRef
    mPosNodeRef.insert(std::pair<uMDInt, GenericNodeId>(data->NPMHE_Position, GID));
    
    return true;
}
//HO 

/*HV*/ bool
NodePositionMapMemWrapper::
/*HV*/ updateElem(GenericNodeId oldGID, GenericNodeId newGID)
//HO ;
{
    //-----
    // called as a result of assimilate dummy
    // update the value of the GID to newGID in the chain for the the oldGID
    // then fix up all the references
    //
    
    nodePosRefIter finder = mNodePosRef.find(oldGID);
    posNodeRefMultiIter posNodeIterPair;
    posNodeRefIter finder2;
    NodePositionMapId head_NPM;
    
    if(finder != mNodePosRef.end())
    {
        head_NPM = finder->second;

        // update the GID in the map
        setGn(newGID, head_NPM);

        // update the position information to match the new GID
        posNodeIterPair = mPosNodeRef.equal_range(getPosition(head_NPM));
        finder2 = posNodeIterPair.first;
        while(finder2 != posNodeIterPair.second)
        {
            if((*finder2).second == oldGID)
            {
                (*finder2).second = newGID;
                break;
            }
            finder2++;
        }
        
        NodePositionMapId next_hit = getNextElem(head_NPM);
        while(NPM_NULL_ID != next_hit) {
            setGn(newGID, next_hit);
            posNodeIterPair = mPosNodeRef.equal_range(getPosition(next_hit));
            finder2 = posNodeIterPair.first;
            while(finder2 != posNodeIterPair.second)
            {
                if((*finder2).second == oldGID)
                {
                    (*finder2).second = newGID;
                    break;
                }
                finder2++;
            }
            next_hit = getNextElem(next_hit);
        }

        // delete this guy from the NodePosRef
        mNodePosRef.erase(finder);

        // insert the new entry into the NodePosRef
        mNodePosRef[newGID] = head_NPM;
        
        return true;
    }
    else
        logError(oldGID << " not found");
    return false;
}
//HO 

/*HV*/ void
NodePositionMapMemWrapper::
/*HV*/ deleteElem(GenericNodeId GID)
//HO ;
{
    //-----
    // completely remove this GID from the object
    //
    
    nodePosRefIter finder = mNodePosRef.find(GID);
    posNodeRefMultiIter posNodeIterPair;
    posNodeRefIter finder2;
    NodePositionMapId head_NPM;
    
    if(finder != mNodePosRef.end())
    {
        head_NPM = finder->second;
        PARANOID_ASSERT_PRINT_L2(isValidAddress(head_NPM), "Invalid npm_ID " << head_NPM);
        posNodeIterPair = mPosNodeRef.equal_range(getPosition(head_NPM));
        finder2 = posNodeIterPair.first;
        while(finder2 != posNodeIterPair.second)
        {
            if(finder2->second == GID)
            {
                mPosNodeRef.erase(finder2);
                break;
            }
            finder2++;
        }
        
        NodePositionMapId next_hit = getNextElem(head_NPM);
        while(NPM_NULL_ID != next_hit) {
            NodePositionMapId this_hit = next_hit;
            posNodeIterPair = mPosNodeRef.equal_range(getPosition(this_hit));
            finder2 = posNodeIterPair.first;
            if(finder2->second == GID)
            {
                mPosNodeRef.erase(finder2);
                break;
            }
            finder2++;
            next_hit = getNextElem(this_hit);
            PARANOID_ASSERT_PRINT_L2(isValidAddress(this_hit), "@Invalid npm_ID " << this_hit);
            deleteNodePositionMapId(this_hit);
        }

        // finally delete this guy from the NodePosRef and delete the elem itself
        mNodePosRef.erase(finder);
        deleteNodePositionMapId(head_NPM);
    }
    else
        logError(GID << " not found");
}
//HO 


/*HV*/ void
NodePositionMapMemWrapper::
/*HV*/ deleteElem(GenericNodeId GID, bool orientation, uMDInt position)
//HO ;
{
    //-----
    // remove the GID matching these charactaristics from the object
    //
    nodePosRefIter finder = mNodePosRef.find(GID);
    posNodeRefMultiIter posNodeIterPair;
    posNodeRefIter finder2;
    NodePositionMapId head_NPM;
    
    if(finder != mNodePosRef.end())
    {
        head_NPM = finder->second;
        // first we find the GID in the posNodeRef
        posNodeIterPair = mPosNodeRef.equal_range(position);
        finder2 = posNodeIterPair.first;
        while(finder2 != posNodeIterPair.second)
        {
            if(finder2->second == GID)
            {
                mPosNodeRef.erase(finder2);
                break;
            }
            finder2++;
        }

        // now we find the GID with the right charactaristics in the nodePosRef
        // and we remove it from the chain
        if(getPosition(head_NPM) == position)
        {
            if(isOrientation(head_NPM) == orientation)
            {
                // we found our guy. We'll need to fix up the listing
                // First get the next hit
                NodePositionMapId next_hit = getNextElem(head_NPM);
                // delete the head
                deleteNodePositionMapId(head_NPM);
                mNodePosRef.erase(finder);
                if(NPM_NULL_ID != next_hit)
                {
                    // make a new head
                    mNodePosRef[GID] = next_hit;
                }
                return;
            }
        }

        // if we're here then we didn't find it in the list
        NodePositionMapId link_from_NPMID = head_NPM;
        NodePositionMapId query_NPMID = getNextElem(head_NPM);
        while(NPM_NULL_ID != query_NPMID)
        {
            if(getPosition(query_NPMID) == position)
            {
                if(isOrientation(query_NPMID) == orientation)
                {
                    // we found our guy
                    NodePositionMapId next_hit = getNextElem(query_NPMID);
                    deleteNodePositionMapId(query_NPMID);
                    setNextElem(next_hit, link_from_NPMID);
                    return;
                }
            }
            // move along
            link_from_NPMID = query_NPMID;
            query_NPMID = getNextElem(query_NPMID);
        }
    }
    else
        logError(GID << " not found: " << orientation << " : " << position);
}
//HO 

/*HV*/ bool
NodePositionMapMemWrapper::
/*HV*/ getHead(NPM_HEAD_ELEM * data, GenericNodeId GID)
//HO ;
{
    //-----
    // Get the first matching kmer from the map
    //
    nodePosRefIter finder = mNodePosRef.find(GID);
    if(finder != mNodePosRef.end())
    {
        data->NPMHE_GID = getGn(finder->second);
        data->NPMHE_Position = getPosition(finder->second);
        data->NPMHE_Reversed = isOrientation(finder->second);
        
        NodePositionMapId next_hit = getNextElem(finder->second);
        if(NPM_NULL_ID == next_hit)
        {
            data->NPMHE_NextHit = NPM_NULL_ID;
        }
        else
        {
            data->NPMHE_NextHit = next_hit;
        }
        // done
        return true;
    }
    // we don't have this fella
    return false;
}
//HO 

/*HV*/ bool
NodePositionMapMemWrapper::
/*HV*/ getNextHead(NPM_HEAD_ELEM * data)
//HO ;
{
    //-----
    // get the next matching kmer from the map
    //
    if(data->NPMHE_NextHit == NPM_NULL_ID || data->NPMHE_NextHit == NodePositionMapId())
    {
        return false;
    }
    else
    {
        data->NPMHE_GID = getGn(data->NPMHE_NextHit);
        data->NPMHE_Position = getPosition(data->NPMHE_NextHit);
        data->NPMHE_Reversed = isOrientation(data->NPMHE_NextHit);
        
        NodePositionMapId next_hit = getNextElem(data->NPMHE_NextHit);
        if(NPM_NULL_ID == next_hit)
        {
            data->NPMHE_NextHit = NPM_NULL_ID;
        }
        else
        {
            data->NPMHE_NextHit = next_hit;
        }
        // done
        return true;
    }
}
//HO 


/*HV*/ bool
NodePositionMapMemWrapper::
/*HV*/ getMaster(NPM_MASTER_ELEM * data, uMDInt position)
//HO ;
{
    //-----
    // Get the master at a position
    //
    posNodeRefMultiIter posNodeIterPair = mPosNodeRef.equal_range(position);
    data->NPMME_ThisHit = posNodeIterPair.first;
    data->NPMME_LastHit = posNodeIterPair.second;
    if(data->NPMME_ThisHit != data->NPMME_LastHit)
    {
        data->NPMME_GID = data->NPMME_ThisHit->second;
        return true;
    }
    else
        return false;
}
//HO 

/*HV*/ bool
NodePositionMapMemWrapper::
/*HV*/ getNextMaster(NPM_MASTER_ELEM * data)
//HO ;
{
    //-----
    // get the next matching kmer from the map
    //
    data->NPMME_ThisHit++;
    
    if(data->NPMME_ThisHit == data->NPMME_LastHit)
    {
        return false;
    }
    else
    {
        data->NPMME_GID = data->NPMME_ThisHit->second;
        return true;
    }

}
//HO 

/*HV*/ void
NodePositionMapMemWrapper::
/*HV*/ printEverything(NodePositionMapId NPMID)
//HO ;
{
    //-----
    // print the information stored in a NPM instance
    //
    std::cout << "Printing all information for: " << NPMID << std::endl;
    std::cout << getGn(NPMID) << " : POS : " << getPosition(NPMID) << " : REV : " << isOrientation(NPMID) << " NEXT: " << getNextElem(NPMID) << std::endl;
    NodePositionMapId next_hit = getNextElem(NPMID);
    while(NPM_NULL_ID != next_hit) {
        std::cout << getGn(next_hit) << " : POS : " << getPosition(next_hit) << " : REV : " << isOrientation(next_hit) << " NEXT: " << getNextElem(NPMID) << std::endl;
        next_hit = getNextElem(next_hit);
    }
    std::cout << "======" << std::endl;
}
//HO 

/*HV*/ void
NodePositionMapMemWrapper::
/*HV*/ printEverything(GenericNodeId GID)
//HO ;
{
    //-----
    // print all the information stored about a GID
    //
    nodePosRefIter finder = mNodePosRef.find(GID);
    posNodeRefMultiIter posNodeIterPair;
    posNodeRefIter finder2;
    std::cout << "Printing all information stored for: " << GID << std::endl;
    if(finder != mNodePosRef.end())
    {
        printEverything(finder->second);
    }
    else
        std::cout << "No information stored for: " << GID << std::endl;
    std::cout << "---" << std::endl;
}
//HO 


/*HV*/ void
NodePositionMapMemWrapper::
/*HV*/ printEverything(uMDInt position)
//HO ;
{
    //-----
    // print all the information stored about a position
    //
    std::cout << "Printing all nodes at position: " << position << std::endl;
    posNodeRefMultiIter posNodeIterPair;
    posNodeRefIter finder;
    posNodeIterPair = mPosNodeRef.equal_range(position);
    finder = posNodeIterPair.first;
    while(finder != posNodeIterPair.second)
    {
        std::cout << finder->second << std::endl;
        finder++;
    }
    std::cout << "---" << std::endl;
}
//HO 
