/******************************************************************************
**
** File: ReadIdentifierMemWrapper.cpp
**
*******************************************************************************
**
** This class stores the read identifeir information for an illumina dataset
** should store it more effciently than just plain text
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
#include <sstream>

#include <map>
//INC #include <map>
//INC 

// local includes
#include "intdef.h"
#include "IdTypeDefs.h"
#include "ReadIdentifierMemWrapper.h"

//INC #include "map_ext.h"
//INC 
#include "map_ext.h"

/******************************************************************************
** #INCLUDES #DEFINES AND PRIVATE VARS
******************************************************************************/
// this guy stores all the multiplex indicies and any othetr common strings for the reads. These are usually strings so we only want
// to store them once and hand tokens out ot the dualnodes
//INC typedef std::map<unsigned int, std::string> ToStringMap;
//INC 
//INC typedef std::map<std::string, unsigned int> FromStringMap;
//INC 
//INC typedef std::map<unsigned int, std::string>::iterator ToStringMapIterator;
//INC 
//INC typedef std::map<std::string, unsigned int>::iterator FromStringMapIterator;
//INC 
//INC class COMMON_STRING_STORE {
//INC 
//INC public:
//INC 
//INC COMMON_STRING_STORE() { mCSIndex = 1; mToStringMap = new ToStringMap(); mFromStringMap = new FromStringMap(); }
//INC 
//INC COMMON_STRING_STORE( std::string loadFile ) { mToStringMap = new ToStringMap(); mFromStringMap = new FromStringMap(); loadCS_Store(loadFile); }
//INC 
//INC ~COMMON_STRING_STORE() { delete mToStringMap; delete mFromStringMap; }
//INC 
//INC std::string getCommonString(unsigned int ref) {
//INC 
//INC     ToStringMapIterator finder = mToStringMap->find(ref);
//INC 
//INC     if(finder != mToStringMap->end()) { return finder->second; }
//INC 
//INC     else { return ""; }
//INC 
//INC }
//INC 
//INC unsigned int addCommonString(std::string commonStr) {
//INC 
//INC     FromStringMapIterator finder = mFromStringMap->find(commonStr);
//INC 
//INC     if(finder != mFromStringMap->end()) { return finder->second; }
//INC 
//INC     else { mCSIndex++; (*mFromStringMap)[commonStr] = mCSIndex; (*mToStringMap)[mCSIndex] = commonStr; return mCSIndex; }
//INC 
//INC  }
//INC 
//INC bool isCommonStringPresent(std::string commonStr) {
//INC 
//INC     FromStringMapIterator finder = mFromStringMap->find(commonStr);
//INC 
//INC     if(finder != mFromStringMap->end()) { return true; }
//INC 
//INC     return false;
//INC 
//INC  }
//INC 
//INC void printContents(void) {
//INC 
//INC     ToStringMapIterator CS_iter_T = mToStringMap->begin();
//INC 
//INC     while(CS_iter_T != mToStringMap->end())
//INC 
//INC     {
//INC 
//INC         std::cout << CS_iter_T->first << " : " << CS_iter_T->second << std::endl;
//INC 
//INC     }
//INC 
//INC     std::cout << "----------------------------" << std::endl;
//INC 
//INC     FromStringMapIterator CS_iter_F = mFromStringMap->begin();
//INC 
//INC     while(CS_iter_F != mFromStringMap->end())
//INC 
//INC     {
//INC 
//INC         std::cout << CS_iter_F->first << " : " << CS_iter_F->second << std::endl;
//INC 
//INC     }
//INC 
//INC }
//INC 
//INC  void saveCS_Store( std::string fileName ) {
//INC 
//INC      std::ofstream myFile(fileName.c_str(), std::ofstream::binary);
//INC 
//INC      myFile.write(reinterpret_cast<char *>(&mCSIndex), sizeof(unsigned int));
//INC 
//INC      writeMap(mFromStringMap, &myFile);
//INC 
//INC      writeMap(mToStringMap, &myFile);
//INC 
//INC      myFile.close();
//INC 
//INC }
//INC 
//INC  void loadCS_Store( std::string fileName ) {
//INC 
//INC      std::ifstream myFile(fileName.c_str(), std::ofstream::binary);
//INC 
//INC      myFile.read(reinterpret_cast<char *>(&mCSIndex), sizeof(unsigned int));
//INC 
//INC      readMap(mFromStringMap, &myFile);
//INC 
//INC      readMap(mToStringMap, &myFile);
//INC 
//INC      myFile.close();
//INC
//INC }
//INC 
//INC private:
//INC 
//INC unsigned int mCSIndex;
//INC 
//INC ToStringMap * mToStringMap;                           // for converting from tokens
//INC 
//INC FromStringMap * mFromStringMap;                       // for converting from tokens
//INC 
//INC };
//INC 

