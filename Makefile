SHELL		= /bin/sh

CC		= gcc
CXX		= g++
CFLAGS		= -Wall -O3 -s -fomit-frame-pointer
CXXFLAGS	= -Wall -O3 -s -fomit-frame-pointer
LIBS		= -lm

VPATH		= models
objects 	= main.o models.o itwom3.0.o los.o inputs.o outputs.o

GCC_MAJOR	:= $(shell $(CXX) -dumpversion 2>&1 | cut -d . -f 1)
GCC_MINOR	:= $(shell $(CXX) -dumpversion 2>&1 | cut -d . -f 2)
GCC_VER_OK	:= $(shell test $(GCC_MAJOR) -ge 4 && \
			   test $(GCC_MINOR) -ge 7 && \
			   echo 1)

ifneq "$(GCC_VER_OK)" "1"
error:
	@echo "Requires GCC version >= 4.7"
	@exit
endif

%.o : %.cc
	@echo -e "  CXX\t$@"
	@$ $(CXX) $(CXXFLAGS) -c $<

%.o : %.c
	@echo -e "  CC\t$@"
	@$ $(CC) $(CFLAGS) -c $<

signalserver: $(objects)
	@echo -e "  LNK\t$@"
	@$(CXX) $(objects) -o $@ ${LIBS}

main.o: main.cc models.cc itwom3.0.cc los.cc inputs.cc outputs.cc common.h

inputs.o: inputs.cc common.h

outputs.o: outputs.cc common.h

los.o: los.cc common.h

.PHONY: clean
clean:
	rm -f $(objects) signalserver
