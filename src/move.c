#include <vgcp.h>

/* marks the tile x, y as a potential move except if it results in
 * a mating of the king */
void addPotentialMove(uint8_t y, uint8_t x, uint8_t pieceY, uint8_t pieceX) {
        struct tile originalTile;
        uint8_t movedPiece;

        originalTile = board[y][x];

        board[y][x].piece = board[pieceY][pieceX].piece;
        board[y][x].color = board[pieceY][pieceX].color;

        movedPiece = board[pieceY][pieceX].piece;
        board[pieceY][pieceX].piece = empty;

        updateBoard();

        if(movedPiece == king) {
                kingLocation[board[y][x].color].x = x;
                kingLocation[board[y][x].color].y = y;

                checkIfMated(board[y][x].color, false);

                kingLocation[board[y][x].color].x = pieceX;
                kingLocation[board[y][x].color].y = pieceY;
        }

        else {
                checkIfMated(board[y][x].color, false);
	}

        if(!kingMated[board[y][x].color])
                board[y][x].tileState |= potentialMove;

        board[pieceY][pieceX].piece = board[y][x].piece;
        board[pieceY][pieceX].color = board[y][x].color;

        board[y][x].piece = originalTile.piece;
        board[y][x].color= originalTile.color;

        updateBoard();
}

uint8_t countAllPotentialMoves(color_t color) {
        uint8_t x, y,
                result;

        checkingIfCheckMated = true;

        for(y = 0; y < 8; y++) {
                for(x = 0; x < 8; x++) {
                        if(board[y][x].color == color) {
                                selectPiece(y, x);
                        }
                }
        }

        result = 0;

        for(y = 0; y < 8; y++) {
                for(x = 0; x < 8; x++) {
                        if(board[y][x].tileState & potentialMove)
                                result++;;
                }
        }

        deselectPiece();

        checkingIfCheckMated = false;

        return result;
}

void movePiece(struct move move) {
        board[move.to.y][move.to.x].piece = move.piece;
        board[move.to.y][move.to.x].color = move.color;
        board[move.to.y][move.to.x].pieceHasMoved = true;
        board[move.from.y][move.from.x].piece = empty;

        /* when a pawn gets to the last row of the defense the pawn
         * becomes a queen, rook, bishop or knight */
        if(move.piece == pawn
#ifndef _NO_PERSPECTIVE_CHANGE
                        && move.to.y == BOARD_ROW(7, board[move.to.y][move.to.x].color)
#else
                        && move.to.y == (board[move.to.y][move.to.x].color ? 0 : 7)
#endif
        ) {
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "a pawn has been promoted");
                board[move.to.y][move.to.x].piece = queen; /* TODO: menu to choose which piece */
        }
        else if(move.piece == king) {
		struct move castlingMove;

                kingLocation[gameTurn].x = move.to.x;
                kingLocation[gameTurn].y = move.to.y;

		castlingMove.piece = rook;
		castlingMove.color = move.color;

                /* this means that the king is castling so we must 
                 * move the rook also */
                if(move.from.x == 4 && move.from.y == BOARD_ROW_PLAYER(0, move.color)
                                && move.to.x == 6 && move.to.y == BOARD_ROW_PLAYER(0, move.color)) {
			castlingMove.to.x = 5;
			castlingMove.to.y = BOARD_ROW_PLAYER(0, move.color);
                        castlingMove.from.x = 7;
                        castlingMove.from.y = BOARD_ROW_PLAYER(0, move.color);
			movePiece(castlingMove);
                }
		else if((move.from.x == 4 && move.from.y == BOARD_ROW_PLAYER(0, move.color)
                                && move.to.x == 2 && move.to.y == BOARD_ROW_PLAYER(0, move.color))) {
                        castlingMove.to.x = 3;
			castlingMove.to.y = BOARD_ROW_PLAYER(0, move.color);
                        castlingMove.from.x = 0;
                        castlingMove.from.y = BOARD_ROW_PLAYER(0, move.color);
			movePiece(castlingMove);
                }
        }

        updateWindow();

#ifndef _NO_PERSPECTIVE_CHANGE
        SDL_Delay(250);
#else
        SDL_Delay(25);
#endif

	/* check if the king has been mated */
        checkIfMated(OPPOSITE_COLOR(gameTurn), true);

	/* if the king is not mated but there are no potential moves for the
	 * opposing player then it's a draw */
	if((!kingMated[OPPOSITE_COLOR(gameTurn)]) 
			&& (countAllPotentialMoves(OPPOSITE_COLOR(gameTurn)) == 0)) {
		/* reset the status of the kings */
		gameOver(noColor);
	}
}

void checkIfMated(color_t color, bool fromMove) {
        updateBoard();

        if(board[kingLocation[color].y][kingLocation[color].x].tileState & (color ? underThreatByWhite: underThreatByBlack)) {
                kingMated[color] = true;

                if(color && fromMove)
                        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "black king mated");
                else if(fromMove)
                        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "white king mated");
                if(!checkingIfCheckMated && fromMove) {
                        if(!countAllPotentialMoves(color)) {
                                gameOver(OPPOSITE_COLOR(color));
                        }
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
                        if(board[y][x].tileState & underThreatByWhite)
                                board[y][x].tileState ^= underThreatByWhite;
                        if(board[y][x].tileState & underThreatByBlack)
                                board[y][x].tileState ^= underThreatByBlack;
                }
        }

        /* update tiles/pieces that are under threat */
        for(y = 0; y < 8; y++) {
                for(x = 0; x < 8; x++) {
                        switch(board[y][x].piece) {
                                case pawn:
                                        if((y != 8 && y != 0)) {
                                                if(x != 7)
                                                        board[y PLUS_OR_MINUS(board[y][x].color, 1)][x+1].tileState
                                                                |= (board[y][x].color ? underThreatByBlack : underThreatByWhite);
                                                if(x != 0)
                                                        board[y PLUS_OR_MINUS(board[y][x].color, 1)][x-1].tileState
                                                                |= (board[y][x].color ? underThreatByBlack : underThreatByWhite);
                                        }

                                        break;

                                case knight:
                                        mapKnightPotentialMoves(x, y, board[y][x].color ? underThreatByBlack : underThreatByWhite);

                                        break;

                                case bishop:
                                        mapBishopPotentialMoves(x, y, board[y][x].color ? underThreatByBlack : underThreatByWhite);

                                        break;

                                case rook:
                                        mapRookPotentialMoves(x, y, board[y][x].color ? underThreatByBlack : underThreatByWhite);

                                        break;

                                case queen:
                                        mapRookPotentialMoves(x, y, board[y][x].color ? underThreatByBlack : underThreatByWhite);
                                        mapBishopPotentialMoves(x, y, board[y][x].color ? underThreatByBlack : underThreatByWhite);

                                        break;

                                case king:
                                        mapKingPotentialMoves(x, y, board[y][x].color ? underThreatByBlack : underThreatByWhite, false);
                        }

                }

        }
}
