#################### START OF CONFIG ####################

# project name
PROJECT = unittests

# C/C++ standard used
STD_CC  = c99
STD_CXX = c++11

# default flags
CC       ?= gcc
CXX      ?= g++
CFLAGS   ?= -std=$(STD_CC)  -Wall -Wextra
CXXFLAGS ?= -std=$(STD_CXX) -Wall -Wextra -I../src
LDFLAGS  ?= 

# release build
RELEASE_CC       = $(CC)
RELEASE_CXX      = $(CXX)
RELEASE_CFLAGS   = $(CFLAGS)   -O2
RELEASE_CXXFLAGS = $(CXXFLAGS) -O2
RELEASE_LDFLAGS  = $(LDFLAGS)

# debug build
DEBUG_CC       = $(CC)
DEBUG_CXX      = $(CXX)
DEBUG_CFLAGS   = $(CFLAGS)   -g3 -O0
DEBUG_CXXFLAGS = $(CXXFLAGS) -g3 -O0
DEBUG_LDFLAGS  = $(LDFLAGS)

# clang sanitizer build
SANITIZER          = address
SANITIZER_CC       = clang
SANITIZER_CXX      = clang++
SANITIZER_CFLAGS   = $(CFLAGS)   -g3 -fsanitize=$(SANITIZER)
SANITIZER_CXXFLAGS = $(CXXFLAGS) -g3 -fsanitize=$(SANITIZER)
SANITIZER_LDFLAGS  = $(LDFLAGS)      -fsanitize=$(SANITIZER)

#################### END OF CONFIG ####################


# generate the source file list
CC_SOURCES  := $(wildcard src/*.c \
                              *.c)
CXX_SOURCES := $(wildcard src/*.cpp src/*.cc src/*.C \
                              *.cpp     *.cc     *.C)
HEADERS     := $(wildcard src/*.hpp src/*.hh src/*.H src/*.h \
                              *.hpp     *.hh     *.H     *.h)

# generate the resulting object file list
CXX_BASENAMES := $(basename $(CXX_SOURCES))
OBJECTS := $(CC_SOURCES:.c=.o) $(CXX_BASENAMES:=.o)


.PHONY: all
all: Makefile.deps $(PROJECT)

$(PROJECT): $(OBJECTS)
ifneq ($(CXX_SOURCES),)
	$(CXX) $(LDFLAGS) $(OBJECTS) $(LOADLIBES) $(LDLIBS) -o $@
else
	$(CC) $(LDFLAGS) $(OBJECTS) $(LOADLIBES) $(LDLIBS) -o $@
endif

.PHONY: clean
clean:
	$(RM) $(OBJECTS) $(PROJECT)

.PHONY: distclean
distclean: clean
	$(RM) Makefile.deps


.PHONY: release
release: CC       := $(RELEASE_CC)
release: CXX      := $(RELEASE_CXX)
release: CFLAGS   := $(RELEASE_CFLAGS)
release: CXXFLAGS := $(RELEASE_CXXFLAGS)
release: LDFLAGS  := $(RELEASE_LDFLAGS)
release: $(PROJECT)

.PHONY: debug
debug: CC       := $(DEBUG_CC)
debug: CXX      := $(DEBUG_CXX)
debug: CFLAGS   := $(DEBUG_CFLAGS)
debug: CXXFLAGS := $(DEBUG_CXXFLAGS)
debug: LDFLAGS  := $(DEBUG_LDFLAGS)
debug: $(PROJECT)

.PHONY: sanitizer
sanitizer: CC       := $(SANITIZER_CC)
sanitizer: CXX      := $(SANITIZER_CXX)
sanitizer: CFLAGS   := $(SANITIZER_CFLAGS)
sanitizer: CXXFLAGS := $(SANITIZER_CXXFLAGS)
sanitizer: LDFLAGS  := $(SANITIZER_LDFLAGS)
sanitizer: $(PROJECT)


# generate and load the dependency graph
Makefile.deps: | $(CC_SOURCES) $(CXX_SOURCES) $(HEADERS)
	$(RM) Makefile.deps
ifneq ($(CC_SOURCES),)
	$(CC)  $(CFLAGS)   -MM $(CC_SOURCES)  >> Makefile.deps
endif
ifneq ($(CXX_SOURCES),)
	$(CXX) $(CXXFLAGS) -MM $(CXX_SOURCES) >> Makefile.deps
endif
-include Makefile.deps
