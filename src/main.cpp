//            File: main.cpp
// Original Author: Michael Imelfort
// Hacked to Death: Dominic Eales
// -----------------------------------------------------
//
// OVERVIEW:
// Implementation file for sassyXX
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

// special define for global variables in headers
#define _MAIN_CPP_

// system includes
#include <iostream>
#include <vector>
#include <string.h>
#include <sstream>
#include <limits.h>
          
// local includes
#include "intdef.h"
#include "IdTypeDefs.h"
#include "LoggerSimp.h"
#include "default_parameters.h"
#include "Assembler.h"
#include "Dataset.h"
#include "Utils.h"
#include "IcpCommands.h"
#include "SassyShell.h"
#include "stlext.h"
#include "ExeDirectory.h"

// vars for logging
std::string mLogFile = "";                                          // where we'll be writing log entries to.
int mLogLevel = SAS_DEF_LOG_LEVEL;                                  // default log level

// effective read length
uint_16t mEffectiveReadLength = (uint_16t)INT_MAX;

// save / load flags
bool mSave = false;
bool mLoad = false;
bool mFull = false;

// Naive and extension offsets
uMDInt mExtOffset = SAS_DEF_UN_OFFSET_DEF_MAX;
uMDInt mNaiveOffset = SAS_DEF_UN_OFFSET_DEF_MIN;

// coverage cut offs
double mCovCutPercent = -1;

// strict mode
bool mStrict = SAS_DEF_DO_STRICT;

// traps?
uMDInt mIgnoreTrap = SAS_DEF_IGNORE_TRAP;

// overlaperation
bool mOverlaperate = SAS_DEF_DO_OVERLAPERATE;
sMDInt mOlapMaxWastage = -1;        // let the Context manager decide
sMDInt mOlapMinOlap = -1;

// scaffolding
uMDInt mScaffMinLinks = SAS_DEF_MIN_SCAFF_LINKS;
uMDInt mScaffMinIns = 0;

// extra output
bool mScaffGraph = false;
bool mCsv = false;

// filesets, directories, small mode size
std::string              mIoDirectory = "";
std::vector<std::string> mInputFilesets;
std::vector<uMDInt>      mInputFilesetMaxReads;

void PrintVersion( int whichCopyRight )
{
    std::cout << "\n***" << SAS_EXENAME << "*** Version: " << SAS_VERSION << std::endl;
    if(whichCopyRight == 1) // interactive mode
        std::cout << SAS_COPYRIGHT_I "\n" << std::endl;
    else
        std::cout << SAS_COPYRIGHT_R "\n" << std::endl;    
}

