//            File: Dataset.cpp
// Original Author: Dominic Eales and Michael Imelfort
// --------------------------------------------------------------------
//
// OVERVIEW:
// for handling all aspects of datasets and filesets
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

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <ostream>
#include <iterator>
#include <algorithm>
#include <iomanip>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <math.h>
#include <limits.h>


#include "fdfile.h"
#include "Dataset.h"
#include "CheckSum.h"
#include "LoggerSimp.h"
#include "math_ext.h"
#include "FormattedFileReader.h"
#include "default_parameters.h"
#include "stlext.h"
#include "Utils.h"
#include "UserInput.h"

// set to "1" if you wish to enable checksums
#define SAS_USE_CS 0

/******************************************************
 ** InsertLibrary handling
 ******************************************************/

InsertLibrary::InsertLibrary(double mean)
{
    _mean = mean;
    _stdev = 0;
    _orientation = TYPE_UNSET;
    _numReads = 0;
    _upperCutOff = 0;
    _lowerCutOff = 0;
    _beta = 0;
    _fsid = 41;
}

InsertLibrary::InsertLibrary(double mean, OrientationType ot)
{
    _mean = mean;
    _stdev = 0;
    _orientation = ot;
    _numReads = 0;
    _upperCutOff = 0;
    _lowerCutOff = 0;
    _beta = 0;
    _fsid = 41;
}

InsertLibrary::~InsertLibrary()
{
}

void InsertLibrary::makeCutOffs(double stdevRange)
{
    //-----
    // get upper and lower cutoffs based on the mean and stdev of the libraries
    //
    _upperCutOff = (uMDInt)(_mean + (stdevRange * _stdev));
    if((stdevRange * _stdev) > _mean)
        _lowerCutOff = 0;
    else
        _lowerCutOff = (uMDInt)(_mean - (stdevRange * _stdev));
}

InsertLibrary::OrientationType InsertLibrary::calcOrientationType(bool seq1Reversed, bool seq2Reversed, sMDInt distance1to2)
{
    //-----
    // get the orientation type for a paired read
    // Each library has an orientation, this works like this:
    // There are at most 4 possible types, which can be represented by the characters DD, DA, AD and AA
    // (D is for disagree and A is for agree) if we let let D be 0 and A be 1 then we have a quick way of
    // getting a number {0, 1, 2, 3}. Looking at the first character D or A the idea is that you look at
    // the first read and see if it disagrees or not with the sequence. Based on that you know if the
    // paired read will be ahead or behind this read in the sequence. The second character tells you
    // whether or not the ahead/behind read agrees with / matches the orientation of the first read.
    //
    // So type 3 = AA says that if the first read agrees with the reference the paired read will be further
    // ahead and will be in the same orientation as the first. If it disagrees then the paired read will be
    // behind but still in a matching orientation to the first.
    //
    // And type 2 = AD says that if the first read agrees with the reference the paired read will be further
    // ahead and will be in the opposite orientation to the first. If it disagrees then the paired read will
    // be behind and still in the opposite orientation to the first.
    //
    //  0  DD  <---  --->   if the first read disagrees with ref then the second is ahead and in the reverse orientation
    //  1  DA  <---  <---   if the first read disagrees with ref then the second is ahead and in the same orientation
    //  2  AD  --->  <---   if the first read agrees with ref then the second is ahead and in the reverse orientation
    //  3  AA  --->  --->   if the first read agrees with ref then the second is ahead and in the same orientation
    //
    // And we can assign a Type to the library
    //

    if(seq1Reversed ^ seq2Reversed)
    {
        // we need to check the sign of the distance
        if(distance1to2 > 0)
        {
            if(seq1Reversed)
            {
                // <-F-  --->
               return InsertLibrary::TYPE_DD; 
            }
            else
            {
                // -F->  <---
                return InsertLibrary::TYPE_AD;
            }
        }
        else
        {
            if(seq1Reversed)
            {
                // --->  <-F-
                return InsertLibrary::TYPE_AD;
            }
            else
            {
                // <---  -F->
                return InsertLibrary::TYPE_DD;
            }
        }
    }
    else
    {
        // distance sign does not matter, type 1 or three only!
        // however, we cannot differentiate so always return type 3
        if(seq1Reversed)
        {
            // <---  <---
            return InsertLibrary::TYPE_AA;
        }
        else
        {
            // --->  --->
            return InsertLibrary::TYPE_AA;
        }
    }
    return InsertLibrary::TYPE_UNSET;
}

bool InsertLibrary::doesMatchLibraryStats(bool seq1Reversed, bool seq2Reversed, sMDInt distance1to2)
{
    // make the distance > 0
    uMDInt inSize = (uMDInt)(fAbs(distance1to2));
    //logInfo("---\nL: " <<  _lowerCutOff << " : " << inSize << " : " << _upperCutOff, 1);
    if(inSize >= _lowerCutOff)
    {
        if(inSize <= _upperCutOff)
        {
            // we've found the library, check the orientation...
            if(calcOrientationType(seq1Reversed, seq2Reversed, distance1to2) == _orientation)
            {
                return true;
            }
        }
    }
    return false;
}

bool InsertLibrary::isDistOrientOK(InsertLibrary::OrientationType ot, uMDInt dist)
{
    //----
    // check to see if this distance and orietation type are within bounds
    //
    if(dist >= _lowerCutOff)
    {
        if(dist <= _upperCutOff)
        {
            if(ot == _orientation)
            {
                return true;
            }
        }
    }
    return false;
}

bool InsertLibrary::doesMatchLibraryStats(bool seq1Reversed, bool seq2Reversed, sMDInt distance1to2, FileSetId FSID)
{
    //-----
    // use this when we haven't got the fsid from the generic node itself
    //
    if(FSID == _fsid)
        return doesMatchLibraryStats(seq1Reversed, seq2Reversed, distance1to2);
    return false;
}

void InsertLibrary::printContents(void)
{
    // print the stuff to std::out
    std::cout << "M: " << _mean << " St: " << _stdev << " O: " << _orientation << " Nr: " << _numReads << " B: " << _beta << " Up: " << _upperCutOff << " L: " << _lowerCutOff << " R: "<< _sizeRank << std::endl;
}

void InsertLibrary::logContents(int logLevel)
{
    // print the stuff to the logfile
    logInfo ("M: " << _mean << " St: " << _stdev << " O: " << _orientation << " Nr: " << _numReads << " B: " << _beta << " Up: " << _upperCutOff << " L: " << _lowerCutOff , logLevel);
}

