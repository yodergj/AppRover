CPP = g++
INCFLAGS = -I $$KDEDIR/include -I $$QTDIR/include -I /usr/include/libxml2 -I ../libsrc
LDFLAGS = -L $$KDEDIR/lib
LIBS = -lkdecore -lkdeui -lxml2
APP_ROVER_DIR = '"/usr/AppRover/"'
CFLAGS = -g -Wall -c $(INCFLAGS) -DAPP_ROVER_DIR=$(APP_ROVER_DIR)
SRC = kMain.cpp kAppRover.cpp kPackageWizard.cpp kInterpreter.cpp kWaitDialog.cpp
OBJS = kMain.o kAppRover.o mkAppRover.o kPackageWizard.o mkPackageWizard.o kInterpreter.o kWaitDialog.o mkWaitDialog.o
ROVERLIB = ../libsrc/AppRover.a

all: kAppRover

clean:
	rm -f kAppRover *.o

install: kAppRover
	install -D kAppRoverui.rc /usr/AppRover/KDE/kAppRoverui.rc
	install -m 0744 $^ /usr/bin

kAppRover: $(OBJS) $(ROVERLIB)
	$(CPP) $(LDFLAGS) -o $@ $(OBJS) $(ROVERLIB) $(LIBS)

$(ROVERLIB):
	@ cd ../libsrc && $(MAKE) && cd ../KDE

dep:
	@ echo "Generating dependencies"
	@ $(CPP) $(INCFLAGS) -MM  $(SRC) > .depends
	@ echo "Finished generating dependencies"

.depends:
	@ touch .depends
	@ $(MAKE) dep

.SUFFIXES : .o .cpp .h

.cpp.o :
	$(CPP) $(CFLAGS) $<

m%.cpp: %.h
	$$QTDIR/bin/moc $< -o $@

include .depends
