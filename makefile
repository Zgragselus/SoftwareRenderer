CC=gcc
CFLAGS=-c -g -m64 -mtune=generic -msse -msse2 -fms-extensions -ffast-math -mfpmath=sse -mpc32 -O3 -pedantic -fomit-frame-pointer -funsafe-math-optimizations -std=c99 -Wall -Wextra `pkg-config --cflags --libs gtk+-2.0 gtkglext-1.0 gtkglext-x11-1.0`
#CFLAGS=-c -g -m64 -std=c99 -Wall -Wextra `pkg-config --cflags --libs gtk+-2.0 gtkglext-1.0 gtkglext-x11-1.0`
LDFLAGS=-export-dynamic `pkg-config --cflags --libs gtk+-2.0 gtkglext-1.0 gtkglext-x11-1.0`
SOURCES=$(wildcard src/*.c) \
				$(wildcard src/graphics/*.c) \
				$(wildcard src/math/*.c) \
				$(wildcard src/memory/*.c) \
				$(wildcard src/textures/*.c)
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=main

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean: 
	rm -rf src/*.o
	rm -rf src/graphics/*.o
	rm -rf src/math/*.o
	rm -rf src/memory/*.o
	rm -rf src/textures/*.o
	
glade:
	gtk-builder-convert data/gui/gWindow.glade data/gui/gWindow.xml
