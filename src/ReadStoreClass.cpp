/******************************************************************************
**
** File: ReadStoreClass.cpp
**
*******************************************************************************
**
** This is the actual class that the assembler interacts with. It includes many
** inline passthrough functions and serves to hide away all the nasty template
** nonsense which is necessary tro make the assembler run fast.
**
** There is a very specific format you must follow when you edit this file
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
#include "ReadStoreMemWrapper.h"
#include "intdef.h"
#include "IdTypeDefs.h"
#include "ReadStoreClass.h"
/*INC*/ #include "LoggerSimp.h"
/*INC*/ 
using namespace std;

/******************************************************************************
** INCLUDES #DEFINES AND PRIVATE VARS
******************************************************************************/
//PV int mReadLength;
//PV 
//PV std::string mSpaceString;
//PV 

/******************************************************************************
** INTIALISATION AND SAVING
******************************************************************************/
/*HV*/ void
ReadStoreClass::
/*HV*/ extendedDestructor(void)
//HO ;
{
    //-----
    // Add anything you would like to happen when the destructor is called
    //
}
//HO 

/*HV*/ bool
ReadStoreClass::
/*HV*/ extendedInitialise(uMDInt originalNumberOfBlocks )
//HO ;
{
    //-----
    // Add anything you would like to happen on a regular intialisation call
    //
    mReadLength = mData->getReadLength();
    mSpaceString = mData->getSpaceString();
    return true;
}
//HO 

/*HV*/ bool
ReadStoreClass::
/*HV*/ extendedInitialise(std::string fileName)
//HO ;
{
    //-----
    // Add anything you would like to happen when reloading a saved instance
    //
    mReadLength = mData->getReadLength();
    mSpaceString = mData->getSpaceString();
    return true;
}
//HO 

/*HV*/ bool
ReadStoreClass::
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
** STRING MANIPULATION AND GET/SET
******************************************************************************/
    /*
    ** Inline wrapper for getting the NULL Id
    */
//HO inline bool isValidReadStoreId(ReadStoreId RID) { return (RID != mData->getNullReadStoreId()); }
//HO 

    /*
    ** Inline wrapper for adding a sequence to the ReadStore
    */
//HO inline ReadStoreId addSequence(bool * isOriginal, bool * reversed, std::string * sequence) { return mData->addSequence(isOriginal, reversed, sequence); }
//HO 

    /*
    ** Inline wrapper for adding a sequence to the ReadStore
    */
//HO inline ReadStoreId addSequence(bool * reversed, std::string * sequence) { return mData->addSequence(reversed, sequence); }
//HO 

    /*
    ** Inline wrapper for getting a string from the ReadStore
    */
//HO inline std::string getSubSequence(unsigned int Kstart, unsigned int Klength, bool Korient, ReadStoreId RID) { return mData->getSubSequence(Kstart, Klength, Korient, RID); }
//HO 
//HO inline std::string getSequence(ReadStoreId RID) { PARANOID_ASSERT_L3(isValidReadStoreId(RID)); return mData->getSequence(RID); }
//HO 

    /*
    ** Inline wrapper for getting Id from sequence
    */
//HO inline bool getReadStoreId(ReadStoreId * RID, std::string * sequence) { return mData->getReadStoreId(RID, sequence); }
//HO 

//HO inline ReadStoreId getReadStoreId(std::string * sequence) { return mData->getReadStoreId(sequence); }
//HO 

    /*
    ** Inline wrapper checking if a sequence is stored
    */
//HO inline bool isSequenceStored(std::string * sequence) { return mData->isSequenceStored(sequence); }
//HO 
    
    /*
    ** Inline wrapper for checking if a sequence is stored at a particular location
    */
//HO inline bool isSequenceAt(std::string * sequence, ReadStoreId RID) { return mData->isSequenceAt(sequence, RID); }
//HO 
    
    /*
    ** Inline wrapper for orientating a sequence
    */
//HO inline void orientateSequence(bool * reversed, std::string * sequence) { mData->orientateSequence(reversed, sequence); }
//HO 
        
    /*
    ** Inline wrapper for reverse complimenting a sequence
    */
//HO inline void revCmpSequence(std::string * sequence) { mData->revCmpSequence(sequence); }
//HO 

    /*
    ** get the readlength
    */
//HO inline int getReadLength(void) { return mReadLength; }
//HO 

    /*
    ** get the number of sequences stored
    */
//HO inline int getNumSequences(void) { return mData->getNumSequences(); }
//HO 

    /*
    ** get the space string
    */
//HO inline std::string getSpaceString(void) { return mSpaceString; }
//HO 

    /*
    ** check overlaps during edge making
    */
//HO inline bool isOverlap(ReadStoreId bSeq, int bTrim, ReadStoreId qSeq, int qTrim, int len, bool revComp) { return mData->isOverlapped(bSeq, bTrim, qSeq, qTrim, len, revComp); }
//HO 
