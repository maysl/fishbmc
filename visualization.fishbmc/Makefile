#path to xbmc includes: addons/include/xbmc_vis...
# below one builds from xbmc/visualisations
XBMC=			../..

ARCH=			$(shell uname)

CC=				gcc
CPP=			g++
MY_CFLAGS=		$(CFLAGS) -I. -c -O3 -fPIC -Wall
MY_CXXFLAGS=	$(CXXFLAGS) -I. -c -O3 -fPIC -Wall
OBJECTS=		src/analyst.o \
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
    
    LDFLAGS = -framework OpenGL -arch i386
    MY_CFLAGS += -arch i386
    MY_CXXFLAGS += -arch i386
    
    PLUGIN=fishbmc-osx.vis

else #ARCH Darwin

    LDFLAGS=-shared
    PLUGIN=fishbmc-linux.vis

endif #ARCH Darwin


all: $(PLUGIN)

$(PLUGIN): $(OBJECTS)

ifneq (,$(findstring Darwin, $(ARCH)))

	$(CPP) $(LDFLAGS) $(OBJECTS) -bundle -o $@

else #ARCH Darwin
	
	$(CPP) $(LDFLAGS) $(OBJECTS) -o $@
	strip --strip-unneeded $@

endif #ARCH Darwin	

.c.o:
	$(CC) $(MY_CFLAGS) $< -o $@

.cpp.o:
	$(CPP) $(MY_CXXFLAGS) -I$(XBMC) $< -o $@

clean:
	rm -f src/*.o
	rm -f $(PLUGIN)

