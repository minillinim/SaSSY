## BASIC VARIABLES
TARGETNAME := Sassy
SRCS := $(wildcard *.cpp)

MACHINE_ARCH = ${shell uname -m}

# DEFAULT
BASE_CCFLAGS := -O2 -Wall -Werror -Wno-unused-function
BASE_LIBS  = -lreadline -lncurses -lpcre

## CHECK FOR MACHINE ARCHITECTURE
ifeq (${MACHINE_ARCH},i686)
	SIZE_OF_INT := 32
else 
    ifeq (${MACHINE_ARCH},x86_64)
	SIZE_OF_INT := 64
    else
	$(error Machine architecture "${MACHINE_ARCH}" is unknown)
    endif
endif

# FOR MEMORY DEBUGGING
ifeq ($(memory),debug)
    BASE_CCFLAGS += -DSHOW_MEM
endif

# UPDATE CCFLAGS
BASE_CCFLAGS += -DSIZE_OF_INT=${SIZE_OF_INT} -DUSE_INTERACTIVE_COMMAND_PROMPT

## DEBUG VARIABLES
DBG_OUT_DIR = .obj-dbg
DBG_TARGETNAME := ${TARGETNAME}${SIZE_OF_INT}-dbg
DBG_CCFLAGS_1 := ${BASE_CCFLAGS} -ggdb -DMAKE_PARANOID -DPARANOID_LAYER_1
DBG_CCFLAGS_2 := ${BASE_CCFLAGS} -ggdb -DMAKE_PARANOID -DPARANOID_LAYER_1 -DPARANOID_LAYER_2
DBG_CCFLAGS_3 := ${BASE_CCFLAGS} -ggdb -DMAKE_PARANOID -DPARANOID_LAYER_1 -DPARANOID_LAYER_2 -DPARANOID_LAYER_3
DBG_CCFLAGS_4 := ${BASE_CCFLAGS} -ggdb -DMAKE_PARANOID -DPARANOID_LAYER_1 -DPARANOID_LAYER_2 -DPARANOID_LAYER_3 -DPARANOID_LAYER_4

## RELEASE VARIABLES
REL_OUT_DIR = .obj-rel
REL_TARGETNAME := ${TARGETNAME}${SIZE_OF_INT}
REL_CCFLAGS := ${BASE_CCFLAGS} 

## SUPER LOGGING
SUPER_LOG := -DSUPER_LOGGING

##################################
## START MAKE CODE
##################################
LIBS = ${BASE_LIBS}

ifeq ($(mode),debug1)
  CCFLAGS = ${DBG_CCFLAGS_1}
  OUT_DIR = ${DBG_OUT_DIR}
  TARGET := ${DBG_TARGETNAME}
else
  ifeq ($(mode),debug2)
    CCFLAGS = ${DBG_CCFLAGS_2}
    OUT_DIR = ${DBG_OUT_DIR}
    TARGET := ${DBG_TARGETNAME}
  else
    ifeq ($(mode),debug3)
      CCFLAGS = ${DBG_CCFLAGS_3}
      OUT_DIR = ${DBG_OUT_DIR}
      TARGET := ${DBG_TARGETNAME}
    else
      ifeq ($(mode),debug4)
        CCFLAGS = ${DBG_CCFLAGS_4}
        OUT_DIR = ${DBG_OUT_DIR}
        TARGET := ${DBG_TARGETNAME}
      else
        mode = release
        CCFLAGS = ${REL_CCFLAGS}
        OUT_DIR = ${REL_OUT_DIR}
        TARGET := ${REL_TARGETNAME}
      endif
    endif
  endif
endif

ifeq ($(logging),super)
  $(shell touch LoggerSimp.h)
  CCFLAGS += ${SUPER_LOG}
endif

OBJS := ${addprefix $(OUT_DIR)/,${SRCS:.cpp=.o}}

LDFLAGS = 
CC = g++