void displayUsage(void)
{
    //-----
    // display the usage of the program
    //
    PrintVersion(0);
    std::cout << "Usage (2 modes):\n" << std::endl;
    std::cout << "  1a) " SAS_EXENAME " -s -d OUTPUTDIR -f FILESET[,MAX_READS] [-f FILESET[,MAX_READS]]" << std::endl;
    std::cout << "  1b) " SAS_EXENAME " -l -d OUTPUTDIR" << std::endl;
    std::cout << "  1c) " SAS_EXENAME " -c -d OUTPUTDIR -f FILESET[,MAX_READS] [-f FILESET[,MAX_READS]]" << std::endl;
    std::cout << std::endl;
    std::cout << "  1) Always required:" << std::endl;
    std::cout << std::endl;
    std::cout << "    --dir -d WORKING_DIR              The directory to save output (-s) and load"  << std::endl;
    std::cout << "                                       input (-l). Also where the assembled" << std::endl;
    std::cout << "                                       contigs are saved" << std::endl;
    std::cout << std::endl;
    std::cout << "  1a) Load reads and save:" << std::endl;
    std::cout << std::endl;
    std::cout << "   Compulsory parameters:" << std::endl;
    std::cout << std::endl;
    std::cout << "    --save -s                         Load data from filesets, do basic processing." << std::endl;
    std::cout << "                                       Then save to output directory and exit" << std::endl;
    std::cout << "    --fileset -f FILESET[,MAX_READS]  Add sequences described by 'filesetfile'" << std::endl;
    std::cout << "                                       'MAX_READS' is maximum paired reads to load" << std::endl;
    std::cout << "   Optional parameters:" << std::endl;
    std::cout << std::endl;
    std::cout << "    --readlength READLENGTH           Set the effective readlength to this value" << std::endl;
    std::cout << "                                       (override values in the filset tag)" << std::endl;
    std::cout << "    --naive_off OFFSET                Maximum offset used to make naive contigs" <<std::endl;
    std::cout << "                                       [default: " << SAS_DEF_UN_OFFSET_DEF_MIN << "]" << std::endl;
    std::cout << "    --ext_off OFFSET                  Maximum offset used during extension" << std::endl;
    std::cout << "                                       [default: " << SAS_DEF_UN_OFFSET_DEF_MAX << "]" << std::endl;
    std::cout << std::endl;
    std::cout << "  1b) Load saved reads and extend:" << std::endl;
    std::cout << std::endl;
    std::cout << "   Compulsory parameters:" << std::endl;
    std::cout << std::endl;
    std::cout << "    --load -l                         Load the data from saved state" << std::endl;
    std::cout << "   Optional parameters:" << std::endl;
    std::cout << std::endl;
    std::cout << "    --no_strict                       Relax the growing algorithm, longer contigs" << std::endl;
    std::cout << "                                       but causes more over assemblies / chimeras" << std::endl;
    std::cout << "                                       [default: STRICT = "<<SAS_DEF_DO_STRICT<<"]" << std::endl;
    std::cout << "    --no_olap                         Skip overlaperation [default: OLAP = "<<SAS_DEF_DO_OVERLAPERATE<<"]" << std::endl;
    std::cout << "    --upper_coverage_percent PERCENT  Set the maximum coverage allowable for a" << std::endl;
    std::cout << "                                       naive Contig to be used in stretching" << std::endl;
    std::cout << "                                       [default: -1 (use all)] (see manual)" << std::endl;
    std::cout << "    --ext_off OFFSET                  Maximum offset used during extension" << std::endl;
    std::cout << "                                       [default: " << SAS_DEF_UN_OFFSET_DEF_MAX << "]" << std::endl;
    std::cout << "    --olap_min_size MIN_SIZE          The minimum number of bases two Contexts" << std::endl;
    std::cout << "                                       must overlap [default: highest upper cut off]" << std::endl;
    std::cout << "    --olap_max_wastage MAX_WASTAGE    The maximum number of bases which can be" << std::endl;
    std::cout << "                                       wasted during an overlaperation event" << std::endl;
    std::cout << "                                       [default: 2 * read length]" << std::endl;
    std::cout << "    --scaff_link_cut_off NUM_LINKS    Minimum number of links needed to scaffold" << std::endl;
    std::cout << "                                       0 to skip scaffolding [default: "<<SAS_DEF_MIN_SCAFF_LINKS<<"]" << std::endl;
    std::cout << "    --scaff_insert_cut_off CUT_OFF    For scaffolding, use only reads with insert" << std::endl;
    std::cout << "                                       greater than CUT_OFF [default: 0]" << std::endl;
    std::cout << "    --ignore_trap TRAP_INSERT         When using a small suplementary insert size" << std::endl;
    std::cout << "                                       to clean the data, ignore this in extensions." << std::endl;
    std::cout << "                                       0 to use all InsertLibs. [default: Ignore = "<<SAS_DEF_IGNORE_TRAP<<"]" << std::endl;
    std::cout << "    --graph                           Output the scaffold graph to stdout in" << std::endl;
    std::cout << "                                       GRAPHVIZ format [default: no graph]" << std::endl;
    std::cout << "    --csv                             Output the information used in scaffolding" << std::endl;
    std::cout << "                                       to stdout in CSV format [default: no csv]" << std::endl;
    std::cout << std::endl;
    std::cout << "  1c) Or in one step:" << std::endl;
    std::cout << std::endl;
    std::cout << "    --complete -c                     Perform complete assembly (no save point)" << std::endl;
    std::cout << "                                       ALL options for -s and -l can be used here." << std::endl;
    std::cout << std::endl;
    std::cout << "  Misc:" << std::endl;
    std::cout << std::endl;
    std::cout << "    --loglevel -v LEVEL               Set log level. 0 for no logging [default: "<<SAS_DEF_LOG_LEVEL<<"]" << std::endl;
    std::cout << "    --version                         Display version and exit" << std::endl;
    std::cout << "    --help -h                         Display this help" << std::endl;
    std::cout << std::endl;
    std::cout << "  2) " SAS_EXENAME " newfileset" << std::endl;
    std::cout << std::endl;
    std::cout << "     Interactively builds new fileset file (follow the prompts)." << std::endl;
//    std::cout << "3) " SAS_EXENAME " (no parameters)" << std::endl;
//    std::cout << "     Run the interactive sassy shell." << std::endl;
    std::cout << std::endl;
}

