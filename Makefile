SHELL		= /bin/sh

CC		= gcc
CXX		= g++
CFLAGS		= -Wall -O3 -s -fomit-frame-pointer
CXXFLAGS	= -Wall -O3 -s -fomit-frame-pointer
LIBS		= -lm

VPATH		= models
objects 	= main.o models.o itwom3.0.o sdf.o

%.o : %.cc
	@echo -e "  CXX\t$@"
	@$ $(CXX) $(CXXFLAGS) -c $<

%.o : %.c
	@echo -e "  CC\t$@"
	@$ $(CC) $(CFLAGS) -c $<

signalserver: $(objects)
	@echo -e "  LNK\t$@"
	@$(CXX) $(objects) -o $@ ${LIBS}

main.o: main.cc models.cc itwom3.0.cc sdf.c common.h

sdf.o: sdf.c common.h

.PHONY: clean
clean:
	rm -f $(objects) signalserver
