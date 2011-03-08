//            File: AssemblerNew.cpp
// Original Author: Dominic Eales
// --------------------------------------------------------------------
//
// OVERVIEW:
// Implementation of AssemblerNew methods.
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

// system includes
#include <math.h>
#include <sys/statvfs.h>
#include <iomanip>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

// local includes
#include "AssemblerNew.h"
#include "stlext.h"
#include "Utils.h"
#include "UserInput.h"

  // Constructor/Destructor

AssemblerNew::AssemblerNew()
{
    mHaveRunSetParams = false;
}

AssemblerNew::~AssemblerNew()
{
}

void AssemblerNew::setRunParameters(void)
{
    if (mHaveRunSetParams == false) {
        Assembler::setRunParameters( &mExtraDataSet );
        mHaveRunSetParams = true;
    }
}

#ifdef USE_INTERACTIVE_COMMAND_PROMPT
int AssemblerNew::runcommand( std::vector<std::string>& parameters, std::ostream& out )
{
    if ( ( parameters[0] == "setoutput" ) || ( parameters[0] == "out" ) ) {
        return Command_SetOutputFile( parameters, out);
    }
    else if ( ( parameters[0] == "addfileset" ) || ( parameters[0] == "afs" ) ) {
        return Command_AddFileSet( parameters, out);
    }
    else if ( ( parameters[0] == "listfilesets" ) || ( parameters[0] == "lfs" ) ) {
        return Command_ListFileSets( parameters, out);
    }
    else if ( ( parameters[0] == "stage1" ) || ( parameters[0] == "s1" ) ) {
        return Command_Stage1( parameters, out);
    }
    else if ( ( parameters[0] == "stage2" ) || ( parameters[0] == "s2" ) ) {
        return Command_Stage2( parameters, out);
    }
    else if ( ( parameters[0] == "stage3" ) || ( parameters[0] == "s3" ) ) {
        return Command_Stage3( parameters, out);
    }
    else if ( ( parameters[0] == "stage4" ) || ( parameters[0] == "s4" ) ) {
        return Command_Stage4( parameters, out);
    }
    else if ( ( parameters[0] == "init" ) ) {
        return Command_Init( parameters, out);
    }
    
    // error occurred
    out << "Unknown command `" << parameters[0] << "' on this object" << std::endl;
    return -1;
}


int AssemblerNew::constructcommand( std::vector<std::string>& parameters, std::ostream& out )
{
    // load user options
    V_FOR_EACH( parameters, std::string, arg) {

        /*
        ** Check for optimised read length
        */ 
        if ((!strcmp(arg->c_str(), "-o") || !strcmp(arg->c_str(), "--opt-read-len")))
        {
            this->mOptimiseForArch = true;
        }
    }
    
    return 0;
}

#endif



void GetConsoleDims(unsigned short& height, unsigned short& width )
{
    struct winsize ws;
    
    if ((ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 &&
         ioctl(STDERR_FILENO, TIOCGWINSZ, &ws) == -1 &&
         ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) == -1) ||
         ws.ws_col == 0) {
        height = 25;
        width = 80;
    } else {
        height = ws.ws_row;
        width = ws.ws_col;
    }
}

int AssemblerNew::Command_SetOutputFile( std::vector<std::string>& parameters, std::ostream& out ) 
{
    const char * syntax = 
            "setoutput filename\n"
            "  filename - The file to write output messages to.\n";

    if ( parameters.size() != 2 ) {
        out << parameters[0] << ": wrong number of parameters" << std::endl;
        out << "SYNTAX:" << std::endl << syntax << std::endl;
        return -1;
    }

    bool isDir;
    IsDirectory( parameters[1], isDir );
    if (isDir == true ) {
        out << "Error: cannot use a directory as the output file." << std::endl;
        return -3;
    }
    else if (true == FileExists(parameters[1]) ) {
        if (true == GetYesNoFromUser( std::cin, out, "File exists, overwrite? [y,n] ", "Bad response, try again.") ) {
            out << "Aborted." << std::endl;
            return -4;
        }
    }
    
    mStdOutFile = parameters[1];

    return 0;
}

int AssemblerNew::Command_AddFileSet( std::vector<std::string>& parameters, std::ostream& out ) 
{
    const char * syntax = 
            "addfileset filename [maxreads]\n"
            "\n"
            "  filename - the fileset descriptor file to add.\n"
            "  maxreads - (optional) only load this many reads from this file-set.\n"
            "             If not set, or is zero, all reads will be loaded.\n";

    if ( ( parameters.size() != 2 ) &&
           ( parameters.size() != 3 ) ){
        out << parameters[0] << ": wrong number of parameters" << std::endl;
        out << "SYNTAX:" << std::endl << syntax << std::endl;
        return -1;
    }
    
    int retVal = 0;

    FileSet fs;
    if ( false == fs.FillFrom_File( parameters[1] ) ) {
        out << "error: cannot load fileset from `" << parameters[1] << "'" << std::endl;
        retVal = -2;
    }
    else {
        
        uMDInt maxReads = 0;
        if ( parameters.size() == 3 ) {
            from_string<uMDInt>(maxReads,parameters[2],std::dec);
        }
        mDataSet->addFileSet( fs, maxReads );
    }

    return retVal;
}

