OS = $(shell uname -s)
CPP = g++
AR = ar cvr
ifeq ($(OS),BeOS)
INCFLAGS = -I /boot/home/config/include/libxml2
LDFLAGS =
LIBS =
APP_ROVER_DIR = '"/boot/home/config/AppRover/"'
else
INCFLAGS = -I /usr/include/libxml2
LDFLAGS =
LIBS =
APP_ROVER_DIR = '"/usr/AppRover/"'
endif
CFLAGS = -g -Wall -c $(INCFLAGS) -DAPP_ROVER_DIR=$(APP_ROVER_DIR)
OBJS = AppRover.o ConfigMgr.o Package.o XMLUtils.o Repository.o InstallLog.o InstallEntry.o PackageIndex.o PackageDependency.o Fetcher.o DirectoryUtils.o Interpreter.o InstructionSequence.o Revision.o ActionDescription.o FeatureOption.o SourceFile.o BooleanExpression.o InstalledFile.o

all: AppRover.a

test: test.o
	g++ -L /boot/home/config/lib -o test test.o -lxml2 -lz

clean:
	rm -f AppRover.a *.o

AppRover.a: $(OBJS)
	$(AR) $@ $?

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
