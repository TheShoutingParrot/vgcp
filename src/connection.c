#include <vgcp.h>

bool connectToBlack(void) {
	gameServer.opt = 1;

	gameServer.serverfd = socket(AF_INET, SOCK_STREAM, 0);
	if(gameServer.serverfd == 0) {
		perror("socket failed");
		return false;
	}

	if(setsockopt(gameServer.serverfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &gameServer.opt, sizeof(gameServer.opt))) {
		perror("setsockopt failed");
		return false;
	}

	gameServer.address.sin_family = AF_INET;
	gameServer.address.sin_addr.s_addr = INADDR_ANY;
	gameServer.address.sin_port = htons(gameServer.port);

	if(bind(gameServer.serverfd, (struct sockaddr *)&gameServer.address, sizeof(gameServer.address))) {
		perror("binding failed");
		return false;
	}

	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Server has been created!");

	if(listen(gameServer.serverfd, 3)) {
		perror("listening failed");
		return false;
	}

	int addrlen = sizeof(gameServer.address);
	gameServer.socket = accept(gameServer.serverfd, (struct sockaddr *)&gameServer.address, (socklen_t *)&addrlen);

	if(gameServer.socket < 0) {
		perror("socket failed");
		return false;
	}

	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Connected to player black");

	memset(&(gameServer.in), 0, 30);

	strcpy(gameServer.out, "b\n");
	send(gameServer.socket, gameServer.out, strlen(gameServer.out), 30);

	puts("message sent");

	read(gameServer.socket, gameServer.in, 30);

	printf("received: %s", gameServer.in);

	if(strcmp(gameServer.in, "r\n")) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "client didn't respond correctly");
		return false;
	}

	return true;
}

static uint8_t convertCharToPiece(char c) {
	switch(c) {
		case 'P':
			return pawn;
		case 'N':
			return knight;
		case 'B':
			return bishop;
		case 'R':
			return rook;
		case 'Q':
			return queen;
		case 'K':
			return king;
		case '0':
			return empty;
	}

	return empty;
}

static char convertPieceToChar(uint8_t p) {
	switch(p) {
		case pawn:
			return 'P';
		case knight:
			return 'N';
		case bishop:
			return 'B';
		case rook:
			return 'R';
		case queen:
			return 'Q';
		case king:
			return 'K';
		case empty:
			return '0';
	}

	return 'N';
}

static void blackQuits(void) {
	SDL_SemPost(serverDataLock);
	SDL_SemPost(msgDataLock);

	blackOnPort = false; /* this may be unsafe? */

	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "black connection handling thread quits!");

	exit(0); /* this seems to make both the threads quit */
}

struct move convertPlayerMsgToMove(const char *msg) {
	struct move move;

	printf("msg %s\n", msg);

	move.from.x = (*(msg+2) - 'a');
	move.from.y = (*(msg+3) - '1');

	move.piece = position.board[move.from.y][move.from.x].piece;
	move.color = position.board[move.from.y][move.from.x].color;

	move.to.x = (*(msg+5) - 'a');
	move.to.y = (*(msg+6) - '1');

	move.capturedPiece = convertCharToPiece(*(msg+8));

	return move;
}

void closeSocket(int socket) {
	close(socket);

	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Socket has been closed");
}