void InsertLibrary::save(std::string filesetName, std::string fileName)
{
    std::ofstream myFile(fileName.c_str(), std::ofstream::binary);

    // save the important member vars
    myFile << "M:" << _mean << std::endl;
    myFile << "S:" << _stdev << std::endl;
    myFile << "U:" << _upperCutOff << std::endl;
    myFile << "L:" << _lowerCutOff << std::endl;
    myFile << "N:" << _numReads << std::endl;
    myFile << "O:" << _orientation << std::endl;
    
#if 0
    myFile.write(reinterpret_cast<char *>(&_mean), sizeof(double));
    myFile.write(reinterpret_cast<char *>(&_stdev), sizeof(double));
    myFile.write(reinterpret_cast<char *>(&_upperCutOff), sizeof(uMDInt));
    myFile.write(reinterpret_cast<char *>(&_lowerCutOff), sizeof(uMDInt));
    myFile.write(reinterpret_cast<char *>(&_numReads), sizeof(int));
    myFile.write(reinterpret_cast<char *>(&_orientation), sizeof(OrientationType));
#endif
    // close the file
    myFile.close();
}

void InsertLibrary::load(std::string filesetName, std::string fileName)
{
    //-----
    // load the insert libs tuff from file.
    //
    // File looks like:
    //
    // #Comment1
    // M:179.56
    // S:14.6785
    // U:201
    // L:140
    // N:29937746
    // O:2
    //
    std::ifstream saved_fh(fileName.c_str(), std::ifstream::binary);
    std::string line;
    if (saved_fh.is_open())
    {
        while (! saved_fh.eof() )
        {
            if(getline (saved_fh,line))
            {
                switch(line[0])
                {
                    case '#':
                    case '\n':
                        break;
                    case 'M':
                    {
                        from_string<double>(_mean, line.substr(2), std::dec);
                        break;
                    }
                    case 'S':
                    {
                        from_string<double>(_stdev, line.substr(2), std::dec);
                        break;
                    }
                    case 'U':
                    {
                        from_string<uMDInt>(_upperCutOff, line.substr(2), std::dec);
                        break;
                    }
                    case 'L':
                    {
                        from_string<uMDInt>(_lowerCutOff, line.substr(2), std::dec);
                        break;
                    }
                    case 'N':
                    {
                        from_string<int>(_numReads, line.substr(2), std::dec);
                        break;
                    }
                    case 'O':
                    {
                        int tmp_ot;
                        from_string<int>(tmp_ot, line.substr(2), std::dec);
                        switch(tmp_ot)
                        {
                            case 0:
                                _orientation = InsertLibrary::TYPE_DD;
                                break;
                            case 2:
                                _orientation = InsertLibrary::TYPE_AD;
                                break;
                            case 3:
                            case 1:
                                _orientation = InsertLibrary::TYPE_AA;
                                break;
                            default:
                                logError("Invalid OT");
                        }
                        break;
                    }
                    default:
                        logError("Unknown symbol in insert file: [" << line <<"]");
                    }
            }
        }
        saved_fh.close();
    }
#if 0
    std::cout << filesetName << std::endl;
    std::ifstream myFile(fileName.c_str(), std::ifstream::binary);

    // load the important member vars
    myFile.read(reinterpret_cast<char *>(&_mean), sizeof(double));
    myFile.read(reinterpret_cast<char *>(&_stdev), sizeof(double));
    myFile.read(reinterpret_cast<char *>(&_upperCutOff), sizeof(uMDInt));
    myFile.read(reinterpret_cast<char *>(&_lowerCutOff), sizeof(uMDInt));
    myFile.read(reinterpret_cast<char *>(&_numReads), sizeof(int));
    myFile.read(reinterpret_cast<char *>(&_orientation), sizeof(OrientationType));

    // close the file
    myFile.close();
#endif
}

std::string InsertLibrary::ContentsAsString(void)
{
    std::stringstream ss;
    ss << "M:" << _mean << "  St:" << _stdev << "  O:" << _orientation << "  Nr:" << _numReads;
    return ss.str();
}

std::string InsertLibrary::sayOrientLikeAHuman(OrientationType ot)
{
    //-----
    // output the orientation as a user readable string
    //
    switch(ot)
    {
        case TYPE_DD:
            return "DD - 0";
            break;
//        case TYPE_DA:
//            return "DA - 1";
//            break;
        case TYPE_AD:
            return "AD - 2";
            break;
        case TYPE_AA:
            return "AA - 3";
            break;
        case TYPE_UNSET:
        default:
            return "US - 4";
            break;
    }   
    return "UKNOWN";
}

bool InsertLibrary::FillFromString(std::string str)
{
    int valsRead;
    int orientation;
    valsRead = sscanf(str.c_str(),"M:%lg  St:%lg  O:%d  Nr:%d",&_mean,&_stdev,&orientation,&_numReads);
    _orientation = (InsertLibrary::OrientationType)orientation;
    return (valsRead == 4);
}

bool InsertLibrary::setString(ostringstream * oss)
{
    //-----
    // Save the contents to a string stream instead of a file
    //
    oss->write(reinterpret_cast<char *>(&_mean), sizeof(double));
    oss->write(reinterpret_cast<char *>(&_stdev), sizeof(double));
    oss->write(reinterpret_cast<char *>(&_upperCutOff), sizeof(uMDInt));
    oss->write(reinterpret_cast<char *>(&_lowerCutOff), sizeof(uMDInt));
    oss->write(reinterpret_cast<char *>(&_numReads), sizeof(int));
    oss->write(reinterpret_cast<char *>(&_orientation), sizeof(OrientationType));
    return true;
}

bool InsertLibrary::resetFromString(istringstream * iss)
{
    //-----
    // Load the contents from a string stream instead of a file
    //
    iss->read(reinterpret_cast<char *>(&_mean), sizeof(double));
    iss->read(reinterpret_cast<char *>(&_stdev), sizeof(double));
    iss->read(reinterpret_cast<char *>(&_upperCutOff), sizeof(uMDInt));
    iss->read(reinterpret_cast<char *>(&_lowerCutOff), sizeof(uMDInt));
    iss->read(reinterpret_cast<char *>(&_numReads), sizeof(int));
    iss->read(reinterpret_cast<char *>(&_orientation), sizeof(OrientationType));
    return true;
}

/******************************************************
 ** File-set handling
 ******************************************************/

bool FileSet_FormatTestingCallback( std::map<std::string,std::string> recordInfo, void *callbackdata)
{
    // if we get to this function it means that the regular expression in the
    // format file worked, therefore the format is very probably correct.
    
    // We'll just save the recordInfo in the callback data
    std::map<std::string,std::string> * pCopyOfRecordData = (std::map<std::string,std::string> *)callbackdata;
    *pCopyOfRecordData = recordInfo;
    
    return false; // this will break the loop after the first record.
}

FileSet::FileSet()
{
    _filled = false;
}

FileSet::~FileSet()
{
}

// access
InsertLibrary * FileSet::getLongestInsLib(void)
{
    //-----
    // get the longest insert library for this fileSet
    //
    InsertLibrary * ret_ins = NULL;
    double max_mean = 0;
    
    std::vector<InsertLibrary>::iterator ins_iter = _insertLibs.begin();
    while(ins_iter != _insertLibs.end())
    {
        if(ins_iter->_mean > max_mean)
        {
            max_mean = ins_iter->_mean;
            ret_ins = &(*ins_iter);
        }
        ins_iter++;
    }
    return ret_ins;
}

