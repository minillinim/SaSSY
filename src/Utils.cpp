//            File: Utils.cpp
// Original Author: Dominic Eales
// --------------------------------------------------------------------
//
// OVERVIEW:
// Filesystem utilities (adapted for linux)
// 
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

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <ostream>
#include <iterator>
#include <algorithm>
#include <iomanip>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <math.h>
#include <limits.h>

/******************************************************
 ** File operation functions
 ******************************************************/

void RecursiveMkdir(std::string dir) 
{
    std::string tmp;
    size_t pos = 0;
    while ( std::string::npos != (pos = dir.find('/',pos+1)) ) {
        tmp = dir.substr(0,pos);
        mkdir(tmp.c_str(), S_IRWXU);
    }
    mkdir(dir.c_str(), S_IRWXU);
}

unsigned int LinesInFile(std::string filename)
{
    std::ifstream in(filename.c_str());
    unsigned int total = 
            std::count(std::istreambuf_iterator<char>(in),
                       std::istreambuf_iterator<char>(),'\n');
    return total;
}

bool IsDirectory(std::string filename, bool& isDir)
{
    struct stat fst;
    isDir = false;
    
    // get file/dir info
    if ( 0 != stat(filename.c_str(), &fst) ) {
        return false;
    }
    // check if it's a file
    if (S_ISDIR(fst.st_mode)) {
        isDir = true;
    }
    return true;
}

bool FileExists(std::string strFilename) 
{
    struct stat stFileInfo;
    bool blnReturn = false;
    int intStat;

    // Attempt to get the file attributes
    intStat = stat(strFilename.c_str(),&stFileInfo);
    if(intStat == 0) {
        if ( !(S_ISDIR(stFileInfo.st_mode)) ) {
            // We were able to get the file attributes
            // so the file obviously exists.
            blnReturn = true;
        }
    }

    return(blnReturn);
}

/******************************************************
 ** String functions
 ******************************************************/

std::string str_join(const std::vector<std::string> & vec,const std::string & sep)
{
    if (vec.size() == 0) {
        return "";
    }

    // precalculate size
    std::string::size_type size = sep.length() * vec.size();
    for (unsigned int i=0; i<vec.size(); i++)
    {
        size += vec[i].size();
    }

    // reserve space for output
    std::string tmp;
    tmp.reserve(size);

    // do concat
    tmp = vec[0];
    for(unsigned int i=1; i<vec.size(); i++)
    {
        tmp = tmp + sep + vec[i];
    }
    return tmp;
}

void str_trim( std::string& s )
{
    static const char whitespace[] = " \n\t\v\r\f";
    s.erase( 0, s.find_first_not_of(whitespace) );
    s.erase( s.find_last_not_of(whitespace) + 1U );
}

/******************************************************
 ** Path and file name interacting functions
 ******************************************************/

std::string ExtractDirectory( const std::string& path )
{
    return path.substr( 0, path.find_last_of( '/' ) +1 );
}

std::string ExtractFilename( const std::string& path )
{
    return path.substr( path.find_last_of( '/' ) +1 );
}

std::string ChangeExtension( const std::string& path, const std::string& ext )
{
    std::string filename = ExtractFilename( path );
    return ExtractDirectory( path ) +filename.substr( 0, filename.find_last_of( '.' ) ) +ext;
}

