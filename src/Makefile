#****************************************************************************************
#
# Makefile for Sassy.
#
# Author:  Michael Imelfort
# Since:   13/June/08 : Major Revision 4/Sept/09 (Thanks DOM)
#
#****************************************************************************************
# Copyright (C) 2009 2010 2011 Michael Imelfort and Dominic Eales
#
# This file is part of the Sassy Assembler Project.
#
# Sassy is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Sassy is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Sassy.  If not, see <http://www.gnu.org/licenses/>.
#****************************************************************************************
#
#
# Compiler flags
#

include .makefile_pieces/Makefile.inc
include .makefile_pieces/Makefile.special

TMPMAKS = ${addprefix $(OUT_DIR)/,${SRCS:.cpp=.mak}}
CONFS := ${subst ../autocode/templates/${MACHINE_ARCH}/GlobalDefs.conf,,${wildcard ../autocode/templates/${MACHINE_ARCH}/*.conf}}
AUTOCODINGS := ${addprefix ../autocode/bin/maike.pl --silent --temps ../autocode/templates/${MACHINE_ARCH}/ --confO ,${subst ../autocode/templates/${MACHINE_ARCH}/,,${CONFS}}}
CONFMAKS := ${addsuffix .mak,${addprefix ${OUT_DIR}/,${subst ../autocode/templates/${MACHINE_ARCH}/,,${CONFS}}}}
TMPCPPMAKS := ${TMPMAKS} ${addsuffix MemManager.mak, ${addprefix ${OUT_DIR}/,${subst Defs.conf,,${subst ../autocode/templates/${MACHINE_ARCH}/,,${CONFS}}}}}
CPPMAKS := ${sort ${TMPCPPMAKS}}

.PHONY: all clean distclean
 
#################   
all:: Meta-${TARGET}

clean::
	-rm -fR ${REL_OUT_DIR} ${DBG_OUT_DIR}  
	-rm -f *~ ${REL_TARGETNAME} ${DBG_TARGETNAME}
	-rm *hm
        
distclean: clean
#################

${info -}
${info ---------------------------------------------}
${info You are building ${TARGETNAME}${SIZE_OF_INT}}
${info > Target architecture is ${MACHINE_ARCH} (${SIZE_OF_INT} bit)}
${info ---------------------------------------------}
${info -}

#
# If the dependencies are fixed we can run make on the created Makefile.
# 
Meta-${TARGET}: ${OUT_DIR}/donefirstrun ${OUT_DIR}/alldeps.mak
	@echo [---3---] Running \`make -f ${OUT_DIR}/alldeps.mak\'
	@touch ${OUT_DIR}/*.mak
	@$(MAKE) -f ${OUT_DIR}/alldeps.mak
	@touch ${OUT_DIR}/*.mak

# 
# This code concanenates all the mini Makefiles into one file named 
# 'alldeps.mak' in the object directory
# 
${OUT_DIR}/alldeps.mak: ${CONFMAKS} ${CPPMAKS} ${OUT_DIR}/special.mak
	@echo [---2---] Creating global makefile ${OUT_DIR}/alldeps.mak
	@cat .makefile_pieces/Makefile.inc .makefile_pieces/Makefile.rules ${CPPMAKS} ${CONFMAKS} ${OUT_DIR}/special.mak > ${OUT_DIR}/alldeps.mak

HM_EXTENSIONS := .virt.hm .autoheader.hm .priVar.hm .includes.hm
SPECCASES_CPP_NOEXT := ${SPECCASES_CPP:.cpp=}
SPECCASES_HDR_NOEXT := ${SPECCASES_HDR:.h=}
TEMP := $(foreach case,${SPECCASES_HDR_NOEXT},$(foreach ext,${HM_EXTENSIONS},$(case)$(ext):\|$(case).h))

#
# Create 
#
${OUT_DIR}/special.mak: .makefile_pieces/Makefile.special
	@awk -f .makefile_pieces/create_specialmak.awk .makefile_pieces/Makefile.special > ${OUT_DIR}/special.mak

#
# This code forces the makfile to create all the hm files for all the classes before it tries to make much more
# also forces the autocoding to be done first
#
${OUT_DIR}/donefirstrun: .makefile_pieces/Makefile.special
	@echo [--0.1--] Forcing first run autocoding
	@mkdir -p ${OUT_DIR}/
	@echo ${AUTOCODINGS} > ${OUT_DIR}/auto.sh
	@sed -i 's/\.conf \.\.\/auto/\.conf\n \.\.\/auto/g' ${OUT_DIR}/auto.sh
	@sh ${OUT_DIR}/auto.sh
	@rm ${OUT_DIR}/auto.sh
	@echo [--0.2--] Forcing hm creation
	@grep '\.' .makefile_pieces/Makefile.special > ${OUT_DIR}/hm.sh
	@sed -i 's/ \\//g' ${OUT_DIR}/hm.sh
	@sed -ie 's/.*/awk -f .makefile_pieces\/makehm.awk &/' ${OUT_DIR}/hm.sh
	@sh ${OUT_DIR}/hm.sh
	@rm ${OUT_DIR}/hm.sh
	@touch ${OUT_DIR}/donefirstrun
	@rm *.template
        
#
# This code creates mini Makefiles for each object file (*.o)
#
${CPPMAKS}: ${OUT_DIR}/%.mak: %.cpp Makefile .makefile_pieces/*
	@echo [--1.p--] Creating makefile for ${OUT_DIR}/${<:.cpp=.o}
	@mkdir -p ${OUT_DIR}/
	@${CC} ${CCFLAGS} -MM -MG -MT ${OUT_DIR}/${<:.cpp=.o} $< > ${OUT_DIR}/temp.dep
	@cat ${OUT_DIR}/temp.dep .makefile_pieces/Makefile.cc.cmd > $@
	@rm ${OUT_DIR}/temp.dep

#
# This code creates mini Makefiles for the files created by MAIKE.PL
# 
${CONFMAKS}: ${OUT_DIR}/%Defs.conf.mak: ../autocode/templates/${MACHINE_ARCH}/%Defs.conf  Makefile .makefile_pieces/*
	@echo [--1.c--] Creating makefile for ../autocode/templates/${MACHINE_ARCH}/$*Defs.conf
	@mkdir -p ${OUT_DIR}/
	@echo $*Class.cpp.template $*Class.h $*MemManager.cpp $*Manager.h $*MemWrapper.cpp.template $*MemWrapper.h $*MemWrapper.extended.h: $< > ${OUT_DIR}/temp.conf.mak
	@cat ${OUT_DIR}/temp.conf.mak > $@
	@echo '	@touch $$@' >> $@
	@cat .makefile_pieces/Makefile.maike.cmd >> $@
	@rm ${OUT_DIR}/temp.conf.mak
	

#****************************************************************************************
#****************************************************************************************

