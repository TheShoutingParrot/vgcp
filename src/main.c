#include <vgcp.h>

SDL_Point	selectedPiece	= {-1, -1},
		kingLocation[2] = {{4, 0}, {4, 7}};

bool		kingMated[2] = {false, false},
                checkingIfCheckMated = false;

struct tile unusedBoard[8][8] = {
        BOARD_FIRST_ROW(colorWhite),
        BOARD_SECOND_ROW(colorWhite),
        BOARD_EMPTY_ROW,
        BOARD_EMPTY_ROW,
        BOARD_EMPTY_ROW,
        BOARD_EMPTY_ROW,
        BOARD_SECOND_ROW(colorBlack),
        BOARD_FIRST_ROW(colorBlack),
};

char *pieceName[empty] = {"pawn", "knight", "bishop", "rook", "queen", "king"};

int8_t potentialKnight[4][2] = {{2, 1}, {2, -1}, {-2, 1}, {-2, -1}};

int main(void) {
	color_t gameWinner;

	puts("version: "PROGRAM_VERSION);

	gameTurn = colorWhite;

	if(init())
		die("Initialization failed");

	initBoard();

	SDL_Event event;

mainGameLoop:
	updateWindow();
	updateBoard();

	for(;;) {
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_APP_TERMINATING:
				case SDL_QUIT:
					goto quitGame;
					break;

				case SDL_WINDOWEVENT:
					updateWindow();
					break;

				case SDL_MOUSEBUTTONDOWN:
					handleMousebuttonEvent(event.button);
					updateWindow();
					break;

				case SDL_USEREVENT:
					gameWinner = *((color_t *)event.user.data1);
					goto gameOverLoop;

					break;
			}
		}

		SDL_Delay(100);
	}

gameOverLoop:

	updateWindow();
	SDL_Delay(250);

	drawGameOverText(gameWinner);
	SDL_RenderPresent(gameRenderer);

	SDL_Delay(250);

	newGameButton.state = normal;

	for(;;) {
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_APP_TERMINATING:
				case SDL_QUIT:
					goto quitGame;
					break;

				case SDL_WINDOWEVENT:
					updateWindow();
					drawGameOverText(gameWinner);
					SDL_RenderPresent(gameRenderer);
					break;

				case SDL_MOUSEMOTION:
					if(event.motion.x <= newGameButton.backgroundRect.x +
							newGameButton.backgroundRect.w
							&& event.motion.x >= newGameButton.backgroundRect.x
							&& event.motion.y <= newGameButton.backgroundRect.y +
							newGameButton.backgroundRect.h
							&& event.motion.y >= newGameButton.backgroundRect.y) {
						newGameButton.state = mouseIn;

						drawGameOverText(gameWinner);
						SDL_RenderPresent(gameRenderer);
					}

					else {
						newGameButton.state = normal;

						drawGameOverText(gameWinner);
						SDL_RenderPresent(gameRenderer);
					}

					break;

				case SDL_MOUSEBUTTONDOWN:
					if(newGameButton.state == mouseIn) {
						newGameButton.state = clicked;

						drawGameOverText(gameWinner);
						SDL_RenderPresent(gameRenderer);

						SDL_Delay(100);

						initBoard();
						gameTurn = colorWhite;

						goto mainGameLoop;
					}

					break;
			}
		}

		SDL_Delay(100);
	}

quitGame:
	cleanup();

	return EXIT_SUCCESS;
}

void handleMousebuttonEvent(SDL_MouseButtonEvent event) {
	uint8_t x, y;

	/* if the clicked point is out of range then do nothing */
	if(event.x < 0 || event.x > GAME_WINDOW_WIDTH
			|| event.y < 0 || event.y > GAME_WINDOW_HEIGHT)
		return;

	x = event.x / 100;
	y = BOARD_ROW(event.y / 100, !gameTurn);

	if(event.button != SDL_BUTTON_LEFT)
		return;

	if(board[y][x].piece != empty
			&& selectedPiece.x == -1
			&& board[y][x].color == gameTurn)
		selectPiece(y, x);

	else if(selectedPiece.x != -1) {
		if(board[y][x].tileState & potentialMove)
			movePiece(y, x);

		else
			deselectPiece();
	}

	updateBoard();
}

void gameOver(color_t winner) {
	printf("game winner %d\n", winner);
	SDL_Event winEvent;
	color_t *winnerPtr;

	winnerPtr = (color_t *)malloc(sizeof(color_t));

	if(winner == noColor)
		SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "the match results in a draw\n");
	else
		SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s checkmated, %s wins\n",
			COLOR_INT_TO_STR(!winner), COLOR_INT_TO_STR(winner));
	*winnerPtr = winner;

	winEvent.user.type = GAMEOVER_EVENT;
	winEvent.user.data1 = winnerPtr;
	
	winEvent.type = SDL_USEREVENT;

	SDL_PushEvent(&winEvent);
}
