#include <vgcp.h>

static void allocateMoreToList(void);
static bool compareBoards(struct tile *board1, struct tile *board2);
static bool compareCastlingRights(bool *castlingRights1, bool *castlingRights2);

void initPosition(void) {
        uint8_t i, j;

	/* the amount of pieces at the start of the game */
	const uint8_t startingPieces[empty] = { 
		16,	/* pawns			*/
		4,	/* knights 			*/
		4,	/* bishops			*/
		4,	/* rooks			*/
		2, 	/* queens			*/
		30,	/* total (kings aren't counted)	*/
	};

        initBoard();

	/* this indicates that before this move there were 
	 * no previous moves (because the game just started) */
        position.prevMove.color = noColor;
	position.prevMove.piece = empty;
	position.prevMove.capturedPiece = empty;

        position.playerToMove = colorWhite;

        for(i = 0; i < 2; i++)
                for(j = 0; j < 2; j++)
                        position.castlingRights[i][j] = true;

	for(i = 0; i < empty; i++)
		position.piecesArray[i] = startingPieces[i];


	addToPositionList();
}

void updatePosition(struct move move) {
        position.prevMove = move;
        position.playerToMove = OPPOSITE_COLOR(move.color);

	if(position.prevMove.capturedPiece != empty) {
		position.piecesArray[position.prevMove.capturedPiece]--;
		position.piecesArray[king]--; /* also update the total amount of pieces */
	}
}

/* initializes the move list */
void initPositionList(void) {
	positionList.allocated = 5;
	positionList.positions = (struct position *)calloc(positionList.allocated,
			sizeof(struct position));
	positionList.n = 0;
}

void removePositionList(void) {
	if(positionList.positions != NULL)
		free(positionList.positions);
	positionList.positions = NULL;
	positionList.allocated = 0;
	positionList.n = 0;
}

/* allocates more space to the "positionList.positions" pointer */
static void allocateMoreToList(void) {
	positionList.allocated += 5;
	positionList.positions = (struct position *)realloc(positionList.positions,
			positionList.allocated * sizeof(struct position));

	if(positionList.positions == NULL)
		die("memory reallocation failed");
}

/* adds current position to positionList 
 * NOTE: Must not be called before calling initPositionList */
void addToPositionList(void) {
	uint8_t x, y;

	/* if it's the first move we can skip the irreversible move check */
	if(positionList.n == 0);

	/* if this is true than it's an irreversible move so we don't need to
	 * check if it repeats. In practice, this means that we can delete the 
	 * current position list */
	else if(position.prevMove.piece == pawn 
			|| (position.prevMove.capturedPiece != empty)
			|| !compareCastlingRights((bool *)&(positionList.positions[positionList.n-1].castlingRights), (bool *)&(position.castlingRights))) {
		removePositionList();
		initPositionList();
	}

	/* if too little is allocated for the pointer (which stores the moves)
	 * then more must be allocated */
	if(positionList.allocated <= positionList.n + 1) {
		allocateMoreToList();
	}

	for(y = 0; y < 8; y++) {
		for(x = 0; x < 8; x++) {
			positionList.positions[positionList.n].board[y][x] = position.board[y][x];
		}
	}

	for(y = 0; y < 2; y++) {
		for(x = 0; x < 2; x++) {
			positionList.positions[positionList.n].castlingRights[y][x] =
				position.castlingRights[y][x];
		}
	}

	positionList.positions[positionList.n].prevMove = position.prevMove;

	positionList.n += 1;
}

bool checkForRepitition(uint8_t n) {
	uint8_t i, repitition;

	repitition = 0;

	for(i = 0; i < positionList.n; i++) {
		if(compareBoards((struct tile *)&(positionList.positions[i].board), (struct tile *)&(position.board))) {
			repitition++;
			if(repitition >= (n - 1))
				return true;
		}
	}

#ifdef _DEBUG
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "this position has been repeated %d times\n", repitition);
#endif

	if(repitition >= n)
		return true;
	return false;
}

static bool compareBoards(struct tile *board1, struct tile *board2) {
	uint8_t i;

	for(i = 0; i < 64; i++) {
		if((board1+i)->piece == (board2+i)->piece &&
				(board1+i)->color == (board2+i)->color);
		else if((board1+i)->piece == empty && (board2+i)->piece == empty);
		else
			return false;
	}
	return true;
}

static bool compareCastlingRights(bool *castlingRights1, bool *castlingRights2) {
	uint8_t i;

	for(i = 0; i < 4; i++) {
		if(*(castlingRights1+i) != *(castlingRights2+i))
			return false;
	}

	return true;
}

void printPositionList(void) {
	uint8_t i;

	puts("printing out position list");
	printf("number of entries %d and allocated %d\n", positionList.n, positionList.allocated);

	for(i = 0; i < positionList.n; i++) {
		printf("entry %d:  prevMove %d, %d -> %d, %d square 0,0 piece %d %s square 7,7 piece %d %s rights white: %d, %d ;; black: %d, %d\n",
				i,
				positionList.positions[i].prevMove.from.x, 
				positionList.positions[i].prevMove.from.y,
				positionList.positions[i].prevMove.to.x,
				positionList.positions[i].prevMove.to.y,
				positionList.positions[i].board[0][0].piece,
				COLOR_INT_TO_STR(positionList.positions[i].board[0][0].color),
				positionList.positions[i].board[7][7].piece,
				COLOR_INT_TO_STR(positionList.positions[i].board[7][7].color),
				positionList.positions[i].castlingRights[colorWhite][0], 
				positionList.positions[i].castlingRights[colorWhite][1], 
				positionList.positions[i].castlingRights[colorBlack][0], 
				positionList.positions[i].castlingRights[colorBlack][1] 
				);
	}
}