bool FileSet::FillFrom_User( std::istream& in, std::ostream& out)
{
    if (_filled == true) {
        // can't fill it again
        return _filled;
    }

    int formatChoice;
    std::map<std::string,std::map<std::string,std::string> > formats;
    std::vector<std::string> choices;
    std::string unknownStr = "Unknown file format";
    std::string choiceStr;

    //------------------------------------------//
    //----- Ask user for the file format -------//

    // get the file defs
    formats = FormatDefFiles::GetList();

    // create the menu choices
    std::map<std::string,std::map<std::string,std::string> >::iterator mapit;
    for (mapit=formats.begin(); mapit!=formats.end(); mapit++) {
        choices.push_back(mapit->first);
    }
    choices.push_back(unknownStr);

    out << "NOTE: If you get an error saying the format does not match when you are sure it does\nthen chances are you will need to convert the file from windows style newlines to dos" << std::endl;
    
    // get the choice
    formatChoice = GetMenuChoice(in,out,"\nPlease choose the format of the file-set:",choices,"Choice: ","**Error**: Try again.");
    choiceStr = choices[formatChoice];

    // get the choice
    if ( formatChoice < (int)(choices.size()-1) ) {

        std::map<std::string,std::string> formatVars = formats[choiceStr];

        // set the format name
        _formatName = choiceStr;

        //------------------------------------//
        //----- Ask user for the files -------//
        unsigned int numLines = 0;
        unsigned int numFiles;
        from_string<unsigned int>(numFiles,formatVars["number-of-files"], std::dec);

        out << std::endl;
        out << "** All files must be in the same directory." << std::endl;
        out << "** All files must have the same number of lines." << std::endl;
        out << std::endl;

        while (_filesInFileSet.size() != numFiles) {

            std::stringstream str, tempstr;
            tempstr << "file-" << (_filesInFileSet.size()+1) << "-label";
            str << "Enter file " << (_filesInFileSet.size()+1) << " (" << formatVars[tempstr.str()] << " file): ";
            std::string enteredStr = 
                GetFilename( 
                    in, out,
                    str.str().c_str(),
                    "**Error**: Please choose an accessable file");

            // do some stuff
            if ( _filesInFileSet.size() == 0 ) {
                _filesetDirectory = ExtractDirectory(enteredStr);
                numLines = LinesInFile(enteredStr);
            }
            else {
                // check if they are in the same directory
                if ( _filesetDirectory != ExtractDirectory(enteredStr) ) {
                    out << "**Error**: Files must be in the same directory" << std::endl;
                    continue;
                }
                // check number of lines is the same
                if ( numLines != LinesInFile(enteredStr) ) {
                    out << "**Error**: Files must have the same number of lines" << std::endl;
                    continue;
                }
            }

            // add filename to list
            _filesInFileSet.push_back( ExtractFilename( enteredStr ) );
        }

        // FileSet name
        _filesetName = _filesInFileSet[0];

        // make a descriptor file name based on the common characters in the file path
        _filesetFilename = MakeFilesetFilename();
        
        //------------------------------------------//
        // We've filled out enough fields to test if
        // the format is valid and we can attempt to get
        // the read length.
        //
        // Let's try to parse the files with the format
        // chosen
        FormattedFileReader ffr;
        std::map<std::string,std::string> copyOfRecordData;
        bool ffrRetVal = ffr.LoadFileset( (*this), FileSet_FormatTestingCallback, &copyOfRecordData);
        if ( ffrRetVal == false ) {
            out << "**Error**: The files chosen do not match the format chosen. Please start again." << std::endl;
            _filled = false;
            return false;
        }
        // if we got here the format is a match for the filees chosen and 'copyOfRecordData'
        // has a copy of the first record

        //------------------------------------------//
        //----- Estimate the number of reads -------//
        unsigned int linesPerRecord;
        from_string<unsigned int>(linesPerRecord,formatVars["lines-per-record"], std::dec);
        _numReads = (numLines/linesPerRecord);


        //--------------------------------------------------------//
        //----- Get the read length from the parsed record -------//
        _readLen = copyOfRecordData["sequence-data"].size();


        //--------------------------------------//
        //----- Ask user for trim length -------//
        _trimLen = GetRangedValueFromUser<uint_16t>( 
                in, out, 
                "\nNumber of bases to trim from end of read: [0 for none] ", 
                "**Error**: Value not valid", 
                0, _readLen-1);

        //---------------------------------------//
        //----- Ask user for insert sizes -------//
        uint_16t insSize;
        uint_16t oType;
        out << std::endl;
        while (true) {

            std::stringstream str;
            str << "Estimated library insert size #" << (_insertLibs.size()+1) << ": [0 to stop] ";
            insSize = GetFromUser<uint_16t>( 
                                in, out, 
                                str.str().c_str(), 
                                "**Error**: Value not valid" );
            if ( insSize == 0 ) {
                if (_insertLibs.size() == 0) {
                    out << "**Error**: At least one library insert size must be given" << std::endl;
                    continue;
                }
                break;
            }
            str << "\nOrientation code for insert #" << (_insertLibs.size()+1) << "\n"
                    << "0  <---  --->  \n"
                    << "1  <---  <---  \n"
                    << "2  --->  <---  \n"
                    << "3  --->  --->  \n: ";
            oType = GetFromUser<uint_16t>(
                                in, out,
                                str.str().c_str(),
                                "**Error**: Value not valid" );

            InsertLibrary inslib( static_cast<double>(insSize), static_cast<InsertLibrary::OrientationType>(oType));
            _insertLibs.push_back( inslib );
        }
        out << std::endl;

        _filled = true;
    }
    else { // check for unknown 
        out << std::endl << "Cannot determine format... no file-set created" << std::endl;
    }

    return _filled;
}

