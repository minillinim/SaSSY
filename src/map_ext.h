//            File: map_ext.h
// Original Author: Michael Imelfort
// --------------------------------------------------------------------
//
// OVERVIEW:
// This file contains some map extensions (namely saving and loading)
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

#ifndef map_ext_h
    #define map_ext_h


//******************************************************
//  OK, I know that this is a little ad-hoc
//  I know that there are more elegant ways
//  to do this, but it works and it required no brains
//  I *promise* I will fix it *later*   -- Mike  17/7/08 
//******************************************************
      
#include <map>
#include <vector>

//
// use these for maps of type <T1, vector<string> > where
// T1 is in {int, bool, double, float, char etc...}
//

template <class T>
void readMap(std::map<T, std::vector<std::string> > * readmap, ifstream * inFile)
{
    //-----
    // read a map from a filestream
    //
    // clear the map
    readmap->clear();
    
    int size;
    T key;
    std::vector<std::string> value;
    std::string str_value;
    // read in the size of the map
    inFile->read(reinterpret_cast<char *>(&size), sizeof(unsigned int));
    while(size > 0)
    {
        // reset the string vector
        value.clear();
        char temp;
        // read in the key
        inFile->read(reinterpret_cast<char *>(&key), sizeof(T));
        // read in the size of the string vector
        int str_vec_size;
        inFile->read(reinterpret_cast<char *>(&str_vec_size), sizeof(int));
        for(int j = 0; j < str_vec_size; j++)
        {
            // for each entry, read in the string...
            int str_size;
            str_value = "";
            inFile->read(reinterpret_cast<char *>(&str_size), sizeof(int));
            for(int i = 0; i < str_size; i++)
            {
                inFile->read(reinterpret_cast<char *>(&temp), sizeof(char));
                str_value += temp;
            }
            // load this string into the vector
            value.push_back(str_value);
        }
        // load the value into the map
        (*readmap)[key] = value;
        size--;
    }
}

template <class T>
        void writeMap(std::map<T, std::vector<std::string> > * writemap, ofstream * outFile)
{
    //-----
    // write a map to a file stream
    //
    // get the number of elements in the map
    int size = writemap->size();
    // write this to the file
    outFile->write(reinterpret_cast<char *>(&size), sizeof(unsigned int));
    // go through each element in the list
    typename std::map<T, std::vector<std::string> >::iterator map_iter = writemap->begin();
    typename std::map<T, std::vector<std::string> >::iterator map_last = writemap->end();
    while(map_iter != map_last)
    {
        // write the key
        T key = map_iter->first;
        outFile->write(reinterpret_cast<char *>(&key), sizeof(T));
        // extract the vector of strings and write the size to file
        std::vector<std::string> str_vec = map_iter->second;
        int str_vec_size = str_vec.size();
        outFile->write(reinterpret_cast<char *>(&str_vec_size), sizeof(int));
        // now write the strings
        std::vector<std::string>::iterator str_vec_iter = str_vec.begin();
        std::vector<std::string>::iterator str_vec_last = str_vec.end();
        while(str_vec_iter != str_vec_last)
        {
            // record the length of the string
            int str_size = str_vec_iter->size();
            outFile->write(reinterpret_cast<char *>(&str_size), sizeof(int));
            // now write out the chars
            for(int i = 0; i < str_size; i++)
            {
                outFile->write(reinterpret_cast<char *>(&((*str_vec_iter)[i])), sizeof(char));
            }
            str_vec_iter++;
        }
        map_iter++;
    }
}

//
// use these for maps of type <T1, vector<T2> > where
// T1, T2 is in {int, bool, double, float, char etc...}
// strings have already been handled
//

template <class T1, class T2>
        void writeMap(std::map<T1, std::vector<T2> > * writemap, ofstream * outFile)
{
    //-----
    // write a map to a file stream
    //
    // get the number of elements in the map
    int size = writemap->size();
    // write this to the file
    outFile->write(reinterpret_cast<char *>(&size), sizeof(int));
    // go through each element in the list
    typename std::map<T1, std::vector<T2> >::iterator map_iter = writemap->begin();
    typename std::map<T1, std::vector<T2> >::iterator map_last = writemap->end();
    while(map_iter != map_last)
    {
        // write the key
        T1 key = map_iter->first;
        outFile->write(reinterpret_cast<char *>(&key), sizeof(T1));
        // extract the vector of T2s and write the size to file
        std::vector<T2> vec = map_iter->second;
        int vec_size = vec.size();
        outFile->write(reinterpret_cast<char *>(&vec_size), sizeof(unsigned int));
        // now write the contents of the vectors
        typename std::vector<T2>::iterator vec_iter = vec.begin();
        typename std::vector<T2>::iterator vec_last = vec.end();
        while(vec_iter != vec_last)
        {
            T2 value = *vec_iter;
            outFile->write(reinterpret_cast<char *>(&value), sizeof(T2));
            vec_iter++;
        }
        map_iter++;
    }
}

template <class T1, class T2>
        void readMap(std::map<T1, std::vector<T2> > * readmap, ifstream * inFile)
{
    //-----
    // read a map from a filestream
    //
    // clear the map
    readmap->clear();
    
    int size;
    T1 key;
    std::vector<T2> value;
    T2 T2_value;
    // read in the size of the map
    inFile->read(reinterpret_cast<char *>(&size), sizeof(unsigned int));
    while(size > 0)
    {
        // reset the T2 vector
        value.clear();
        // read in the key
        inFile->read(reinterpret_cast<char *>(&key), sizeof(T1));
        // read in the size of the T2 vector
        int vec_size;
        inFile->read(reinterpret_cast<char *>(&vec_size), sizeof(int));
        for(int j = 0; j < vec_size; j++)
        {
            inFile->read(reinterpret_cast<char *>(&T2_value), sizeof(T2));
            // load this T2 into the vector
            value.push_back(T2_value);
        }
        // load the value into the map
        (*readmap)[key] = value;
        size--;
    }
}

