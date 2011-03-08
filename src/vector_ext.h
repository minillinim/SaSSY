//            File: vector_ext.h
// Original Author: Michael Imelfort
// -----------------------------------------------------
//
// OVERVIEW:
// This file contains some stl::vector extensions
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

#ifndef vector_ext_h
    #define vector_ext_h

using namespace std;
#include <vector>
#include <fstream>
#include "Triple.h"

template <class T>
bool isElementOf(vector<T> * checkVector, T element)
{
    //-----
    // generic vector is_element_of function
    //
    typename vector<T>::iterator t_iter = checkVector->begin();
    typename vector<T>::iterator t_last = checkVector->end();
    while(t_iter != t_last)
    {
        if(*t_iter == element)
            return true;
        t_iter++;
    }
    return false;
}

template <class T>
bool addIfAbsent(vector<T> * addVec, T value)
{
    //-----
    // add an int to the vector if it's not there
    // return true if we added the node
    //
    typename vector<T>::iterator add_iter = addVec->begin();
    typename vector<T>::iterator add_last = addVec->end();
    while(add_iter != add_last)
    {
        if(*add_iter == value)
            return false;
        add_iter++;
    }
    addVec->push_back(value);
    return true;
}

// strings
void writeVector(vector<string> *, ofstream*);
void readVector(vector<string> *, ifstream*);

// triples
void writeVector(vector<Triple<int, int, int> > *, ofstream*);
void readVector(vector<Triple<int, int, int> > *, ifstream*);

// general class...
template <class T>
void writeVector(vector<T> * writeVec, ofstream * outFile)
{
    //-----
    // write a vector to a file stream
    //
    int size = writeVec->size();
    T value;
    outFile->write(reinterpret_cast<char *>(&size), sizeof(int));
    typename vector<T>::iterator vector_iter = writeVec->begin();
    typename vector<T>::iterator vector_last = writeVec->end();
    while(vector_iter != vector_last)
    {
        value = *vector_iter;
        outFile->write(reinterpret_cast<char *>(&value), sizeof(T));
        vector_iter++;
    }
}

template <class T>
void readVector(vector<T> * readVec, ifstream * inFile)
{
    //-----
    // read a vector from a filestream
    //
    // clear the vector
    readVec->clear();
    int size;
    T value;
    inFile->read(reinterpret_cast<char *>(&size), sizeof(int));
    readVec->reserve(size);
    while(size > 0)
    {
        inFile->read(reinterpret_cast<char *>(&value), sizeof(T));
        readVec->push_back(value);
        size--;
    }
}


template <class T>
void completelyClear(T& container)
{
    //-----
    // make sure memory is de-allocated nicely...
    // we mainly use this for stl::vector
    //
    if(container.size() != 0)
    {
        T().swap(container);
    }
}

#endif // vector_ext_h
