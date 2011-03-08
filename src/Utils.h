//            File: Utils.h
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


#ifndef _UTILS_H_
#define _UTILS_H_

#include <vector>
#include <string>
#include <sstream>

/******************************************************
 ** File operation functions
 ******************************************************/

void RecursiveMkdir(std::string dir) ;
unsigned int LinesInFile(std::string filename);
bool IsDirectory(std::string filename, bool& isDir);
bool FileExists(std::string strFilename) ;

/******************************************************
 ** String functions
 ******************************************************/

std::string str_join(const std::vector<std::string> & vec,const std::string & sep);

void str_trim( std::string& s );

template <class T>
bool from_string(T& t, const std::string& s, std::ios_base& (*f)(std::ios_base&))
{
    std::istringstream iss(s);
    return !(iss >> f >> t).fail();
}

template <class T>
std::string toStr(T t)
{
    std::ostringstream iss;
    iss << t;
    return iss.str();
}

/******************************************************
 ** Path and file name interacting functions
 ******************************************************/

std::string ExtractDirectory( const std::string& path );
std::string ExtractFilename( const std::string& path );
std::string ChangeExtension( const std::string& path, const std::string& ext );

#endif //#ifndef _UTILS_H_
