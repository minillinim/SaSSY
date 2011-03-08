//            File: SassyShell.cpp
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

#include <map>
#include <string>
#include <iostream>
#include <string.h>

#include "SassyShell.h"
#include "FormattedFileReader.h"
#include "stlext.h"

SassyShell::SassyShell() 
{
}

SassyShell::~SassyShell() 
{
	std::map<std::string, SassyIcpObj>::iterator varobj;
	while ( _vars.size() ) {
		varobj = _vars.begin();
		
		// delete commandable objects
		if (  ( varobj->second.type == SASSY_OBJ_COMMANDABLE )
			&&( varobj->second.obj  != NULL ) ) {
			
			SassyShellObjectWithCommands * cmdobj =(SassyShellObjectWithCommands*)(varobj->second.obj);
			delete cmdobj;
		}
		
		// delete variable
		_vars.erase( varobj );
	}
}


SassyShellObjectType SassyShell::vartype( const char * varname ) {

    // check the object vars
    std::map<std::string, SassyIcpObj>::iterator objvar = _vars.find(varname);
    if ( objvar != _vars.end() ) {
        return objvar->second.type;
    }
    // check the string vars
    std::map<std::string, std::string>::iterator varStr = _varStrings.find(varname);
    if ( varStr != _varStrings.end() ) {
        return SASSY_OBJ_STRING;
    }
    // check environment vars
    char * envvar = getenv( varname );
    if ( envvar != NULL ) {
        return SASSY_OBJ_STRING;
    }

    return SASSY_OBJ_NOTEXISTS;
}

bool SassyShell::getvar(const char * varname, void*& value, SassyShellObjectType type) {

    value = NULL;

    // check the object vars
    std::map<std::string, SassyIcpObj>::iterator var = _vars.find(varname);
    if ( var != _vars.end() ) {
        if ( var->second.type == type ) {
            value = var->second.obj;
        }
        return true;
    }
    return false;
}

bool SassyShell::getvar(const char * varname, std::string& value) {

    // check the string vars
    std::map<std::string, std::string>::iterator varStr = _varStrings.find(varname);
    if ( varStr != _varStrings.end() ) {
        value = varStr->second;
        return true;
    }

    // check for enviro vars
    char * envvar = getenv( varname );
    if ( envvar != NULL ) {
        value = std::string(envvar);
        return true;
    }

    // check the object vars
    std::map<std::string, SassyIcpObj>::iterator var = _vars.find(varname);
    if ( var != _vars.end() ) {
        if ( ( var->second.type == SASSY_OBJ_COMMANDABLE ) &&( var->second.obj  != NULL ) ) {

            SassyShellObjectWithCommands * cmdobj =(SassyShellObjectWithCommands*)(var->second.obj);
            value = std::string("(") + std::string(cmdobj->gettypename()) + std::string(" object)");
        }
        else {
            value = "(object)";
        }
        return true;
    }

    return false;
}

bool SassyShell::setvar(const char *varname, void *value, SassyShellObjectType type) {

    // check the variable name doesn't exist as a string var
    std::map<std::string, std::string>::iterator varStr = _varStrings.find(varname);

    // or as an environment variable
    char * envvar = getenv( varname );

    // do the check now
    if ( ( varStr == _varStrings.end() ) && (envvar == NULL) ) {
        SassyIcpObj varval;
        varval.type = type;
        varval.obj  = value;

        _vars[varname] = varval;

        return true;
    }
    return false;
}

bool SassyShell::setvar(const char *varname, std::string value) {

    // check the variable name doesn't exist as an object
    std::map<std::string, SassyIcpObj>::iterator var = _vars.find(varname);
    if ( var == _vars.end() ) {
        _varStrings[varname] = value;
        return true;
    }
    return false;
}

bool SassyShell::getallvars(std::vector<std::string>& varNames) {

    varNames.clear();

    // add varaible names
    M_FOR_EACH( _vars, std::string, SassyIcpObj, mapIter1, key, value ) {
        varNames.push_back(key);
    }
    M_END;

    // add varaible names
    M_FOR_EACH( _varStrings, std::string, std::string, mapIter2, key, value ) {
        varNames.push_back(key);
    }
    M_END;

    return true;
}

#endif //#ifdef USE_INTERACTIVE_COMMAND_PROMPT