int AssemblerNew::Command_ListFileSets( std::vector<std::string>& parameters, std::ostream& out ) 
{
    const char * syntax = "listfilesets";

    if ( parameters.size() != 1 ) {
        out << parameters[0] << ": wrong number of parameters" << std::endl;
        out << "SYNTAX:" << std::endl << syntax << std::endl;
        return -1;
    }
    
    int retVal = 0;

    std::vector<FileSet> * fsets;
    fsets = mDataSet->getFileSets();

    if (fsets->size() != 0) {
        enum {COL_IDX, COL_FMT, COL_READS, COL_READLEN, COL_TRIM, COL_INSLIBS, COL_FILE, NUMHEADINGS};
        const char *       headings[] = {"IDX","FORMAT","READS","READLEN","TRIMLEN","INSERTLIBS","FILE"};
        unsigned int       col;
        unsigned int       colWidth[NUMHEADINGS];
        std::string        cells[NUMHEADINGS][fsets->size()+1];
        
        for (col=0; col<NUMHEADINGS; col++) {
            cells[col][0] = headings[col];
            colWidth[col] = strlen(headings[col]);
        }
        
        int counter = 1;
        V_FOR_EACH((*fsets),FileSet,it) {
            cells[COL_IDX][counter]     = toStr(counter);
            cells[COL_FMT][counter]     = it->getFormatName();
            cells[COL_READS][counter]   = toStr(it->getNumReads());
            cells[COL_READLEN][counter] = toStr(it->getReadLength());
            cells[COL_TRIM][counter]    = toStr(it->getTrimLength());
            cells[COL_FILE][counter]    = it->GetFullPathname();

            // do insert libs
            std::vector<InsertLibrary> * libs = it->getInsLibs();
            cells[COL_INSLIBS][counter] = "";
            V_FOR_EACH((*libs),InsertLibrary,libit) {
                cells[COL_INSLIBS][counter] += toStr(libit->_mean);
                cells[COL_INSLIBS][counter] += ";";
            }

            // fix widths
            for (col=0; col<NUMHEADINGS; col++) {
                colWidth[col] = max(((unsigned int)cells[col][counter].size()),colWidth[col]);
            }

            counter++;
        }

        unsigned int total = 0;
        unsigned short h,w;
        GetConsoleDims(h,w);
        for (col=0; col<NUMHEADINGS; col++) {
            colWidth[col]++;
            total += colWidth[col];
            if ( total >= w ) {
                colWidth[col] -= (total-w);
                total = w;
            }
        }
        
        unsigned int row;
        for (row=0; row<fsets->size()+1; row++) {
            for (col=0; col<NUMHEADINGS; col++) {
                if (cells[col][row].size() > colWidth[col]) {
                    cells[col][row].erase(colWidth[col]);
                }
                if (colWidth[col]) {
                    out << std::left << setw(colWidth[col]) << setfill(' ') << cells[col][row];
                }
            }
            out << std::endl;
        }
    }

    return retVal;
}

int AssemblerNew::Command_Stage1( std::vector<std::string>& parameters, std::ostream& out )
{
    const char * syntax = "stage1";
    if ( parameters.size() != 1 ) {
        out << parameters[0] << ": wrong number of parameters" << std::endl;
        out << "SYNTAX:" << std::endl << syntax << std::endl;
        return -1;
    }
    setRunParameters();
    stage1();
    return 0;
}


int AssemblerNew::Command_Stage2( std::vector<std::string>& parameters, std::ostream& out )
{
    const char * syntax = "stage2";
    if ( parameters.size() != 1 ) {
        out << parameters[0] << ": wrong number of parameters" << std::endl;
        out << "SYNTAX:" << std::endl << syntax << std::endl;
        return -1;
    }
    setRunParameters();
    stage2();
    return 0;
}

int AssemblerNew::Command_Stage3( std::vector<std::string>& parameters, std::ostream& out )
{
    const char * syntax = "stage3";
    if ( parameters.size() != 1 ) {
        out << parameters[0] << ": wrong number of parameters" << std::endl;
        out << "SYNTAX:" << std::endl << syntax << std::endl;
        return -1;
    }
    setRunParameters();
    stage3();
    return 0;
}

int AssemblerNew::Command_Stage4( std::vector<std::string>& parameters, std::ostream& out )
{
    const char * syntax = "stage4";
    if ( parameters.size() != 1 ) {
        out << parameters[0] << ": wrong number of parameters" << std::endl;
        out << "SYNTAX:" << std::endl << syntax << std::endl;
        return -1;
    }
    setRunParameters();
    stage4();
    return 0;
}

int AssemblerNew::Command_Init( std::vector<std::string>& parameters, std::ostream& out )
{
    const char * syntax = "init";
    if ( parameters.size() != 1 ) {
        out << parameters[0] << ": wrong number of parameters" << std::endl;
        out << "SYNTAX:" << std::endl << syntax << std::endl;
        return -1;
    }
    setRunParameters();
    return 0;
}

