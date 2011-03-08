//            File: CaptureSysCmdOut.h
// Original Author: Dominic Eales
// --------------------------------------------------------------------
//
// OVERVIEW:
// Class to capture system command output
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

#include <stdio.h>

class CaptureSysCmdOut
{
    public:
        CaptureSysCmdOut() {}

        bool RunSysCmd( const std::string &cmd, std::vector<std::string> * pOutput = NULL )
        {
            // Open a new pipe
            FILE *pfd = popen( cmd.c_str(), "r" );
            // check for no pipe
            if ( pfd == 0 )
            {
                return false;
            }
            // get all the data from it
            while ( !feof(pfd) )
            {
                char buf[ 1024 ] = {0};

                if ( fgets(buf, sizeof(buf), pfd) > 0 )
                {
                    m_results.push_back( std::string(buf) );
                }
            }
            // close it
            pclose( pfd );

            // copy output if required
            if ( pOutput != NULL ) {
                (*pOutput) = m_results;
            }

            // return success!
            return true;
        }

        const std::vector<std::string>& GetOutput() const
        {
            return m_results;
        }

    private:
        std::vector<std::string> m_results;
};
