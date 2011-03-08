//            File: fdfile.cpp
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

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <ostream>
#include <algorithm>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

#include "default_parameters.h"
#include "FormattedFileReader.h"
#include "fdfile.h"
#include "ExeDirectory.h"



// This is a call back we use when trying to guess the format
// of a sequence file.
// The logic is: if this function is called then the regexp matched
// therefore the file format is good. If we get to the end of the
// file and nothing ever matched, then we know it's bad.
bool loadseq_guessformat_callback( 
    std::map<std::string,std::string> recordInfo, 
    void * context )
{
    return false; // return false means: stop parsing file
}


bool loadseq_addnode_callback( 
    std::map<std::string,std::string> recordInfo, 
    void * context )
{
    return true;
}

bool CheckFileAgainstFileDef( 
        std::string filename,
        std::string fdfile )
{
#if 0
    // check that the chosen format suits the input file
    FormattedFileReader ffr;
    bool retVal = ffr.ReadFile(
                        fdfile,
                        filename,
                        loadseq_guessformat_callback,
                        NULL );
    return retVal;
#endif
    return false;
}

bool GetFormatDefinitionFile(
        std::map<std::string, std::string> formatsAvail,
        std::ostream&     out, 
        std::string       dataFilename,
        std::string&      fdFilename,
        std::string*      pSuggestedFormat )
{
    std::string chosenFormat;

    // let's sort out the file format
    if ( pSuggestedFormat != NULL ) {
        //check the format exists
        std::map<std::string, std::string>::iterator formatIt = formatsAvail.find(*pSuggestedFormat);
        if ( formatsAvail.end() == formatIt ) {
            out << "error: format `" << *pSuggestedFormat << "' is not available. Try command `listformats' to see formats available." << std::endl;
            return false;
        }

        // check that the chosen format suits the input file
        if (false == CheckFileAgainstFileDef(dataFilename, formatIt->second) ) {
            out << "error: format `" << *pSuggestedFormat << "' does not correspond to file `"<< dataFilename <<"'" << std::endl;
            return false;
        }

        chosenFormat = *pSuggestedFormat;
    }
    else {
        // we need to guess the format... so let's try each one
        std::map<std::string, std::string>::iterator mapIt;
        for (mapIt = formatsAvail.begin(); mapIt != formatsAvail.end(); mapIt++) {

            #define LOOP_FEEDBACK 0
            #if LOOP_FEEDBACK
            out << "Attempting to read file as `" << mapIt->first << "'... ";
            #endif

            if (false == CheckFileAgainstFileDef(dataFilename, mapIt->second)) {
                #if LOOP_FEEDBACK
                out << "failed" << std::endl;
                #endif
                continue;
            }

            #if LOOP_FEEDBACK
            out << "success!" << std::endl;
            #endif
            break;
        }

        if (mapIt == formatsAvail.end() ) {
            out << "error: no format to correspond to file `" << dataFilename << "'" << std::endl;
            return false;
        }

        // remember the matched format
        chosenFormat = mapIt->first;
    }

    // set return value
    fdFilename = formatsAvail[chosenFormat];

    return true;
}


FormatDefFiles::FormatDefFiles() 
{
    std::string fdDir = ExecutableDirectory;
    fdDir += SAS_DEF_FDFILE_DIR;

    struct dirent *dp;
    struct stat    fst;

    // get all the "fd.*" files
    DIR *dir = opendir(fdDir.c_str());
    if (dir == NULL ) {
        return;
    }

    // loop through directory entries
    while ((dp=readdir(dir)) != NULL) {

        // check if starts with "fd."
        if (  ( 0 == strncmp(  dp->d_name, "fd.", 3) )
            &&( 0 == strncmp( &dp->d_name[strlen(dp->d_name)-6], ".sassy", 6)) ) {

            std::string fullpath = fdDir + "/";
            fullpath.append(dp->d_name);

            // get file/dir info
            if ( 0 != stat(fullpath.c_str(), &fst) ) {
                continue;
            }
            // check if it's a file
            if (S_ISDIR(fst.st_mode)) {
                continue;
            }

            // check the format is good
            FormattedFileReader rdr;
            std::map<std::string, std::string> formatInfo;
            if ( false == rdr.CheckFormatDefinitionFile(fullpath,formatInfo) ) {
                // no good -- skip
                continue;
            }

            // finally add it to our list
            formatInfo["filepath"] = fullpath;
            _fdFormatVars[formatInfo["name"]] = formatInfo;
        }
    }
    closedir(dir);
}