bool FileSet::FillFrom_File( std::string filename )
{
    if (_filled == true) {
        // can't fill it again
        return _filled;
    }
    
    bool isDir;
    if ((!IsDirectory(filename,isDir)) || (isDir)){
        std::cout << "Error: not a file `" << filename << "'" << std::endl;
        return false;
    }

    // open the file
    std::ifstream fsfile(filename.c_str());
    if ( fsfile.is_open() ) {

        std::stringstream data;

#if SAS_USE_CS
        // import file
        unsigned int crc = 0x01;
        std::string inputline;
        while (!fsfile.eof()) {
            getline(fsfile, inputline);
            if (inputline.substr(0,2) == "--") {
                std::stringstream crcstr(inputline.substr(2,8));
                if ( crcstr >> std::setw(8) >> std::setbase(16) >> crc ) {
                    CheckSum cs;
                    cs.add(data.str().c_str());
                    crc = crc ^ cs.get();
                }
                break;
            }
            else {
                data << inputline << std::endl;
            }
        }
        fsfile.close();

        // check checksum value
        if ( crc != 0x0000000 ) {
            std::cout << "Error: load failed: bad checksum" << std::endl;
            return false;
        }

#else
        std::string inputline;
        while (!fsfile.eof()) {
            getline(fsfile, inputline);
            if (inputline.substr(0,2) == "--") {
                break;
            }
            else {
                data << inputline << std::endl;
            }
        }
        fsfile.close();
#endif
        // read the data
#define FSBITMASK(fLD) (0x01<<fLD)
        enum FileSetFields {
            FSF_NAME,
            FSF_FORMAT,
            FSF_FILES,
            FSF_NUMREADS,
            FSF_READLEN,
            FSF_TRIMLEN,
            FSF_INSERTLIBS,
            FSF__LAST_FIELD_POS
        };
        uint_32t fieldFilled = 0x00;

        std::string fieldname, fielddata;
        while (!data.eof()) {
            getline(data, inputline);

            // split into field name and data
            size_t colonpos = inputline.find(':');
            if ( colonpos == std::string::npos ) continue;

            fieldname = inputline.substr(0,colonpos);
            fielddata = inputline.substr(colonpos+1);

            if (fieldname == "name") {
                if (!(fieldFilled & FSBITMASK(FSF_NAME)) ) {
                    _filesetName = fielddata;
                    fieldFilled |= FSBITMASK(FSF_NAME);
                }
            }
            else if (fieldname == "format") {
                if (!(fieldFilled & FSBITMASK(FSF_FORMAT)) ) {
                    _formatName = fielddata;
                    fieldFilled |= FSBITMASK(FSF_FORMAT);
                }
            }
            else if (fieldname == "file_names") {
                if (!(fieldFilled & FSBITMASK(FSF_FILES)) ) {
                    size_t commapos;
                    while ( (commapos = fielddata.find(',')) != std::string::npos ) {
                        _filesInFileSet.push_back(fielddata.substr(0,commapos));
                        fielddata.erase(0,commapos+1);
                    }
                    _filesInFileSet.push_back(fielddata);
                    fieldFilled |= FSBITMASK(FSF_FILES);
                }
            }
            else if (fieldname == "num_reads") {
                if (!(fieldFilled & FSBITMASK(FSF_NUMREADS)) ) {
                    from_string(_numReads, fielddata, std::dec);
                    fieldFilled |= FSBITMASK(FSF_NUMREADS);
                }
            }
            else if (fieldname == "read_length") {
                if (!(fieldFilled & FSBITMASK(FSF_READLEN)) ) {
                    from_string(_readLen, fielddata, std::dec);
                    fieldFilled |= FSBITMASK(FSF_READLEN);
                }
            }
            else if (fieldname == "trim_length") {
                if (!(fieldFilled & FSBITMASK(FSF_TRIMLEN)) ) {
                    from_string(_trimLen, fielddata, std::dec);
                    fieldFilled |= FSBITMASK(FSF_TRIMLEN);
                }
            }
            else if (fieldname == "insert_lib" ) {
                fieldFilled |= FSBITMASK(FSF_INSERTLIBS);
                InsertLibrary insLib(0);
                if (true == insLib.FillFromString(fielddata) ) {
                    _insertLibs.push_back(insLib);
                }
            }
        }

#define FIELDMISSING(fLD) ((!(fieldFilled & FSBITMASK(fLD)))?#fLD ",":"")

        // check all fields filled
        if ( fieldFilled != (FSBITMASK(FSF__LAST_FIELD_POS)-1) ) {
            
            std::cout << "(" << fieldFilled << "," << (FSBITMASK(FSF__LAST_FIELD_POS)-1) << "," << FSBITMASK(FSF_INSERTLIBS) << ")" << std::endl;

            std::cout << "Error: load failed: missing fields: "
                    << FIELDMISSING(FSF_NAME)
                    << FIELDMISSING(FSF_FORMAT)
                    << FIELDMISSING(FSF_FILES)
                    << FIELDMISSING(FSF_NUMREADS)
                    << FIELDMISSING(FSF_READLEN)
                    << FIELDMISSING(FSF_TRIMLEN)
                    << FIELDMISSING(FSF_INSERTLIBS)
                    << std::endl;
            return false;
        }

        // everything is fine so populate the file and dir
        _filesetDirectory = ExtractDirectory(filename);
        _filesetFilename  = ExtractFilename(filename);

        _filled = true;
    }

    // make sure all the FSIDs are ok for the insert libs
    fixInsFSIDs();

    return _filled;
}

bool FileSet::SaveTo_File( void )
{
    return SaveTo_File(this->GetFullPathname());
}

bool FileSet::SaveTo_File( std::string filename )
{
    if ( _filled == false ) {
        // must be filled to save
        return false;
    }
    //-----------------------------------------------//
    //----- save information in FileSet.sassy -------//

    std::stringstream FileSetDescData;
    FileSetDescData << "name:" << _filesetName << std::endl;
    FileSetDescData << "format:" << _formatName << std::endl;
    FileSetDescData << "file_names:" << str_join(_filesInFileSet,std::string(",")) << std::endl;
    FileSetDescData << "num_reads:" << _numReads << std::endl;
    FileSetDescData << "read_length:" << _readLen << std::endl;
    FileSetDescData << "trim_length:" << _trimLen << std::endl;

    V_FOR_EACH( _insertLibs, InsertLibrary, it) {
        FileSetDescData << "insert_lib:" << it->ContentsAsString() << std::endl;
    }

    std::ofstream fsfile;
    fsfile.open( filename.c_str() );

    if ( fsfile.is_open() ) {

        // write the data
        fsfile << FileSetDescData.str();

#if SAS_USE_CS
    // get a checksum
        CheckSum cs;
        cs.add(FileSetDescData.str().c_str());

        // write the checksum
        fsfile << "--" << std::setw(8) << std::setfill('0') << std::setbase(16) << cs.get() << "--" << std::endl;
#else
        fsfile << "--" << std::endl;
#endif
        
        fsfile.close();

        return true; // success
    }

    return false; // failed
}
void FileSet::addInsLib(InsertLibrary newLib)
{
    //-----
    // we need to insert this guy such that the means will be in in ascending order
    //
    bool added = false;
    V_FOR_EACH( _insertLibs, InsertLibrary, ins_iter)
    {
        if(ins_iter->_mean >= newLib._mean)
        {
            _insertLibs.insert(ins_iter, newLib);
            added = true;
            break;
        }
    }
    if(!added)
        _insertLibs.push_back(newLib);

    // set the fsid for this guy
    newLib._fsid = _fsid;
}

void FileSet::clearAllInsLibs(void)
{
    _insertLibs.clear();
}

std::vector<InsertLibrary> * FileSet::getInsLibs(void)
{
    return &_insertLibs;
}

