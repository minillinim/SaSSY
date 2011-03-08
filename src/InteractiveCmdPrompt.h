//            File: InteractiveCommmandPrompt.h
// Original Author: Dominic Eales
// --------------------------------------------------------------------
//
// OVERVIEW:
// This file defines and interactive command prompt which could be used
// in many projects
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

#ifdef USE_INTERACTIVE_COMMAND_PROMPT

#ifndef _INTERACTIVECMDPROMPT_H_
#define _INTERACTIVECMDPROMPT_H_

#include <string>
#include <vector>
#include <map>
#include <ostream>
#include <limits.h>

#define ICP_STOP INT_MIN

class InteractiveCmdPrompt;

class ICP_CmdDefinition {
public:
    ICP_CmdDefinition() {};
    virtual ~ICP_CmdDefinition() {};

    virtual int execute( 
                    std::vector<std::string>& parameters, 
                    std::ostream& out,
                    void * userdata ) = 0;
    virtual const char * name( void ) = 0;
    virtual const char * description( void ) = 0;
    virtual const char * syntax( void ) = 0;
};

#define CmdMap std::map<std::string, ICP_CmdDefinition*>

class InteractiveCmdPrompt {

public:
    InteractiveCmdPrompt();
    ~InteractiveCmdPrompt();

    void SetDisplay_ReturnValues(bool value)  {_displayReturnValues = value;};
    void SetPrompt(const char * prompt) { _strPrompt.assign( prompt ); }
    void SetCmdUserData(void * userdata) { _userdata = userdata; }

    // command functions
    void AddCommand(ICP_CmdDefinition * cmdToAdd);
    void SetDefaultCommand(ICP_CmdDefinition * cmdToAdd);
    void SetHelpCommand( const char * helpCmdName, const char * helpCmdDescription );

    // Help related
    void DisplayHelp(std::ostream& outStream, const char * cmdname);

    //
    bool Run(std::ostream& outStream);
    int  ExecuteCommandFromParams(std::vector<std::string>& parameters, std::ostream& outStream);
    int  ExecuteCommandFromString(std::string cmdline, std::ostream& outStream);

private:
    std::string         _strPrompt;
    CmdMap              _cmds;
    bool                _displayReturnValues;
    void       *        _userdata;
    std::string         _helpCmdName;
    std::string         _helpCmdDescription;
    ICP_CmdDefinition * _defaultCmd;

    int ConvertStringToArgcArgv( 
            const char *inputString, 
            std::vector<std::string> * outStrings );

    ICP_CmdDefinition * GetCmdDef( std::string cmdName );

};

#endif //_INTERACTIVECMDPROMPT_H_

#endif //#ifdef USE_INTERACTIVE_COMMAND_PROMPT
