OS = $(shell uname -s)
CC = gcc
CPP = g++
AR = ar cvr
ifeq ($(OS),BeOS)
INCFLAGS =
LDFLAGS =
LIBS =
APP_ROVER_DIR = '"/boot/home/config/AppRover/"'
else
INCFLAGS =
LDFLAGS =
LIBS = -ldl
APP_ROVER_DIR = '"/usr/AppRover/"'
endif
SRC = watcher.c
TARGET = libwatcher.so
CFLAGS = -Wall -fPIC -shared $(INCFLAGS) -DAPP_ROVER_DIR=$(APP_ROVER_DIR)

all: libwatcher.so

clean:
	rm -f *.so *.o *.lreg

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

ifeq ($(OS),BeOS)
install: $(TARGET)
	install $< /boot/home/config/lib
else
install: $(TARGET)
	install -m 0755 $^ /usr/lib
endif

dep:
