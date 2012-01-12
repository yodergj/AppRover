OS = $(shell uname -s)
CPP = g++
ifeq ($(OS),BeOS)
INCFLAGS = -I /boot/home/config/include/libxml2 -I ../libsrc
LDFLAGS = -L /boot/home/config/lib
LIBS = -lxml2 -lz
APP_ROVER_DIR = '"/boot/home/config/AppRover/"'
else
INCFLAGS = -I /usr/include/libxml2 -I ../libsrc
LDFLAGS =
LIBS = -lxml2
APP_ROVER_DIR = '"/usr/AppRover/"'
endif
CFLAGS = -g -Wall -c $(INCFLAGS) -DAPP_ROVER_DIR=$(APP_ROVER_DIR)
OBJS = CLI.o
ROVERLIB = ../libsrc/AppRover.a

all: AppRover

clean:
	rm -f AppRover *.o

ifeq ($(OS),BeOS)
install: AppRover
	install $< /boot/home/config/bin
else
install: AppRover
	install -m 0744 $^ /usr/bin
endif

AppRover: $(OBJS) $(ROVERLIB)
	$(CPP) $(LDFLAGS) -o $@ $(OBJS) $(ROVERLIB) $(LIBS)

$(ROVERLIB):
	cd ../libsrc && $(MAKE) && cd ../CLI

dep:
	@ echo "Generating dependencies"
	@ $(CPP) $(INCFLAGS) -MM  $(OBJS:.o=.cpp) > .depends
	@ echo "Finished generating dependencies"

.depends:
	@ touch .depends
	@ $(MAKE) dep

.SUFFIXES : .o .cpp

.cpp.o :
	$(CPP) $(CFLAGS) $<

include .depends
