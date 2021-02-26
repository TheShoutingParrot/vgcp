#include <vgcp.h>

/* marks the tile x, y as a potential move except if it results in
 * a mating of the king */
void addPotentialMove(uint8_t y, uint8_t x, uint8_t pieceY, uint8_t pieceX) {
        struct tile originalTile;
        uint8_t movedPiece;

        originalTile = position.board[y][x];

        position.board[y][x].piece = position.board[pieceY][pieceX].piece;
        position.board[y][x].color = position.board[pieceY][pieceX].color;

        movedPiece = position.board[pieceY][pieceX].piece;
        position.board[pieceY][pieceX].piece = empty;

        updateBoard();

        if(movedPiece == king) {
                position.kingLocation[position.board[y][x].color].x = x;
                position.kingLocation[position.board[y][x].color].y = y;

                checkIfMated(position.board[y][x].color, false);

                position.kingLocation[position.board[y][x].color].x = pieceX;
                position.kingLocation[position.board[y][x].color].y = pieceY;
        }

        else {
                checkIfMated(position.board[y][x].color, false);
	}

        if(!kingMated[position.board[y][x].color])
                position.board[y][x].tileState |= potentialMove;

        position.board[pieceY][pieceX].piece = position.board[y][x].piece;
        position.board[pieceY][pieceX].color = position.board[y][x].color;

        position.board[y][x].piece = originalTile.piece;
        position.board[y][x].color = originalTile.color;

        updateBoard();
}

/* marks a potential castling move except if it results in
 * a mating of the king */
void addPotentialCastling(color_t color, bool longCastle) {
        struct tile originalTile;
	SDL_Point castle,
		  king;

	castle.y = color ? 7 : 0;
	castle.x = longCastle ? 2 : 6;

	king.y = castle.y;
	king.x = 4;

        originalTile = position.board[castle.y][castle.x];

	position.board[castle.y][castle.x].piece = position.board[king.y][king.x].piece;
        position.board[castle.y][castle.x].color = position.board[king.y][king.x].color;

	position.board[king.y][king.x].piece = empty;

        updateBoard();

        position.kingLocation[color].x = castle.x;
        position.kingLocation[color].y = castle.y;

        checkIfMated(position.board[castle.y][castle.x].color, false);

        position.kingLocation[color].x = king.x;
        position.kingLocation[color].y = king.y;

        if(!kingMated[position.board[castle.y][castle.x].color])
                position.board[castle.y][castle.x].tileState |= potentialCastling;

        position.board[king.y][king.x].piece = position.board[castle.y][castle.x].piece;
        position.board[king.y][king.x].color = position.board[castle.y][castle.x].color;

        position.board[castle.y][castle.x].piece = originalTile.piece;
        position.board[castle.y][castle.x].color = originalTile.color;

        updateBoard();
}

/* marks a potential "en passant" move */
void addPotentialEnPassant(uint8_t capturedY, uint8_t capturedX, uint8_t y, uint8_t x, uint8_t pieceY, uint8_t pieceX) {
	struct tile originalTile;
	uint8_t movedPiece;

	originalTile = position.board[y][x];

	position.board[y][x].piece = position.board[pieceY][pieceX].piece;
        position.board[y][x].color = position.board[pieceY][pieceX].color;

	/* the captured pawn's tile becomes empty */
	position.board[capturedY][capturedX].piece = empty;

        movedPiece = position.board[pieceY][pieceX].piece;
        position.board[pieceY][pieceX].piece = empty;

        updateBoard();

        checkIfMated(position.board[y][x].color, false);

        if(!kingMated[position.board[y][x].color])
                position.board[y][x].tileState |= potentialEnPassant;

        position.board[pieceY][pieceX].piece = position.board[y][x].piece;
        position.board[pieceY][pieceX].color = position.board[y][x].color;

        position.board[y][x].piece = originalTile.piece;
        position.board[y][x].color = originalTile.color;

	position.board[capturedY][capturedX].piece = pawn;

        updateBoard();
}

uint8_t countAllPotentialMoves(color_t color) {
        uint8_t x, y,
                result;

        checkingIfCheckMated = true;

        for(y = 0; y < 8; y++) {
                for(x = 0; x < 8; x++) {
                        if(position.board[y][x].color == color) {
                                selectPiece(y, x);
                        }
                }
        }

        result = 0;

        for(y = 0; y < 8; y++) {
                for(x = 0; x < 8; x++) {
                        if(position.board[y][x].tileState & potentialMove)
                                result++;;
                }
        }

        deselectPiece();

        checkingIfCheckMated = false;

        return result;
}

