CC		= gcc
CFLAGS		= -I $(INC_DIR) -I $(SDL_INC_DIR) -g
LIBS		= -lSDL2 -lSDL2_image -lSDL2_ttf

INC_DIR 	= include
SDL_INC_DIR	= /usr/include/SDL2
SRC_DIR		= src
OBJ_DIR 	= obj

PREFIX		= /usr/local
APPLICATIONS	= /usr/share/applications

_DEPS		= vgcp.h
DEPS		= $(patsubst %,$(INC_DIR)/%,$(_DEPS))

_OBJ		= main.o init.o draw.o move.o piece.o position.o select.o connection.o util.o
OBJ		= $(patsubst %,$(OBJ_DIR)/%,$(_OBJ))

all: vgcp clean

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(DEPS)
	mkdir -p $(OBJ_DIR)
	$(CC) -c -o $@ $< $(CFLAGS)

vgcp: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	rm -f $(OBJ_DIR)/*.o

install: vgcp clean
	mkdir -p $(PREFIX)/bin
	cp vgcp $(PREFIX)/bin/vgcp
	chmod +x $(PREFIX)/bin/vgcp
	mkdir -p $(PREFIX)/share/vgcp
	cp -r assets/ $(PREFIX)/share/vgcp
	cp vgcp.desktop $(APPLICATIONS)/vgcp.desktop

uninstall:
	rm -f $(PREFIX)/bin/vgcp
	rm -f -r $(PREFIX)/share/vgcp
	rm $(APPLICATIONS)/vgcp.desktop

.PHONY: clean install uninstall
