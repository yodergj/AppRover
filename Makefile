OS = $(shell uname -s)
REL_NUMBER = $(shell date +%Y%m%d)
REL_NAME = AppRover-$(REL_NUMBER)
REL_DIR = ~/AppRover-$(REL_NUMBER)
ifeq ($(OS),BeOS)
APP_ROVER_DIR = /boot/home/config/AppRover
all: lib AppRover bAppRover watcherlib
else
APP_ROVER_DIR = /usr/AppRover
all: lib AppRover kAppRover watcherlib
endif
APP_ROVER_DIRS = $(APP_ROVER_DIR) $(APP_ROVER_DIR)/utils $(APP_ROVER_DIR)/repo $(APP_ROVER_DIR)/storage $(APP_ROVER_DIR)/work $(APP_ROVER_DIR)/files

ifeq ($(OS),BeOS)
install: all
	@ cd CLI && $(MAKE) install && cd ..
	@ cd BeOS && $(MAKE) install && cd ..
	@ cd watcher && $(MAKE) install && cd ..
	install -d $(APP_ROVER_DIRS)
else
install: all
	@ cd CLI && $(MAKE) install && cd ..
	@ cd KDE && $(MAKE) install && cd ..
	@ cd watcher && $(MAKE) install && cd ..
	install -d $(APP_ROVER_DIRS)
endif

clean:
	@ cd CLI && $(MAKE) clean && cd ..
	@ cd KDE && $(MAKE) clean && cd ..
	@ cd BeOS && $(MAKE) clean && cd ..
	@ cd libsrc && $(MAKE) clean && cd ..
	@ cd watcher && $(MAKE) clean && cd ..
	@ cd utils && $(MAKE) clean && cd ..

AppRover:
	@ cd CLI && $(MAKE) && cd ..

kAppRover:
	@ cd KDE && $(MAKE) && cd ..

bAppRover:
	@ cd BeOS && $(MAKE) && cd ..

lib:
	@ cd libsrc && $(MAKE) && cd ..

watcherlib:
	@ cd watcher && $(MAKE) && cd ..

utilities:
	@ cd utils && $(MAKE) && cd ..

release: clean
	@ mkdir $(REL_DIR)
	@ cp -r * $(REL_DIR)
	@ cd $(REL_DIR) && rm -rf CVS .depends
	@ cd $(REL_DIR)/BeOS && rm -rf CVS .depends
	@ cd $(REL_DIR)/CLI && rm -rf CVS .depends
	@ cd $(REL_DIR)/KDE && rm -rf CVS .depends
	@ cd $(REL_DIR)/libsrc && rm -rf CVS .depends
	@ cd $(REL_DIR)/utils && rm -rf CVS .depends
	@ cd $(REL_DIR)/watcher && rm -rf CVS .depends
	@ cd $(REL_DIR)/docs && rm -rf CVS
	@ cd $(REL_DIR)/docs/html && rm -rf CVS
	@ cd $(REL_DIR)/docs/latex && rm -rf CVS
	@ cd ~ && tar -jcvf $(REL_NAME).tar.bz2 $(REL_NAME)
	@ rm -r $(REL_DIR)

ifeq ($(OS),BeOS)
dep:
	@ cd CLI && $(MAKE) dep && cd ..
	@ cd BeOS && $(MAKE) dep && cd ..
	@ cd libsrc && $(MAKE) dep && cd ..
else
dep:
	@ cd CLI && $(MAKE) dep && cd ..
	@ cd KDE && $(MAKE) dep && cd ..
	@ cd libsrc && $(MAKE) dep && cd ..
endif
