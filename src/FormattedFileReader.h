//            File: FormattedFileReader.h
// Original Author: Dominic Eales
// --------------------------------------------------------------------
//
// OVERVIEW:
// Class for reading in files of different formats
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

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>

#include <pcre.h>

#include "Dataset.h"

//#define __DEBUG__

typedef bool (*FFRRecordCallbackFuncPtr)(std::map<std::string,std::string> recordInfo, void *callbackdata);

#define FFD_READ_BUF_SIZE 1000
#define FFD_MAX_CHARACTERS_IN_RECORD 10000
#define FREE_PCRE_OBJ(oBJ) {if(oBJ!=NULL){pcre_free(oBJ);oBJ=NULL;}}

class FFRInputFile {
    public:
        enum FFRIFRetVal {
            FFRIFRETVAL_SUCCESS,
            FFRIFRETVAL_NO_RECORD_FOUND,
            FFRIFRETVAL_END_OF_FILE,
            FFRIFRETVAL_ERROR
        };

        FFRInputFile() {
            _recordRegExpObj = NULL;
            _recordRegExpObjMore = NULL;
            _matchedOnce = false;
        }
        ~FFRInputFile() {
            if (_stream.is_open()) {
                _stream.close();
            }
            FREE_PCRE_OBJ(_recordRegExpObj);
            FREE_PCRE_OBJ(_recordRegExpObjMore);
        }

        bool EndOfFile(void) {
            return ( (_stream.is_open() ) && ( _stream.eof() ) );
        }

        bool MatchedAnyRecords(void) {
            return _matchedOnce;
        }

        bool Open(std::string  fname, std::string  recordRegExp, std::string& errorMsg);

        FFRIFRetVal GetNextRecord( std::vector<std::string>& subStrings );

    private:
        std::ifstream _stream;
        std::string   _filename;
        pcre *        _recordRegExpObj;
        pcre_extra *  _recordRegExpObjMore;
        std::string   _inputBuffer;
        char          _readBuffer[FFD_READ_BUF_SIZE];
        bool          _matchedOnce;
};

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

class FormattedFileReader {

    enum FormatParsingType {
        FPTYPE_ONE_PAIRED_SEQ_PER_FILE,
        FPTYPE_ONE_PAIRED_SEQ_ACROSS_FILES,
        FPTYPE_WRONG
    };

public:
    FormattedFileReader();
    ~FormattedFileReader();

    const char * getError(void);

    bool LoadFileset(std::string filesetDescriptorFile, FFRRecordCallbackFuncPtr callbackFunc, void * callbackData);

    bool LoadFileset(FileSet fileSet, FFRRecordCallbackFuncPtr callbackFunc, void * callbackData);
    
    bool CheckFormatDefinitionFile( std::string formatFileName, std::map<std::string, std::string>& formatInfo );

private: // variables

    // special variables
    std::string  _varBaseFileName;

    // format information
    std::string  _formatName;
    unsigned int _formatNumFiles;
    FormatParsingType _formatType;

    // file streams
    std::vector<FFRInputFile*>  _inputFiles;

    // error
    std::string _strError;

    // variable values
    std::map<std::string, std::string>  _mapVariables;

    // format variables
    std::map<std::string, std::string>  _mapFormatVars;

    // format variables
    std::map<std::string, std::string>  _mapFileNumberVars;

    // output variables
    std::map<std::string, std::string>  _mapOutputVars;

private: // functions

    size_t FindNextComma( std::string in, size_t pos );

    // Syntax parsing
    bool GetExpressionValue( std::string& varValue, unsigned int& charsUsed, std::string code);
    bool GetQuotedValue( std::string& varValue, unsigned int& charsUsed, std::string code);
    bool GetVariableValue( std::string& varValue, unsigned int& charsUsed, std::string code);
    bool ResolveSyntax( std::string& value );

    // file processing
    bool PreprocessFormatFile(std::string formatFileName );
    bool PrepareForParsing( std::vector<std::string> inputFiles );
    bool PopulateMatchedSubStringVariables(FFRInputFile *ffrif, unsigned int index);
    bool ResolveOutputVars( std::map<std::string,std::string>& outpVars );

    // probably redundant (written before the financial crisis)
    bool FinancesInOrder(void) { return true; };

    // string values
    static void trim( std::string& s );
    static void tidystring( std::string& s );
    static unsigned int string_to_uint( const std::string& s );

};

