#ifndef __VGCP_H
#define __VGCP_H

#include <config.h>

#ifdef  _DEBUG
#warning "DEBUG OPTION IS ON!!"
#endif

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

/* vgcp's version (MAJOR-MINOR) */
#define PROGRAM_VERSION         "00-0001"

#define GAME_NAME               "vgcp"
#define GAME_LOGICAL_WIDTH      800
#define GAME_LOGICAL_HEIGHT     800
#define GAME_WINDOW_WIDTH       1000
#define GAME_WINDOW_HEIGHT      1000


#define BOARD_FIRST_ROW(c)      {{rook, c, normalState, false}, {knight, c, normalState, false}, {bishop, c, normalState, false}, {queen, c, normalState, false}, {king, c, normalState, false}, {bishop, c, normalState, false}, {knight, c, normalState, false}, {rook, c, normalState, false}}
#define BOARD_SECOND_ROW(c)     {{pawn, c, normalState, false}, {pawn, c, normalState, false}, {pawn, c, normalState, false}, {pawn, c, normalState, false}, {pawn, c, normalState, false}, {pawn, c, normalState, false}, {pawn, c, normalState, false}, {pawn, c, normalState, false}}
#define BOARD_EMPTY_ROW         {{empty, 0, normalState, false}, {empty, 0, normalState, false}, {empty, 0, normalState, false}, {empty, 0, normalState, false}, {empty, 0, normalState, false}, {empty, 0, normalState, false}, {empty, 0, normalState, false}, {empty, 0, normalState, false}}

#define BOARD_ROW_REVERSE(y)    (7 - y)

#ifndef _NO_PERSPECTIVE_CHANGE
#define BOARD_ROW(y, r)         (r ? BOARD_ROW_REVERSE(y) : y)
#else
#define BOARD_ROW(y, r)         (BOARD_ROW_REVERSE(y))
#endif

#define PLUS_OR_MINUS(c, x)     + (c ? -x : x)

#define COLOR_INT_TO_STR(c)     (c == colorWhite ? "white" : "black")

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

enum color {
        colorWhite = false,
        colorBlack = true,
};

enum pieces {
        pawn,
        knight,
        bishop,
        rook ,
        queen,
        king,
        empty,
};

enum tileState {
        normalState             = 0x01,
        potentialMove           = 0x02,
        underThreatByWhite      = 0x04,
        underThreatByBlack      = 0x08,
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

struct tile {
        uint8_t         piece;
        bool            color;
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

SDL_Window      *gameWindow;
SDL_Renderer    *gameRenderer;

extern SDL_Point	selectedPiece,
			kingLocation[2];
bool            	gameTurn;
extern bool		kingMated[2],
                	checkingIfCheckMated;

extern struct tile unusedBoard[8][8];
struct tile board[8][8];

extern char *pieceName[empty];

SDL_Texture     *pieceTexture[empty][2];
SDL_Texture     *textTexture[4];
struct button   newGameButton; 

TTF_Font        *gameFont;

extern int8_t potentialKnight[4][2];

/* function prototypes*/

/* main.c */
void handleMousebuttonEvent(SDL_MouseButtonEvent event);
void gameOver(bool winner);

/* init.c*/
bool init(void);
bool loadMedia(void);
void initBoard(void);

/* draw.c */
void updateWindow(void);
void drawBoard(void);
void drawBoardRow(bool *color, SDL_Rect *tileRect, uint8_t y); 
void drawGameOverText(void);
SDL_Texture *loadTexture(char *path);
SDL_Texture *renderText(char *textStr, SDL_Color textColor);

/* move.c */
void addPotentialMove(uint8_t y, uint8_t x, uint8_t pieceY, uint8_t pieceX);
uint8_t countAllPotentialMoves(bool color);
void movePiece(uint8_t y, uint8_t x);
void checkIfMated(bool color, bool fromMove);
void updateBoard(void);

/* select.c */
void selectPiece(uint8_t y, uint8_t x);
void deselectPiece(void);

/* piece.c */
void mapPawnPotentialMoves(uint8_t x, uint8_t y);
void mapKnightPotentialMoves(uint8_t x, uint8_t y, uint8_t state);
void mapBishopPotentialMoves(uint8_t x, uint8_t y, uint8_t state);
void mapRookPotentialMoves(uint8_t x, uint8_t y, uint8_t state);
void mapKingPotentialMoves(uint8_t x, uint8_t y, uint8_t state, bool fromMove);

/* util.c */
void cleanup(void);
void die(char *fmt, ...);

#endif /* #ifndef __VGCP_H */
