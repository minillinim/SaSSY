//            File: Zobrist.h
// Original Author: Dominic Eales
// --------------------------------------------------------------------
//
// OVERVIEW:
// Doms playground
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

#ifndef _STLEXT_H_
#define _STLEXT_H_

#include <fstream>
#include <ostream>
#include <string>

/////////////////////////////////////////////////////
// VECTOR FOR EACH
// Example use: 
//
// std::vector<std::string> myVector;
// V_FOR_EACH( myVector, std::string, vecIter ) {
//     std::cout << (*vecIter) << std::endl;
// }

#define V_FOR_EACH(vECTOR_,tYPE_,iTERATOR_) \
    for(\
        std::vector<tYPE_>::iterator iTERATOR_ = vECTOR_.begin(), e = vECTOR_.end(); \
        iTERATOR_ != e; \
        ++iTERATOR_)


/*///////////////////////////////////////////////////
    MAP FOR EACH
    Example use: 

        std::map<int,std::string> myMap;
        M_FOR_EACH( myMap, int, std::string, mapIter, key, value ) {
            std::cout << key << "=" << value << std::endl;
        }
        // key and value are still valid until M_END is added
        M_END;

///////////////////////////////////////////////////*/

#define M_FOR_EACH(mAP_,tYPE1_,tYPE2_,iTERATOR_,kEYvAR_,vALUEvAR_) \
{\
    if (mAP_.size() != 0) {\
        std::map<tYPE1_,tYPE2_>::iterator iTERATOR_ = mAP_.begin(), eND_ = mAP_.end();\
        tYPE1_ kEYvAR_   = iTERATOR_->first; \
        tYPE2_ vALUEvAR_ = iTERATOR_->second; \
        for (; \
            iTERATOR_ != eND_; \
            ++iTERATOR_,\
            kEYvAR_   = (iTERATOR_!=eND_? iTERATOR_->first : kEYvAR_),\
            vALUEvAR_ = (iTERATOR_!=eND_? iTERATOR_->second : vALUEvAR_) \
            )

#define M_END \
    }\
}

/////////////////////////////////////////////////////
// MAP KEY EXISTS

#define M_KEY_EXISTS(mAP_,kEY_) ((mAP_).find(kEY_) != (mAP_).end())


/////////////////////////////////////////////////////
// redirector -- for stream redirection

// Stream redirecter.
class redirecter
{
    public:
    // Constructing an instance of this class causes
    // anything written to the source stream to be redirected
    // to the destination stream.
    redirecter(const char *outputFile, std::ostream & src) : src(src), srcbuf(src.rdbuf()), dst(outputFile)
    {
        src.rdbuf(dst.rdbuf());
    }
    
    // The destructor restores the original source stream buffer
    ~redirecter()
    {
        src.rdbuf(srcbuf);
    }

    private:
    std::ostream & src;
    std::streambuf * const srcbuf;
    std::ofstream  dst;
};


/////////////////////////////////////////////////////
// Redirecting std::cout

// Just to make code easier to read
#define STD_COUT_VAR __pStdOutRedirect

// Housekeeping
extern std::ostream * STD_COUT_VAR;
// Define the object only once globally in main.cpp
#ifdef _MAIN_CPP_
std::ostream * STD_COUT_VAR = &std::cout;
#endif

//---------------------------------------------//
// *** USE THIS MACRO INSTEAD OF std::cout *** //
//---------------------------------------------//
#define STD_COUT (*STD_COUT_VAR)
//---------------------------------------------//

//-----------------------------------------------//
// *** USE THIS FUNCTION TO CHANGE std::cout *** //
//-----------------------------------------------//
inline std::ostream * SetStdOutStream( std::ostream * pNewStdOut ) 
{ 
    std::ostream * pOldStdOut = STD_COUT_VAR; 
    STD_COUT_VAR = pNewStdOut; 
    return pOldStdOut; 
}
//-----------------------------------------------//


#endif //_STLEXT_H_
