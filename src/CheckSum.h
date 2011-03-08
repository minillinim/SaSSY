//            File: CheckSum.h
// Original Author: Dominic Eales
// --------------------------------------------------------------------
//
// OVERVIEW:
// Simple checksum used in sequence set identifiers
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

#ifndef _CHECKSUM_H_
#define _CHECKSUM_H_

#define BYTE unsigned char
#define WORD unsigned short
#define DWORD unsigned int

class CheckSum {
public:
    CheckSum() { clear(); }
    void clear() { sum = 0; r = 55665; c1 = 52845; c2 = 22719;}
    void add(BYTE value) {
        BYTE cipher = (value ^ (r >> 8));
        r = (cipher + r) * c1 + c2;
        sum += cipher;
    }
    void add(const char * str) {
        while (*str) {
            add(*str);
            str++;
        }
    }
    DWORD get() { return sum; }
protected:
    WORD r;
    WORD c1;
    WORD c2;
    DWORD sum;
};

#endif //_CHECKSUM_H_