char *convertMsgToString(struct msg msg) {
	char *msgStr;
	msgStr = (char *)malloc(30 * sizeof(char));

	switch(msg.type) {
		case MSG_MOVE:
			strcpy(msgStr, "m-a1>b1xN\n");

			*(msgStr+2) = msg.data.move.from.x + 'a';
			*(msgStr+3) = msg.data.move.from.y + '1';

			*(msgStr+5) = msg.data.move.to.x + 'a';
			*(msgStr+6) = msg.data.move.to.y + '1';

			*(msgStr+8) = convertPieceToChar(msg.data.move.capturedPiece);

			break;

		case MSG_MOVE_AND_END:
			strcpy(msgStr, "M-a1>b1xNrD");

			*(msgStr+2) = msg.data.lastMove.move.from.x + 'a';
			*(msgStr+3) = msg.data.lastMove.move.from.x + '1';

			*(msgStr+5) = msg.data.lastMove.move.to.x + 'a';
			*(msgStr+6) = msg.data.lastMove.move.to.x + '1';

			*(msgStr+8) = convertPieceToChar(msg.data.lastMove.move.capturedPiece);

			*(msgStr+10) = (msg.data.lastMove.winner == noColor) ? 'D' : ((msg.data.lastMove.winner == colorBlack) ? 'B' : 'W');

			break;

		case MSG_NEW_GAME:
			puts("NEW GAYME");

			SDL_SemWait(serverDataLock);

			if(msgServer.data.playerColor == colorWhite)
				strcpy(gameServer.out, "w");
			else
				strcpy(gameServer.out, "b");

			send(gameServer.socket, gameServer.out, strlen(gameServer.out), 30);
					
			msgServer.empty = true;

			SDL_SemPost(msgDataLock);
			SDL_SemPost(serverDataLock);

			break;


		default:
			SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "attempted to convert an unknown msg type");
			break;
	}

	return msgStr;
}

struct msg convertStringToMsg(char *str) {
	struct msg msg;

	switch(*str) {
		case 'm':
			msg.empty = false;
			msg.to = noColor;
			msg.type = MSG_MOVE;

			msg.data.move = convertPlayerMsgToMove(str);

			break;

		case 'q':
			msg.empty = false;
			msg.to = noColor;
			msg.type = MSG_QUITTING; /* this tells the receiver that whoever sent this is quitting 
						  * (this can be interpreted as a resignation... because it kind of is) */

			break;

		default:
			msg.empty = true; /* we didn't recognize the type of message so we just assume it's trash and label the message as empty */
			msg.type = MSG_UNKNOWN; /* just for good measure we label the message as unknown */

			SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "the string sent by the client can't be converted to the correct message type!");

			break;
	}

	return msg;
}

static void waitForReceivingMsg(void) {
	for(;;) {
		printf("waiting...\n");

		SDL_SemWait(msgDataLock);
		if(!msgServer.empty) {
			switch(msgServer.type) {
				case MSG_ROGER:
					msgServer.empty = true;
					SDL_SemPost(msgDataLock);
					
					SDL_SemWait(serverDataLock);

					strcpy(gameServer.out, "r\n"); /* we send 'r' as in roger because we have received the move */

					send(gameServer.socket, gameServer.out, strlen(gameServer.out), 30);

					SDL_SemPost(msgDataLock);
					SDL_SemPost(serverDataLock);

					return;

				case MSG_GAME_ENDS:
					SDL_SemWait(serverDataLock);

					strcpy(gameServer.out, "E-N\n");

					if(msgServer.data.winner == noColor);
					else if(msgServer.data.winner == colorWhite)
						*(gameServer.out+2) = 'w';
					else
						*(gameServer.out+2) = 'b';

					send(gameServer.socket, gameServer.out, strlen(gameServer.out), 30);

					/* now that we handled this we can just label the data as trash (aka: labeling it as empty) */
					msgServer.empty = true;

					SDL_SemPost(msgDataLock);
					SDL_SemPost(serverDataLock);

					return;

				case MSG_QUITTING:
					SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "unexpected quit message, quitting...");

					blackQuits();

				default:
					SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "wasn't expecting this message type!");
					break;
			}
		}
		
		SDL_SemPost(msgDataLock);
		SDL_SemPost(serverDataLock);

		SDL_Delay(500);
	}
}

