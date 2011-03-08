//            File: Dataset.h
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

#ifndef _DataSet_H_
#define _DataSet_H_

// system includes
#include <vector>
#include <map>

// local includes
#include "intdef.h"
#include "IdTypeDefs.h"

/******************************************************
 ** InsertLibrary handling
 ******************************************************/
class InsertLibrary {
public:
    //
    // Each library has an orientation, this works like this:
    // There are at most 4 possible types, which can be represented by the characters DD, DA, AD and AA
    // (D is for disagree and A is for agree) if we let let D be 0 and A be 1 then we have a quick way of
    // getting a number {DD, DA, AD, AA}. Looking at the first character D or A the idea is that you look at
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
    //  DD  <---  --->   if the first read disagrees with ref then the second is ahead and in the reverse orientation
    //  DA  <---  <---   if the first read disagrees with ref then the second is ahead and in the same orientation (not used, always return type 3)
    //  AD  --->  <---   if the first read agrees with ref then the second is ahead and in the reverse orientation
    //  AA  --->  --->   if the first read agrees with ref then the second is ahead and in the same orientation
    //
    // And we can assign a Type to the library
    //
    enum OrientationType {
        TYPE_DD = 0,
    //    TYPE_DA = 1,
        TYPE_AD = 2,
        TYPE_AA = 3,
        TYPE_UNSET = 4,
    };

  // constructor/destructor
    InsertLibrary(double mean);
    InsertLibrary(double mean, OrientationType ot);
    ~InsertLibrary();

  // operations
    
    // calculate the upper and lower cutoffs
    void makeCutOffs(double stdevRange);
    // calculate the orientation type for a paired end read
    OrientationType calcOrientationType(bool seq1Reversed, bool seq2Reversed, sMDInt distance1to2);
    // check to see if the orientations and distance match this library
    bool doesMatchLibraryStats(bool seq1Reversed, bool seq2Reversed, sMDInt distance1to2);
    bool doesMatchLibraryStats(bool seq1Reversed, bool seq2Reversed, sMDInt distance1to2, FileSetId FSID);
    bool isDistOrientOK(InsertLibrary::OrientationType ot, uMDInt dist);

  // printing and output
    void printContents(void);
    void logContents(int logLevel);
    std::string ContentsAsString(void);
    std::string sayOrientLikeAHuman(OrientationType ot);

  // file IO
    void save(std::string filesetName, std::string fileName);
    void load(std::string filesetName, std::string fileName);
    bool FillFromString(std::string str);

  // set and reset
    bool setString(ostringstream * oss);
    bool resetFromString(istringstream * iss);

  // member variables
    double                    _mean;
    double                    _stdev;
    double                    _beta;
    uMDInt                    _upperCutOff;
    uMDInt                    _lowerCutOff;
    OrientationType           _orientation;
    int                       _numReads;
    int                       _sizeRank;
    FileSetId                 _fsid;             // reference to the fileset this guy comes from

    //-----------------------
    // PERHAPS THIS SHOULD GO
    std::vector<uMDInt>       _insertBin;
};

/******************************************************
 ** File-set handling
 ******************************************************/
class DataSet; // forward declaration (yes we need it)

class FileSet {
public:
  //Constructor/Destructor
    FileSet();
    ~FileSet();

  // housekeeping 
    inline void setFSID(FileSetId FSID) { _fsid = FSID; }
    inline FileSetId getFSID(void) { return _fsid; }
    std::string GetFullPathname(void) {
        if ( _filled == true ) {
            std::string out = _filesetDirectory + _filesetFilename;
            return out;
        }
        return "";
    }
    
  // access
    InsertLibrary * getLongestInsLib(void);     // get the longest insert library for this fileSet
    
  // handling insert libraries
    void addInsLib(InsertLibrary newLib);
    void clearAllInsLibs(void);
    std::vector<InsertLibrary> * getInsLibs(void);
    void fixInsFSIDs(void);
    
  // Handling insert library bins
    bool binInsert(bool seq1Reversed, bool seq2Reversed, sMDInt distance1to2);
    void flushBins(void);

  // operations
    InsertLibrary::OrientationType calcOrientationType(bool seq1Reversed, bool seq2Reversed, sMDInt distance1to2);

  // printing and output
    void printInsertStats(void);
    void logInsertStats(int logLevel);
    void logAll(void);

  //file IO
    void save(std::string fileName);
    void load(std::string fileName);
    bool SaveTo_File( std::string filename );
    bool SaveTo_File( void );
    bool FillFrom_User( std::istream& in, std::ostream& out);
    bool FillFrom_File( std::string filename );
    
  // set and reset
    bool setString(ostringstream * oss);
    bool resetFromString(istringstream * iss);

