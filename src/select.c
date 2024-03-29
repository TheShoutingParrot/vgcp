#include <vgcp.h>

void selectPiece(uint8_t y, uint8_t x) {
        /* if potential coordinate (x or y) is used many times then temporarily stored here */
        SDL_Point potential, test;

        selectedPiece.x = x;
        selectedPiece.y = y;

        /* check for potential moves */
        switch(position.board[y][x].piece) {
                case pawn:
                        mapPawnPotentialMoves(x, y);
                        break;

                case knight:
                        mapKnightPotentialMoves(x, y, potentialMove);

                        break;

                case bishop:
                        mapBishopPotentialMoves(x, y, potentialMove);

                        break;

                case rook:
                        mapRookPotentialMoves(x, y, potentialMove);

                        break;

                case queen:
			/* the queen is basically a bishop + a rook so we don't need a seperate function
			 * for mapping that */
                        mapBishopPotentialMoves(x, y, potentialMove);
                        mapRookPotentialMoves(x, y, potentialMove);

                        break;

                case king:
                        mapKingPotentialMoves(x, y, potentialMove, true);

                        break;
        }
}

void deselectPiece(void) {
        uint8_t x, y;
        selectedPiece.x = -1;

        for(y = 0; y < 8; y++) {
                for(x = 0; x < 8; x++) {
                        if(position.board[y][x].tileState & potentialMove)
                                position.board[y][x].tileState ^= potentialMove;
                        if(position.board[y][x].tileState & potentialCastling)
                                position.board[y][x].tileState ^= potentialCastling;
                        if(position.board[y][x].tileState & potentialEnPassant)
                                position.board[y][x].tileState ^= potentialEnPassant;
                }
        }

}