void FileSet::fixInsFSIDs(void)
{
    //-----
    // make sure all the insert libraries know which
    // fsid they belong to
    //
    V_FOR_EACH( _insertLibs, InsertLibrary, ins_iter)
    {
        ins_iter->_fsid = _fsid;
    }
}

bool FileSet::binInsert(bool seq1Reversed, bool seq2Reversed, sMDInt distance1to2)
{
    // put the insert into the right bin
    //std::cout << seq1Reversed << ", " << seq2Reversed << ", " << distance1to2 << " O: " << calcOrientationType(seq1Reversed, seq2Reversed, distance1to2) << std::endl;
    V_FOR_EACH( _insertLibs, InsertLibrary, ins_iter)
    {
        if(ins_iter->doesMatchLibraryStats(seq1Reversed, seq2Reversed, distance1to2))
        {
            // put it in this bin
            ins_iter->_insertBin.push_back(((uMDInt)fAbs(distance1to2)));
            return true;
        }
    }
    return false;
}

void FileSet::flushBins(void)
{
    //-----
    // clear all the insert library bins
    //
    V_FOR_EACH( _insertLibs, InsertLibrary, ins_iter)
    {
        ins_iter->_insertBin.clear();
    }   
}

InsertLibrary::OrientationType FileSet::calcOrientationType(bool seq1Reversed, bool seq2Reversed, sMDInt distance1to2)
{
    //-----
    // get the orientation type for a paired read
    //
    // every fileset has at least 1 insert library, just ask him!
    //
    return _insertLibs.begin()->calcOrientationType(seq1Reversed, seq2Reversed, distance1to2);
}

void FileSet::printInsertStats(void)
{
    std::cout << "Insert stats for File: " << _fsid << std::endl;
    V_FOR_EACH( _insertLibs, InsertLibrary, ins_iter)
    {
        ins_iter->printContents();
    }
    std::cout << "---" << std::endl;
}

void FileSet::logInsertStats(int logLevel)
{
    logInfo("Insert stats for File: " << _fsid , logLevel);
    V_FOR_EACH( _insertLibs, InsertLibrary, ins_iter)
    {
        ins_iter->logContents(logLevel);
    }
}

void FileSet::logAll(void)
{
    logInfo("FileSET LOGALL", 1);
    logInfo("FSID: " << _fsid, 1);
    logInfo("Filled: " << _filled, 1);
    logInfo("FsetName: " << _filesetName, 1);
    logInfo("FmtName: " << _formatName, 1);
    logInfo("numReads: " <<  _numReads, 1);
    logInfo("RL: " << _readLen, 1);
    logInfo("TL: " << _trimLen, 1);
    logInfo("FsetDir: " << _filesetDirectory, 1);
    logInfo("FsetFname: " << _filesetFilename, 1);
    V_FOR_EACH( _insertLibs, InsertLibrary, ins_iter)
    {
        ins_iter->logContents(1);
    }

    logInfo(">>>FileSET LOGALL", 1); 
}
void FileSet::save(std::string fileName)
{
    std::ofstream myFile(fileName.c_str(), std::ofstream::binary);

    // strings first
    //    myFile << _filesetName << "\n";
    //    myFile << _formatName << "\n";
    
    // save the important member vars
    myFile.write(reinterpret_cast<char *>(&_fsid), sizeof(FileSetId));
    //myFile.write(reinterpret_cast<char *>(&_filled), sizeof(bool));
    myFile.write(reinterpret_cast<char *>(&_readLen), sizeof(uint_16t));
    myFile.write(reinterpret_cast<char *>(&_trimLen), sizeof(uint_16t));
    myFile.write(reinterpret_cast<char *>(&_numReads), sizeof(uMDInt));
        
    int num_ins_libs = _insertLibs.size();
    myFile.write(reinterpret_cast<char *>(&num_ins_libs), sizeof(int));

    // close the file
    myFile.close();

    // save the insert libraries
    int save_string_int = 0;
    std::string save_string = "";
    V_FOR_EACH( _insertLibs, InsertLibrary, all_ins_iter)
    {
        // make the file name
        stringstream ss;
        ss << fileName << ".InsertLibrary_" << save_string_int;
        ss >> save_string;

        // now save it.
        all_ins_iter->save(_filesetFilename, save_string);
        save_string_int++;
    }

}

void FileSet::load(std::string fileName)
{
    std::ifstream myFile(fileName.c_str(), std::ifstream::binary);

    // load strings first
    //    char k; 
    //  myFile >> _filesetName;
    //myFile.get(k);
    //    myFile >> _formatName;
    //  myFile.get(k);
    
    //logInfo(_filesetName,1);
    //logInfo(_formatName,1);
 
    // save the important member vars
    myFile.read(reinterpret_cast<char *>(&_fsid), sizeof(FileSetId));
    //myFile.read(reinterpret_cast<char *>(&_filled), sizeof(bool));
    myFile.read(reinterpret_cast<char *>(&_readLen), sizeof(uint_16t));
    myFile.read(reinterpret_cast<char *>(&_trimLen), sizeof(uint_16t));
    myFile.read(reinterpret_cast<char *>(&_numReads), sizeof(uMDInt));
    
    int num_ins_libs = 0;
    myFile.read(reinterpret_cast<char *>(&num_ins_libs), sizeof(int));

    // close the file
    myFile.close();

    // load the insert libraries
    std::string load_string = "";
    for(int i = 0; i < num_ins_libs; i++)
    {
        stringstream ss;
        ss << fileName << ".InsertLibrary_" << i;
        ss >> load_string;

        // make a new insert library and load the values
        InsertLibrary tmp_lib(0);
        tmp_lib.load(_filesetFilename, load_string);

        // add it to the FileSet
        tmp_lib._fsid = _fsid;
        addInsLib(tmp_lib);
    }
}

bool FileSet::setString(ostringstream * oss)
{
    //-----
    // Save the contents to a string stream instead of a file
    //
    V_FOR_EACH( _insertLibs, InsertLibrary, all_ins_iter)
    {
        all_ins_iter->setString(oss);
    }
    return true;
}

bool FileSet::resetFromString(istringstream * iss)
{
    //-----
    // Load the contents from a string stream instead of a file
    //
    V_FOR_EACH( _insertLibs, InsertLibrary, all_ins_iter)
    {
        all_ins_iter->resetFromString(iss);
    }
    return true;
}

/******************************************************
 ** Data-set handling
 ******************************************************/

DataSet::DataSet()
{
    _totalNumReads = 0;
    _determinedReadLength = 1000;
    _externalReadLength = (uint_16t)INT_MAX;
    _ignoreTrap = SAS_DEF_IGNORE_TRAP;
}

DataSet::~DataSet()
{
    //-----
    // destructor!
    //
    // we need to take care of the ranked inserts.
    std::map<int, std::vector<InsertLibrary *> * >::iterator ri_iter = _rankedInserts.begin();
    std::map<int, std::vector<InsertLibrary *> * >::iterator ri_last = _rankedInserts.end();
    while(ri_iter != ri_last)
    {
        if(ri_iter->second != NULL)
            delete ri_iter->second;
        ri_iter++;
    }
    _rankedInserts.clear();
}

