#include <vgcp.h>

void initPosition(void) {
        uint8_t i, j;

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


	addToPositionList();
}

void updatePosition(struct move move) {
        position.prevMove = move;
        position.playerToMove = OPPOSITE_COLOR(move.color);
}
