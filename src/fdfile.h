//            File: fdfile.h
// Original Author: Dominic Eales
// --------------------------------------------------------------------
//
// OVERVIEW:
// Class for handling file formats
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

#ifndef _FDFILE_H_
#define _FDFILE_H_

#include <iostream>
#include <string>
#include <map>


class FormatDefFiles
{
public:
    static std::map<std::string,std::map<std::string,std::string> > GetList()
    {
        static FormatDefFiles singleton;
        return singleton._fdFormatVars;
    }

// Private variables
private:
    std::map<std::string,std::map<std::string,std::string> > _fdFormatVars;

// Other non-static member functions
private:
    FormatDefFiles();
    FormatDefFiles(const FormatDefFiles&);              // Prevent copy-construction
    FormatDefFiles& operator=(const FormatDefFiles&);   // Prevent assignment
};

bool CheckFileAgainstFileDef( std::string filename, std::string fdfile );

bool GetFormatDefinitionFile(
        std::map<std::string, std::string> formatsAvail,
        std::ostream&     out, 
        std::string       dataFilename,
        std::string&      fdFilename,
        std::string*      pSuggestedFormat );

#endif //_FDFILE_H_