void movePiece(struct move move) {
        position.board[move.to.y][move.to.x].piece = move.piece;
        position.board[move.to.y][move.to.x].color = move.color;
        position.board[move.to.y][move.to.x].pieceHasMoved = true;
        position.board[move.from.y][move.from.x].piece = empty;

	updatePosition(move);

	switch(move.piece) {
        	/* when a pawn gets to the last row of the defense the pawn
	         * becomes a queen, rook, bishop or knight */
		case pawn:
	        	if(move.to.y == (position.board[move.to.y][move.to.x].color ? 0 : 7)) {
        		        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "a pawn has been promoted");
                		position.board[move.to.y][move.to.x].piece = queen; /* TODO: menu to choose which piece */

				/* because a pawn has been promoted we need to remove one pawn from the pieces array
				 * and add one queen */
				position.piecesArray[pawn]--;
				position.piecesArray[queen]++;
	        	}

			else if(move.to.y == (move.color ? 4 : 3)
					&& move.from.y == ((move.color ? 6 : 1))
					&& ((position.board[move.to.y][move.to.x+1].piece == pawn)
						|| (position.board[move.to.y][move.to.x-1].piece == pawn))) {
				position.enPassantRights[move.color][move.to.x] = true;
			}

			break;

		/* if a rook moves from it's original position then it's side
		 * loses so called "castling rights" */
		case rook:
			if(move.from.y == 0 || move.from.y == 7) {
				if(move.from.x == 0)
					position.castlingRights[move.color][0] = false;
				else if(move.from.x == 7)
					position.castlingRights[move.color][1] = false;
			}

			break;

		/* if the king moves then (it's color's) all castling rights
		 * are lost */
		case king:
			position.castlingRights[move.color][0] = false;
			position.castlingRights[move.color][1] = false;

			position.kingLocation[move.color].x = move.to.x;
			position.kingLocation[move.color].y = move.to.y;

			break;
	}

	/* check if the king has been mated */
        checkIfMated(position.playerToMove, true);

	/* if the king is not mated but there are no potential moves for the
	 * opposing player then it's a draw */
	if((!kingMated[position.playerToMove]) 
			&& (countAllPotentialMoves(position.playerToMove) == 0)) {
		/* reset the status of the kings */
		gameOver(noColor);
	}

	/* updates the half move clock (to enforce the 50-move rule) */
	updateHalfmoveClock(move);
	/* checks if the halfmove clock is at 50 (if so the game results in a draw) */
	if(halfmoveClock >= 50) {
		SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "draw as a result of the 50-move rule");
		gameOver(noColor);
	}

	if(checkForRepitition(3)) {
		SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "draw as a result of (threefold) repitition");
		gameOver(noColor);
	}
	
	if(!checkIfEnoughPieces()) {
		SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "draw as a result of insufficent material");
		gameOver(noColor);
	}

	addToPositionList();

#ifdef _DEBUG
	printPositionList();
#endif

	position.enPassantRights[move.color][move.to.x] = false;
}

void castleKing(color_t color, bool longCastle) {
	struct move move;

	move.from.x = 4;
	move.from.y = (color ? 7 : 0);
	move.to.x = (longCastle) ? 2 : 6;
	move.to.y = move.from.y;
	move.piece = king;
	move.color = color;
	move.capturedPiece = empty;

        position.kingLocation[move.color].x = move.to.x;
        position.kingLocation[move.color].y = move.to.y;

	position.castlingRights[move.color][0] = false;
	position.castlingRights[move.color][1] = false;

        position.board[move.to.y][move.to.x].piece = move.piece;
        position.board[move.to.y][move.to.x].color = move.color;
        position.board[move.to.y][move.to.x].pieceHasMoved = true;
        position.board[move.from.y][move.from.x].piece = empty;

	struct move castlingMove;

        /* the king is castling so we must also move the rook */
	castlingMove.piece = rook;
	castlingMove.color = move.color;
	castlingMove.capturedPiece = empty;

        if(!longCastle) {
		castlingMove.to.x = 5;
		castlingMove.to.y = BOARD_ROW_PLAYER(0, move.color);
                castlingMove.from.x = 7;
                castlingMove.from.y = BOARD_ROW_PLAYER(0, move.color);
		movePiece(castlingMove);
        }
	else {
                castlingMove.to.x = 3;
		castlingMove.to.y = BOARD_ROW_PLAYER(0, move.color);
                castlingMove.from.x = 0;
                castlingMove.from.y = BOARD_ROW_PLAYER(0, move.color);
		movePiece(castlingMove);
        }
}

