#include <vgcp.h>

SDL_Window *gameWindow;
SDL_Renderer *gameRenderer;

SDL_Point selectedPiece	= {-1, -1};

bool kingMated[2] = {false, false}, 
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

SDL_Texture *pieceTexture[empty][2];

struct labels textTexture[4];
struct button newGameButton;

TTF_Font *gameFont;

const int8_t potentialKnight[4][2] = {{2, 1}, {2, -1}, {-2, 1}, {-2, -1}};

uint8_t halfmoveClock = 0;

struct position position;
struct positionList positionList;

int main(void) {
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "version: "PROGRAM_VERSION);

#ifdef _DEBUG
	SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "debug option is on! You've been warned!");
#endif

	color_t gameWinner;

	if(init())
		die("Initialization failed");

	initPosition();

	SDL_Event event;

mainGameLoop:
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "game begins");

	halfmoveClock = 0;
	updateBoard();

	updateWindow();

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

						removePositionList();
						initPositionList();

						initPosition();

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
	if(event.x < 0 || event.x > GAME_LOGICAL_WIDTH
			|| event.y < 0 || event.y > GAME_LOGICAL_HEIGHT)
		return;

	x = event.x / 100;
	y = BOARD_ROW(event.y / 100, !position.playerToMove);

	if(event.button != SDL_BUTTON_LEFT)
		return;

	if(position.board[y][x].piece != empty
			&& selectedPiece.x == -1
			&& position.board[y][x].color == position.playerToMove)
		selectPiece(y, x);

	/* this means that a piece is selected, if it's a potential move
	 * then move there, if not then deselect the piece */
	else if(selectedPiece.x != -1) {
		struct move move;

		if(position.board[y][x].tileState & potentialMove) {
			move.to.x = x;
			move.to.y = y;
			move.from = selectedPiece;
			move.color = position.board[move.from.y][move.from.x].color;
			move.piece = position.board[move.from.y][move.from.x].piece;
			move.capturedPiece = position.board[move.to.y][move.to.x].piece;

			deselectPiece();
			movePiece(move);
		}
		
		else if(position.board[y][x].tileState & potentialCastling) {
			deselectPiece();
			castleKing(position.playerToMove, x == 2);
		}

		if(position.board[y][x].tileState & potentialEnPassant) {
			int8_t temp;

			move.to.x = x;
			move.to.y = y;
			move.from = selectedPiece;
			move.color = position.board[move.from.y][move.from.x].color;
			move.piece = position.board[move.from.y][move.from.x].piece;

			/* if en passant happends (which it is) then the captured piece is a pawn for sure */
			move.capturedPiece = pawn; 
	
			temp = y - PLUS_OR_MINUS(position.board[selectedPiece.y][selectedPiece.x].color, 1);
			deselectPiece();
			enPassant(temp, x, move);
		}

		else
			deselectPiece();
	}

	updateBoard();
}

void gameOver(color_t winner) {
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
