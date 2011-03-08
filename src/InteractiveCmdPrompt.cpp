//            File: InteractiveCommmandPrompt.cpp
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

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "InteractiveCmdPrompt.h"
#include "Utils.h"

InteractiveCmdPrompt::InteractiveCmdPrompt()
{
    _userdata = NULL;
    _displayReturnValues = false;
    _strPrompt = "> ";
    _helpCmdName = "help";
    _helpCmdDescription = "Displays the help information for available commands.";
    _defaultCmd = NULL;
}

InteractiveCmdPrompt::~InteractiveCmdPrompt()
{
    // delete the commands object
    CmdMap::iterator mapIt;
    for (mapIt = _cmds.begin(); mapIt != _cmds.end(); mapIt++) {
        delete mapIt->second;
    }
    _cmds.clear();
}

void InteractiveCmdPrompt::AddCommand(ICP_CmdDefinition * cmdToAdd) 
{
    if ( cmdToAdd != NULL ) {
        _cmds[cmdToAdd->name()] = cmdToAdd;
    }
}

void InteractiveCmdPrompt::SetDefaultCommand(ICP_CmdDefinition * defaultCmd)
{
    if ( _defaultCmd != NULL )
        delete _defaultCmd;

    _defaultCmd = defaultCmd;
}

void InteractiveCmdPrompt::SetHelpCommand( const char * helpCmdName, const char * helpCmdDescription ) 
{
    _helpCmdName.assign( helpCmdName );
    _helpCmdDescription.assign( helpCmdDescription );
}


int InteractiveCmdPrompt::ConvertStringToArgcArgv( 
        const char *inputString, 
        std::vector<std::string> * outStrings )
{
    int   inputStringLen;
    int   argc = 0;
    char *pszCmdLine;
    char *allocatedMemory;
    char *startChar;

    // Init output vars
    allocatedMemory = NULL;
    
    // Set to no argv elements, in case we have to bail out
    outStrings->clear();

    // First get a pointer to the system's version of the command line, and
    // figure out how long it is.
    inputStringLen = strlen( inputString );
    if (inputStringLen == 0) {
        return 0;
    }

    // Allocate memory to store a copy of the command line.  We'll modify
    // this copy, rather than the original command line.  Yes, this memory
    // currently doesn't explicitly get freed, but it goes away when the
    // process terminates.
    allocatedMemory = (char *)malloc( inputStringLen+1 );
    if ( !allocatedMemory ) {
        return 0;
    }
    pszCmdLine = allocatedMemory;

    // Copy the system version of the command line into our copy
    strcpy( pszCmdLine, inputString );

    if ( '"' == *pszCmdLine )   // If command line starts with a quote ("),
    {                           // it's a quoted filename.  Skip to next quote.
        pszCmdLine++;

        startChar = pszCmdLine;

        while ( *pszCmdLine && (*pszCmdLine != '"') ) {
            pszCmdLine++;
        }

        if ( *pszCmdLine ) {    // Did we see a non-NULL ending?
            *pszCmdLine++ = 0;  // Null terminate and advance to next char
            outStrings->push_back((const char*)startChar);
            startChar = NULL;
        }
        else {
            free(allocatedMemory);
            return 0;           // Oops!  We didn't see the end quote
        }
    }
    else    // A regular (non-quoted) filename
    {
        startChar = pszCmdLine;

        while ( *pszCmdLine && (' ' != *pszCmdLine) && ('\t' != *pszCmdLine) ) {
            pszCmdLine++;
        }

        if ( *pszCmdLine ) {
            *pszCmdLine++ = 0;  // Null terminate and advance to next char
        }

        if (startChar) {
            outStrings->push_back((const char*)startChar);
            startChar = NULL;
        }
    }

    // Done processing argv[0] (i.e., the executable name).  Now do th
    // actual arguments

    argc = 1;

    while ( 1 )
    {
        // Skip over any whitespace
        while ( (*pszCmdLine) && ((' ' == *pszCmdLine) || ('\t' == *pszCmdLine)) ) {
            pszCmdLine++;
        }

        if ( 0 == *pszCmdLine ) { // End of command line???
            break;
        }

        if ( '"' == *pszCmdLine )   // Argument starting with a quote???
        {
            pszCmdLine++;   // Advance past quote character

            startChar = pszCmdLine;

            // Scan to end quote, or NULL terminator
            while ( *pszCmdLine && (*pszCmdLine != '"') ) {
                pszCmdLine++;
            }
            if ( 0 == *pszCmdLine ) {
                if (startChar) {
                    outStrings->push_back((const char*)startChar);
                    startChar = NULL;
                }
                break;
            }
            
            if ( *pszCmdLine ) {
                *pszCmdLine++ = 0;  // Null terminate and advance to next char
                if (startChar) {
                    outStrings->push_back((const char*)startChar);
                    startChar = NULL;
                }
            }
        }
        else                        // Non-quoted argument
        {
            startChar = pszCmdLine;

            // Skip till whitespace or NULL terminator
            while ( (*pszCmdLine) && ((' '!=*pszCmdLine) && ('\t'!=*pszCmdLine)) ) {
                pszCmdLine++;
            }
            
            if ( 0 == *pszCmdLine ) {
                if (startChar) {
                    outStrings->push_back((const char*)startChar);
                    startChar = NULL;
                }
                break;
            }
            if ( *pszCmdLine ) {
                *pszCmdLine++ = 0;  // Null terminate and advance to next char
                if (startChar) {
                    outStrings->push_back((const char*)startChar);
                    startChar = NULL;
                }
            }
        }
    }

    if (startChar) {
        outStrings->push_back((const char*)startChar);
        startChar = NULL;
    }

    free(allocatedMemory);

    return argc;
}

