#path to xbmc includes: addons/include/xbmc_vis...
XBMC=../../xbmc/xbmc

CC=		gcc
CPP=		g++
CFLAGS=		-c -O2 -fPIC -Wall
CPPFLAGS=	-c -O2 -fPIC -Wall
LDFLAGS=	-shared
OBJECTS=	src/analyst.o \
		src/audiobuffer.o \
		src/blurengine.o \
		src/cpudetect.o \
		src/fische.o \
		src/screenbuffer.o \
		src/vector.o \
		src/vectorfield.o \
		src/wavepainter.o \
		src/fishbmc_addon.o

PLUGIN=fishbmc-linux.vis

all: $(PLUGIN)

$(PLUGIN): $(OBJECTS)
	$(CPP) $(LDFLAGS) $(OBJECTS) -o $@
	strip --strip-unneeded $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

.cpp.o:
	$(CPP) $(CFLAGS) -I$(XBMC) $< -o $@

clean:
	rm -f src/*.o
	rm -f fishbmc-linux.vis