uint_16t DataSet::calculateReadLength(bool optimiseForArch, unsigned int basesPerInt)
{
    // find the minimum trusted read length
    _determinedReadLength = _fileSets[0].getReadLength();
    
    V_FOR_EACH(_fileSets, FileSet, fsIter) {

        // get trusted read length for this FileSet
        uint_16t trustedReadLen = fsIter->getReadLength() - fsIter->getTrimLength();
        _determinedReadLength = min( _determinedReadLength, trustedReadLen );
    }

    // see if the user wants this even shorter
    _determinedReadLength = min( _determinedReadLength, _externalReadLength);
    
    // make sure the determinedReadLength is odd
    if ( !(_determinedReadLength & (0x01)) ) {
        _determinedReadLength--;
    }

    // check for optimisation for the architecture
    if ( optimiseForArch == true ) {
        _determinedReadLength = ((_determinedReadLength / basesPerInt) * basesPerInt) - 1;
    }
    
    return getReadLength();
}

uint_16t DataSet::getReadLength(void)
{
    return _determinedReadLength;
}

uMDInt DataSet::getTotalNumReads(void)
{
    return _totalNumReads;
}

void DataSet::addFileSet(FileSet newSet, uMDInt maxReadsSmallMode)
{
    newSet.setFSID(_fileSets.size());
    _fileSets.push_back(newSet);
    
    // do small mode and max number of reads stuff
    if ( maxReadsSmallMode == 0 ) {
        maxReadsSmallMode = newSet.getNumReads(); // zero means maximum
    }
    else {
        maxReadsSmallMode = min(maxReadsSmallMode, newSet.getNumReads());
    }

    // make sure all the insert libraries know 
    // which fsid they have...
    newSet.fixInsFSIDs();
    
    // add to vector and to total
    _fileSetMaxReads.push_back(maxReadsSmallMode);
    _totalNumReads += maxReadsSmallMode;
}

void DataSet::clearAllFileSets(void)
{
    _fileSets.clear();
    _fileSetMaxReads.clear();
    _totalNumReads = 0;
}

std::vector<FileSet> * DataSet::getFileSets(void)
{
    return &_fileSets;
}

int DataSet::getNumInsertLibs(void)
{
    //-----
    // how many insert libraries are there in total?
    //
    int num_libs = 0;
    std::vector<InsertLibrary> * tmp_vec;
    V_FOR_EACH( _fileSets, FileSet, fs_iter) 
    {
        tmp_vec = getInsLibs(fs_iter->getFSID());
        num_libs += tmp_vec->size();
    }
    return num_libs;
}

std::vector<InsertLibrary> * DataSet::getInsLibs(FileSetId FSID)
{
    std::vector<FileSet>::iterator target_fs = _fileSets.begin();
    target_fs += FSID;
    return target_fs->getInsLibs();
}

std::vector<InsertLibrary *> * DataSet::getInsertLib(int rank)
{
    //-----
    // get the nth ranked insert library
    // don't stuff it up...
    //
    std::map<int, std::vector<InsertLibrary *> * >::iterator finder = _rankedInserts.find(rank);
    if(finder != _rankedInserts.end())
        return finder->second;
    return NULL;
    
}

FileSet * DataSet::getFileSet(FileSetId FSID)
{
    std::vector<FileSet>::iterator target_fs = _fileSets.begin();
    target_fs += FSID;
    return &(*target_fs);
}

uMDInt DataSet::getMaxReadsForFileSet(FileSetId FSID)
{
    std::vector<uMDInt>::iterator it = _fileSetMaxReads.begin();
    it += FSID;
    return (*it);
}

bool DataSet::getSortedBins(std::vector<std::vector<uMDInt> * > * ret_vector, std::vector<double> * means_vector)
{
    //-----
    // get all the insert bins from all the file sets and make sure they're sorted
    // in descending order of mean insert size and also a few caveats...
    //
    // we will pass back pointers to the actual arrays

    // clear the retun vector just in case...
    ret_vector->clear();
    means_vector->clear();

    // for dealing with the insert library
    std::vector<InsertLibrary> * insert_vec;

    // for inserting in a sorted fashion
    std::vector<double>::iterator means_iter;

    // go through all the filesets            
    V_FOR_EACH( _fileSets, FileSet, fs_iter)
    {
        insert_vec = getInsLibs(fs_iter->getFSID());

        V_FOR_EACH( (*insert_vec), InsertLibrary, insert_vec_iter)
        {
            // now we insert into the right place
            bool added = false;
            means_iter = means_vector->begin();

            V_FOR_EACH( (*ret_vector), std::vector<uMDInt>*, ret_iter)
            {
                if(*means_iter < insert_vec_iter->_mean)
                {
                    ret_vector->insert(ret_iter, &(insert_vec_iter->_insertBin));
                    means_vector->insert(means_iter, insert_vec_iter->_mean);
                    added = true;
                    break;
                }
                means_iter++;
            }
            if(!added)
            {
                // push these guys onto the list
                ret_vector->push_back(&(insert_vec_iter->_insertBin));
                means_vector->push_back(insert_vec_iter->_mean);
            }
        }
    }
    return true;
}

    static std::vector<InsertLibrary *> DSG_sorted_insert_vec;
    static std::vector<InsertLibrary *>::iterator DSG_sorted_insert_iter;
bool DataSet::rankInsertLibs(void)
{
    //-----
    // rank all the insert libs by their _means
    // uses a kind of insertion sort algorithm
    //

    double upper_window_multiplier = 2;

    // first we need to "unrank" any previous rankings
    std::map<int, std::vector<InsertLibrary *> * >::iterator ri_iter = _rankedInserts.begin();
    std::map<int, std::vector<InsertLibrary *> * >::iterator ri_last = _rankedInserts.end();
    while(ri_iter != ri_last)
    {
        if(ri_iter->second != NULL)
            delete ri_iter->second;
        ri_iter++;
    }
    _rankedInserts.clear();
    
    // for dealing with the insert libraries of a FS
    std::vector<InsertLibrary> * insert_vec;
    
    // for storing ins_libs in a sorted manner
    DSG_sorted_insert_vec.clear();

    // go through all the filesets
    // and sort the insertlibraries by their insert sizes
    // first we sort in ascending order...
    V_FOR_EACH( _fileSets, FileSet, fs_iter)
    {
        insert_vec = getInsLibs(fs_iter->getFSID());

        // and then through each filesets insert libraries
        V_FOR_EACH( (*insert_vec), InsertLibrary, insert_vec_iter)
        {
            // now we insert into the right place
            bool added = false;
            DSG_sorted_insert_iter = DSG_sorted_insert_vec.begin();
            while(DSG_sorted_insert_iter != DSG_sorted_insert_vec.end())
            {
                if((*DSG_sorted_insert_iter)->_mean > insert_vec_iter->_mean)
                {
                    DSG_sorted_insert_vec.insert(DSG_sorted_insert_iter, (&(*insert_vec_iter)));
                    added = true;
                    break;
                }
                DSG_sorted_insert_iter++;
            }
            if(!added)
            {
                // push these guys onto the list
                DSG_sorted_insert_vec.push_back(&(*insert_vec_iter));
            }
        }
    }

    // at this stage the ranks are in ascending order
    // we would like to group them together in to rank "bands", guys with the "same" insert sizes
    // to do this we say that starting from the smallest guy we grab all the guys within
    // upper_window_multiplier standard deviations of its mean and say they're the same.
    // Then we move onto the next largest and so on.
    DSG_sorted_insert_iter = DSG_sorted_insert_vec.begin();
    int rank = 1;
    // get the first "upper_window"
    double upper_window = (*DSG_sorted_insert_iter)->_mean + (*DSG_sorted_insert_iter)->_stdev * upper_window_multiplier;
    while(DSG_sorted_insert_iter != DSG_sorted_insert_vec.end())
    {
        // set the rank for the InsertLibrary if its mean is below the upper_window
        if((*DSG_sorted_insert_iter)->_mean <= upper_window)
        {
            (*DSG_sorted_insert_iter)->_sizeRank = rank;
        }
        else
        {
            // this guy is larger than the upper window
            // time to make a new upper window
            upper_window = (*DSG_sorted_insert_iter)->_mean + (*DSG_sorted_insert_iter)->_stdev * upper_window_multiplier;
            rank++;
            (*DSG_sorted_insert_iter)->_sizeRank = rank;
        }
        DSG_sorted_insert_iter++;
    }

    // now go through the list again and put all the insert libraries of
    // the same rank into their own vectors
    logInfo("Total of: " << rank << " insert library rank bands", 6);
    int rank_reverser = 1;
    DSG_sorted_insert_iter = DSG_sorted_insert_vec.begin();
    _rankedInserts[rank] = new std::vector<InsertLibrary *>();
    while(DSG_sorted_insert_iter != DSG_sorted_insert_vec.end())
    {
        if((*DSG_sorted_insert_iter)->_sizeRank == rank_reverser)
        {
            // update the rank to reflect decending order
            (*DSG_sorted_insert_iter)->_sizeRank = rank;
            // put this guy onto the rank vector
            (_rankedInserts[rank])->push_back((*DSG_sorted_insert_iter));
            logInfo("InsertLibrary library with mean: " << (*DSG_sorted_insert_iter)->_mean << " put into rank band: " << rank, 4);
        }
        else
        {
            rank_reverser++;
            rank--;
            _rankedInserts[rank] = new std::vector<InsertLibrary *>();
            (*DSG_sorted_insert_iter)->_sizeRank = rank;
            (_rankedInserts[rank])->push_back((*DSG_sorted_insert_iter));
            logInfo("InsertLibrary library with mean: " << (*DSG_sorted_insert_iter)->_mean << " put into rank band: " << rank, 4);
            
        }
        DSG_sorted_insert_iter++;
    }
    return true;
}

    static std::vector<double> DSG_means_vector;
    static std::vector<double>::iterator DSG_means_iter;                   // for holding the means for each bin (sorted too)
    static std::vector<std::vector<uMDInt> * > DSG_bins_vector;            // for holding the binned inserts (sorted)
    static std::vector<std::vector<uMDInt> * >::iterator DSG_all_bins_iter;
    static std::vector<uMDInt>::iterator DSG_bin_iter;
bool DataSet::getBinStats(std::vector<double> * deviations)
{
    //-----
    // Calculate and return a vector of deviations from the means for the bins
    // stored in the insert libraries
    //
    
    // clear the vector just in case...
    deviations->clear();
    DSG_bins_vector.clear();
    DSG_means_vector.clear();
    
    if(getSortedBins(&DSG_bins_vector, &DSG_means_vector))
    {
        DSG_all_bins_iter = DSG_bins_vector.begin();
        DSG_means_iter = DSG_means_vector.begin();

        while(DSG_all_bins_iter != DSG_bins_vector.end())
        {
            // for each bin we'll work out the standard deviation
            double mean = *DSG_means_iter;
            double stdev = 0;
            bool procesed = false;
            
            DSG_bin_iter = (*DSG_all_bins_iter)->begin();
            while(DSG_bin_iter != (*DSG_all_bins_iter)->end())
            {
                procesed = true;
                stdev += pow(((double)(*DSG_bin_iter) - mean),2);
                DSG_bin_iter++;
            }
            if(procesed)
            {
                stdev /= (double)((*DSG_all_bins_iter)->size());
                stdev = sqrt(stdev);
                deviations->push_back(stdev);
            }
            else
                deviations->push_back(INT_MAX);
            // next
            DSG_means_iter++;
            DSG_all_bins_iter++;
        }
        return true;
    }
    else
        return false;
}

void DataSet::flushBins(void)
{
    //-----
    // clear all the insert library bins
    //
    std::vector<FileSet>::iterator fs_iter = _fileSets.begin();
    while(fs_iter != _fileSets.end())
    {
        fs_iter->flushBins();
        fs_iter++;
    }
}

void DataSet::setCutOffs(double range)
{
    //-----
    // update the cutoffs for the insert libraries
    //
    std::vector<InsertLibrary> * tmp_vec;
    std::vector<FileSet>::iterator fs_iter = _fileSets.begin();
    std::vector<InsertLibrary>::iterator tmp_vec_iter;
    while(fs_iter != _fileSets.end())
    {
        tmp_vec = getInsLibs(fs_iter->getFSID());
        tmp_vec_iter = tmp_vec->begin();
        while(tmp_vec_iter != tmp_vec->end())
        {
            tmp_vec_iter->makeCutOffs(range);
            tmp_vec_iter++;
        }
        fs_iter++;
    }
}

uMDInt DataSet::getLowestUpperCut(void)
{
    //-----
    // get the lowest of all the upper cutoss for this datset
    //
    uMDInt lowestUpper = INT_MAX;
    std::vector<InsertLibrary> * tmp_vec;
    std::vector<InsertLibrary>::iterator tmp_vec_iter;
    std::vector<FileSet>::iterator fs_iter = _fileSets.begin();
    while(fs_iter != _fileSets.end())
    {
        tmp_vec = getInsLibs(fs_iter->getFSID());
        tmp_vec_iter = tmp_vec->begin();
        while(tmp_vec_iter != tmp_vec->end())
        {
            if(tmp_vec_iter->_upperCutOff > _ignoreTrap)
            {
                if(tmp_vec_iter->_upperCutOff < lowestUpper)
                    lowestUpper = tmp_vec_iter->_upperCutOff;
            }
            tmp_vec_iter++;
        }
        fs_iter++;
    }
    return lowestUpper;
}

