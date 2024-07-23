# 2024-07-21  William A. Hudson
#
# Compile and run the main program.

# Outputs in $(BINDIR)/ and $(OJ)/

SHELL      = /bin/sh
BINDIR     = ../bin
OJ         = ../obj

		# the librgpio library
INCDIR     = ../../flycapture
LIBDIR     = ?
LIBS       = ?

		# common include dependencies
#INCS       = \

		# objects not including main()
OBJS       = \
	$(OJ)/onecam.o

		# link libraries, order matters
#LIBFLAGS   = -lcap

		# compiler flags
CXXFLAGS   = -Wall  -std=c++11  -I $(INCDIR)


obj:	$(OJ)/onecam.o

run:	compile
	$(BINDIR)/onecam --TESTOP

compile:  $(OJ)  $(BINDIR)  $(BINDIR)/onecam

mkdirs:   $(OJ)  $(BINDIR)

clean:
	rm -f  $(OJ)/*

clobber:
	rm -f  $(OJ)/*
	rm -f  $(BINDIR)/onecam

$(BINDIR):
	mkdir -p  $(BINDIR)

$(OJ):
	mkdir -p  $(OJ)


$(BINDIR)/onecam:	        $(OJ)/onecam.o  $(OBJS)  $(LIBS)
	g++ -o $@ -L $(LIBDIR)  $(OJ)/onecam.o  $(OBJS)  $(LIBFLAGS)


$(OJ)/onecam.o:		onecam.cpp
	g++ $(CXXFLAGS) -o $@  -c onecam.cpp

