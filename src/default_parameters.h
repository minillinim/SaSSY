//            File: default_parameters.h
// Original Author: Michael Imelfort
// --------------------------------------------------------------------
//
// OVERVIEW:
// Configuration file for default parameters for SASSY (Environment variables)
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

#ifndef DEF_PARAM_H
    #define DEF_PARAM_H

// set to 1 to enable read identifiers
#define __USE_RI 0

/******************************************************************************
** DEFAULT OBJECT MEMORY SIZE ( INITIAL )
** Multipliers multiply by the numer of lines in the input data file
** Size is exactly that, a size
******************************************************************************/
#define SAS_DEF_ASS_GN_MULT         (1.3)                           // the size of the GenericNode object within Assembler
#define SAS_DEF_ASS_CONTIG_SIZE     10000                           // the size of the Contig object within Assembler
#define SAS_DEF_ASS_CONTEXT_SIZE    50                              // the size of the Context object within Assembler
#define SAS_DEF_GN_DN_MULT          2                               // the size of the DualNode object within GenericNode
#define SAS_DEF_GN_RI_MULT          2                               // the size of the ReadIdentifier object within GenericNode
#define SAS_DEF_GN_RS_MULT          (0.7)                           // the size of the ReadStore object within GenericNode
#define SAS_DEF_GN_UN_MULT          (2.5)                           // the size of the UniNode object within GenericNode
#define SAS_DEF_UN_KMM_SIZE         50                              // the size of the KmerMatchMap object within UniNode
#define SAS_DEF_UN_KM_MULT          2                               // the size of the KmerMap and KmerStore objects within UniNode
#define SAS_DEF_CTX_NPM_SIZE        100000                          // default size for a node positions map within context
#define SAS_DEF_CTX_NPM_SML_SIZE    1000                            // default size for a node positions map within context
#define SAS_DEF_CTX_NC_NODE_SIZE    40000                           // default size for the node checker which stores context nodes
#define SAS_DEF_CTX_NC_SHOOT_SIZE   5000                            // default size for the node checker which stores context shoot nodes
/******************************************************************************
** main.cpp DEFAULTS
******************************************************************************/
#define SAS_DEF_MIN_SCAFF_LINKS     5                               // default minimum number of links needed to make a scaffold
#define SAS_DEF_IGNORE_TRAP         0                               // by default, do NOT ignore the traps
#define SAS_DEF_DO_STRICT           true                            // default is to do a strict assembly
#define SAS_DEF_DO_OVERLAPERATE     true                            // default is to overlaperate
#define SAS_DEF_LOG_LEVEL           0                               // logging off by default
/******************************************************************************
** DEFAULT DATA INFO SETTINGS
******************************************************************************/
#define SAS_DEF_MAX_READLENGTH      110                             // bigger than the max readlength in ReadStoreMemWrapper.tmpconf.h but not oo big...
#define SAS_DEF_UN_OFFSET_MAX_MAX   16                              // the maximum max offset we'll allow
#define SAS_DEF_UN_OFFSET_DEF_MAX   12                              // default max
#define SAS_DEF_UN_OFFSET_DEF_MIN   4                               // default min
#define SAS_DEF_UN_OFFSET_MIN_MIN   2                               // the minimum min offset
#define SAS_DEF_UN_SHAVE_DEPTH      15                              // shave all dead end paths of 15 or less
#define SAS_DEF_CTX_SHAVE_DEPTH     5                               // light shaving for withing context making
/******************************************************************************
** DEFAULT GROW PARAMETERS
******************************************************************************/
#define SAS_DEF_CONTEXT_START       (2000000000)                    // the start position in the context
#define SAS_DEF_CONTEXT_MAX_TREE    (4)                             // default greatest depth we'll search to recurrsively when building a context
#define SAS_DEF_MEAN_RANGE          (2)                             // how many standard deviations from the mean we'll allow reads to be
#define SAS_DEF_TOLL_MIN            (1)                             // the minimum tollerance (# std devs fromt the mean)
#define SAS_DEF_TOLL_MAX            (3.5)
#define SAS_DEF_TOLL_STEPS          (4)                             // the number of steps beteen the min and the max tollerance
#define SAS_DEF_MIN_EDGE_OFFSET     (4)                             // Accept no edges between any closer nodes
/******************************************************************************
** FILE EXTENSIONS AND SETTINGS
******************************************************************************/
#define SAS_DEF_FASTA_LINE_LENGTH   100                             // the default length of lines in a fasta file
#define SAS_DEF_NAIVE_CONTIG_EXT    ".naive_contigs.fa"             // the default extension for writing contigs to (naive)
#define SAS_DEF_SUPP_CONTIG_EXT     ".supp_contigs.fa"              // the default extension for writing contigs to (supplementary)
#define SAS_DEF_ADVANCED_CONTIG_EXT ".advan_contigs.fa"             // the default extension for writing contigs to (intermediate)
#define SAS_DEF_FINAL_CONTIG_EXT    ".final_contigs.fa"             // the default extension for writing contigs to (final)
#define SAS_DEF_SCAFF_CONTIG_EXT    ".scaffolds.fa"                 // the default extension for writing contigs to (scaffolded))
#define SAS_DEF_LOGFILE_EXTENSION   "sassy.log"                     // default log file
#define SAS_DEF_MAPPING_FILE        "sassy.mappings"                // default file for writing mappings
#define SAS_DEF_ASS_COV_FILE        "cov_map.sassy"                 // default file to write coverage to
#define SAS_DEF_FDFILE_DIR          "formatdefs"                    // the dir in the sassy dir that holds the file definition files
#define SAS_DEF_CONT_END_LEN        2000                            // how many bases to print when getting the end of a contig
/******************************************************************************
** PROGRAM NAME
******************************************************************************/
#if(SIZE_OF_INT == 64)
# define SAS_EXENAME "Sassy64"
#elif (SIZE_OF_INT == 32)
# define SAS_EXENAME "Sassy32"
#else
# define SAS_EXENAME "Sassy"
#endif
#endif // DEF_PARAM_H
