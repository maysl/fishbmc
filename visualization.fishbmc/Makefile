#path to xbmc includes: addons/include/xbmc_vis...
# below one builds from xbmc/visualisations
XBMC=../..

ARCH=$(shell uname)

CC=		gcc
CPP=		g++
CFLAGS=		-c -O3 -fPIC -Wall
CPPFLAGS=	-c -O3 -fPIC -Wall
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

ifneq (,$(findstring Darwin, $(ARCH)))
    LDFLAGS += -framework OpenGL -arch i386
    CFLAGS += -arch i386
    CPPFLAGS += -arch i386
    PLUGIN=fishbmc-osx.vis
else
    LDFLAGS=	-shared
    PLUGIN=fishbmc-linux.vis
endif

all: $(PLUGIN)

$(PLUGIN): $(OBJECTS)
ifneq (,$(findstring Darwin, $(ARCH)))
	$(CPP) $(LDFLAGS) $(OBJECTS) -bundle -o $@
else
	$(CPP) $(LDFLAGS) $(OBJECTS) -o $@
	strip --strip-unneeded $@
endif	

.c.o:
	$(CC) $(CFLAGS) $< -o $@

.cpp.o:
	$(CPP) $(CFLAGS) -I$(XBMC) $< -o $@

clean:
	rm -f src/*.o
	rm -f $(PLUGIN)

