#ifndef __VGCP_H
#define __VGCP_H

#include <config.h>

/* warnings */

#ifdef  _DEBUG
#warning "DEBUG OPTION IS ON!"
#endif

/* header files */

/* standard libraries */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

/* libraries for graphics */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

/* libraries for socket communication */
#include <sys/socket.h>
#include <netinet/in.h>

/* misc libraries */
#include <SDL2/SDL_thread.h>
#include <errno.h>
#include <fcntl.h>

/* definitions */

/* vgcp's version (MAJOR-MINOR) */
#define PROGRAM_VERSION         "00-0016"

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

typedef enum userEvents {
        GAMEOVER_EVENT,
	MOVED_EVENT,
} userEvents_t;


enum buttonStates {
        normal,
        mouseIn,
        clicked,
        allButtonStates,
};

typedef enum msgTypes {
	/* all of these are moves */
	MSG_MOVE,
	MSG_MOVE_AND_END, /* this is here just to simplify messaging */
	MSG_CASTLING_MOVE,
	MSG_ENPASSANT_MOVE,
	/* ---------------------- */

	MSG_INFO,
	MSG_ROGER,
	MSG_QUITTING,
	MSG_GAME_ENDS,
	MSG_NEW_GAME,
	MSG_UNKNOWN,
} msgType_t;

typedef enum serverStates {
	standby,
	waitingForBlack,
	waitingForWhite,
} serverState_t; 

/* structs (and unions) */

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

struct server {
	uint16_t port;

	struct sockaddr_in address;

	int		serverfd,
			socket,
			valread,
			opt;

	serverState_t	state;

	char 		in[30],
			out[30];
};

/* this is only used as a msgDataType */
struct lastMove {
	struct move	move;
	color_t 	winner; /* important to know if it's a draw */
};

union msgDataTypes {
	struct move 	move;
	struct lastMove	lastMove;
	color_t		winner,
			playerColor;
	char 		other[30];
};

struct msg {
	bool 			empty;
	msgType_t		type;
	union msgDataTypes 	data;
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

extern struct server		whiteServer;
extern struct server		blackServer;

extern SDL_Thread 		*whiteThreadID,
				*blackThreadID;

extern SDL_sem			*whiteServerDataLock,
       				*blackServerDataLock,
				*whiteMsgDataLock,
				*blackMsgDataLock;

extern bool			whiteOnPort,
      				blackOnPort;

extern struct msg		msgBlack,
				msgToBlack,
				msgWhite,
				msgToWhite;

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
void pushMoveEvent(color_t color);
bool checkMoveLegality(struct move move, color_t color);

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

/* connection.c */
bool connectToClient(struct server *gameServer, color_t color);
struct move convertPlayerMsgToMove(const char *msg);
char *convertMsgToString(struct msg msg);
void closeSocket(int socket);
int connectionHandlingThread(void *data);
void createBlackThread(void);
void createWhiteThread(void);

#endif /* #ifndef __VGCP_H */