//PV COMMON_STRING_STORE * mMults;                          // for storing multiplex indicies
//PV 
//PV COMMON_STRING_STORE * mCommonIds;                      // for storing the common part of read ids                                                          
//PV

/******************************************************************************
** INTIALISATION AND SAVING
******************************************************************************/

/*HV*/ void
ReadIdentifierMemWrapper::
/*HV*/ extendedDestructor(void)
//HO ;
{
    //-----
    // Add anything you would like to happen when the destructor is called
    //
    if(mMults != NULL)
    {
        delete mMults;
        mMults = NULL;
    }
    if(mCommonIds != NULL)
    {
        delete mCommonIds;
        mCommonIds = NULL;
    }
}
//HO 

/*HV*/ bool
ReadIdentifierMemWrapper::
/*HV*/ extendedInitialise(uMDInt originalNumberOfBlocks )
//HO ;
{
    //-----
    // Add anything you would like to happen on a regular intialisation call
    //
    mMults = new COMMON_STRING_STORE();
    mCommonIds = new COMMON_STRING_STORE();
    return true;
}
//HO 

/*HV*/ bool
ReadIdentifierMemWrapper::
/*HV*/ extendedInitialise(std::string fileName)
//HO ;
{
    //-----
    // Add anything you would like to happen when reloading a saved instance
    //
    mMults = new COMMON_STRING_STORE(fileName + ".mults");
    mCommonIds = new COMMON_STRING_STORE(fileName + ".commons");
    return true;
}
//HO 

/*HV*/ bool
ReadIdentifierMemWrapper::
/*HV*/ extendedSave(std::string fileName)
//HO ;
{
    //-----
    // Add anything you would like to happen when saving (including saving private vars)
    //
    mMults->saveCS_Store(fileName + ".mults");
    mCommonIds ->saveCS_Store(fileName + ".commons");
    return true;
}
//HO 

/******************************************************************************
** HOUSEKEEPING
******************************************************************************/
/*HV*/ ReadIdentifierId
ReadIdentifierMemWrapper::
/*HV*/ addReadIdentifier(DualNodeId DID, std::string * insName, std::string * mPlex, uMDInt tileNum, uMDInt laneNum, uMDInt X_Cord, uMDInt Y_Cord)
//HO ;
{
    //-----
    // Add a new read identifier for the dualnode
    //
    uMDInt mPlexToken = (uMDInt)addMultString(*mPlex);
    uMDInt insToken = (uMDInt)addCommonString(*insName);

    // make the new ID
    ReadIdentifierId RiID = newReadIdentifierId();

    // set all the values
    setCommon(insToken, RiID);
    setMultiplex(mPlexToken, RiID);
    setTile(tileNum, RiID);
    setLane(laneNum, RiID);
    setXCoord(X_Cord, RiID);
    setYCoord(Y_Cord, RiID);
    setDualnode(DID, RiID);

    // return the new ID
    return RiID;
}
//HO 

// getting out the readidentifier string...
/*HV*/ std::string
ReadIdentifierMemWrapper::
/*HV*/ getReadIdString(ReadIdentifierId RiID)
//HO ;
{
    std::string mult_str = getMultString(getMultiplex(RiID));
    std::stringstream ss;
    if(mult_str != "(undef)")
        ss << getCommonString(getCommon(RiID)) << ":" << getLane(RiID) << ":" << getTile(RiID) << ":" << getXCoord(RiID) << ":" << getYCoord(RiID) << ":#" << getMultString(getMultiplex(RiID)) << "/";
    else
        ss << getCommonString(getCommon(RiID)) << ":" << getLane(RiID) << ":" << getTile(RiID) << ":" << getXCoord(RiID) << ":" << getYCoord(RiID) << "/";
    return ss.str();
}
//HO 

// MULTIPLEX CALLS
//HO inline uMDInt addMultString(std::string multStr) { return (uMDInt)(mMults->addCommonString(multStr)); }
//HO 

//HO inline std::string getMultString(uMDInt ref) { return mMults->getCommonString(ref); }
//HO 

//HO inline bool isMultStringPresent(std::string multStr) { return mMults->isCommonStringPresent(multStr); }
//HO 

// COMMONSTRING CALLS
//HO inline uMDInt addCommonString(std::string commonStr) { return (uMDInt)(mCommonIds->addCommonString(commonStr)); }
//HO 

//HO inline std::string getCommonString(uMDInt ref) { return mCommonIds->getCommonString(ref); }
//HO 

//HO inline bool isCommonStringPresent(std::string commonStr) { return mCommonIds->isCommonStringPresent(commonStr); }
//HO 
