#include <vgcp.h>

void mapPawnPotentialMoves(uint8_t x, uint8_t y) {
        SDL_Point potential;

        potential.y = y PLUS_OR_MINUS(board[y][x].color, 1);

        if(board[potential.y][x].piece == empty) {
                addPotentialMove(potential.y, x, y, x);

                potential.y = y PLUS_OR_MINUS(board[y][x].color, 2);

                if(!(board[y][x].pieceHasMoved) && (board[potential.y][x].piece == empty))
                        addPotentialMove(potential.y, x, y, x);
        }


        potential.y = y PLUS_OR_MINUS(board[y][x].color, 1);
        potential.x = x - 1;

        if(x != 0 && board[potential.y][potential.x].piece != empty && board[potential.y][potential.x].color != board[y][x].color)
                addPotentialMove(potential.y, potential.x, y, x);

        potential.x = x + 1;

        if(x != 7 && board[potential.y][potential.x].piece != empty && board[potential.y][potential.x].color != board[y][x].color)
                addPotentialMove(potential.y, potential.x, y, x);
}

void mapKnightPotentialMoves(uint8_t x, uint8_t y, uint8_t state) {
        SDL_Point potential;


        for(potential.x = 0; potential.x < 2; potential.x++) {
                for(potential.y = 0; potential.y < 4; potential.y++) {
                        if(((y + potentialKnight[potential.y][potential.x]) < 8)
                                        && ((y + potentialKnight[potential.y][potential.x]) >= 0)
                                        && ((x + potentialKnight[potential.y][!potential.x]) < 8)
                                        && ((x + potentialKnight[potential.y][!potential.x]) >= 0)) {

                                if(board[y + potentialKnight[potential.y][potential.x]][x + potentialKnight[potential.y][!potential.x]].color == board[y][x].color
                                                && board[y + potentialKnight[potential.y][potential.x]][x + potentialKnight[potential.y][!potential.x]].piece != empty
                                                && (state & potentialMove));
                                else if(state & potentialMove)
                                        addPotentialMove(y + potentialKnight[potential.y][potential.x], x + potentialKnight[potential.y][!potential.x], y, x);
                                else
                                        board[y + potentialKnight[potential.y][potential.x]][x + potentialKnight[potential.y][!potential.x]].tileState |= state;
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
                                                && (board[potential.y][potential.x].piece != empty)
                                                && (board[potential.y][potential.x].color == board[y][x].color));
                                else if(state & potentialMove)
                                        addPotentialMove(potential.y, potential.x, y, x);
                                else
                                        board[potential.y][potential.x].tileState |= state;

                                if(board[potential.y][potential.x].piece != empty)
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
                                && (board[y][potential.x].piece != empty)
                                && (board[y][potential.x].color == board[y][x].color));
                        else if(state & potentialMove)                                addPotentialMove(y, potential.x, y, x);
                        else
                                board[y][potential.x].tileState |= state;

                        if(board[y][potential.x].piece != empty)
                                break;

                        potential.x += dX;
                }
        }

        for(dY = -1; dY < 2; dY += 2) {
                potential.y = y + dY;

                while(potential.y >= 0 && potential.y < 8) {
                        if((state & potentialMove)
                                && (board[potential.y][x].piece != empty)
                                && (board[potential.y][x].color == board[y][x].color));
                        else if(state & potentialMove)
                                addPotentialMove(potential.y, x, y, x);
                        else
                                board[potential.y][x].tileState |= state;

                        if(board[potential.y][x].piece != empty)
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
                        else if(((board[potential.y][potential.x].piece == empty) || (board[potential.y][potential.x].color != board[y][x].color))
                                        && potential.y >= 0
                                        && potential.x >= 0
                                        && potential.y < 8
                                        && potential.x < 8) {
                                if((state & potentialMove) && board[potential.y][potential.x].piece != empty) {
                                        if(board[potential.y][potential.x].color != board[y][x].color)
                                                addPotentialMove(potential.y, potential.x, y, x);
                                        else;
                                }
                                else if(state & potentialMove)
                                        addPotentialMove(potential.y, potential.x, y, x);
                                else
                                        board[potential.y][potential.x].tileState |= state;
                        }
                }
        }

        /* castling */
        if(((board[y][x].color == colorWhite && x == 4 && y == 0
                                        && !(board[y][x].tileState & underThreatByBlack))
                        || (board[y][x].color == colorBlack && x == 4 && y == 7)
                                        && !(board[y][x].tileState & underThreatByWhite))
                        && !board[y][x].pieceHasMoved
                        && fromMove) {
                potential.y = board[y][x].color ? 7 : 0;

                for(potential.x = 5; potential.x < 7; potential.x++)  {
                        if(board[potential.y][potential.x].piece == empty);
                        else {
                                goto longCastle;
                        }
                }

                if(board[y][x].color == colorWhite
                                && !board[0][7].pieceHasMoved && board[0][7].piece == rook) {
                        addPotentialMove(0, 6, y, x);
                }
                if(board[y][x].color == colorBlack
                                && !board[7][7].pieceHasMoved && board[7][7].piece == rook) {
                        addPotentialMove(7, 6, y, x);
                }

                /* long castle */
longCastle:

                potential.y = board[y][x].color ? 7 : 0;

                for(potential.x = 3; potential.x > 1; potential.x--)  {
                        if(board[potential.y][potential.x].piece == empty);
                        else {
                                goto endKingMoveMapping;                        }
                }

                if(board[y][x].color == colorWhite
                                && !board[0][0].pieceHasMoved && board[0][0].piece == rook) {
                        addPotentialMove(0, 2, y, x);                }
                if(board[y][x].color == colorBlack
                                && !board[7][0].pieceHasMoved && board[7][0].piece == rook) {
                        addPotentialMove(7, 2, y, x);
                }
        }

endKingMoveMapping:
        return;
}
