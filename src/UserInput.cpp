//            File: UserInput.h
// Original Author: Dominic Eales
// --------------------------------------------------------------------
//
// OVERVIEW:
// Implementation of helper class for getting/parsing user input
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

#include "UserInput.h"
#include "Utils.h"

/******************************************************
 ** User Input handling
 ******************************************************/

std::string GetFilename(std::istream& in, std::ostream& out, const char *prompt, const char *errorstr)
{
    std::string output = "";
    while (true) {
        std::string input;
        out << prompt;
        getline(in, input);
        std::stringstream myStream(input);
        if (myStream >> output) {
            if ( true == FileExists(output) ) {
                break;
            }
        }
        out << errorstr << std::endl;
    }
    return output;
}

int GetMenuChoice(
                  std::istream& in, 
                  std::ostream& out, 
                  const char *title, 
                  std::vector<std::string> choices,
                  const char *prompt, 
                  const char *errorstr )
{
    int indexout = -1;

    // create menu
    std::stringstream myStream;
    int counter = 1;
    myStream << title << std::endl;
    std::vector<std::string>::iterator it;
    for (it=choices.begin(); it!=choices.end(); it++, counter++) {
        myStream << counter << ". " << (*it) << std::endl;
    }
    myStream << prompt;

    indexout = GetRangedValueFromUser<int>(in,out,myStream.str().c_str(),errorstr,1,(int)choices.size()) - 1;

    return indexout;
}

bool GetYesNoFromUser( std::istream& in, std::ostream& out, const char *prompt, const char *errorstr ) 
{
    bool retVal = false;
    std::string yesNoStr;
    while (true) {
        std::string input;
        out << prompt;
        getline(in, input);
        std::stringstream myStream(input);
        if (myStream >> yesNoStr)
            break;
        std::transform(yesNoStr.begin(), yesNoStr.end(), yesNoStr.begin(), ::tolower);
        if ( (yesNoStr == "y") || (yesNoStr == "yes") ) {
            retVal = true;
            break;
        }
        else if ( (yesNoStr == "n") || (yesNoStr == "no") ) {
            retVal = false;
            break;
        }
        out << errorstr << std::endl;
    }
    return retVal;
}