  //get functions
    inline std::string              getFormatName(void) { return _formatName; }
    inline std::string              getDirectory(void)  { return _filesetDirectory; }
    inline std::vector<std::string> getFileList(void)   { return _filesInFileSet; }
    inline uMDInt                   getNumReads(void)  { return _numReads; }
    inline uint_16t                 getReadLength(void)  { return _readLen; }
    inline uint_16t                 getTrimLength(void)  { return _trimLen; }

private:
    // Housekeeping variables
    bool                       _filled;
    FileSetId                  _fsid;             // index in the DataSet array, but this guy needs to know it

    // decided by the powers that are
    std::string                _filesetName;

    // Required values entered by user
    std::string                _formatName;      // may be optional one day
    std::vector<std::string>   _filesInFileSet;
    uMDInt                     _numReads;
    uint_16t                   _readLen;
    uint_16t                   _trimLen;
    std::vector<InsertLibrary> _insertLibs;

    // special variables relating to files
    std::string                _filesetDirectory; // the directory the file-set descriptor file and file(s) are in
    std::string                _filesetFilename;  // use the function MakeFilesetFilename() to set this

private: // functions
    inline std::string MakeFilesetFilename(void) {
        return "fileset." + _filesetName + ".sassy";
    }
    friend class DataSet;
    
#if 0
    // Values calculated by Sassy
    std::string               _formatDefFile;
    unsigned int              _determinedTrimLen;
    std::vector<unsigned int> _determinedInsertLibs;
#endif
};

/******************************************************
 ** Data-set handling
 ******************************************************/
class DataSet {

public:
    DataSet();
    ~DataSet();

  // adding and removing FileSets
    void addFileSet(FileSet newSet, uMDInt maxReadsSmallMode = 0);
    void clearAllFileSets(void);
    void setIgnoreTrap(uMDInt ignoreTrap) { _ignoreTrap = ignoreTrap; }
    
  // giving access to the FileSets and insert libraries / bins
    std::vector<FileSet> * getFileSets(void);
    int getNumInsertLibs(void);
    std::vector<InsertLibrary> * getInsLibs(FileSetId FSID);
    std::vector<InsertLibrary *> * getInsertLib(int rank);
    FileSet * getFileSet(FileSetId FSID);
    uMDInt getMaxReadsForFileSet(FileSetId FSID);
    bool getSortedBins(std::vector<std::vector<uMDInt> * > * ret_vector, std::vector<double> * means_vector);
    bool rankInsertLibs(void);              // rank all the insert libs by their _means
    bool getBinStats(std::vector<double> * deviations);
    void flushBins(void);
    void setCutOffs(double range);
    uMDInt getLowestUpperCut(void);         // get the lowest of all the upper cutoffs for this datset
    uMDInt getLowestLowerCut(void);         // get the lowest of all the lower cutoffs for this datset
    uMDInt getHighestUpperCut(void);        // get the highest of all the upper cutoffs for this datset

    // Populate the maps with the appropriate values for the largest InserLibrary for each FileSet
    // Used for the scaffolder...
    bool getFileStats(std::map<FileSetId, 
                      InsertLibrary::OrientationType> * longestInserts, 
                      std::map<FileSetId, uMDInt> * upperCuts, 
                      std::map<FileSetId, uMDInt> * lowerCuts, 
                      std::map<FileSetId, uMDInt> * means);
    
    std::map<int, double> * getOverShoots(void) { return &_overShoots; }
    std::map<int, double> * getUnderShoots(void) { return &_underShoots; }
    inline int getNumRankBands(void) { return _rankedInserts.size(); }
    
  // determined readLength
    uint_16t calculateReadLength(bool optimiseForArch, unsigned int basesPerInt);
    uint_16t getReadLength(void);
    uMDInt   getTotalNumReads(void);
    void     setExternalReadLength(uint_16t erl) { _externalReadLength = erl; }
        
  // printing and output
    void printFileStats(void);
    void logAll(void);
    void logFileStats(int logLevel);

  // file IO
    void save(std::string fileName);
    void load(std::string fileName);

  // set and reset
    bool setString(void);
    bool resetFromString(void);

    std::string           _internalSaveString;
    
private:
    // housekeeping
    std::vector<FileSet>  _fileSets;
    std::vector<uMDInt>   _fileSetMaxReads;
    uint_16t              _determinedReadLength;
    uint_16t              _externalReadLength;
    uMDInt                _totalNumReads;
    std::string           _loadFileName;
    std::map<int, std::vector<InsertLibrary *> * > _rankedInserts;
    uMDInt                _ignoreTrap;
    
    // for each ranked band of insert, what is the average stdev for overshoots and also for undershoots?
    std::map<int, double> _overShoots;
    std::map<int, double> _underShoots;
};

#endif //_DataSet_H_
