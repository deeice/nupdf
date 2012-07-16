CC=mipsel-linux-gcc
STRIP=mipsel-linux-strip
CFLAGS=-static  -lz  -ldl -lSDL -lmupdf  -lfitz -lfitzdraw -lcmaps -lfonts -lfreetype -lpthread -lm -ljpeg -lz  -L.
SOURCES=main.c pdfapp.c font.c spf.c

all: dingoo nanonote

dingoo:
	$(CC) $(SOURCES) $(CFLAGS) -DDINGOO_BUILD -o nupdf.dge
	$(STRIP) nupdf.dge

nanonote:
	$(CC) $(SOURCES) $(CFLAGS) -o nupdf
	$(STRIP) nupdf

debug: debug_dingoo debug_nanonote

debug_dingoo:
	$(CC) $(SOURCES) $(CFLAGS) -DDINGOO_BUILD -Wall -g -o nupdf.dge

debug_nanonote:
	$(CC) $(SOURCES) $(CFLAGS) -Wall -g -o nupdf

clean:
	rm nupdf nupdf.dge