/* special moving function to do "en passant" (which is a special pawn capture) */
void enPassant(uint8_t capturedY, uint8_t capturedX, struct move move) {
        position.board[move.to.y][move.to.x].piece = move.piece;
        position.board[move.to.y][move.to.x].color = move.color;
        position.board[move.to.y][move.to.x].pieceHasMoved = true;
        position.board[move.from.y][move.from.x].piece = empty;

	position.board[capturedY][capturedX].piece = empty;
	position.board[capturedY][capturedX].color = noColor;

	updatePosition(move);

#ifndef _NO_PERSPECTIVE_CHANGE
        SDL_Delay(250);
#else
        SDL_Delay(25);
#endif

	/* check if the king has been mated */
        checkIfMated(position.playerToMove, true);

	/* if the king is not mated but there are no potential moves for the
	 * opposing player then it's a draw */
	if((!kingMated[position.playerToMove]) 
			&& (countAllPotentialMoves(position.playerToMove) == 0)) {
		/* reset the status of the kings */
		gameOver(noColor);
	}

	/* updates the half move clock (to enforce the 50-move rule) */
	updateHalfmoveClock(move);
	if(halfmoveClock >= 50) {
		SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "draw as a result of the 50-move rule");
		gameOver(noColor);
	}

	addToPositionList();

#ifdef _DEBUG
	printPositionList();
#endif
}

void checkIfMated(color_t color, bool fromMove) {
        updateBoard();
	
        if(position.board[position.kingLocation[color].y][position.kingLocation[color].x].tileState & (color ? underThreatByWhite: underThreatByBlack)) {
                kingMated[color] = true;

                if(color && fromMove)
                        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "black king mated");
                else if(fromMove)
                        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "white king mated");
                if(!checkingIfCheckMated && fromMove) {
                        if(!countAllPotentialMoves(color))
                                gameOver(OPPOSITE_COLOR(color));
		}
        }
        else
                kingMated[color] = false;
}

void updateBoard(void) {
        uint8_t x, y;

        /* remove previous threats (for a piece has probably moved) */
        for(y = 0; y < 8; y++) {
                for(x = 0; x < 8; x++) {
                        if(position.board[y][x].tileState & underThreatByWhite)
                                position.board[y][x].tileState ^= underThreatByWhite;
                        if(position.board[y][x].tileState & underThreatByBlack)
                                position.board[y][x].tileState ^= underThreatByBlack;
                }
        }

        /* update tiles/pieces that are under threat */
        for(y = 0; y < 8; y++) {
                for(x = 0; x < 8; x++) {
                        switch(position.board[y][x].piece) {
                                case pawn:
                                        if((y != 8 && y != 0)) {
                                                if(x != 7)
                                                        position.board[y PLUS_OR_MINUS(position.board[y][x].color, 1)][x+1].tileState
                                                                |= (position.board[y][x].color ? underThreatByBlack : underThreatByWhite);
                                                if(x != 0)
                                                        position.board[y PLUS_OR_MINUS(position.board[y][x].color, 1)][x-1].tileState
                                                                |= (position.board[y][x].color ? underThreatByBlack : underThreatByWhite);
                                        }

                                        break;

                                case knight:
                                        mapKnightPotentialMoves(x, y, position.board[y][x].color ? underThreatByBlack : underThreatByWhite);

                                        break;

                                case bishop:
                                        mapBishopPotentialMoves(x, y, position.board[y][x].color ? underThreatByBlack : underThreatByWhite);

                                        break;

                                case rook:
                                        mapRookPotentialMoves(x, y, position.board[y][x].color ? underThreatByBlack : underThreatByWhite);

                                        break;

                                case queen:
                                        mapRookPotentialMoves(x, y, position.board[y][x].color ? underThreatByBlack : underThreatByWhite);
                                        mapBishopPotentialMoves(x, y, position.board[y][x].color ? underThreatByBlack : underThreatByWhite);

                                        break;

                                case king:
                                        mapKingPotentialMoves(x, y, position.board[y][x].color ? underThreatByBlack : underThreatByWhite, false);

					break;
                        }

                }

        }
}

void updateHalfmoveClock(struct move move) {
	if(move.piece == pawn
			|| (move.capturedPiece != empty))
		halfmoveClock = 0;
	else
		halfmoveClock++;
}