bool CheckArchitecture(void)
{
#if (SIZE_OF_INT == 64)
    //-----
    // This software is built for 64 bit integers
    // It compiles well on gcc version 4.3.2 [gcc-4_3-branch revision 141291] (SUSE Linux)
    // If your compiler does not define the keyword long as a 64 bit int then you'll need to
    // update intdef.h to make that happen. You should only need to edit that file...
    // This test will cause a failure if that is not the case.
    //
    uMDInt test_int_64;
    if(sizeof(test_int_64) != 8)
    {
        // ERROR!!!
        cout << "\n"
                "Error: \n"
                "It appears that your compiler did not correctly interpret the uMDInt\n"
                "an unsigned 64 bit integer. Please modify the file \"intdef.h\" and \n"
                "make sure that uMDInt is defined as a 64 bit integer.\n" << endl;
        
        return false;
    }
#endif
    return true;
}

SassyShell * NewShellObj( bool fullInteraction )
{
    SassyShell * shell = new SassyShell;
    
    shell->SetCmdUserData( shell );
    shell->SetPrompt( "sassy> " );
    shell->AddCommand( new SassyCmd_exit );
    shell->AddCommand( new SassyCmd_quit );
    shell->AddCommand( new SassyCmd_dispretval );
    shell->AddCommand( new SassyCmd_dispvar );
    shell->AddCommand( new SassyCmd_setvar );
    shell->AddCommand( new SassyCmd_listformats );
    shell->AddCommand( new SassyCmd_with );
    shell->AddCommand( new SassyCmd_newfileset );
    shell->AddCommand( new SassyCmd_loadfileset );
    shell->AddCommand( new SassyCmd_newassem );
    shell->AddCommand( new SassyCmd_showw );
    shell->AddCommand( new SassyCmd_showc );

    shell->SetDefaultCommand( new SassyCmd_Default );
        
    if ( fullInteraction ) {
        shell->Run(std::cout);
    }

    return shell;
}

