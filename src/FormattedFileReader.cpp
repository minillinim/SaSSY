//            File: FormattedFileReader.cpp
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
#include <string.h>

#include <pcre.h>

#include "FormattedFileReader.h"
#include "LoggerSimp.h"
#include "Dataset.h"
#include "fdfile.h"
#include "stlext.h"

int 
ExecuteRegularExpression( 
                         const pcre *              regexpObj, 
                         const pcre_extra *        regexpObjExtra, 
                         std::string *             inStr,
                         std::vector<std::string>& outSubstrList,
                         unsigned int *            pFullMatchOffset,
                         unsigned int *            pFullMatchLength
                        )
{
    int rc;
    int sstrOffsets[50];

    // clear any previous substrings
    outSubstrList.clear();

    // set all the offsets to 0
    memset( &sstrOffsets, 0, sizeof(sstrOffsets) );

    // run the expression against the input string
    rc = pcre_exec(
                   regexpObj,      /* result of pcre_compile() */
                   regexpObjExtra, /* we didn’t study the pattern */
                   inStr->c_str(),  /* the subject string */
                   inStr->length(), /* the length of the subject string */
                   0,              /* start at offset 0 in the subject */
                   0,              /* default options */
                   sstrOffsets,    /* vector of integers for substring information */
                   50);            /* number of elements (NOT size in bytes) */

    if ( rc <= 0 ) {
        return -1;
    }

    // push back all the substrings onto the output vector
    for (int i=0; i<(rc*2); i+=2) {
        if ( ( sstrOffsets[i] >= 0) && ( sstrOffsets[i+1] >= 0) ) {
            outSubstrList.push_back( inStr->substr( sstrOffsets[i], sstrOffsets[i+1] - sstrOffsets[i]) );
        }
        else {
            outSubstrList.push_back( "(undef)" );
        }
    }

    // save full match information
    if (pFullMatchOffset != NULL) {
        *pFullMatchOffset = sstrOffsets[0];
    }
    if (pFullMatchLength != NULL) {
        *pFullMatchLength = sstrOffsets[1] - sstrOffsets[0];
    }

    return outSubstrList.size();
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

FormattedFileReader::FormattedFileReader() 
{
}

FormattedFileReader::~FormattedFileReader() 
{
    // free the input file classes
    for (unsigned int i=0; i<_inputFiles.size(); i++) {
        if ( _inputFiles[i] != NULL ) {
            delete _inputFiles[i];
            _inputFiles[i] = NULL;
        }
    }
}

const char * FormattedFileReader::getError(void) 
{
    return _strError.c_str();
}

bool FormattedFileReader::CheckFormatDefinitionFile(std::string formatFileName, std::map<std::string, std::string>& formatInfo)
{
    // preprocess format file
    if ( false == PreprocessFormatFile( formatFileName ) ) {
#ifdef __DEBUG__
        std::cout << "PreprocessFormatFile failed" << std::endl;
#endif
        return false;
    }

    // return the format name
    formatInfo = _mapFormatVars;

    return true;
}

static std::vector<std::string>  FFRG_subStrings;

bool FormattedFileReader::PopulateMatchedSubStringVariables(FFRInputFile *ffrif, unsigned int index)
{
    FFRInputFile::FFRIFRetVal retVal;
    FFRG_subStrings.clear();

    // get some sub-strings
    retVal = ffrif->GetNextRecord( FFRG_subStrings );
    
    // check for bad return values  
    if ( ( FFRInputFile::FFRIFRETVAL_NO_RECORD_FOUND == retVal )
        || (FFRInputFile::FFRIFRETVAL_END_OF_FILE == retVal ) ) {

        // means we are finished with this file, as there is either
        // 1. no matched record after a certain amount of the file
        // 2. reached end of file
        // 3. Both (1) and (2)

        return false;
    }
    
    // add results to variable list
    unsigned int substrIdx;
    for (substrIdx = 0; substrIdx < FFRG_subStrings.size(); substrIdx++) {

        std::stringstream ss;
        ss << index << "." << substrIdx;

        // add regexp results to variable table
        _mapVariables[ss.str()] = FFRG_subStrings[substrIdx];
    }
    return true;
}

bool FormattedFileReader::ResolveOutputVars( std::map<std::string,std::string>& outpVars )
{
    // get the output values
    M_FOR_EACH( _mapOutputVars, std::string, std::string, mapIt, key, value) 
    {
        
        std::string temp = value;
        if ( false == ResolveSyntax( temp ) ) {
            std::cout << "**Error**: can't resolve output." << key << "=" << value << std::endl;
            return false;
        }
        outpVars[key] = temp;
    }
    M_END;

    return true;
}

bool FormattedFileReader::LoadFileset(std::string filesetDescriptorFile, FFRRecordCallbackFuncPtr callbackFunc, void * callbackData)
{
    FileSet fileset;
    //-----------------
    // Load the file-set file
    logInfo("1", 1);
    if ( false == fileset.FillFrom_File( filesetDescriptorFile ) ) {
        std::cout << "**Error**: cannot load file-set file" << std::endl;
        return false;
    }
    
    return LoadFileset(fileset,callbackFunc,callbackData);
}

bool FormattedFileReader::LoadFileset(FileSet fileset, FFRRecordCallbackFuncPtr callbackFunc,void * callbackData)
{
    std::map<std::string,std::map<std::string,std::string> > formats;
    std::map<std::string,std::string> ourFormat;
    std::map<std::string,std::string> outpVars;
    unsigned int fidx;

    //-----------------
    // Now assuming the user has chosen the correct format

    //-----------------
    // Get the format file name
    formats = FormatDefFiles::GetList();
    if ( !M_KEY_EXISTS(formats,fileset.getFormatName()) ) {
        std::cout << "**Error**: selected format file missing" << std::endl;
        return false;
    }
    ourFormat = formats[fileset.getFormatName()];
    std::string formatFilename = ourFormat["filepath"];

    //-----------------
    // preprocess format file
    if ( false == PreprocessFormatFile( formatFilename ) ) {
        std::cout << "**Error**: format file cannot be loaded, contains errors" << std::endl;
        return false;
    }

    //-----------------
    // get the file list and add directories
    std::vector<std::string> fileList = fileset.getFileList();
    V_FOR_EACH(fileList, std::string, fileIt) {
        (*fileIt) = fileset.getDirectory() + (*fileIt);
    }

    //-----------------
    // prepare for input parsing
    if ( false == PrepareForParsing( fileList ) ) {
        std::cout << "**Error**: cannot prepare input files for loading" << std::endl;
        return false;
    }

    //-----------------
    // do input record parsing
    //----------------------
    //----------------------
    if ( _formatType == FormattedFileReader::FPTYPE_ONE_PAIRED_SEQ_ACROSS_FILES ) {

        // This format requires that we read both files before we have a valid
        // input record.

        bool bNoMatch = false;
                    
        while (true) {

            // Get next new records
            for (fidx = 0; fidx < _formatNumFiles; fidx++ ) {

                // get some sub-strings
                if ( false == PopulateMatchedSubStringVariables( _inputFiles[fidx], fidx+1 ) ) {

                    // -- to get here either EOF or no matches --

                    if ( _inputFiles[fidx]->MatchedAnyRecords() == false ) {
                        bNoMatch = true;
                    }
                    break;
                }
            }

            // check if finished (we broke the loop early)
            if (fidx != _formatNumFiles) {
                break;
            }

            // Resolve output variables
            if ( false == ResolveOutputVars(outpVars) ) {
                std::cout << "**Error**: cannot resolve output for format" << std::endl;
                return false;
            }

            // call the callback function
            if ( false == callbackFunc( outpVars, callbackData ) ) {
                break;
            }
        }

        // check for no matches
        if ( bNoMatch == true ) {
            std::cout << "**Error**: no matches for file-set `" << fileset.GetFullPathname() << "'" << std::endl;
            return false;
        }
    }
    //----------------------
    //----------------------
    else if ( _formatType == FormattedFileReader::FPTYPE_ONE_PAIRED_SEQ_PER_FILE ) {

        // This format requires that we process each file for a valid input record.

        bool bNoMatch = false;
//        bool bUserBreak = false;

        int num_loaded = 0;
        while (true) {

            num_loaded++;
            // Get next new records
            for (fidx = 0; fidx < _formatNumFiles; fidx++ ) {

                // get some sub-strings
                if ( false == PopulateMatchedSubStringVariables( _inputFiles[fidx], 1 ) ) {

                    // -- to get here either EOF or no matches --

                    if ( _inputFiles[fidx]->MatchedAnyRecords() == false ) {
                        bNoMatch = true;
                    }
                    break;
                }

                // Resolve output variables
                if ( false == ResolveOutputVars(outpVars) ) {
                    std::cout << "**Error**: cannot resolve output for format" << std::endl;
                    break;
                }
                
                // call the callback function
                if ( false == callbackFunc( outpVars, callbackData ) ) {
                    break;
                }
            }

            // check if finished (we broke the loop early)
            if (fidx != _formatNumFiles) {
                break;
            }
        }

        // check for no matches
        if ( bNoMatch == true ) {
            std::cout << "**Error**: no matches for file-set `" << fileset.GetFullPathname() << "'" << std::endl;
            return false;
        } 
    }
    return true;
}

bool FormattedFileReader::GetVariableValue(std::string& varValue, unsigned int& charsUsed, std::string code)
{
    std::string::size_type pos;

    // find next % sign
    pos = code.find( '%', 1 );
    if (pos == std::string::npos) {
        // bad variable format
        return false;

    }

    std::string varName = code.substr(1,pos-1);
    if ( !M_KEY_EXISTS(_mapVariables,varName.c_str()) ) {
        // variable does not exist
        return false;
    }

    charsUsed = pos+1;
    varValue  = _mapVariables[varName];

    return true;
}

bool FormattedFileReader::GetQuotedValue(std::string& varValue, unsigned int& charsUsed, std::string code)
{
    std::string::size_type pos;

    // find next quote
    pos = code.find( '"', 1 );
    if (pos == std::string::npos) {
        // bad quoted value
        return false;
    }

    varValue = code.substr(1,pos-1);
    charsUsed = pos+1;

    return true;
}

bool FormattedFileReader::GetExpressionValue(std::string& varValue, unsigned int& charsUsed, std::string code)
{
    std::string::size_type pos;

    // remove
    pos = code.rfind( '/' );
    if (pos == std::string::npos) {
        // bad regexp value
        return false;
    }

    varValue = code.substr(1,pos-1);
    charsUsed = pos+1;

    return true;
}

bool FormattedFileReader::ResolveSyntax( std::string& value )
{
    std::string strCatted;
    std::string temp;
    unsigned int consumedChars;

    while (value.length()) {
        FormattedFileReader::trim(value);

        switch (value[0]) {
            case '%':
                if ( false == GetVariableValue( temp, consumedChars, value) ) {
                    // bad variable
                    return false;
                }
                break;
            case '"':
                if ( false == GetQuotedValue( temp, consumedChars, value) ) {
                    // bad variable
                    return false;
                }
                break;
            case '/':
                if ( false == GetExpressionValue( temp, consumedChars, value) ) {
                    // bad variable
                    return false;
                }
                break;
            default:
                // unknown character
                return false;
        }

        // consume the chars used
        value.erase( 0, consumedChars);
        FormattedFileReader::trim(value);

        // add value into cat stuff
        strCatted += temp;

        // check for +
        if ( value.length() ) { 
            if ( value[0] != '+' ) {
                // no '+' sign -- bad syntax
                return false;
            }
            // delete '+' sign
            value.erase( 0, 1);
        }
    }

    // set the value
    value = strCatted;

    return true;
}

bool FormattedFileReader::PrepareForParsing( std::vector<std::string> inputFilenames ) 
{
    //------------
    // We have two types of parsing:
    //
    // 1. Both files are of the same format and each file represents one element of the read pair.
    //    In this case the 'format.file-set-style = ONE_PAIRED_SEQ_PER_FILE'
    //
    // 2. The second file contains complimentary information to the first file (eg. quality information)
    //    In this case the 'format.file-set-style = ONE_PAIRED_SEQ_ACROSS_FILES'
    // 
    // Obviously if our file format requires only one file (eg. FASTQ) then we just parse it like a 
    // ONE_PAIRED_SEQ_PER_FILE type of format.
    
    //-----------
    // Did they parse me something stupid
    if ( inputFilenames.size() != _formatNumFiles ) {
        std::cout << "**Error**: wrong number of input files, expected " << _formatNumFiles << std::endl;
        return false;
    }
    
    //-----------
    // check for required file-dependent variables
    for (unsigned int i=0; i<_formatNumFiles; i++ ) {

        // get the regular expression
        std::string regExp;
        if ( _formatType == FormattedFileReader::FPTYPE_ONE_PAIRED_SEQ_PER_FILE ) {
            regExp = _mapFormatVars["common-record-expr"];
        }
        else {
            std::stringstream ssrecordexp;
            ssrecordexp << (i+1) << ".record-expr";

            regExp = _mapFileNumberVars[ssrecordexp.str()];
        }

        // Resolve the reg exp syntax
        if ( false == ResolveSyntax( regExp ) ) {
            // bad filename parsing
            std::cout << "**Error**: bad regular expression `"<< regExp <<"'" << std::endl;
            return false;
        }

        // create an input file parser
        std::string errorMsg;
        FFRInputFile * pInputFile = new FFRInputFile();
        PARANOID_ASSERT_L2(pInputFile != NULL);
        if ( false == pInputFile->Open( inputFilenames[i], regExp, errorMsg ) ) {
            // can't prepare file or regexp
            std::cout << "**Error**: can't prepare regular expression `"<< errorMsg <<"'" << std::endl;
            return false;
        }

        // add the input file parser to our list
        _inputFiles.push_back( pInputFile );
    }
    
    return true;
}

bool FormattedFileReader::PreprocessFormatFile(std::string formatFileName)
{

    typedef std::string::size_type pos;
    const std::string delim  = "=";  // separator
    const std::string comm   = "#";    // comment
    const pos skip = delim.length();        // length of separator
    unsigned int linenum = 0;

    std::ifstream ifsFormatFile;
    std::string   strInputLine;
    
    _formatType = FPTYPE_WRONG;

    //#### READ IN THE FORMAT FILE

    // open format file
    ifsFormatFile.open( formatFileName.c_str() );
    if (!ifsFormatFile.is_open()) {
        return false;
    }

    while (!ifsFormatFile.eof()) {

        // Read an entire line at a time
        std::string line;
        std::getline( ifsFormatFile, line );
        linenum++;

        FormattedFileReader::tidystring(line);

        // Ignore comments
        if (0 == line.find(comm)) {
            line = line.substr( 0, line.find(comm) );
        }

        // Ingore blank lines
        if (line.length() == 0) {
            continue;
        }

        // Parse the line if it contains a delimiter
        pos delimPos = line.find( delim );
        if( delimPos < std::string::npos )
        {
            // Extract the key
            std::string key = line.substr( 0, delimPos );
            line.replace( 0, delimPos+skip, "" );

            // Store key and value
            FormattedFileReader::trim(key);
            FormattedFileReader::trim(line);

            // process the variables a little
            std::string::size_type pos = key.find('.');
            if ( pos == std::string::npos ) {
                // no dot -> bad variable
                std::cout << formatFileName << ":warning: line "<< linenum <<" skipped, bad variable name" << std::endl;
                continue;
            }
            std::string grouping = key.substr(0,pos);
            std::string subkey = key.substr(pos+1);

            // check the grouping
            if ( grouping == "format" ) {

                FormattedFileReader::tidystring(line);
                _mapFormatVars[subkey] = line;

                // save an int version of number of files
                if ( subkey == "number-of-files" ) {
                    _formatNumFiles = FormattedFileReader::string_to_uint( _mapFormatVars["number-of-files"] );
                }
                else if ( subkey == "file-set-style" ) {
                    // set the format type variable
                    if ( _mapFormatVars["file-set-style"] == "ONE_PAIRED_SEQ_PER_FILE" ) {
                        _formatType = FormattedFileReader::FPTYPE_ONE_PAIRED_SEQ_PER_FILE;
                    }
                    else if ( _mapFormatVars["file-set-style"] == "ONE_PAIRED_SEQ_ACROSS_FILES" ) {
                        _formatType = FormattedFileReader::FPTYPE_ONE_PAIRED_SEQ_ACROSS_FILES;
                    }
                    else {
                        std::cout << formatFileName << ":error: variable `format.file-set-style' must be 'ONE_PAIRED_SEQ_PER_FILE' or 'ONE_PAIRED_SEQ_ACROSS_FILES'" << std::endl;
                        return false;
                    }
                }
            }
            else if ( grouping == "output" ) {
                _mapOutputVars[subkey] = line;
            }
            else {

                // check if the grouping is a valid number
                unsigned int fileNumberGroup;
                std::istringstream ss(grouping);
                if (!(ss >> fileNumberGroup) ) {
                    // bad grouping skip
                    std::cout << formatFileName << ":warning: line "<< linenum <<" skipped, bad variable group" << std::endl;
                    continue;
                }

                // we are zero-based not one-based
                fileNumberGroup--;

                // check the file exists for this file grouping
                if (fileNumberGroup >= _formatNumFiles) {
                    // bad grouping skip
                    std::cout << formatFileName << ":warning: line "<< linenum <<" skipped, file grouping number too large" << std::endl;
                    continue;
                }

                // push code and name on to the map
                _mapFileNumberVars[key] = line;
            }
        }
        else {
            std::cout << formatFileName << ":warning: line "<< linenum <<" skipped, syntax incorrect" << std::endl;
        }
    }
    ifsFormatFile.close();

    // check required variables exist
    if ( !M_KEY_EXISTS(_mapFormatVars,"name") ) {
        std::cout << formatFileName << ":error: missing required variable `format.name'" << std::endl;
        return false;
    }
    else if ( !M_KEY_EXISTS(_mapFormatVars,"number-of-files") ) {
        std::cout << formatFileName << ":error: missing required variable `format.number-of-files'" << std::endl;
        return false;
    }
    else if ( !M_KEY_EXISTS(_mapFormatVars,"file-set-style") ) {
        std::cout << formatFileName << ":error: missing required variable `format.file-set-style'" << std::endl;
        return false;
    }
    else if ( !M_KEY_EXISTS(_mapFormatVars,"lines-per-record") ) {
        std::cout << formatFileName << ":error: missing required variable `format.lines-per-record'" << std::endl;
        return false;
    }
    
    // check for common expression
    if ( ( _formatType == FormattedFileReader::FPTYPE_ONE_PAIRED_SEQ_PER_FILE )
           && ( !M_KEY_EXISTS(_mapFormatVars,"common-record-expr") ) ) {
        std::cout << formatFileName << ":error: with `format.file-set-style = ONE_PAIRED_SEQ_PER_FILE', missing required variable `common-record-expr'" << std::endl;
        return false;
    }
    
    // check for the file labels and the record expressions
    for (unsigned int fileNum=0; fileNum<_formatNumFiles; fileNum++) {
        
        std::stringstream ss1;
        ss1 << "file-" << (fileNum+1) << "-label";
        if ( !M_KEY_EXISTS( _mapFormatVars, ss1.str().c_str()) ) {
            std::cout << formatFileName << ":error: missing required variable `" << ss1.str() << "'" << std::endl;
            return false;
        }
        
        // check the record expressions are there
        if ( _formatType == FormattedFileReader::FPTYPE_ONE_PAIRED_SEQ_ACROSS_FILES ) {
            std::stringstream ss2;
            ss2 << (fileNum+1) << ".record-expr";
            if ( !M_KEY_EXISTS( _mapFileNumberVars, ss2.str().c_str()) ) {
                std::cout << formatFileName << ":error: with `format.file-set-style = ONE_PAIRED_SEQ_ACROSS_FILES', missing required variable `" << ss2.str() << "'" << std::endl;
                return false;
            }
        }
    }
    
    return true;
}

/* static */
void FormattedFileReader::tidystring(std::string& s)
{
    FormattedFileReader::trim(s);

    // Remove leading and trailing quotes
    if ((s[0] == '"')&&(s[s.length()-1] == '"')) {
        s = s.substr(1,s.length()-2);
    }

    FormattedFileReader::trim(s);
}

/* static */
void FormattedFileReader::trim(std::string& s)
{
    static const char whitespace[] = " \n\t\v\r\f";
    s.erase( 0, s.find_first_not_of(whitespace) );
    s.erase( s.find_last_not_of(whitespace) + 1U );
}

/* static */
unsigned int  FormattedFileReader::string_to_uint(const std::string& s)
{
    // Convert from a string to a T
    // Type T must support >> operator
    unsigned int t;
    std::istringstream ist(s);
    ist >> t;
    return t;
}

/*****************************************************************************************
 ** Methods specific to the FFR Input file object (one per fileset)
*****************************************************************************************/

bool FFRInputFile::Open(std::string  fname, std::string  recordRegExp, std::string& errorMsg)
{
    const char * errorStr;
    int errorOffset;

    //---
    // open the file
    _stream.open( fname.c_str(), std::ios::binary );
    if (!_stream.is_open()) {
        errorMsg = "file error: cannot open file: ";
        errorMsg += fname;
        return false;
    }
    _filename = fname;

    //---
    // compile reg exp
    _recordRegExpObj = pcre_compile( recordRegExp.c_str(), 0, &errorStr, &errorOffset, NULL);
    if (_recordRegExpObj == NULL) {
        std::ostringstream ss;
        ss << "expression error: [offset " << errorOffset << "] " << errorStr;
        errorMsg = ss.str();
        return false;
    }

    //---
    // study reg exp
    _recordRegExpObjMore = pcre_study( _recordRegExpObj, 0, &errorStr);

    // set matched once
    _matchedOnce = false;

    return true;
}

FFRInputFile::FFRIFRetVal FFRInputFile::GetNextRecord(std::vector<std::string>& subStrings)
{

    int retVal;
    unsigned int matchOffset = 0;
    unsigned int matchLength = 0;

    while (true) {
        
        subStrings.clear();
        
        // do regexp again
        retVal = ExecuteRegularExpression( 
                        _recordRegExpObj,
                        _recordRegExpObjMore,
                        &_inputBuffer,
                        subStrings,
                        &matchOffset, 
                        &matchLength );
    
        if (retVal <= 0) {

            // let's not read the whole file into memory... so do an
            // extra check, if we've read too many characters without a match, let's break out
            if ( _inputBuffer.size() >= FFD_MAX_CHARACTERS_IN_RECORD ) {
                return FFRInputFile::FFRIFRETVAL_NO_RECORD_FOUND;
            }

            // end of file?
            if ( _stream.eof() ) {
                break;
            }

            // get more data from the file and try again
            _stream.read( _readBuffer, FFD_READ_BUF_SIZE-1);
            _readBuffer[_stream.gcount()] = 0;
            _inputBuffer += _readBuffer;
        }
        else {
            break;
        }
    }

    // end of file condition:
    // we could have a match and be at the end of the file, so
    // we have to check the regexp return value to be sure
    if (( retVal <= 0 ) && ( _stream.eof() )) {
        
        // nat matching anything is more interesting than the end of file.
        if (_matchedOnce == false) {
            return FFRInputFile::FFRIFRETVAL_NO_RECORD_FOUND;
        }
        
        // we match something, but found the end of file
        return FFRInputFile::FFRIFRETVAL_END_OF_FILE;
    }

    // consume all the string up to end of new match
    _inputBuffer.erase(0,matchOffset+matchLength);

    // set matched once
    _matchedOnce = true;

    return FFRInputFile::FFRIFRETVAL_SUCCESS;
}
