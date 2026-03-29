# You may modify this file as you wish, but `make animate.o` should always
# compile all necessary code into a single object file.

# SHELL PATH and FILE SUFFIXES used in the compiling process
SHELL = /bin/sh
.SUFFIXES:
.SUFFIXES: .c .o

# COMPILER AND COMPILER FLAGS
CC = gcc
CFLAGS := -Wall -Werror
DEBUG ?= 0
ifeq ($(DEBUG), 1)
	CFLAGS += -DDEBUG -g
endif

# SOURCE FILES, HEADER FILES
INCDIR = include
SRCDIR = src
OBJDIR = obj

VPATH = $(SRCDIR):$(INCDIR):.
INC = -Iinclude	-I.	# let gcc know where to look for .h files included (#include)

HEADERS = $(wildcard $(INCDIR)/*.h)
SRCFILES = $(wildcard $(SRCDIR)/*.c) main_simple.c
OTHER_OBJS = $(addprefix $(OBJDIR)/, $(notdir $(SRCFILES:.c=.o)))
OBJFILES = $(filter-out $(OBJDIR)/animate.o $(OBJDIR)/main_simple.o, $(OTHER_OBJS))

# to make it verbose, add VERBOSE=1 to the make command
ifeq ($(VERBOSE),1)
Q =
else
Q = @
endif


# MAKE RULES
# left depends on right
# $@ = The file name of the target of the rule
# $< = first thing on the right (for .c to .o)
# $^ = everything on the right (for linking)

default: animate.o test

test: $(OBJDIR)/main_simple.o animate.o
	$(Q)$(CC) $(CFLAGS) $(INC) $^ -o $@

test_asan: $(OBJDIR)/main_simple.o animate.o
	$(Q)$(CC) $(CFLAGS) -fsanitize=address $(INC) $^ -o $@

animate.o: $(OBJDIR)/animate.o $(OBJFILES)
	$(Q)ld -r $^ -o $@

$(OBJDIR)/%.o: %.c $(HEADERS) animate.h
	$(Q)mkdir -p $(OBJDIR)
	$(Q)$(CC) $(CFLAGS) $(INC) -fPIC -c $< -o $@

API_DOC=PointerProAnimateRefman.pdf
doc: $(API_DOC)

# Generates a doxygen configuration file
Doxyfile:
	doxygen -g
	# Adjust params
	sed -i 's/\(GENERATE_HTML *= *\).*/\1NO/g' $@
	sed -i 's/\(EXTRACT_ALL *= *\).*/\1YES/g' $@
	sed -i 's/\(INPUT *= *\).*/\1"animate.h"/g' $@
	sed -i 's/\(PROJECT_NAME *= *\).*/\1"PointerPro Animate"/g' $@
	sed -i 's/\(OPTIMIZE_OUTPUT_FOR_C *= *\).*/\1YES/g' $@

$(API_DOC): DOC_MAKEFILE=latex/Makefile
$(API_DOC): DOC_TOP=latex/refman.tex
$(API_DOC): Doxyfile | animate.h
	doxygen $^
	# Don't index
	sed -i 's/\t$$(MKIDX/\t#/g'             $(DOC_MAKEFILE)
	# Remove some dead chapters and ToC
	sed -i 's/^ *\\clearemptydoublepage//g' $(DOC_TOP)
	sed -i 's/^ *\\tableofcontents//g'      $(DOC_TOP)
	sed -i 's/^ *\\chapter{File Index}//g'  $(DOC_TOP)
	sed -i 's/^ *\\input{files}//g'         $(DOC_TOP)
	cd latex && make
	cp latex/refman.pdf $@

clean:
	$(Q)rm -rf $(OBJDIR)
	$(Q)rm -f test_simple
	$(Q)rm -f animate.o
	$(Q)rm -f Doxyfile
	$(Q)rm -rf latex

clobber: clean
	$(Q)rm -f $(API_DOC)

.PHONY: doc clean clobber default test