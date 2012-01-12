OS = $(shell uname -s)
CPP = g++
INSTALL = /bin/install
ifeq ($(OS),BeOS)
INCFLAGS =
LDFLAGS =
LIBS =
APP_ROVER_DIR = /boot/home/config/AppRover
else
INCFLAGS =
LDFLAGS =
LIBS =
APP_ROVER_DIR = /usr/AppRover
endif
APP_ROVER_DIRS = $(APP_ROVER_DIR) $(APP_ROVER_DIR)/utils $(APP_ROVER_DIR)/repo $(APP_ROVER_DIR)/storage $(APP_ROVER_DIR)/work $(APP_ROVER_DIR)/files
CFLAGS = -g -Wall -c $(INCFLAGS)
OBJS = cp.o mv.o ln.o install.o mkdir.o core.o hijack.o restore.o install-info.o pod2man.o
CP_OBJS = cp.o core.o
MV_OBJS = mv.o core.o
INSTALL_OBJS = install.o core.o
MKDIR_OBJS = mkdir.o core.o
LN_OBJS = ln.o core.o
HIJACK_OBJS = hijack.o core.o
RESTORE_OBJS = restore.o core.o
INSTALL_INFO_OBJS = install-info.o core.o
POD2MAN_OBJS = pod2man.o core.o

all: cp mv install mkdir ln hijack_system restore_system install-info pod2man

clean:
	rm -f cp mv install mkdir ln hijack_system restore_system install-info pod2man *.o

install_utils: cp mv install mkdir ln hijack_system restore_system install-info pod2man
	$(INSTALL) -d $(APP_ROVER_DIRS)
	$(INSTALL) $^ $(APP_ROVER_DIR)/utils

cp: $(CP_OBJS)
	$(CPP) $(LDFLAGS) -o $@ $(CP_OBJS) $(LIBS)

mv: $(MV_OBJS)
	$(CPP) $(LDFLAGS) -o $@ $(MV_OBJS) $(LIBS)

install: $(INSTALL_OBJS)
	$(CPP) $(LDFLAGS) -o $@ $(INSTALL_OBJS) $(LIBS)

mkdir: $(MKDIR_OBJS)
	$(CPP) $(LDFLAGS) -o $@ $(MKDIR_OBJS) $(LIBS)

ln: $(LN_OBJS)
	$(CPP) $(LDFLAGS) -o $@ $(LN_OBJS) $(LIBS)

hijack_system: $(HIJACK_OBJS)
	$(CPP) $(LDFLAGS) -o $@ $(HIJACK_OBJS) $(LIBS)

restore_system: $(RESTORE_OBJS)
	$(CPP) $(LDFLAGS) -o $@ $(RESTORE_OBJS) $(LIBS)

install-info: $(INSTALL_INFO_OBJS)
	$(CPP) $(LDFLAGS) -o $@ $(INSTALL_INFO_OBJS) $(LIBS)

pod2man: $(POD2MAN_OBJS)
	$(CPP) $(LDFLAGS) -o $@ $(POD2MAN_OBJS) $(LIBS)

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