bool InteractiveCmdPrompt::Run(std::ostream& outStream)
{
    std::string cmdline;

    while (true)
    {
        // get a new line
        char *z = readline(_strPrompt.c_str());

        // check for error or EOF
        if (!z) {
            return false;
        }

        if (*z) {
            // if there was input, add to history
            add_history(z);

            // save in the format of a string
            cmdline.assign((const char *)z);

            // execute the command
            int cmdresult = ExecuteCommandFromString(cmdline,outStream);
            if (cmdresult == ICP_STOP) {
                return ICP_STOP;
            }
            if ( _displayReturnValues ) {
                outStream << "Return value: " << cmdresult << std::endl;
            }

            // free readline memory
            free(z);
        }
    }
    return true;
}

int InteractiveCmdPrompt::ExecuteCommandFromString(std::string cmdline, std::ostream& outStream)
{
    int cmdresult = 0; 
    int numParams;
    std::vector<std::string> parameters;

    // find parameters
    str_trim( cmdline );
    numParams = ConvertStringToArgcArgv( cmdline.c_str(), &parameters);

    // search for command
    if ( numParams > 0 ) {

        // execute the command
        cmdresult = ExecuteCommandFromParams(parameters, outStream);

    }
    return cmdresult;
}

int InteractiveCmdPrompt::ExecuteCommandFromParams(std::vector<std::string>& parameters, std::ostream& outStream)
{
    int cmdresult = -1;

    // check for help
    if (parameters[0] == _helpCmdName ) {
        const char * cmdname = NULL;
        if ( parameters.size() > 1 ) {
            cmdname = parameters[1].c_str();
        }
        DisplayHelp(outStream, cmdname);
        return 0;
    }

    // find the function to call
    ICP_CmdDefinition *cmdDef = GetCmdDef(parameters[0]);
    if ( cmdDef == NULL ) {
        if ( _defaultCmd != NULL ) {
            cmdresult = _defaultCmd->execute( parameters, outStream, _userdata);
        }
        else {
            outStream << "Command not found: `" << parameters[0] << "'" << std::endl;
        }
    }
    else {
        cmdresult = cmdDef->execute( parameters, outStream, _userdata);
    }
    return cmdresult;
}

void InteractiveCmdPrompt::DisplayHelp(std::ostream& outStream, const char * cmdname)
{
    CmdMap::iterator mapIt;
    unsigned int longestCmdNameLength = _helpCmdName.length();
    std::string spaces;

    // print out specific help info
    if ( cmdname != NULL ) {
        
        ICP_CmdDefinition *cmdDef = GetCmdDef(cmdname);
        if ( cmdDef != NULL ) {
            outStream << cmdname << " - " << cmdDef->description() << std::endl;
            outStream << "SYNTAX" << std::endl << "    " << cmdDef->syntax() << std::endl;
            return;
        }
        outStream << "No help on command: `" << cmdname << "'" << std::endl;
    }

    // find command name lengths
    for (mapIt = _cmds.begin(); mapIt != _cmds.end(); mapIt++) {
        if (mapIt->first.length() > longestCmdNameLength ) {
            longestCmdNameLength = mapIt->first.length();
        }
    }

    outStream << "Available commands:" << std::endl;

    // print the commands
    for (mapIt = _cmds.begin(); mapIt != _cmds.end(); mapIt++) {

        spaces.assign( longestCmdNameLength - mapIt->first.length(), ' ');
        outStream << " " << mapIt->first << spaces << "- " << mapIt->second->description() << std::endl;
    }

    spaces.assign( longestCmdNameLength - _helpCmdName.length(), ' ');
    outStream << " " << _helpCmdName << spaces << "- " << _helpCmdDescription << std::endl;
}

ICP_CmdDefinition * InteractiveCmdPrompt::GetCmdDef( std::string cmdName ) 
{
    CmdMap::iterator findcmd = _cmds.find(cmdName);
    if ( findcmd != _cmds.end() ) {
        return findcmd->second;
    }
    return NULL;
}

#endif //#ifdef USE_INTERACTIVE_COMMAND_PROMPT
