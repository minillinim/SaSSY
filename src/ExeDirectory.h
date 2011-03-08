//            File: ExeDirectory.h
// Original Author: Dominic Eales
// --------------------------------------------------------------------
//
// OVERVIEW:
// Class to manage the directory the executable is in.
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

#include <vector>
#include <string>
#include <unistd.h> 
#include "CaptureSysCmdOut.h"
#include "Utils.h"

class ExeDirectory
{
public:
    ExeDirectory() {
        _initialised = false;

        // fall back as PWD
        char path[PATH_MAX+1];
        if(getcwd(path,PATH_MAX)) {}
        _exeDir = path;
    }

    // get the exe directory
    // main() call this with argv[0] as parameter
    // other functions call with NULL
    inline const std::string& Get(const char * argv0 = NULL) {
        if ( argv0 != NULL ) {
            Initialise(argv0);
        }
        return _exeDir;
    }
private:

    inline bool Initialise( const char * argv0 ) {
        if ( _initialised == false ) {
            // some variables
            CaptureSysCmdOut whichCmd;
            std::vector<std::string> out;

            // assuming the "which" command exists
            whichCmd.RunSysCmd( std::string("which \"") + argv0 + "\"", &out);
            if (out.size() >= 1) {
                _initialised = true;
                str_trim(out[0]);
                out[0] = out[0].substr( 0, out[0].find_last_of( '/' ) +1 );
                _exeDir = out[0];
            }
        }
        return _initialised;
    }

    // variables
    bool _initialised;
    std::string _exeDir;
};

// extern for everyone
extern ExeDirectory __exeDir;

#ifdef _MAIN_CPP_
ExeDirectory __exeDir;
#define InitExecutableDirectory(aRGV0) __exeDir.Get(aRGV0)
#endif //_MAIN_CPP_

// -----------------------------------------------//
// USE THIS MACRO TO GET THE EXECUTABLE DIRECTORY
// -----------------------------------------------//
#define ExecutableDirectory __exeDir.Get(NULL)
// -----------------------------------------------//

