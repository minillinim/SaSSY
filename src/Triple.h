// File: Triple.h
// Original Author: Michael Imelfort
// -----------------------------------------------------
//
// OVERVIEW:
// This file contains the class definition and implementation for a 3 vector
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

#ifndef Triple_h
 #define Triple_h

using namespace std;
#include <fstream>

template <class T1, class T2, class T3>
class Triple {
public:
    
    Triple(T1 Fst, T2 Sec, T3 Thd )
    {
        first = Fst; second = Sec; third = Thd;
    }
    
    Triple(void)
    {}

    // operators
    
// define overloaded operators
    // we hijack this operator, so that we can
    // do quicker comparisons on the sort.
    bool operator<= (const Triple& comp) const
    {
        if(comp.second < second)
        {
            return false;
        }
        return true;
    }

    bool operator< (const Triple& comp) const
    {
        if(first < comp.first)
        {
            return true;
        }
        else if(first > comp.first)
        {
            return false;
        }
        else
        {
            if(second < comp.second)
            {
                return true;
            }
            else if(second > comp.second)
            {
                return false;
            }
            else
            {
                if(third < comp.third)
                    return true;
                else
                    return false;
            }
        }
    }

    bool operator> (const Triple& comp) const
    {
        if(first > comp.first)
        {
            return true;
        }
        else if(first < comp.first)
        {
            return false;
        }
        else
        {
            if(second > comp.second)
            {
                return true;
            }
            else if(second < comp.second)
            {
                return false;
            }
            else
            {
                if(third > comp.third)
                    return true;
                else
                    return false;
            }
        }
    }

    bool operator== (const Triple& comp) const
    {
        if(first == comp.first && second == comp.second && third == comp.third)
            return true;
        else
            return false;
    }

    // file IO

    void writeTriple(ofstream * outFile)
    {
        //-----
        // save a triple to file
        // should only use this for ints!!!
        //
        outFile->write(reinterpret_cast<char *>(&first), sizeof(int));
        outFile->write(reinterpret_cast<char *>(&second), sizeof(int));
        outFile->write(reinterpret_cast<char *>(&third), sizeof(int));
    }
    
    void readTriple(ifstream * inFile)
    {
        //-----
        // save a triple to file
        // should only use this for ints!!!
        //
        inFile->read(reinterpret_cast<char *>(&first), sizeof(int));
        inFile->read(reinterpret_cast<char *>(&second), sizeof(int));
        inFile->read(reinterpret_cast<char *>(&third), sizeof(int));
    }
    
    T1 first;
    T2 second;
    T3 third;
};

#endif
