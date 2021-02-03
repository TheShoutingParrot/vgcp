#include <vgcp.h>

static void allocateMoreToList(void);

/* initializes the move list */
void initMoveList(void) {
	moves.allocated = 5;
	moves.moves = (struct move *)calloc(moves.allocated, sizeof(struct move));
	moves.n = 0;
}

void removeMoveList(void) {
	free(moves.moves);
	moves.moves = NULL;
	moves.allocated = 0;
	moves.n = 0;
}

/* allocates more space to the "moves" variable's pointer */
static void allocateMoreToList(void) {
	moves.allocated += 5;
	moves.moves = (struct move *)realloc(moves.moves, moves.allocated * sizeof(struct move));

	if(moves.moves == NULL) {
		die("memory reallocation failed");
	}
}

/* adds newMove to moveList 
 * NOTE: Must not be called before calling initMoveList */
void addToMoveList(struct move newMove) {
	/* if too little is allocated for the pointer (which stores the moves)
	 * then more must be allocated */
	if(moves.allocated <= moves.n + 1) {
		allocateMoreToList();
	}

	moves.moves[moves.n] = newMove;
	moves.n += 1;
}

void printMoveList(void) {
	uint8_t i;

	puts("printing out move list");
	printf("number of entries %d and allocated %d\n", moves.n, moves.allocated);

	for(i = 0; i < moves.n; i++) {
		printf("entry %d:\t%d, %d -> %d %d color %s piece %d\n", i,
				(moves.moves+i)->from.x, (moves.moves+i)->from.y,
				(moves.moves+i)->to.x, (moves.moves+i)->to.y,
				COLOR_INT_TO_STR((moves.moves+i)->color),
				(moves.moves+i)->piece);
	}
}
