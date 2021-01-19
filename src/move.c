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

        else
                checkIfMated(board[y][x].color, false);

        if(!kingMated[board[y][x].color])
                board[y][x].tileState |= potentialMove;

        board[pieceY][pieceX].piece = board[y][x].piece;
        board[pieceY][pieceX].color = board[y][x].color;

        board[y][x].piece = originalTile.piece;
        board[y][x].color= originalTile.color;

        updateBoard();
}

uint8_t countAllPotentialMoves(bool color) {
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


void movePiece(uint8_t y, uint8_t x) {
        board[y][x].piece = board[selectedPiece.y][selectedPiece.x].piece;
        board[y][x].color = board[selectedPiece.y][selectedPiece.x].color;
        board[y][x].pieceHasMoved = true;
        board[selectedPiece.y][selectedPiece.x].piece = empty;


        /* when a pawn gets to the last row of the defense the pawn
         * becomes a queen, rook, bishop or knight */
        if(board[y][x].piece == pawn
#ifndef _NO_PERSPECTIVE_CHANGE
                        && y == BOARD_ROW(7, board[y][x].color)
#else
                        && y == (board[y][x].color ? 0 : 7)
#endif
        ) {
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "a pawn has been promoted");
                board[y][x].piece = queen; /* TODO: menu to choose which piece */
        }
        else if(board[y][x].piece == king) {
                kingLocation[gameTurn].x = x;
                kingLocation[gameTurn].y = y;

                /* this means that the king is castling so we must 
                 * move the rook also */
                if((board[selectedPiece.y][selectedPiece.x].color == colorWhite
                                && selectedPiece.x == 4 && selectedPiece.y == 0
                                && x == 6 && y == 0)) {
                        SDL_Point oldSelect;
                        oldSelect = selectedPiece;

                        selectedPiece.x = 7;
                        selectedPiece.y = 0;

                        movePiece(0, 5);
                        /* because we call movePiece we need to correct the
                         * game turn */
                        gameTurn = colorWhite;

                        selectedPiece = oldSelect;
                }
                else if((board[selectedPiece.y][selectedPiece.x].color == colorBlack
                                && selectedPiece.x == 4 && selectedPiece.y == 7
                                && x == 6 && y == 7)) {
                        SDL_Point oldSelect;
                        oldSelect = selectedPiece;

                        selectedPiece.x = 7;
                        selectedPiece.y = 7;

                        movePiece(7, 5);
                        /* because we call movePiece we need to correct the
                         * game turn */
                        gameTurn = colorBlack;

                        selectedPiece = oldSelect;
                }

                if((board[selectedPiece.y][selectedPiece.x].color == colorWhite
                                && selectedPiece.x == 4 && selectedPiece.y == 0
                                && x == 2 && y == 0)) {
                        SDL_Point oldSelect;
                        oldSelect = selectedPiece;

                        selectedPiece.x = 0;
                        selectedPiece.y = 0;

                        movePiece(0, 3);
                        /* because we call movePiece we need to correct the
                         * game turn */
                        gameTurn = colorWhite;

                        selectedPiece = oldSelect;
                }
                else if((board[selectedPiece.y][selectedPiece.x].color == colorBlack
                                && selectedPiece.x == 4 && selectedPiece.y == 7
                                && x == 2 && y == 7)) {
                        SDL_Point oldSelect;
                        oldSelect = selectedPiece;

                        selectedPiece.x = 0;
                        selectedPiece.y = 7;

                        movePiece(7, 3);
                        /* because we call movePiece we need to correct the
                         * game turn */
                        gameTurn = colorBlack;

                        selectedPiece = oldSelect;
                }
        }

        deselectPiece();

        updateWindow();

#ifndef _NO_PERSPECTIVE_CHANGE
        SDL_Delay(250);
#else
        SDL_Delay(25);
#endif

        checkIfMated(!gameTurn, true);

        gameTurn = !gameTurn; /* switch the turns after a piece has moved */
}

void checkIfMated(bool color, bool fromMove) {
        updateBoard();

        if(board[kingLocation[color].y][kingLocation[color].x].tileState & (color ? underThreatByWhite: underThreatByBlack)) {
                kingMated[color] = true;
                if(color && fromMove)
                        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "black king mated");
                else if(fromMove)
                        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "white king mated");

                if(!checkingIfCheckMated && fromMove)
                        if(!countAllPotentialMoves(color)) {
                                gameOver(!color);
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
