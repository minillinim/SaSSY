//            File: AssemblerNew.h
// Original Author: Dominic Eales
// -----------------------------------------------------
//
// OVERVIEW:
// This file contains the class definition and functions for assembling
// short read data
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

#ifndef AssemblerNew_h
 #define AssemblerNew_h

// system includes
#include <map>
#include <vector>
#include <string>

// local includes
#include "StatsManager.h"
#include "Triple.h"
#include "LoggerSimp.h"
#include "IdTypeDefs.h"
#include "GenericNodeClass.h"
#include "ContigClass.h"
#include "ContextClass.h"
#include "stlext.h"

#include "Assembler.h"

#ifdef USE_INTERACTIVE_COMMAND_PROMPT
#include "SassyShell.h"
#endif

class AssemblerNew : public Assembler
#ifdef USE_INTERACTIVE_COMMAND_PROMPT
, public SassyShellObjectWithCommands
#endif
{
private:
    std::string    mStdOutFile;
    redirecter   * mStdOutRedirect;
    bool mHaveRunSetParams;
    void setRunParameters(void);
    
public:

  // Constructor/Destructor
    AssemblerNew();
    ~AssemblerNew();

#ifdef USE_INTERACTIVE_COMMAND_PROMPT
        // runs commands from the command processor
    virtual const char * gettypename(void) const {return "Assembler";}
    virtual int runcommand( std::vector<std::string>& parameters, std::ostream& out );
    virtual int constructcommand( std::vector<std::string>& parameters, std::ostream& out );
#endif

//********************************************************
  // COMMANDS
//********************************************************
    int Command_SetOutputFile( std::vector<std::string>& parameters, std::ostream& out );
    int Command_AddFileSet  ( std::vector<std::string>& parameters, std::ostream& out );
    int Command_ListFileSets( std::vector<std::string>& parameters, std::ostream& out );
    int Command_Stage1( std::vector<std::string>& parameters, std::ostream& out );
    int Command_Stage2( std::vector<std::string>& parameters, std::ostream& out );
    int Command_Stage3( std::vector<std::string>& parameters, std::ostream& out );
    int Command_Stage4( std::vector<std::string>& parameters, std::ostream& out );
    int Command_Init( std::vector<std::string>& parameters, std::ostream& out );

};

#endif //AssemblerNew_h

