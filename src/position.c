#include <vgcp.h>

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