int checkParams(int argc, char* argv[], int arg_counter)
{
    // run, load user options
    std::string tmpStr;
    size_t pos;
    
    do {
        
// Housekeeping
        /*
        ** Check for directory
        */
        if((!strcmp(argv[arg_counter], "-d") || !strcmp(argv[arg_counter], "--dir")))
        {
            if(argc == arg_counter)
            {
                std::cout << "\n" << SAS_EXENAME << " Error: Something is wrong with the parameters" << std::endl;
                displayUsage();
                return -5;
            }
            arg_counter++;
            mIoDirectory = argv[arg_counter];
        }
        /*
        ** Logging level
        */        
        else if(!strcmp(argv[arg_counter], "-v") || !strcmp(argv[arg_counter], "--loglevel"))
        {
            // set the loglevel...
            if(argc == arg_counter)
            {
                std::cout << "\n" << SAS_EXENAME << " Error: Something is wrong with the parameters" << std::endl;
                displayUsage();
                return -5;
            }
            
            arg_counter++;
            from_string<int>(mLogLevel, std::string(argv[arg_counter]), std::dec);
        }
        /*
        ** Name of the raw data file
        */
        else if(!strcmp(argv[arg_counter], "-f") || !strcmp(argv[arg_counter],"--fileset"))
        {
            // set the input file
            if(argc == arg_counter)
            {
                std::cout << "\n" << SAS_EXENAME << " Error: Something is wrong with the parameters" << std::endl;
                displayUsage();
                return -5;
            }
            arg_counter++;
            
            // check for maxreads
            tmpStr = argv[arg_counter];
            
            // check for a comma
            pos = tmpStr.find_last_of(',');
            if ( pos != std::string::npos ) {
                uMDInt maxReads;
                
                from_string<uMDInt>( maxReads, tmpStr.substr(pos+1), std::dec);
                
                // add the max reads given
                mInputFilesetMaxReads.push_back( maxReads );
                
                // delete max reads from the argument
                tmpStr.erase( pos );
            }
            else {
                // add maxreads as 0
                mInputFilesetMaxReads.push_back( 0 );
            }
            // add fileset file
            mInputFilesets.push_back( tmpStr  );
        }
        /*
        ** Display help information
        */
        else if(!strcmp(argv[arg_counter], "-h") || !strcmp(argv[arg_counter], "-?") || !strcmp(argv[arg_counter], "--help"))
        {
            displayUsage();
            return -5;
        }
        /*
        ** Display the version information
        */ 
        else if( !strcmp(argv[arg_counter], "--version") )
        {
            PrintVersion(0);
            return 0;
        }
// Save / load flags
        /*
        ** Load data and save?
        */        
        else if(!strcmp(argv[arg_counter], "-s") || !strcmp(argv[arg_counter], "--save"))
        {
            // set the file name to save to
            mSave = true;
        }
        /*
        ** Assemble form save point?
        */        
        else if(!strcmp(argv[arg_counter], "-l") || !strcmp(argv[arg_counter], "--load"))
        {
            // set the file name to load from
            mLoad = true;
        }
        /*
        ** Run full assembly instead?
        */
        else if(!strcmp(argv[arg_counter], "-c") || !strcmp(argv[arg_counter], "--complete"))
        {
            
            mFull = true;
        }
// Effective read length
        /*
        ** Set the effective readlength
        */
        else if(!strcmp(argv[arg_counter], "--readlength"))
        {
            // set now!
            if(argc == arg_counter)
            {
                std::cout << "\n" << SAS_EXENAME << " Error: Something is wrong with the parameters" << std::endl;
                displayUsage();
                return -5;
            }
            arg_counter++;
            from_string<uint_16t>(mEffectiveReadLength, std::string(argv[arg_counter]), std::dec);
        }

// Naive and extension offsets
        /*
        ** Override the default naive offset
        */
        else if(!strcmp(argv[arg_counter], "--naive_off"))
        {
            
            if(argc == arg_counter)
            {
                std::cout << "\n" << SAS_EXENAME << " Error: Something is wrong with the parameters" << std::endl;
                displayUsage();
                return -5;
            }
            arg_counter++;from_string<uMDInt>( mNaiveOffset, std::string(argv[arg_counter]), std::dec);
        } 
        /*
        ** Override the default extension offset
        */
        else if(!strcmp(argv[arg_counter], "--ext_off"))
        {
            if(argc == arg_counter)
            {
                std::cout << "\n" << SAS_EXENAME << " Error: Something is wrong with the parameters" << std::endl;
                displayUsage();
                return -5;
            }
            arg_counter++;
            from_string<uMDInt>( mExtOffset, std::string(argv[arg_counter]), std::dec);
        }    
// Coverage percent
        /*
        ** Set the coverage percent
        */
        else if(!strcmp(argv[arg_counter], "--upper_coverage_percent")) 
        {
            if(argc == arg_counter)
            {
                std::cout << "\n" << SAS_EXENAME << " Error: Something is wrong with the parameters" << std::endl;
                displayUsage();
                return -5;
            }
            arg_counter++;
            from_string<double>( mCovCutPercent , std::string(argv[arg_counter]), std::dec);
            if(-1 != mCovCutPercent)
            {
                if((1 < mCovCutPercent) || (0 > mCovCutPercent))
                {
                    std::cout << "\n" << SAS_EXENAME << " Error: --upper_coverage_percent must lie between 0 and 1 (or be -1)" << std::endl;
                    displayUsage();
                    return -5;
                }
            }
        }
        
// Non-strict mode
        /*
        ** Allow non-strict extension
        */
        else if(!strcmp(argv[arg_counter], "--no_strict")) 
        {
            mStrict = false;
        }
// Ignore trap?
        /*
        ** Handle trap inserts
        */
        else if(!strcmp(argv[arg_counter], "--ignore_trap"))
        {
            
            if(argc == arg_counter)
            {
                std::cout << "\n" << SAS_EXENAME << " Error: Something is wrong with the parameters" << std::endl;
                displayUsage();
                return -5;
            }
            arg_counter++;from_string<uMDInt>( mIgnoreTrap, std::string(argv[arg_counter]), std::dec);
        } 
// overlaperation
        /*
        ** Do overlaperation?
        */
        else if(!strcmp(argv[arg_counter], "--no_olap")) 
        {
            mOverlaperate = false;
        }
        /*
        ** Override the default minimum overlap
        */
        else if(!strcmp(argv[arg_counter], "--olap_min_size"))
        {
            if(argc == arg_counter)
            {
                std::cout << "\n" << SAS_EXENAME << " Error: Something is wrong with the parameters" << std::endl;
                displayUsage();
                return -5;
            }
            arg_counter++;
            from_string<sMDInt>(mOlapMinOlap , std::string(argv[arg_counter]), std::dec);
        }    
        /*
        ** Override the default maximum wasteage
        */
        else if(!strcmp(argv[arg_counter], "--olap_max_wastage"))
        {
            if(argc == arg_counter)
            {
                std::cout << "\n" << SAS_EXENAME << " Error: Something is wrong with the parameters" << std::endl;
                displayUsage();
                return -5;
            }
            arg_counter++;
            from_string<sMDInt>(mOlapMaxWastage, std::string(argv[arg_counter]), std::dec);
        }
// Scaffolding
        /*
        ** Minimum scaffolding links
        */
        else if(!strcmp(argv[arg_counter], "--scaff_insert_cut_off"))
        {
            if(argc == arg_counter)
            {
                std::cout << "\n" << SAS_EXENAME << " Error: Something is wrong with the parameters" << std::endl;
                displayUsage();
                return -5;
            }
            arg_counter++;
            from_string<uMDInt>( mScaffMinIns, std::string(argv[arg_counter]), std::dec);
        }    
        /*
        ** 
        */
        else if(!strcmp(argv[arg_counter], "--scaff_link_cut_off"))
        {
            if(argc == arg_counter)
            {
                std::cout << "\n" << SAS_EXENAME << " Error: Something is wrong with the parameters" << std::endl;
                displayUsage();
                return -5;
            }
            arg_counter++;
            from_string<uMDInt>( mScaffMinLinks, std::string(argv[arg_counter]), std::dec);
        }    
// Extra output
        /*
        ** Output scaffoldinfo to csv?
        */
        else if(!strcmp(argv[arg_counter], "--csv"))
        {
            mCsv = true;
        }    
        /*
        ** Print the scaffolding graph?
        */
        else if(!strcmp(argv[arg_counter], "--graph"))
        {
            mScaffGraph = true;
        }    
// SPEW!        
        else
        {
            std::cout << "\n" << SAS_EXENAME << " Error: Unknown parameter/argument " << argv[arg_counter] << std::endl;
            displayUsage();
            return -7;
        }
        arg_counter++;
    } while (arg_counter < argc);
    
    // now we can check if everything is in order then we can proceed!!!
    if (mIoDirectory == "")
    {
        std::cout << "\n" << SAS_EXENAME << " Error: Must specify output directory with -d or --dir" << std::endl;
        displayUsage();
        return -5;
    }
    else if ( (mFull & (mSave || mLoad)) || (!mFull && (mSave == mLoad)) )
    {
        std::cout << "\n" << SAS_EXENAME << " Error: Please choose exactly one of -l, -s, -c" << std::endl;
        displayUsage();
        return -5;
    }
    else if ( mInputFilesets.size() == 0 ) // no filesets added
    {
        if ( !mLoad )
        {
            std::cout << "\n" << SAS_EXENAME << " Error: There's no input fileset(s)" << std::endl;
            displayUsage();
            return -5;
        }
    }
    
    PrintVersion(0);
    return 0;
}