//
// use these for maps of type <T, string> where
// T1 is in {int, double, float, char etc...}
// vectors are not used for these
//

template <class T>
void writeMap(std::map<T, std::string> * writemap, std::ofstream * outFile)
{
    //-----
    // write a map to a file stream
    //
    unsigned int size = writemap->size();
    //cout << "S <T, string>: " << size << "\n";
    outFile->write(reinterpret_cast<char *>(&size), sizeof(unsigned int));
    typename std::map<T, std::string>::iterator map_iter = writemap->begin();
    typename std::map<T, std::string>::iterator map_last = writemap->end();
    while(map_iter != map_last)
    {
        T id = map_iter->first;
        string value = map_iter->second;
        int str_size = value.size();
        outFile->write(reinterpret_cast<char *>(&str_size), sizeof(int));
        outFile->write(reinterpret_cast<char *>(&id), sizeof(T));
        for(int i = 0; i < str_size; i++)
        {
            outFile->write(reinterpret_cast<char *>(&(value[i])), sizeof(char));
        }
        map_iter++;
    }
}

template <class T>
void readMap(std::map<T, std::string> * readmap, std::ifstream * inFile)
{
    //-----
    // read a map from a filestream
    //
    // clear the map
    readmap->clear();
    unsigned int size;
    T id;
    string value;
    inFile->read(reinterpret_cast<char *>(&size), sizeof(unsigned int));
    //cout << "L <T, string>: " << size << "\n";
    while(size > 0)
    {
        value = "";
        char temp;
        int str_size;
        inFile->read(reinterpret_cast<char *>(&str_size), sizeof(int));
        inFile->read(reinterpret_cast<char *>(&id), sizeof(T));
        for(int i = 0; i < str_size; i++)
        {
            inFile->read(reinterpret_cast<char *>(&temp), sizeof(char));
            value += temp;
        }
        (*readmap)[id] = value;
        size--;
    }
}


//
// use these for maps of type <T, string> where
// T1 is in {int, double, float, char etc...}
// vectors are not used for these
//

template <class T>
void writeMap(std::map<std::string, T> * writemap, std::ofstream * outFile)
{
    //-----
    // write a map to a file stream
    //
    unsigned int size = writemap->size();
    outFile->write(reinterpret_cast<char *>(&size), sizeof(unsigned int));
    typename std::map<std::string, T>::iterator map_iter = writemap->begin();
    typename std::map<std::string, T>::iterator map_last = writemap->end();
    while(map_iter != map_last)
    {
        std::string id = map_iter->first;
        T value = map_iter->second;
        int str_size = id.size();
        outFile->write(reinterpret_cast<char *>(&str_size), sizeof(int));
        for(int i = 0; i < str_size; i++)
        {
            outFile->write(reinterpret_cast<char *>(&(id[i])), sizeof(char));
        }
        outFile->write(reinterpret_cast<char *>(&value), sizeof(T));
        map_iter++;
    }
}

template <class T>
void readMap(std::map<std::string, T> * readmap, std::ifstream * inFile)
{
    //-----
    // read a map from a filestream
    //
    // clear the map
    readmap->clear();
    unsigned int size;
    std::string id;
    T value;
    inFile->read(reinterpret_cast<char *>(&size), sizeof(unsigned int));
    while(size > 0)
    {
        id = "";
        char temp;
        int str_size;
        inFile->read(reinterpret_cast<char *>(&str_size), sizeof(int));
        for(int i = 0; i < str_size; i++)
        {
            inFile->read(reinterpret_cast<char *>(&temp), sizeof(char));
            id += temp;
        }
        inFile->read(reinterpret_cast<char *>(&value), sizeof(T));
        (*readmap)[id] = value;
        size--;
    }
}

//
// use these for maps of type <T1, T2> where
// T1, T2 E {int, double, float, char etc...}
// strings, and vectors are not used for these
//
                 
template <class T1, class T2>
void writeMap(std::map<T1, T2> * writemap, ofstream * outFile)
{
    //-----
    // write a map to a file stream
    //
    int size = writemap->size();
    //cout << "S <T1, T2>: " << size << "\n";
    outFile->write(reinterpret_cast<char *>(&size), sizeof(unsigned int));
    typename std::map<T1, T2>::iterator map_iter = writemap->begin();
    typename std::map<T1, T2>::iterator map_last = writemap->end();
    while(map_iter != map_last)
    {
        T1 id = map_iter->first;
        T2 value = map_iter->second;
        outFile->write(reinterpret_cast<char *>(&id), sizeof(T1));
        outFile->write(reinterpret_cast<char *>(&value), sizeof(T2));
        map_iter++;
    }
}

template <class T1, class T2>
void readMap(std::map<T1, T2> * readmap, ifstream * inFile)
{
    //-----
    // read a map from a filestream
    //
    // clear the map
    readmap->clear();
    int size;
    T1 id;
    T2 value;
    inFile->read(reinterpret_cast<char *>(&size), sizeof(unsigned int));
    //cout << "L <T1, T2>: " << size << "\n";
    while(size > 0)
    {
        inFile->read(reinterpret_cast<char *>(&id), sizeof(T1));
        inFile->read(reinterpret_cast<char *>(&value), sizeof(T2));
        (*readmap)[id] = value;
        size--;
    }
}

#endif // map_ext_h
