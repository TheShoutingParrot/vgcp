#include <vgcp.h>

void updateWindow(void) {
        drawBoard();

        SDL_RenderPresent(gameRenderer);
}

void drawBoard(void) {
        SDL_Rect tileRect;
        bool tileColor;
        int8_t y;

        tileColor = colorBlack;

        tileRect.w = 100;
        tileRect.h = 100;

        SDL_SetRenderDrawColor(gameRenderer, 0x00, 0x00, 0x00, 0xff);
        SDL_RenderClear(gameRenderer);

        for(y = 7; y > -1; y--) {
                drawBoardRow(&tileColor, &tileRect, y);
                tileColor = !tileColor;
        }
}

void drawBoardRow(bool *color, SDL_Rect *tileRect, uint8_t y) {
        uint8_t x;
        SDL_Rect potentialMoveRect;

        potentialMoveRect.w = 20;
        potentialMoveRect.h = 20;

        for(x = 0; x < 8; x++) {
                if((BOARD_ROW(y, !position.playerToMove)) == selectedPiece.y &&
                                x == selectedPiece.x)
                        SDL_SetRenderDrawColor(gameRenderer, 0xf9, 0xf9, 0x00, 0xff);
                else if(*color)
                        SDL_SetRenderDrawColor(gameRenderer, 0x66, 0x00, 0x00, 0xff);
                else
                        SDL_SetRenderDrawColor(gameRenderer, 0xfb, 0xf5, 0xde, 0xff);

                *color = !(*color);

                tileRect->x = x * tileRect->w;
                tileRect->y = y * tileRect->h;

                SDL_RenderFillRect(gameRenderer, tileRect);

#ifdef _DEBUG
		if(y == BOARD_ROW(position.kingLocation[0].y, !position.playerMove) && x == position.kingLocation[0].x) {
			SDL_SetRenderDrawColor(gameRenderer, 0x00, 0xff, 0x00, 0x00);
                	SDL_RenderFillRect(gameRenderer, tileRect);
		}
		else if(y == BOARD_ROW(position.kingLocation[1].y, !position.playerToMove) && x == position.kingLocation[1].x) {
			SDL_SetRenderDrawColor(gameRenderer, 0xff, 0x00, 0x00, 0x00);
                	SDL_RenderFillRect(gameRenderer, tileRect);
		}
#endif

                if(position.board[BOARD_ROW(y, !position.playerToMove)][x].piece != empty) {
                        SDL_RenderCopy(gameRenderer, 
					pieceTexture[(position.board[BOARD_ROW(y, !position.playerToMove)][x].piece)][(position.board[BOARD_ROW(y, !position.playerToMove)][x].color)],
					NULL, tileRect);
                }

                if((position.board[BOARD_ROW(y, !position.playerToMove)][x].tileState) & potentialMove
				|| (position.board[BOARD_ROW(y, !position.playerToMove)][x].tileState) 
					& potentialCastling
				|| (position.board[BOARD_ROW(y, !position.playerToMove)][x].tileState) 
					& potentialEnPassant) {
                        potentialMoveRect.x = tileRect->x + 40;
                        potentialMoveRect.y = tileRect->y + 40;

                        SDL_SetRenderDrawColor(gameRenderer, 0x0a, 0x0a, 0x0a, 0xaa);
                        SDL_RenderFillRect(gameRenderer, &potentialMoveRect);
                }

#ifdef  _DEBUG
                if((position.board[BOARD_ROW(y, !position.playerToMove)][x].tileState) & underThreatByWhite) {
                        potentialMoveRect.x = tileRect->x + 10;
                        potentialMoveRect.y = tileRect->y + 10;

                        SDL_SetRenderDrawColor(gameRenderer, 0x00, 0xff, 0x00, 0xaa);
                        SDL_RenderFillRect(gameRenderer, &potentialMoveRect);
                }

		if((position.board[BOARD_ROW(y, !position.playerToMove)][x].tileState) & underThreatByBlack) {
			potentialMoveRect.x = tileRect->x + 70;
			potentialMoveRect.y = tileRect->y + 10;

			SDL_SetRenderDrawColor(gameRenderer, 0xff, 0x00, 0x00, 0xaa);
			SDL_RenderFillRect(gameRenderer, &potentialMoveRect);
		}
#endif

	}
}

void drawGameOverText(color_t winner) {
        SDL_Rect dstRect;
	uint8_t text;

	text = winner + 1;

        /* render text background */
	if(winner == noColor) {
        	dstRect.w = 240;
	        dstRect.h = 110;
        	dstRect.x = (GAME_LOGICAL_WIDTH / 2) - (dstRect.w / 2);
	}

        dstRect.y = 95;

	/* render "DRAW" text (if the match resulted in a draw) */
	if(winner == noColor) {
        	SDL_SetRenderDrawColor(gameRenderer, 0x00, 0x00, 0x00, 0xff);
	        SDL_RenderFillRect(gameRenderer, &(textTexture[3].background));

	        SDL_RenderCopy(gameRenderer, textTexture[text].texture, NULL, &(textTexture[3].rect));
	}

        /* render "CHECKMATE" text */
	else { 
	        SDL_SetRenderDrawColor(gameRenderer, 0x00, 0x00, 0x00, 0xff);
        	SDL_RenderFillRect(gameRenderer, &(textTexture[0].background));

	        SDL_RenderCopy(gameRenderer, textTexture[0].texture, NULL, &(textTexture[0].rect));
	}

	/* render x player wins only if the game didn't result
	 * in a draw */
	if(winner != noColor) {
	        SDL_SetRenderDrawColor(gameRenderer, 0x00, 0x00, 0x00, 0xff);
        	SDL_RenderFillRect(gameRenderer, &(textTexture[text].background));
        	
		/* render "(player color) wins" */
	        SDL_RenderCopy(gameRenderer, textTexture[text].texture, NULL, &(textTexture[text].rect));
	}

	/* render "new game" button (text and background) */
        SDL_SetRenderDrawColor(gameRenderer,
                        newGameButton.backgroundColor[newGameButton.state].r,
                        newGameButton.backgroundColor[newGameButton.state].g,
                        newGameButton.backgroundColor[newGameButton.state].b,
                        0xff);
	SDL_RenderFillRect(gameRenderer, &(newGameButton.backgroundRect));
        SDL_RenderCopy(gameRenderer, newGameButton.texture[newGameButton.state],
                        NULL, &(newGameButton.rect));
}

SDL_Texture *loadTexture(char *path) {
        SDL_Surface *imageSurface;
        SDL_Texture *imageTexture;

        imageSurface = IMG_Load(path);
        if(imageSurface == NULL) {
                die("Failed to import image: %s", SDL_GetError());
        }

        imageTexture = SDL_CreateTextureFromSurface(gameRenderer, imageSurface);
        SDL_FreeSurface(imageSurface);

        if(imageTexture == NULL) {
                die("Failed to convert surface to texture: %s", SDL_GetError());
        }

        return imageTexture;
}

SDL_Texture *renderText(char *textStr, SDL_Color textColor) {
        SDL_Surface *textSurface;
        SDL_Texture *textTexture;

        textSurface = TTF_RenderText_Solid(gameFont, textStr, textColor);

        if(textSurface == NULL) {
                die("Failed to render text: %s", TTF_GetError());
        }

        textTexture = SDL_CreateTextureFromSurface(gameRenderer, textSurface);
        SDL_FreeSurface(textSurface);

        if(textTexture == NULL) {
                die("Failed to convert surface to texture: %s", SDL_GetError());
        }

        return textTexture;
}
