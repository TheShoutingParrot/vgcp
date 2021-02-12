#include <vgcp.h>

static void allocateMoreToList(void);

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

	positionList.positions[positionList.n].castlingRights;
	positionList.positions[positionList.n].prevMove = position.prevMove;

	positionList.n += 1;
}

void printPositionList(void) {
	uint8_t i;

	puts("printing out position list");
	printf("number of entries %d and allocated %d\n", positionList.n, positionList.allocated);

	for(i = 0; i < positionList.n; i++) {
		printf("entry %d:  prevMove %d, %d -> %d %d square 0,0 %d %s square 7,7 %d %s rights hwite: %d, %d ;; bleck: %d, %d\n",
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
