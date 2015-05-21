CC=gcc
CXX=g++
CFLAGS=-Wall -O3 -s -fomit-frame-pointer
CXXFLAGS=
LDFLAGS=
LIBS=-lm

objects = main.o models.o itwom3.0.o sdf.o

signalserver: $(objects)
	@echo -e "  LNK\t$@"
	@$(CXX) $(LDFLAGS) $(objects) -o signalserver ${LIBS}

main.o: main.cpp itwom3.0.cpp sdf.c common.h
	@echo -e "  CXX\t$@"
	@$(CXX) $(CFLAGS) -c main.cpp ${INCS}

models.o: models.cpp
	@echo -e "  CXX\t$@"
	@$(CXX) $(CFLAGS) -c models.cpp ${INCS}

itwom3.0.o: itwom3.0.cpp
	@echo -e "  CXX\t$@"
	@$(CXX) $(CFLAGS) -c itwom3.0.cpp ${INCS}

sdf.o: sdf.c common.h
	@echo -e "  CC\t$@"
	@$(CC) $(CFLAGS) -c sdf.c ${INCS}

clean:
	rm -f *.o signalserver