uMDInt DataSet::getLowestLowerCut(void)
{
    //-----
    // get the lowest of all the upper cutoss for this datset
    //
    uMDInt lowestLower = INT_MAX;
    std::vector<InsertLibrary> * tmp_vec;
    std::vector<InsertLibrary>::iterator tmp_vec_iter;
    std::vector<FileSet>::iterator fs_iter = _fileSets.begin();
    while(fs_iter != _fileSets.end())
    {
        tmp_vec = getInsLibs(fs_iter->getFSID());
        tmp_vec_iter = tmp_vec->begin();
        while(tmp_vec_iter != tmp_vec->end())
        {
            if(tmp_vec_iter->_lowerCutOff < lowestLower)
                lowestLower = tmp_vec_iter->_lowerCutOff;
            tmp_vec_iter++;
        }
        fs_iter++;
    }
    return lowestLower;
}

uMDInt DataSet::getHighestUpperCut(void)
{
    //-----
    // get the lowest of all the upper cutoss for this datset
    //
    uMDInt highestUpper = 0;
    std::vector<InsertLibrary> * tmp_vec;
    std::vector<InsertLibrary>::iterator tmp_vec_iter;
    std::vector<FileSet>::iterator fs_iter = _fileSets.begin();
    while(fs_iter != _fileSets.end())
    {
        tmp_vec = getInsLibs(fs_iter->getFSID());
        tmp_vec_iter = tmp_vec->begin();
        while(tmp_vec_iter != tmp_vec->end())
        {
            if(tmp_vec_iter->_upperCutOff > highestUpper)
                highestUpper = tmp_vec_iter->_upperCutOff;
            tmp_vec_iter++;
        }
        fs_iter++;
    }
    return highestUpper;
}

bool DataSet::getFileStats(std::map<FileSetId, 
                  InsertLibrary::OrientationType> * longestInserts, 
                  std::map<FileSetId, uMDInt> * upperCuts, 
                  std::map<FileSetId, uMDInt> * lowerCuts, 
                  std::map<FileSetId, uMDInt> * means)
{
    //-----
    // Used in the Scaffolder. Populate the maps with the appropriate values for
    // the largest InserLibrary
    //
    std::vector<FileSet>::iterator fs_iter = _fileSets.begin();
    while(fs_iter != _fileSets.end())
    {
        
        FileSetId fsid = fs_iter->getFSID();
        InsertLibrary * tmp_ins = fs_iter->getLongestInsLib();
        if(NULL != tmp_ins)
        {
            (*longestInserts)[fsid] = tmp_ins->_orientation;
            (*upperCuts)[fsid] = tmp_ins->_upperCutOff;
            (*lowerCuts)[fsid] = tmp_ins->_lowerCutOff;
            (*means)[fsid] = (uMDInt)(tmp_ins->_mean);
        }
        fs_iter++;
    }
    return true;
}
                  

void DataSet::printFileStats(void)
{
    std::vector<FileSet>::iterator fs_iter = _fileSets.begin();
    while(fs_iter != _fileSets.end())
    {
        fs_iter->printInsertStats();
        fs_iter++;
    }
    std::cout << "+++" << std::endl;
}

void DataSet::logAll(void)
{
    logInfo("DataSET LOGALL", 1);
    logInfo("FSS : " << _fileSets.size(), 1);
    logInfo("DRL : " << _determinedReadLength, 1);
    logInfo("ERL : " << _externalReadLength, 1);
    logInfo("TNR : " << _totalNumReads, 1);
    logInfo("LFN : " << _loadFileName, 1);
    std::vector<FileSet>::iterator fs_iter = _fileSets.begin();
    while(fs_iter != _fileSets.end())
    {
        fs_iter->logAll();
        fs_iter++;
    }
    logInfo(">>>DataSET LOGALL", 1);
}

void DataSet::logFileStats(int logLevel)
{
    std::vector<FileSet>::iterator fs_iter = _fileSets.begin();
    while(fs_iter != _fileSets.end())
    {
        fs_iter->logInsertStats(logLevel);
        fs_iter++;
    }
}

void DataSet::save(std::string fileName)
{
    std::ofstream myFile(fileName.c_str(), std::ofstream::binary);

    // save the important member vars
    int num_file_sets = _fileSets.size();
    myFile.write(reinterpret_cast<char *>(&num_file_sets), sizeof(int));
    myFile.write(reinterpret_cast<char *>(&_totalNumReads), sizeof(uMDInt));
    myFile.write(reinterpret_cast<char *>(&_determinedReadLength), sizeof(uint_16t));
    
    // close the file
    myFile.close();

    // save the insert libraries
    std::vector<FileSet>::iterator all_fs_iter = _fileSets.begin();
    int save_string_int = 0;
    while(all_fs_iter != _fileSets.end())
    {
        // make the file name
        stringstream ss;
        ss << fileName << ".FileSet_" << save_string_int;
        all_fs_iter->save(ss.str());
        all_fs_iter++;
        save_string_int++;
    }
}

void DataSet::load(std::string fileName)
{
    _loadFileName = fileName;
    
    std::ifstream myFile(fileName.c_str(), std::ifstream::binary);

    // save the important member vars
    int num_file_sets = 0;
    myFile.read(reinterpret_cast<char *>(&num_file_sets), sizeof(int));
    myFile.read(reinterpret_cast<char *>(&_totalNumReads), sizeof(uMDInt));
    myFile.read(reinterpret_cast<char *>(&_determinedReadLength), sizeof(uint_16t));

    uMDInt tnr = _totalNumReads;
    
    // close the file
    myFile.close();

    // load the insert libraries
    std::string load_string = "";
    for(int i = 0; i < num_file_sets; i++)
    {
        stringstream ss;
        ss << fileName << ".FileSet_" << i;
        ss >> load_string;

        // make a new insert library and load the values
        FileSet tmp_fs;
        tmp_fs.load(load_string);

        // add it to the fileset
        addFileSet(tmp_fs);
    }
    
    _totalNumReads = tnr ;
}

bool DataSet::setString(void)
{
    //-----
    // set the internal save string
    //
    ostringstream oss;
    
    // save the insert libraries via the filesets
    std::vector<FileSet>::iterator all_fs_iter = _fileSets.begin();
    while(all_fs_iter != _fileSets.end())
    {
        // make the file name
        all_fs_iter->setString(&oss);
        all_fs_iter++;
    }

    // set the internal string
    _internalSaveString = oss.str();
    return true;
}

bool DataSet::resetFromString(void)
{
    //-----
    // load the internal save string
    //
    istringstream iss(_internalSaveString);
    
    std::vector<FileSet>::iterator all_fs_iter = _fileSets.begin();
    while(all_fs_iter != _fileSets.end())
    {
        // make the file name
        all_fs_iter->resetFromString(&iss);
        all_fs_iter++;
    }
    return true;
}

