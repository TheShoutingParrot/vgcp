#include <vgcp.h>

void mapPawnPotentialMoves(uint8_t x, uint8_t y) {
        SDL_Point potential;

        potential.y = y PLUS_OR_MINUS(position.board[y][x].color, 1);

        if(position.board[potential.y][x].piece == empty) {
                addPotentialMove(potential.y, x, y, x);

                potential.y = y PLUS_OR_MINUS(position.board[y][x].color, 2);

                if(!(position.board[y][x].pieceHasMoved) && (position.board[potential.y][x].piece == empty))
                        addPotentialMove(potential.y, x, y, x);
        }

        potential.y = y PLUS_OR_MINUS(position.board[y][x].color, 1);
        potential.x = x - 1;

        if(x != 0 && position.board[potential.y][potential.x].piece != empty && position.board[potential.y][potential.x].color != position.board[y][x].color)
                addPotentialMove(potential.y, potential.x, y, x);

        potential.x = x + 1;

        if(x != 7 && position.board[potential.y][potential.x].piece != empty && position.board[potential.y][potential.x].color != position.board[y][x].color)
                addPotentialMove(potential.y, potential.x, y, x);


	if(x != 7 && y == BOARD_ROW_PLAYER(4, position.board[y][x].color) && position.board[potential.y][potential.x].piece == empty &&
			position.prevMove.from.y == BOARD_ROW_PLAYER(6, position.board[y][x].color) &&
			position.prevMove.to.y == BOARD_ROW_PLAYER(4, position.board[y][x].color) &&
			position.prevMove.from.x == potential.x && position.prevMove.piece == pawn)
		addPotentialEnPassant(position.prevMove.to.y, position.prevMove.to.x,
				potential.y, potential.x, y, x);

        potential.x = x - 1;

        if(x != 0 && y == BOARD_ROW_PLAYER(4, position.board[y][x].color) && position.board[potential.y][potential.x].piece == empty &&
			position.prevMove.from.y == BOARD_ROW_PLAYER(6, position.board[y][x].color) &&
			position.prevMove.to.y == BOARD_ROW_PLAYER(4, position.board[y][x].color) &&
			position.prevMove.from.x == potential.x && position.prevMove.piece == pawn)
		addPotentialEnPassant(position.prevMove.to.y, position.prevMove.to.x,
				potential.y, potential.x, y, x);

}

void mapKnightPotentialMoves(uint8_t x, uint8_t y, uint8_t state) {
        SDL_Point potential;


        for(potential.x = 0; potential.x < 2; potential.x++) {
                for(potential.y = 0; potential.y < 4; potential.y++) {
                        if(((y + potentialKnight[potential.y][potential.x]) < 8)
                                        && ((y + potentialKnight[potential.y][potential.x]) >= 0)
                                        && ((x + potentialKnight[potential.y][!potential.x]) < 8)
                                        && ((x + potentialKnight[potential.y][!potential.x]) >= 0)) {

                                if(position.board[y + potentialKnight[potential.y][potential.x]][x + potentialKnight[potential.y][!potential.x]].color == position.board[y][x].color
                                                && position.board[y + potentialKnight[potential.y][potential.x]][x + potentialKnight[potential.y][!potential.x]].piece != empty
                                                && (state & potentialMove));
                                else if(state & potentialMove)
                                        addPotentialMove(y + potentialKnight[potential.y][potential.x], x + potentialKnight[potential.y][!potential.x], y, x);
                                else
                                        position.board[y + potentialKnight[potential.y][potential.x]][x + potentialKnight[potential.y][!potential.x]].tileState |= state;
                        }
                }
        }
}

void mapBishopPotentialMoves(uint8_t x, uint8_t y, uint8_t state) {
        SDL_Point potential;
        int8_t dX, dY;

        for(dX = -1; dX < 2; dX += 2) {
                for(dY = -1; dY < 2; dY += 2) {
                        potential.y = y + dY;
                        potential.x = x + dX;

                        while(potential.x >= 0 && potential.y >= 0 && potential.x < 8 && potential.y < 8) {
                                if((state & potentialMove)
                                                && (position.board[potential.y][potential.x].piece != empty)
                                                && (position.board[potential.y][potential.x].color == position.board[y][x].color));
                                else if(state & potentialMove)
                                        addPotentialMove(potential.y, potential.x, y, x);
                                else
                                        position.board[potential.y][potential.x].tileState |= state;

                                if(position.board[potential.y][potential.x].piece != empty)
                                        break;

                                potential.y += dY;
                                potential.x += dX;
                        }
                }
        }
}

