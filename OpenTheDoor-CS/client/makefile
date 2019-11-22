BINFILE = door_client

ifdef VERBOSE
        Q =
        E = @true 
else
        Q = @
        E = @echo 
endif

$(shell if [ ! -d build/obj ]; then mkdir -p build; fi; )

CFILES := $(shell find src -mindepth 1 -maxdepth 4 -name "*.c")
CXXFILES := $(shell find src -mindepth 1 -maxdepth 4 -name "*.cpp")

INFILES := $(CFILES) $(CXXFILES)

OBJFILES := $(CXXFILES:src/%.cpp=%) $(CFILES:src/%.c=%)
DEPFILES := $(CXXFILES:src/%.cpp=%) $(CFILES:src/%.c=%)
OFILES := $(OBJFILES:%=build/obj/%.o)


#COMMONFLAGS := -Wextra -pedantic -O3 
COMMONFLAGS := -O3 
INCLUDE := -I. -I./include
LIBS    := -ljsoncpp \
					 -lpthread \
					 -lwiringPi \
					 -lcurl -lssl\
		       -lcrypto \
					 -lz

ifdef DEBUG
        COMMONFLAGS := $(COMMONFLAGS) -g
endif
CFLAGS := $(COMMONFLAGS)  $(INCLUDE)
CXXFLAGS := $(COMMONFLAGS) --std=c++14 $(INCLUDE)
DEPDIR := deps

all: $(BINFILE)
ifeq ($(MAKECMDGOALS),)
-include build/Makefile.dep
endif
ifneq ($(filter-out clean, $(MAKECMDGOALS)),)
-include build/Makefile.dep
endif

CC = gcc
CXX = g++


-include build/Makefile.local

.PHONY: clean all depend
.SUFFIXES:
build/obj/%.o: src/%.c
	$(E)C-compiling $<
	$(Q)if [ ! -d `dirname $@` ]; then mkdir -p `dirname $@`; fi
	$(Q)$(CC) -o $@ -c $< $(CFLAGS)
build/obj/%.o: src/%.cpp
	$(E)C++-compiling $<
	$(Q)if [ ! -d `dirname $@` ]; then mkdir -p `dirname $@`; fi
	$(Q)$(CXX) -o $@ -c $< $(CXXFLAGS)
build/Makefile.dep: $(CFILES) $(CXXFILES)
	$(E)Depend
	$(Q)for i in $(^); do $(CXX) $(CXXFLAGS) -MM "$${i}" -MT build/obj/`basename $${i%.*}`.o; done > $@


$(BINFILE): $(OFILES)
	$(E)Linking $@
	$(Q)$(CXX) -o $@ $(OFILES) $(LIBS)
clean:
	$(E)Removing files
	$(Q)rm -f $(BINFILE) build/obj/* build/Makefile.dep
