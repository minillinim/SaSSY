//            File: UserInput.h
// Original Author: Dominic Eales
// --------------------------------------------------------------------
//
// OVERVIEW:
// Helper class for getting/parsing user input
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

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <ostream>
#include <algorithm>
#include <iomanip>


/******************************************************
 ** User Input handling
 ******************************************************/
template <class T>
T GetFromUser( std::istream& in, std::ostream& out, const char *prompt, const char *errorstr )
{
    T retVal = 0;
    while (true) {
        std::string input;
        out << prompt;
        getline(in, input);
        std::stringstream myStream(input);
        if (myStream >> retVal)
            break;
        out << errorstr << std::endl;
    }
    return retVal;
}


template <class T>
        T GetRangedValueFromUser( std::istream& in, std::ostream& out, const char *prompt, const char *errorstr, T minVal, T maxVal ) 
{
    T retVal = 0;
    while ( ((retVal=GetFromUser<T>(in,out,prompt,errorstr))<minVal) || (retVal>maxVal) ) {
        out << errorstr << std::endl;
    }
    return retVal;
}

template <class T>
        T GetNonZeroValueFromUser( std::istream& in, std::ostream& out, const char *prompt, const char *errorstr ) 
{
    T retVal = 0;
    while ((retVal = GetFromUser<T>(in,out,prompt,errorstr)) == 0) {
        out << errorstr << std::endl;
    }
    return retVal;
}

std::string GetFilename(std::istream& in, std::ostream& out, const char *prompt, const char *errorstr);

int GetMenuChoice(
                  std::istream& in, 
                  std::ostream& out, 
                  const char *title, 
                  std::vector<std::string> choices,
                  const char *prompt, 
                  const char *errorstr );

bool GetYesNoFromUser( std::istream& in, std::ostream& out, const char *prompt, const char *errorstr ) ;
