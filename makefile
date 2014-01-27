CXX=/usr/usc/gnu/gcc/4.8.1/bin/g++
CXXFLAGS=-std=c++0x -Wall -O3 
PROGRAM=pbmt
PURE=pure
INCLUDE=-L/usr/usc/gnu/gcc/4.8.1/lib -L/usr/usc/boost/1.51.0/lib
LINK=-lboost_program_options -lboost_regex

OBJ = Alignment.o \
	JKArgs.o \
	utils.o \
	PhraseTable.o \
	Hypothesis.o \
	Decode.o \
	main.o 

PO = pure.o \
	mp1.o \
	mp2.o \
	mp-init.o \
	mp.o \
	FracType.o \
	JKArgs.o

.SUFFIXES:
.SUFFIXES: .o .c .cpp

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $*.cpp

$(PROGRAM) : $(OBJ)
	$(CXX) -o $(PROGRAM) $(OBJ)

UPDATE :
	git pull origin master

$(PURE) : UPDATE $(PO)
	$(CXX) $(INCLUDE) $(LINK) -o $(PURE) $(PO)

all : $(PROGRAM)

clean :
	rm -f $(PROGRAM) $(PURE)
	rm -f *.gch
	rm -f *.o

