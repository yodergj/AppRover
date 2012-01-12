CPP = g++
INCFLAGS = -I /boot/home/config/include/libxml2
LDFLAGS = -L /boot/home/config/lib
LIBS = -lxml2 -lz -lbe
CFLAGS = -g -c $(INCFLAGS)
SRC = bMain.cpp bAppRover.cpp bAppRoverWin.cpp bAppRoverView.cpp
OBJS = bMain.o bAppRover.o bAppRoverWin.o bAppRoverView.o
ROVERLIB = ../libsrc/AppRover.a

all: bAppRover

clean:
	rm -f bAppRover *.o

bAppRover: $(OBJS) $(ROVERLIB)
	$(CPP) $(LDFLAGS) -o $@ $(OBJS) $(ROVERLIB) $(LIBS)

install: bAppRover
	install $^ /boot/home/config/bin

$(ROVERLIB):
	cd ../libsrc && $(MAKE) && cd ../BeOS

dep:
	@ echo "Generating dependencies"
	@ $(CPP) $(INCFLAGS) -MM  $(SRC) > .depends
	@ echo "Finished generating dependencies"

.depends:
	@ touch .depends
	@ $(MAKE) dep

.SUFFIXES : .o .cpp

.cpp.o :
	$(CPP) $(CFLAGS) $<

include .depends
