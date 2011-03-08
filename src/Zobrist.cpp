//            File: Zobrist.cpp
// Original Author: Michael Imelfort
// --------------------------------------------------------------------
//
// OVERVIEW:
// Implementation of Zobrist methods.
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

// SYSTEM INCLUDES
//
#include <iostream>
#include <fstream>      // saving and loading
#include <time.h>       // for seeding rand
#include <map>
#include <cstdlib>      // for rand

// LOCAL INCLUDES
//
#include "Zobrist.h"
#include "paranoid.h"
#include "char_encodings.h"
#include "intdef.h"

  // Constructor/Destuctor
    Zobrist::Zobrist(std::string fileName)
    {
        // this will be used only when we want to load
        // data from file
        mSeed = NULL;
        loadZobrist(fileName);
    }

    Zobrist::Zobrist(int stringLength)
    {
        mStringLength = stringLength;
        mSeed = new idInt[mStringLength * 4];
        PARANOID_ASSERT_L4(mSeed != NULL);
        makeMap();
    }

    Zobrist::~Zobrist(void)
    {
        if(mSeed != NULL)
            delete[] mSeed;
        mSeed = NULL;
    }

  // Get methods

    idInt Zobrist::hashString(std::string * query)
    {
        //-----
        // given a string return an address but we don't check to
        // see if the string needs to be reversed, IT MUST BE
        // also we do no checking if the chars are correct
        //
        
        idInt xorEr = 0;
        const char * q_chars = query->c_str();
        PARANOID_ASSERT_PRINT_L4(((int)(*query).length() == mStringLength), *query << " : " << ((int)(*query).length()) << " == " << mStringLength);
        
        xorEr = mLookupMap[0][*q_chars];

        int i = 1;
        while(i < mStringLength)
        {
            xorEr = xorEr ^ mLookupMap[i][*q_chars];
            q_chars++;
            i++;
        }
        return xorEr;
    }

  // operations
    
    void Zobrist::makeMap()
    {
        //-----
        // initialise the map, make sure none are 0 or all 1s
        //
#if (SIZE_OF_IDTYPE == 64)
        idInt all_1s = 0xFFFFFFFFFFFFFFFF;
        idInt low_word = 0;
#elif (SIZE_OF_IDTYPE == 32)
        idInt all_1s = 0xFFFFFFFF;
#else
        idInt all_1s = 0; std::cout << "Zobrist::makeMap  ***_ERROR _*** Unexpected integer size: " << SIZE_OF_IDTYPE << std::endl;
#endif
        srand ( time(NULL) );
        int seed_counter = 0;
        for(int i = 0; i < mStringLength; i ++)
        {
            mSeed[seed_counter] = rand();
            while(mSeed[seed_counter] == 0 || mSeed[seed_counter] == all_1s)
            {
                mSeed[seed_counter] = rand();
            }
#if (SIZE_OF_IDTYPE == 64)
            // we need to shift this 32 bit random down and append another 32 random bits to the end...
            mSeed[seed_counter] =  mSeed[seed_counter] << 32;
            low_word = rand();
            while(low_word == 0 || low_word == all_1s)
            {
                low_word = rand();
            }
            mSeed[seed_counter] |= low_word;
#endif
            mLookupMap[i][SAS_CE_D_0] =  mSeed[seed_counter];
            seed_counter++;

            mSeed[seed_counter] = rand();
            while(mSeed[seed_counter] == 0 || mSeed[seed_counter] == all_1s)
            {
                mSeed[seed_counter] = rand();
            }
#if (SIZE_OF_IDTYPE == 64)
            // we need to shift this 32 bit random down and append another 32 random bits to the end...
            mSeed[seed_counter] =  mSeed[seed_counter] << 32;
            low_word = rand();
            while(low_word == 0 || low_word == all_1s)
            {
                low_word = rand();
            }
            mSeed[seed_counter] |= low_word;
#endif
            mLookupMap[i][SAS_CE_D_1] = mSeed[seed_counter];
            seed_counter++;

            mSeed[seed_counter] = rand();
            while(mSeed[seed_counter] == 0 || mSeed[seed_counter] == all_1s)
            {
                mSeed[seed_counter] = rand();
            }
#if (SIZE_OF_IDTYPE == 64)
            // we need to shift this 32 bit random down and append another 32 random bits to the end...
            mSeed[seed_counter] =  mSeed[seed_counter] << 32;
            low_word = rand();
            while(low_word == 0 || low_word == all_1s)
            {
                low_word = rand();
            }
            mSeed[seed_counter] |= low_word;
#endif
            mLookupMap[i][SAS_CE_D_2] = mSeed[seed_counter];
            seed_counter++;

            mSeed[seed_counter] = rand();
            while(mSeed[seed_counter] == 0 || mSeed[seed_counter] == all_1s)
            {
                mSeed[seed_counter] = rand();
            }
#if (SIZE_OF_IDTYPE == 64)
            // we need to shift this 32 bit random down and append another 32 random bits to the end...
            mSeed[seed_counter] =  mSeed[seed_counter] << 32;
            low_word = rand();
            while(low_word == 0 || low_word == all_1s)
            {
                low_word = rand();
            }
            mSeed[seed_counter] |= low_word;
#endif
            mLookupMap[i][SAS_CE_D_3] = mSeed[seed_counter];
            seed_counter++;
        }
    }

  // file IO

    bool Zobrist::saveZobrist(std::string fileName)
    {    
        //-----
        // save the zobrist to file
        // 4 ints to a char -- keep this in mind!
        //
        std::ofstream myFile(fileName.c_str(), std::ofstream::binary);
        // write the readLength
        myFile.write(reinterpret_cast<char *>(&mStringLength), sizeof(int));
        // next write the map
        myFile.write(reinterpret_cast<char *>(mSeed), mStringLength * 4 * sizeof(idInt));
        // close the file
        myFile.close();
        return true;
    }

    bool Zobrist::loadZobrist(std::string fileName)
    {
        //-----
        // load a zobrist from file
        //
        // open file containing the seed
        std::ifstream myFile(fileName.c_str(), std::ifstream::binary);

        // load the member vars
        myFile.read(reinterpret_cast<char *>(&mStringLength), sizeof(int));

        // load the seed container
        if(mSeed != NULL)
            delete [] mSeed;
        mSeed = new idInt[mStringLength * 4];
        PARANOID_ASSERT_L4(mSeed != NULL);
        
        myFile.read(reinterpret_cast<char *>(mSeed), mStringLength * 4 * sizeof(idInt));

        // close the file
        myFile.close();
        
        // re-create the lookup map
        int seed_counter = 0;
        for(int i = 0; i < mStringLength; i ++)
        {
            mLookupMap[i][SAS_CE_D_0] =  mSeed[seed_counter];
            seed_counter++;
            mLookupMap[i][SAS_CE_D_1] = mSeed[seed_counter];
            seed_counter++;
            mLookupMap[i][SAS_CE_D_2] = mSeed[seed_counter];
            seed_counter++;
            mLookupMap[i][SAS_CE_D_3] = mSeed[seed_counter];
            seed_counter++;
        } 
        return true;
    }
