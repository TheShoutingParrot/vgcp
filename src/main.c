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

struct server whiteServer;
struct server blackServer;

SDL_Thread *blackThreadID,
	   *whiteThreadID;

SDL_sem *whiteServerDataLock,
	*whiteMsgDataLock,
	*blackServerDataLock,
	*blackMsgDataLock;

bool whiteOnPort = false;
bool blackOnPort = false;

struct msg msgBlack,
	   msgToBlack,
	   msgWhite,
	   msgToWhite;

int main(int argc, char *args[]) {
	uint8_t i;

	/* parses the command line arguments */
	for(i = 1; i < argc; i++) { 
		if(*args[i] == '-') {
			if(strlen(args[i]) == 2) {
				switch(*(args[i]+1)) {
					case 'v':
						SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "version: "PROGRAM_VERSION);
						return EXIT_SUCCESS;
					case 'h':
						help(args[0]);
					case 'b':
						if((i+1) >= argc)
							usage(args[0]);
						
						blackServer.port = atoi(args[++i]);
						SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "port %d will play as black\n", blackServer.port);
						SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "This feature (other programs controlling a player) hasn't been tested well at all, please don't use this if you aren't debugging!");

						/* if the connection failed then die */
						SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "waiting for black to connect");
						if(!connectToClient(&blackServer, colorBlack))
							die("Connection between the GUI and the player failed");

						SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "black connected succesfully");

						blackOnPort = true;

						break;

					case 'w':
						if((i+1) >= argc)
							usage(args[0]);
						
						whiteServer.port = atoi(args[++i]);
						SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "port %d will play as white\n", whiteServer.port);
						SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "This feature (other programs controlling a player) hasn't been tested well at all, please don't use this if you aren't debugging!");

						/* if the connection failed then die */
						SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "waiting for white to connect");
						if(!connectToClient(&whiteServer, colorWhite))
							die("Connection between the GUI and the player failed");

						SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "black connected succesfully");

						whiteOnPort = true;

						break;

					default:
						usage(args[0]);
				}
			}

			else {
				usage(args[0]);
			}
		}

		else {
			usage(args[0]);
		}
	}

	msgBlack.empty = true;
	msgToBlack.empty = true;
	msgWhite.empty = true;
	msgToWhite.empty = true;
	
	if(whiteOnPort)
		createWhiteThread();

	if(blackOnPort)
		createBlackThread();

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
					updateWindow();

					switch(*((userEvents_t *)event.user.data2)) {
						case GAMEOVER_EVENT:
							gameWinner = *((color_t *)event.user.data1);

							free(event.user.data1);
							free(event.user.data2);

							if(blackOnPort) {
								SDL_SemWait(blackMsgDataLock);

								if(position.playerToMove == colorBlack) {
									msgToBlack.empty = false;
									msgToBlack.type = MSG_MOVE_AND_END;
									msgToBlack.data.lastMove.move = position.prevMove;
									msgToBlack.data.lastMove.winner = gameWinner;
								}
								else {
									msgToBlack.empty = false;
									msgToBlack.type = MSG_GAME_ENDS;
									msgToBlack.data.winner = gameWinner;
								}

								SDL_SemPost(blackMsgDataLock);
							}

							if(whiteOnPort) {
								SDL_SemWait(whiteMsgDataLock);

								if(position.playerToMove == colorWhite) {
									msgToWhite.empty = false;
									msgToWhite.type = MSG_MOVE_AND_END;
									msgToWhite.data.lastMove.move = position.prevMove;
									msgToWhite.data.lastMove.winner = gameWinner;
								}
								else {
									msgToWhite.empty = false;
									msgToWhite.type = MSG_GAME_ENDS;
									msgToWhite.data.winner = gameWinner;
								}

								SDL_SemPost(whiteMsgDataLock);
							}

							goto gameOverLoop;

						case MOVED_EVENT:
							if(whiteOnPort) {
								SDL_SemWait(whiteMsgDataLock);

								msgToWhite.empty = false;

								if(*((color_t *)event.user.data1) == colorBlack) {
									msgToWhite.type = MSG_MOVE;
									msgToWhite.data.move = position.prevMove;
								}

								else {
									msgToWhite.type = MSG_ROGER; /* we want to tell the client that we recieved this move and it worked */
								}

								SDL_SemPost(whiteMsgDataLock);
							}

							if(blackOnPort) {
								SDL_SemWait(blackMsgDataLock);

								msgToBlack.empty = false;

								if(*((color_t *)event.user.data1) == colorWhite) {
									msgToBlack.type = MSG_MOVE;
									msgToBlack.data.move = position.prevMove;
								}

								else {
									msgToBlack.type = MSG_ROGER; /* we want to tell the client that we recieved this move and it worked */
								}

								SDL_SemPost(blackMsgDataLock);
							}
							break;

						default:
							SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "unknown custom event received!");
					}
							
					free(event.user.data1);
					free(event.user.data2);

					break;
			}
		}

		if(blackOnPort && !msgBlack.empty) {
			switch(msgBlack.type) {
				case MSG_MOVE:
					if(checkMoveLegality(msgBlack.data.move, colorBlack))
						movePiece(msgBlack.data.move);
					else {
						SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "the move made by the client (black) was illegal!... quitting connection");

						msgToBlack.empty = false;
						msgToBlack.type = MSG_QUITTING; /* we want to tell the client that we recieved this move and it worked */
					}
					break;
				default:
					SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "unknown type of message sent to the main thread!");
					break;
			}

			msgBlack.empty = true; /* now that we handled this we can label the black message as trash */
		}

		if(whiteOnPort && !msgWhite.empty) {
			switch(msgWhite.type) {
				case MSG_MOVE:
					if(checkMoveLegality(msgWhite.data.move, colorWhite))
						movePiece(msgWhite.data.move);
					else {
						SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "the move made by the client (white) was illegal!... quitting connection");

						msgToWhite.empty = false;
						msgToWhite.type = MSG_QUITTING; /* we want to tell the client that we recieved this move and it worked */
					}
					break;
				default:
					SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "unknown type of message sent to the main thread!");
					break;
			}

			msgWhite.empty = true; /* now that we handled this we can label the black message as trash */
		}

		SDL_Delay(50);
	}