int main(int argc, char* argv[])
{
    // check architecture
    if ( CheckArchitecture() == false ) {
        return -1;
    }

    // get executable directory
    InitExecutableDirectory(argv[0]);

    // process the command line opts
    // program defaults (edit config.h to change these)

    //----------------------------------
    // check for full interactive mode
    //----------------------------------
    if ( argc == 1 ) {
        // full interactive shell
        
        // MIKE HACK
        // NOT IMPLEMENTED IN THIS VERSION
        displayUsage();
        return -5;
        
        // END MIKE HACK
        
        PrintVersion(1);
        std::cout << "Type 'exit' to exit or 'help' for help" << std::endl;
        delete NewShellObj( true );
        return 0;
    }
        
    // init counter
    int arg_counter = 1;
    
    //----------------------------------
    // check for single command mode
    //----------------------------------
    if (argv[arg_counter][0] != '-') {
        
        std::vector<std::string> parameters;

        SassyShell * shell = NewShellObj(false);
        
        PrintVersion(0);

        // populate the arguments
        while (arg_counter != argc ) {
            parameters.push_back( (const char*)argv[arg_counter++] );
        }

        // run the command
        int returnValue = shell->ExecuteCommandFromParams( parameters, std::cout );

        // delete this shell object
        delete shell;
        shell = NULL;

        // exit the way the command wanted
        return returnValue;
    }

    //----------------------------------
    // MUST be in full command line mode
    //----------------------------------
        
    int param_ret = checkParams(argc, argv, arg_counter);
    if(0 != param_ret) { return param_ret; }
        
    // make the assembler
    Assembler * mAssembler = new Assembler();

    // init directories
    mAssembler->setOutputDirectory( mIoDirectory );
    mIoDirectory = mAssembler->getOutputDirectory();
   
    RecursiveMkdir( mIoDirectory );

    // make the logger
    mLogFile = mIoDirectory + SAS_DEF_LOGFILE_EXTENSION;
    intialiseGlobalLogger(mLogFile, mLogLevel);

    // log the parameters:
    std::stringstream parms;
    for(int i = 0; i < argc; i++)
    {
        parms << argv[i] << " ";
    }
    logTimeStamp();
    logInfo("Parameters: " << parms.str(), 1);
    logInfo("-----------------------------------",1);

    // create data set
    if ( mSave || mFull ) {
        // make the data set manager
        // for storing all the information about the data
        // NOTE This guy is made here but is passed onto the Assembler
        // and it becomes the assemblers responsibility...
        DataSet * mDataSet = new DataSet();
        mDataSet->setExternalReadLength(mEffectiveReadLength);
        std::vector<uMDInt>::iterator maxReadsIter = mInputFilesetMaxReads.begin();
        V_FOR_EACH( mInputFilesets, std::string, fsnameIter ) {
            FileSet fs;
            if ( false == fs.FillFrom_File( *fsnameIter ) ) {
                std::cout << "\n" << SAS_EXENAME << " Error: Input fileset `" << (*fsnameIter) << "' cannot be loaded. Exiting..." << std::endl;
                return -4;
            }
            mDataSet->addFileSet( fs, (*maxReadsIter) );
            mDataSet->setIgnoreTrap(mIgnoreTrap);
            maxReadsIter++;
        }
        
        // Pass it to the Assembler
        mAssembler->setRunParameters(mDataSet);
    }
    
    // set the various modes
    mAssembler->intialise(mStrict, mScaffGraph, mCsv, mIgnoreTrap);
    
    //---------------------------
    // now let's have a party
    if(mLoad)
    {
        mAssembler->assembleFromSavedData(mIoDirectory, mExtOffset, mOverlaperate, mOlapMinOlap, mOlapMaxWastage, mScaffMinLinks, mScaffMinIns, mCovCutPercent);
    }
    else if(mSave)
    {
        mAssembler->loadRawDataThenSave(mIoDirectory, mNaiveOffset, mExtOffset);
    }
    else if (mFull)// full assembly
    {
        mAssembler->doFullRunAssembley(mNaiveOffset, mExtOffset, mOverlaperate, mOlapMinOlap, mOlapMaxWastage, mScaffMinLinks, mScaffMinIns, mCovCutPercent);
    }

    logInfo("Deleting Assembler Object", 4);
    if (mAssembler != NULL) {
        delete mAssembler;
        mAssembler = NULL;
    }

    logInfo("Run complete", 1);

    return 0;
}
