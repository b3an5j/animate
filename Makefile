# You may modify this file as you wish, but `make animate.o` should always
# compile all necessary code into a single object file.

# SHELL PATH and FILE SUFFIXES used in the compiling process
SHELL = /bin/sh
.SUFFIXES:
.SUFFIXES: .c .o

MAKEFLAGS += --no-print-directory

# COMPILER AND COMPILER FLAGS
CC = gcc
CFLAGS := -Wall -Werror
LDFLAGS :=

# SOURCE FILES, HEADER FILES
INCDIR = include
SRCDIR = src
OBJDIR = obj

VPATH = $(SRCDIR):$(INCDIR):.
INC = -Iinclude	-I.	# let gcc know where to look for .h files included (#include)

HEADERS = $(wildcard $(INCDIR)/*.h) animate.h
SRCFILES = $(wildcard $(SRCDIR)/*.c)
TEST_SRC = main_simple.c

ANIMATE_OBJ = $(addprefix $(OBJDIR)/, $(notdir $(SRCFILES:.c=.o)))
EXAMPLE_OBJ = $(OBJDIR)/main_simple.o

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

default: animate.o example

test_asan:
	$(Q)$(MAKE) OBJDIR=obj/asan \
	CFLAGS="$(CFLAGS) -fsanitize=address -g" \
	LDFLAGS="$(LDFLAGS) -fsanitize=address" \
	default

test_debug:
	$(Q)$(MAKE) OBJDIR=obj/debug \
	CFLAGS="$(CFLAGS) -g" \
	default

animate.o: $(ANIMATE_OBJ)
	$(Q)rm -f animate.o
	@echo "Bundling into \"$@\" ..."
	$(Q)$(CC) $(LDFLAGS) -r $^ -o $@
	@echo "Done linking \"$@\"."

example: $(EXAMPLE_OBJ) animate.o
	$(Q)rm -f tests/example
	@echo "Linking example executable \"tests/$@\" ..."
	$(Q)$(CC) $(CFLAGS) $^ -o "./tests/$@"
	@echo "Done compiling \"$@\"."

$(OBJDIR)/%.o: %.c $(HEADERS)
	@mkdir -p $(dir $@D)
	$(Q)$(CC) $(CFLAGS) $(INC) -fPIC -c $< -o $@


API_DOC=PointerProAnimateRefman.pdf
doc: $(API_DOC)

# Generates a doxygen configuration file
Doxyfile:
	doxygen -g
	# Adjust params
	sed -i 's/\(GENERATE_HTML *= *\).*/\1NO/g' $@
	sed -i 's/\(EXTRACT_ALL *= *\).*/\1YES/g' $@
	sed -i 's/\(EXTRACT_STATIC *= *\).*/\1YES/g' $@
	sed -i 's/\(INLINE_INFO *= *\).*/\1YES/g' $@
	sed -i 's|\(INPUT *= *\).*|\1$(HEADERS)|g' $@
	sed -i 's/\(PROJECT_NAME *= *\).*/\1"PointerPro Animate"/g' $@
	sed -i 's/\(OPTIMIZE_OUTPUT_FOR_C *= *\).*/\1YES/g' $@
	sed -i 's/\(ENABLE_PREPROCESSING *= *\).*/\1YES/g' $@
	sed -i 's/\(MACRO_EXPANSION *= *\).*/\1YES/g' $@
	sed -i 's/\(PREDEFINED *= *\).*/\1DEBUG/g' $@
	sed -i 's/\(ENUM_VALUES_PER_LINE *= *\).*/\11/g' $@

$(API_DOC): DOC_MAKEFILE=latex/Makefile
$(API_DOC): DOC_TOP=latex/refman.tex
$(API_DOC): Doxyfile | animate.h
	doxygen $^
	# Don't index
	sed -i 's/\t$$(MKIDX/\t#/g'             $(DOC_MAKEFILE)
	# Remove some dead chapters and ToC
	sed -i 's/^ *\\clearemptydoublepage//g' $(DOC_TOP)
	sed -i 's/^ *\\chapter{File Index}//g'  $(DOC_TOP)
	sed -i 's/^ *\\input{files}//g'         $(DOC_TOP)
	cd latex && make
	cp latex/refman.pdf $@

clean:
	$(Q)rm -rf obj
	$(Q)rm -f Doxyfile
	$(Q)rm -rf latex

clobber: clean
	$(Q)rm -f animate.o
	$(Q)rm -rf tests/frames
	$(Q)rm -rf tests/out
	$(Q)rm -f tests/example
	$(Q)rm -f $(API_DOC)

.PHONY: doc clean clobber default example test_asan test_debug animate.o