int connectionHandlingThread(void *data) {
	for(;;) {
		SDL_SemWait(serverDataLock);
		SDL_SemWait(msgDataLock);

		switch(gameServer.state) {
			case waitingForWhite:
				if(msgServer.to == colorBlack && msgServer.empty == false) {
					strcpy(gameServer.out, convertMsgToString(msgServer));

					msgServer.empty = true;
		
					printf("msg: %s\n", gameServer.out); 
		
					send(gameServer.socket, gameServer.out, strlen(gameServer.out), 30);

					read(gameServer.socket, gameServer.in, 30);

					switch(gameServer.in[0]) {
						case 'r':
							if(msgServer.type ==  MSG_MOVE_AND_END || msgServer.type ==  MSG_GAME_ENDS) {
								gameServer.state = standby;
							}
							else
								gameServer.state = waitingForBlack;

							SDL_SemPost(msgDataLock);
							SDL_SemPost(serverDataLock);
		
							break;

						default:
							SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "unknown response to message!");
						case 'q':
							blackQuits();
					}
				}

				break;

			case waitingForBlack:
				SDL_SemWait(serverDataLock);

				SDL_SemWait(msgDataLock);

				msgServer.empty = true;
		
				read(gameServer.socket, gameServer.in, 30);
				printf("in: %s\n", gameServer.in); 

				msgBlack = convertStringToMsg(gameServer.in);

				if(msgBlack.type == MSG_QUITTING) {
					SDL_SemPost(msgDataLock);
					SDL_SemPost(serverDataLock);

					blackQuits();
				}

				SDL_SemPost(msgDataLock);
				SDL_SemPost(serverDataLock);

				/* the main thread needs to check that the move was legal and did the move result in the game ending
				 * (draw or checkmate) 
				 * REMEMBER: The main thread currently doesn't check if the move is legal! */
				waitForReceivingMsg();

				SDL_SemWait(msgDataLock);
				SDL_SemWait(serverDataLock);

				if(msgServer.type == MSG_GAME_ENDS) {
					gameServer.state = standby;

					read(gameServer.socket, gameServer.in, 30);

					switch(gameServer.in[0]) {
						case 'r':

							if(msgServer.type ==  MSG_MOVE_AND_END || msgServer.type ==  MSG_GAME_ENDS) {
								gameServer.state = standby;
							}
							else
								gameServer.state = waitingForBlack;

							SDL_SemPost(msgDataLock);
							SDL_SemPost(serverDataLock);
		
							break;

						default:
							SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "unknown response to message!");
						case 'q':
							blackQuits();
					}
				}
				else {
					gameServer.state = waitingForWhite;
				}

				SDL_Delay(100);
		
				SDL_SemPost(msgDataLock);
				SDL_SemPost(serverDataLock);

				break;

			case standby: /* this waits for the next game to start */
				SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "the connection thread is on standby!");
				if(!msgServer.empty) {
					strcpy(gameServer.out, convertMsgToString(msgServer));
msgServer.empty = true;
		
					printf("msg: %s\n", gameServer.out); 
		
					send(gameServer.socket, gameServer.out, strlen(gameServer.out), 30);

					read(gameServer.socket, gameServer.in, 30);

					printf("reading..... %s\n", gameServer.in);

					switch(gameServer.in[0]) {
						case 'r':
							SDL_SemPost(msgDataLock);
							SDL_SemPost(serverDataLock);

							gameServer.state = waitingForWhite;

							break;

						default:
							SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "unknown response to message!");
						case 'q':
							blackQuits();
					}
				}

				break;
			
			default:
				SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "unknown server state encountered!!");
				break;
		}

		SDL_SemPost(msgDataLock);

		SDL_SemPost(serverDataLock);

		SDL_Delay(500);

	}

	blackQuits();
}

void createBlackThread(void) {
	blackThreadID = SDL_CreateThread(connectionHandlingThread, "Black", NULL);

	serverDataLock = SDL_CreateSemaphore(1);
	msgDataLock = SDL_CreateSemaphore(1);

	blackOnPort = true;

	gameServer.state = waitingForWhite;
}