gameOverLoop:
	updateWindow();

	SDL_Delay(50);

	drawGameOverText(gameWinner);
	SDL_RenderPresent(gameRenderer);

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

						if(blackOnPort) {
							SDL_SemWait(blackServerDataLock);

							msgToBlack.empty = false;
							msgToBlack.type = MSG_NEW_GAME;
							msgToBlack.data.playerColor = colorBlack;

							SDL_SemPost(blackServerDataLock);
						}

						if(whiteOnPort) {
							SDL_SemWait(whiteServerDataLock);

							msgToWhite.empty = false;
							msgToWhite.type = MSG_NEW_GAME;
							msgToWhite.data.playerColor = colorWhite;

							SDL_SemPost(whiteServerDataLock);
						}

						goto mainGameLoop;
					}

					break;
			}
		}

		SDL_Delay(100);
	}

quitGame:
	if(blackOnPort) {
		SDL_SemWait(blackMsgDataLock);

		msgToBlack.empty = false;
		msgToBlack.type = MSG_QUITTING;

		SDL_SemPost(blackMsgDataLock);

		SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "waiting for the black thread to finish...");
		SDL_Delay(100);

		SDL_WaitThread(blackThreadID, NULL);
	}

	if(whiteOnPort) {
		SDL_SemWait(whiteMsgDataLock);

		msgToWhite.empty = false;
		msgToWhite.type = MSG_QUITTING;

		SDL_SemPost(whiteMsgDataLock);

		SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "waiting for the white thread to finish...");
		SDL_Delay(100);

		SDL_WaitThread(whiteThreadID, NULL);
	}

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
			&& position.board[y][x].color == position.playerToMove) {
		if(position.playerToMove == colorBlack && blackOnPort);
		else if(position.playerToMove == colorWhite && whiteOnPort);
		else
			selectPiece(y, x);
	}

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
	userEvents_t *typePtr;
	color_t *winnerPtr;

	if(winner == noColor)
		SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "the match results in a draw\n");
	else
		SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s checkmated, %s wins\n",
			COLOR_INT_TO_STR(!winner), COLOR_INT_TO_STR(winner));

	winnerPtr = (color_t *)malloc(sizeof(color_t));
	typePtr = (userEvents_t *)malloc(sizeof(userEvents_t));

	*winnerPtr = winner;
	*typePtr = GAMEOVER_EVENT;

	winEvent.type = SDL_USEREVENT;

	winEvent.user.data1 = winnerPtr;
	winEvent.user.data2 = typePtr;

	SDL_PushEvent(&winEvent);
}
