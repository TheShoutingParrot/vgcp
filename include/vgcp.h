#ifndef __VGCP_H
#define __VGCP_H

#include <config.h>

/* warnings */
#ifdef  _DEBUG
#warning "DEBUG OPTION IS ON!"
#endif

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

/* vgcp's version (MAJOR-MINOR) */
#define PROGRAM_VERSION         "00-0010"

#define GAME_NAME               "vgcp"
#define GAME_LOGICAL_WIDTH      800
#define GAME_LOGICAL_HEIGHT     800
#define GAME_WINDOW_WIDTH       800
#define GAME_WINDOW_HEIGHT      800

#define BOARD_FIRST_ROW(c)      {{rook, c, normalState, false}, {knight, c, normalState, false}, {bishop, c, normalState, false}, {queen, c, normalState, false}, {king, c, normalState, false}, {bishop, c, normalState, false}, {knight, c, normalState, false}, {rook, c, normalState, false}}
#define BOARD_SECOND_ROW(c)     {{pawn, c, normalState, false}, {pawn, c, normalState, false}, {pawn, c, normalState, false}, {pawn, c, normalState, false}, {pawn, c, normalState, false}, {pawn, c, normalState, false}, {pawn, c, normalState, false}, {pawn, c, normalState, false}}
#define BOARD_EMPTY_ROW         {{empty, noColor, normalState, false}, {empty, noColor, normalState, false}, {empty, noColor, normalState, false}, {empty, noColor, normalState, false}, {empty, noColor, normalState, false}, {empty, noColor, normalState, false}, {empty, noColor, normalState, false}, {empty, noColor, normalState, false}}

#define BOARD_ROW_REVERSE(y)    (7 - y)

#define BOARD_ROW(y, r)         (BOARD_ROW_REVERSE(y))

#define BOARD_ROW_PLAYER(y, r)	(r ? BOARD_ROW_REVERSE(y) : y)

#define PLUS_OR_MINUS(c, x)     + (c ? -x : x)

#define COLOR_INT_TO_STR(c)     (c == colorWhite ? "white" : "black")

#define OPPOSITE_COLOR(c)	(c == colorWhite ? colorBlack : colorWhite)

#ifdef _INSTALL
#define ASSETS_PATH             _INSTALL_PREFIX "assets/"
#else
#define ASSETS_PATH             "assets/"
#endif

#define PIECES_IMG_PATH         ASSETS_PATH "pieces/"
#define FONT_PATH		ASSETS_PATH "font/"

#define ANSI_ESCAPE_CODE(c)     "\033["c"m"

#define ESCAPE_CODE_RESET       "0"
#define ESCAPE_CODE_BOLD        "1"
#define ESCAPE_CODE_RED         "31"

/* enums */

typedef enum color {
        colorWhite 	= 0,
        colorBlack 	= 1,
	noColor 	= 2,
} color_t;

enum pieces {
        pawn,
        knight,
        bishop,
        rook,
        queen,
        king,
        empty,
};

enum tileState {
        normalState             = 0x01,
        potentialMove           = 0x02,
        underThreatByWhite      = 0x04,
        underThreatByBlack      = 0x08,
	potentialCastling	= 0x10,
	potentialEnPassant	= 0x20,
};

enum userEvents {
        GAMEOVER_EVENT,
};

enum buttonStates {
        normal,
        mouseIn,
        clicked,
        allButtonStates,
};

/* structs */

struct tile {
        uint8_t         piece;
        color_t		color;
        uint8_t         tileState;
        bool            pieceHasMoved;
};

struct button {
        SDL_Texture     *texture[allButtonStates];
        SDL_Rect        rect,
                        backgroundRect;
        SDL_Color       backgroundColor[allButtonStates];
        uint8_t         state;
};

struct labels {
	SDL_Texture	*texture;
	SDL_Rect	rect,
			background;
};

struct move {
	SDL_Point	to,
		  	from;
	color_t		color;
	uint8_t		piece,
			capturedPiece;
};

struct position {
	struct tile	board[8][8];
	struct move	prevMove;
	bool		castlingRights[2][2],
			enPassantRights[2][8];
	SDL_Point	kingLocation[noColor];
	uint8_t		piecesArray[empty]; /* here we store the amount of pieces */
	color_t		playerToMove;
};

struct positionList {
	struct position	*positions;
	uint16_t	n,
			allocated;
};

/* variables */

extern SDL_Window		*gameWindow;
extern SDL_Renderer		*gameRenderer;

extern SDL_Point		selectedPiece;

extern bool			kingMated[2],
				checkingIfCheckMated;

extern struct tile 		unusedBoard[8][8];

extern char 			*pieceName[empty];

extern SDL_Texture		*pieceTexture[empty][2];

extern struct labels		textTexture[4];
extern struct button		newGameButton;

extern TTF_Font			*gameFont;

extern const int8_t		potentialKnight[4][2];

extern uint8_t			halfmoveClock;

extern struct position		position;
extern struct positionList	positionList;

/* function prototypes (files can be all found in the "src/" directory) */

/* main.c */
void handleMousebuttonEvent(SDL_MouseButtonEvent event);
void gameOver(color_t winner);

/* init.c*/
bool init(void);
bool loadMedia(void);
void initBoard(void);

/* draw.c */
void updateWindow(void);
void drawBoard(void);
void drawBoardRow(bool *color, SDL_Rect *tileRect, uint8_t y); 
void drawGameOverText(color_t winner);
SDL_Texture *loadTexture(char *path);
SDL_Texture *renderText(char *textStr, SDL_Color textColor);

/* move.c */
void addPotentialMove(uint8_t y, uint8_t x, uint8_t pieceY, uint8_t pieceX);
void addPotentialCastling(color_t color, bool longCastle);
void addPotentialEnPassant(uint8_t capturedY, uint8_t capturedX, uint8_t y, uint8_t x, uint8_t pieceY, uint8_t pieceX);
uint8_t countAllPotentialMoves(color_t color);
void movePiece(struct move move);
void castleKing(color_t color, bool longCastle);
void enPassant(uint8_t capturedY, uint8_t capturedX, struct move move);
void checkIfMated(color_t color, bool fromMove);
void updateBoard(void);
void updateHalfmoveClock(struct move move);

/* select.c */
void selectPiece(uint8_t y, uint8_t x);
void deselectPiece(void);

/* piece.c */
void mapPawnPotentialMoves(uint8_t x, uint8_t y);
void mapKnightPotentialMoves(uint8_t x, uint8_t y, uint8_t state);
void mapBishopPotentialMoves(uint8_t x, uint8_t y, uint8_t state);
void mapRookPotentialMoves(uint8_t x, uint8_t y, uint8_t state);
void mapKingPotentialMoves(uint8_t x, uint8_t y, uint8_t state, bool fromMove);
bool checkIfEnoughPieces(void);

/* position.c */
void updatePosition(struct move move);
void initPosition(void);
void initPositionList(void);
void removePositionList(void);
void addToPositionList(void);
void printPositionList(void);
bool checkForRepitition(uint8_t n);

/* util.c */
void cleanup(void);
void usage(const char *name);
void help(const char *name);
void die(char *fmt, ...);

#endif /* #ifndef __VGCP_H */
