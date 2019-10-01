#Compiler and Linker
CC          := g++

#The Target Binary Program
TARGET      := potions

#The Directories, Source, Includes, Objects, Binary and Resources
SRCDIR      := .
INCDIR      := .
BUILDDIR    := obj
TARGETDIR   := bin
RUNDIR      := run
SRCEXT      := cpp
DEPEXT      := d
OBJEXT      := o

UNAME_S := $(shell uname -s)

CFLAGS := -pedantic -pedantic-errors -std=c++17 -DRAPIDJSON_HAS_STDSTRING=1
CFLAGS += -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wold-style-cast -Wcast-align \
          -Wunused -Woverloaded-virtual -Wpedantic -Wconversion -Wsign-conversion \
		  -Wmisleading-indentation -Wduplicated-cond -Wduplicated-branches \
		  -Wlogical-op -Wnull-dereference -Wuseless-cast -Wdouble-promotion \
		  -Wformat=2
LIB    :=  

ifeq ($(UNAME_S),Darwin)
	LIB += -L/usr/local/opt/openssl@1.1/lib -L/usr/local/opt/curl/lib -L/usr/local/lib -L/usr/local/opt/llvm/lib -L/usr/local/Cellar/llvm/7.0.1/lib -lc++fs
	INC := -I/usr/local/opt/openssl@1.1/include -I/usr/local/opt/curl/include
	CC := clang++
else
	LIB += -lstdc++fs
	INC := -I/usr/include
endif

#Flags, Libraries and Includes
INC         += -I.. -I$(INCDIR) -I/usr/local/include -I/usr/local/curl/include
INCDEP      := $(INC)

SOURCES     := $(shell find $(SRCDIR) -type f -name "*.$(SRCEXT)")
OBJECTS     := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.$(OBJEXT)))

ifeq ($(DEBUG), 1)
	CFLAGS += -O0 -ggdb3 -DDEBUG $(GIT_FLAGS)
else
	CFLAGS += -Werror -O3 -ggdb3 -DRELEASE -DNDEBUG $(GIT_FLAGS)
endif

#Defauilt Make
all: resources directories $(TARGETDIR)/$(TARGET)

#Make the Directories
directories:
	@mkdir -p $(TARGETDIR)
	@mkdir -p $(BUILDDIR)
	@mkdir -p $(RUNDIR)

clean:
	@$(RM) -rf $(BUILDDIR)
	@$(RM) -f $(TARGETDIR)/$(TARGET)

#Pull in dependency info for *existing* .o files
-include $(OBJECTS:.$(OBJEXT)=.$(DEPEXT))

#Link
$(TARGETDIR)/$(TARGET): $(OBJECTS)
	$(CC) -o $(TARGETDIR)/$(TARGET) $^ $(LIB)

#Compile
$(BUILDDIR)/%.$(OBJEXT): $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<
	@$(CC) $(CFLAGS) $(INCDEP) -MM $(SRCDIR)/$*.$(SRCEXT) > $(BUILDDIR)/$*.$(DEPEXT)
	@cp -f $(BUILDDIR)/$*.$(DEPEXT) $(BUILDDIR)/$*.$(DEPEXT).tmp
	@sed -e 's|.*:|$(BUILDDIR)/$*.$(OBJEXT):|' < $(BUILDDIR)/$*.$(DEPEXT).tmp > $(BUILDDIR)/$*.$(DEPEXT)
	@sed -e 's/.*://' -e 's/\\$$//' < $(BUILDDIR)/$*.$(DEPEXT).tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $(BUILDDIR)/$*.$(DEPEXT)
	@rm -f $(BUILDDIR)/$*.$(DEPEXT).tmp

#Non-File Targets
.PHONY: all clean cleaner resources
