//            File: SassyShell.h
// Original Author: Dominic Eales
// --------------------------------------------------------------------
//
// OVERVIEW:
// An implementation of the ICP specific to Sassy
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

#ifndef _SASSYSHELL_H_
#define _SASSYSHELL_H_

#include <typeinfo>
#include <map>
#include <string>
#include <cstdlib>

#include "InteractiveCmdPrompt.h"

typedef enum  {
    SASSY_OBJ_STRING,
    SASSY_OBJ_ICP,
    SASSY_OBJ_COMMANDABLE,
    SASSY_OBJ_UNKNOWN,
    SASSY_OBJ_NOTEXISTS
} SassyShellObjectType;


class SassyIcpObj {
public:
    SassyShellObjectType type;
    void *          obj;
};

class SassyShell : public InteractiveCmdPrompt {
public:
    SassyShell();
    ~SassyShell();

    SassyShellObjectType vartype( const char * varname );
    bool getvar(const char * varname, void*& value, SassyShellObjectType type);
    bool getvar(const char * varname, std::string& value);
    bool setvar(const char *varname, void *value, SassyShellObjectType type);
    bool setvar(const char *varname, std::string value);

    bool getallvars(std::vector<std::string>& varNames);

private:
    // vars
    std::map<std::string, SassyIcpObj> _vars;
    std::map<std::string, std::string> _varStrings;
};

class SassyShellObjectWithCommands {
public:
	// need a virtual destructor
	virtual ~SassyShellObjectWithCommands() {};
	
    virtual const char * gettypename(void) const = 0;
    virtual int constructcommand( std::vector<std::string>& parameters, std::ostream& out ) = 0;
    virtual int runcommand( std::vector<std::string>& parameters, std::ostream& out ) = 0;
};

#endif //_SASSYSHELL_H_

#endif //#ifdef USE_INTERACTIVE_COMMAND_PROMPT
