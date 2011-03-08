//            File: char_encodings.h
// Original Author: Michael Imelfort
// --------------------------------------------------------------------
//
// OVERVIEW:
// Configuration of character encodings for SASSY
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

#ifndef C_ENC_H
    #define C_ENC_H

#include "intdef.h"                                                     // Just so we get access to the size of an int

#define SAS_CE_N_CHAR       'N'                                         // the 'N' char!

#ifdef SOLID_DATA_
# define SAS_CE_D_0         '0'                                         // this is the character set we are using for colorspace
# define SAS_CE_D_1         '1'
# define SAS_CE_D_2         '2'
# define SAS_CE_D_3         '3'
# define SAS_CE_DS_0        "0"
# define SAS_CE_DS_1        "1"
# define SAS_CE_DS_2        "2"
# define SAS_CE_DS_3        "3"
# define SAS_CE_R_0         '0'                                         // reverse compliment (same for SOLiD)
# define SAS_CE_R_1         '1'
# define SAS_CE_R_2         '2'
# define SAS_CE_R_3         '3'
# define SAS_CE_RS_0        "0"
# define SAS_CE_RS_1        "1"
# define SAS_CE_RS_2        "2"
# define SAS_CE_RS_3        "3"
#else
# define SAS_CE_D_0         'A'                                         // this is the character set we are using for DNA space
# define SAS_CE_D_1         'C'
# define SAS_CE_D_2         'G'
# define SAS_CE_D_3         'T'
# define SAS_CE_DS_0        "A"
# define SAS_CE_DS_1        "C"
# define SAS_CE_DS_2        "G"
# define SAS_CE_DS_3        "T"
# define SAS_CE_R_0         'T'                                         //reverse compliment
# define SAS_CE_R_1         'G'
# define SAS_CE_R_2         'C'
# define SAS_CE_R_3         'A'
# define SAS_CE_RS_0        "T"
# define SAS_CE_RS_1        "G"
# define SAS_CE_RS_2        "C"
# define SAS_CE_RS_3        "A"
#endif

#define SAS_CE_H_0          0x0                                         // .... 0000 this is the hex equivalent
#define SAS_CE_H_1          0x1                                         // .... 0001
#define SAS_CE_H_2          0x2                                         // .... 0010
#define SAS_CE_H_3          0x3                                         // .... 0011

#if (SIZE_OF_INT == 64)
# define SAS_CE_U_0         0x0000000000000000                          // this is the same thing but up the other end! (mirrored)
# define SAS_CE_U_1         0x4000000000000000                          // 0100 ....
# define SAS_CE_U_2         0x8000000000000000                          // 1000 ....
# define SAS_CE_U_3         0xC000000000000000                          // 1100 ....
#else
# define SAS_CE_U_0         0x00000000                                  // this is the same thing but up the other end! (mirrored)
# define SAS_CE_U_1         0x40000000                                  // 0100 ....
# define SAS_CE_U_2         0x80000000                                  // 1000 ....
# define SAS_CE_U_3         0xC0000000                                  // 1100 ....
#endif

#define SAS_CE_CU_0         0x00                                        // this is the same thing as above but for chars!
#define SAS_CE_CU_1         0x40                                        // 01
#define SAS_CE_CU_2         0x80                                        // 10
#define SAS_CE_CU_3         0xC0                                        // 11

#define SAS_CE_NUM_NUCS     4                                           // number of nucleotides


// convert between. used in the scaffolder
static short NUC2INT(char ch){
    switch (ch) {
        case SAS_CE_D_0 : return 0;
        case SAS_CE_D_1 : return 1;
        case SAS_CE_D_2 : return 2;
        case SAS_CE_D_3 : return 3;
    }
    return -1;
}

static char INT2NUC(short x){
    switch (x) {
        case 0 : return SAS_CE_D_0;
        case 1 : return SAS_CE_D_1;
        case 2 : return SAS_CE_D_2;
        case 3 : return SAS_CE_D_3;
    }
    return 'X';
}

#endif // C_ENC_H