void mapRookPotentialMoves(uint8_t x, uint8_t y, uint8_t state) {
        SDL_Point potential;
        int8_t dX, dY;

        for(dX = -1; dX < 2; dX += 2) {
                potential.x = x + dX;

                while(potential.x >= 0 && potential.x < 8) {
			if((state & potentialMove)
                                && (position.board[y][potential.x].piece != empty)
                                && (position.board[y][potential.x].color == position.board[y][x].color));
                        else if(state & potentialMove)
				addPotentialMove(y, potential.x, y, x);
                        else
                                position.board[y][potential.x].tileState |= state;

                        if(position.board[y][potential.x].piece != empty)
                                break;

                        potential.x += dX;
                }
        }

        for(dY = -1; dY < 2; dY += 2) {
                potential.y = y + dY;

                while(potential.y >= 0 && potential.y < 8) {
                        if((state & potentialMove)
                                && (position.board[potential.y][x].piece != empty)
                                && (position.board[potential.y][x].color == position.board[y][x].color));
                        else if(state & potentialMove)
                                addPotentialMove(potential.y, x, y, x);
                        else
                                position.board[potential.y][x].tileState |= state;

                        if(position.board[potential.y][x].piece != empty)
                                break;

                        potential.y += dY;
                }
        }
}

void mapKingPotentialMoves(uint8_t x, uint8_t y, uint8_t state, bool fromMove) {
        SDL_Point potential;

        for(potential.y = y-1; potential.y < y+2; potential.y++) {
                for(potential.x = x-1; potential.x < x+2; potential.x++) {
                        if(potential.y == y && potential.x == x);
                        else if(((position.board[potential.y][potential.x].piece == empty) || (position.board[potential.y][potential.x].color != position.board[y][x].color))
                                        && potential.y >= 0
                                        && potential.x >= 0
                                        && potential.y < 8
                                        && potential.x < 8) {
                                if((state & potentialMove) && position.board[potential.y][potential.x].piece != empty) {
                                        if(position.board[potential.y][potential.x].color != position.board[y][x].color)
                                                addPotentialMove(potential.y, potential.x, y, x);
                                        else;
                                }
                                else if(state & potentialMove)
                                        addPotentialMove(potential.y, potential.x, y, x);
                                else
                                        position.board[potential.y][potential.x].tileState |= state;
                        }
                }
        }

        /* castling */
	if(((position.playerToMove == colorWhite 
				&& position.castlingRights[colorWhite][1]
				&& !(position.board[y][x].tileState & underThreatByBlack))
			|| (position.playerToMove == colorBlack
				&& position.castlingRights[colorBlack][1]
				&& !(position.board[y][x].tileState & underThreatByWhite)))
			&& fromMove) {
                potential.y = position.board[y][x].color ? 7 : 0;

                for(potential.x = 5; potential.x < 7; potential.x++)  {
                        if(position.board[potential.y][potential.x].piece == empty);
                        else
				goto longCastle;
                }

		addPotentialCastling(position.board[y][x].color, false);
	}
        /* long castling */
longCastle:
	if(((position.playerToMove == colorWhite
				&& position.castlingRights[colorWhite][0]
				&& !(position.board[y][x].tileState & underThreatByBlack))
			|| (position.playerToMove == colorBlack
				&& position.castlingRights[colorBlack][0]
				&& !(position.board[y][x].tileState & underThreatByWhite)))
			&& fromMove) {

                potential.y = position.board[y][x].color ? 7 : 0;

                for(potential.x = 3; potential.x > 1; potential.x--)  {
                        if(position.board[potential.y][potential.x].piece == empty);
                        else
                                goto endKingMoveMapping;
		}

		addPotentialCastling(position.board[y][x].color, true);
        }
	
endKingMoveMapping:
        return;
}

/* this function checks if there's enough pieces for either side to get a checkmate */
bool checkIfEnoughPieces(void) {
	/* what is considered insufficent material:
	 * 	- lone king (total pieces = 0)
	 * 	- king and bishop 
	 * 	- king and knight */
#ifdef DEBUG
	printf("%d %d %d %d %d\n", (position.piecesArray[king] == 0),
			(position.piecesArray[king] == 2 && position.piecesArray[bishop] == 2),
			(position.piecesArray[king] == 1 && position.piecesArray[bishop] == 1),
			(position.piecesArray[king] == 2 && position.piecesArray[knight] == 2),
			(position.piecesArray[king] == 1 && position.piecesArray[knight] == 1),
			(position.piecesArray[king] == 2 && position.piecesArray[bishop] == 1
				&& position.piecesArray[knight] == 1));
#endif /* #ifdef DEBUG */

	if((position.piecesArray[king] == 0)
			|| (position.piecesArray[king] == 2 && position.piecesArray[bishop] == 2)
			|| (position.piecesArray[king] == 1 && position.piecesArray[bishop] == 1)
			|| (position.piecesArray[king] == 2 && position.piecesArray[knight] == 2)
			|| (position.piecesArray[king] == 1 && position.piecesArray[knight] == 1)
			|| (position.piecesArray[king] == 2 && position.piecesArray[bishop] == 1
				&& position.piecesArray[knight] == 1))
		return false;

	return true;
}
