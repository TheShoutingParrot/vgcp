#include <vgcp.h>

#ifdef _SOCKET_DEBUG
#warning "socket debugging mode on... you will receive more and possibly more verbose socket related logs"
#endif

bool connectToClient(struct server *gameServer, color_t color) {
	gameServer->opt = 1;

	gameServer->serverfd = socket(AF_INET, SOCK_STREAM, 0);
	if(gameServer->serverfd == 0) {
		perror("socket init failed");
		return false;
	}

	if(setsockopt(gameServer->serverfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &(gameServer->opt), sizeof(gameServer->opt))) {
		perror("setsockopt failed");
		return false;
	}

	gameServer->address.sin_family = AF_INET;
	gameServer->address.sin_addr.s_addr = INADDR_ANY;
	gameServer->address.sin_port = htons(gameServer->port);

	if(bind(gameServer->serverfd, (struct sockaddr *)&(gameServer->address), sizeof(gameServer->address))) {
		perror("binding failed");
		return false;
	}

	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Server has been created!");

	if(listen(gameServer->serverfd, 3)) {
		perror("listening failed");
		return false;
	}

	int addrlen = sizeof(gameServer->address);
	gameServer->socket = accept(gameServer->serverfd, (struct sockaddr *)&gameServer->address, (socklen_t *)&addrlen);

	if(gameServer->socket < 0) {
		perror("socket failed");
		return false;
	}

	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Connected to player black");

	memset(&(gameServer->in), 0, 30);

	if(color == colorWhite)
		strcpy(gameServer->out, "w\n");
	else
		strcpy(gameServer->out, "b\n");

	send(gameServer->socket, gameServer->out, strlen(gameServer->out), 30);

	read(gameServer->socket, gameServer->in, 30);

	if(strcmp(gameServer->in, "r\n")) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "client didn't respond correctly");
		return false;
	}

	if(fcntl(gameServer->socket, F_SETFL, (fcntl(gameServer->socket, F_GETFL, 0) | O_NONBLOCK), 0) < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "couldn't set the socket to be non blocking");
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
	SDL_SemPost(blackServerDataLock);
	SDL_SemPost(blackMsgDataLock);

	blackOnPort = false; /* this may be unsafe? */

	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "black connection handling thread quits!");

	exit(0); /* this seems to make both the threads quit */
}

static void whiteQuits(void) {
	SDL_SemPost(whiteServerDataLock);
	SDL_SemPost(whiteMsgDataLock);

	whiteOnPort = false; /* this may be unsafe? */

	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "white connection handling thread quits!");

	exit(0); /* this seems to make both the threads quit */
}

struct move convertPlayerMsgToMove(const char *msg) {
	struct move move;

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
			if(msg.data.playerColor == colorWhite)
				strcpy(msgStr, "w");
			else
				strcpy(msgStr, "b");

			break;

		case MSG_QUITTING:
			strcpy(msgStr, "q");

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
			msg.type = MSG_MOVE;

			msg.data.move = convertPlayerMsgToMove(str);

			break;

		case 'q':
			msg.empty = false;
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

static void whiteWaitForReceivingMsg(void) {
	for(;;) {
		SDL_SemWait(whiteMsgDataLock);
		if(!msgToWhite.empty) {
			switch(msgToWhite.type) {
				case MSG_ROGER:
					msgToWhite.empty = true;
					SDL_SemPost(whiteMsgDataLock);
					
					SDL_SemWait(whiteServerDataLock);

					strcpy(whiteServer.out, "r\n"); /* we send 'r' as in roger because we have received the move */

					send(whiteServer.socket, whiteServer.out, strlen(whiteServer.out), 30);

					SDL_SemPost(whiteMsgDataLock);
					SDL_SemPost(whiteServerDataLock);

					return;

				case MSG_GAME_ENDS:
					SDL_SemWait(whiteServerDataLock);

					strcpy(whiteServer.out, "E-N\n");

					if(msgToWhite.data.winner == noColor);
					else if(msgToWhite.data.winner == colorWhite)
						*(whiteServer.out+2) = 'w';
					else
						*(whiteServer.out+2) = 'b';

					send(whiteServer.socket, whiteServer.out, strlen(whiteServer.out), 30);

					/* now that we handled this we can just label the data as trash (aka: labeling it as empty) */
					msgToWhite.empty = true;

					SDL_SemPost(whiteMsgDataLock);
					SDL_SemPost(whiteServerDataLock);

					return;

				case MSG_QUITTING:
					SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "unexpected quit message, quitting...");

					strcpy(whiteServer.out, convertMsgToString(msgToBlack));
					send(whiteServer.socket, whiteServer.out, strlen(whiteServer.out), 30);

					SDL_SemPost(whiteMsgDataLock);
					SDL_SemPost(whiteServerDataLock);

					whiteQuits();

				default:
					SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "wasn't expecting this message type!");
					break;
			}
		}
		
		SDL_SemPost(whiteMsgDataLock);
		SDL_SemPost(whiteServerDataLock);

		SDL_Delay(25);
	}
}

static void blackWaitForReceivingMsg(void) {
	for(;;) {
		SDL_SemWait(blackMsgDataLock);
		if(!msgToBlack.empty) {
			switch(msgToBlack.type) {
				case MSG_ROGER:
					msgToBlack.empty = true;
					SDL_SemPost(blackMsgDataLock);
					
					SDL_SemWait(blackServerDataLock);

					strcpy(blackServer.out, "r\n"); /* we send 'r' as in roger because we have received the move */

					send(blackServer.socket, blackServer.out, strlen(blackServer.out), 30);

					SDL_SemPost(blackMsgDataLock);
					SDL_SemPost(blackServerDataLock);

					return;

				case MSG_GAME_ENDS:
					SDL_SemWait(blackServerDataLock);

					strcpy(blackServer.out, "E-N\n");

					if(msgToBlack.data.winner == noColor);
					else if(msgToBlack.data.winner == colorWhite)
						*(blackServer.out+2) = 'w';
					else
						*(blackServer.out+2) = 'b';

					send(blackServer.socket, blackServer.out, strlen(blackServer.out), 30);

					/* now that we handled this we can just label the data as trash (aka: labeling it as empty) */
					msgToBlack.empty = true;

					SDL_SemPost(blackMsgDataLock);
					SDL_SemPost(blackServerDataLock);

					return;

				case MSG_QUITTING:
					SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "unexpected quit message, quitting...");

					strcpy(blackServer.out, convertMsgToString(msgToBlack));
					send(blackServer.socket, blackServer.out, strlen(blackServer.out), 30);

					SDL_SemPost(blackMsgDataLock);
					SDL_SemPost(blackServerDataLock);

					blackQuits();

				default:
					SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "wasn't expecting this message type!");
					break;
			}
		}
		
		SDL_SemPost(blackMsgDataLock);
		SDL_SemPost(blackServerDataLock);

		SDL_Delay(25);
	}
}

static void whiteWaitForRoger(uint8_t stateIfRoger) {
	int r, err;
	
#ifdef _SOCKET_DEBUG
	puts("wait for roger");
#endif

	for(;;) {
		r = read(whiteServer.socket, whiteServer.in, 30);
		err = errno; /* we save errno for it can change before we need it */

		if(r < 0) {
			if(err == EAGAIN || err == EWOULDBLOCK)
#ifdef _SOCKET_DEBUG
				puts("would block");
#else
				SDL_Delay(10);
#endif
			else
				SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "reading socket failed");
		}

		else {
			switch(whiteServer.in[0]) {
				case 'r':
					if(msgToWhite.type ==  MSG_MOVE_AND_END || msgToWhite.type ==  MSG_GAME_ENDS) {
						whiteServer.state = standby;
					}
					else
						whiteServer.state = stateIfRoger;

					SDL_SemPost(whiteMsgDataLock);
					SDL_SemPost(whiteServerDataLock);
		
					goto end_wait_for_roger_white;

				default:
						SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "unknown response to message!");
				case 'q':
						whiteQuits();
			}
		}

		SDL_SemPost(whiteMsgDataLock);
		SDL_SemPost(whiteServerDataLock);

		SDL_Delay(25); /* in this time the main thread could send this thread a message */

		SDL_SemWait(whiteMsgDataLock);
		SDL_SemWait(whiteServerDataLock);

		if(!msgToWhite.empty && msgToWhite.type == MSG_QUITTING) {
			SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "quitting command received from main thread!");

			strcpy(whiteServer.out, convertMsgToString(msgToWhite));

			send(whiteServer.socket, whiteServer.out, strlen(whiteServer.out), 30);

			msgToWhite.empty = true;

			SDL_SemPost(whiteMsgDataLock);
			SDL_SemPost(whiteServerDataLock);

			whiteQuits();
		}
	}

end_wait_for_roger_white:
	return;
}

static void blackWaitForRoger(uint8_t stateIfRoger) {
	int r, err;
	
#ifdef _SOCKET_DEBUG
	puts("wait for roger");
#endif

	for(;;) {
		r = read(blackServer.socket, blackServer.in, 30);
		err = errno; /* we save errno for it can change before we need it */

		if(r < 0) {
			if(err == EAGAIN || err == EWOULDBLOCK)
#ifdef _SOCKET_DEBUG
				puts("would block");
#else
				SDL_Delay(10);
#endif
			else
				SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "reading socket failed");
		}

		else {
			switch(blackServer.in[0]) {
				case 'r':
					if(msgToBlack.type ==  MSG_MOVE_AND_END || msgToBlack.type ==  MSG_GAME_ENDS) {
						blackServer.state = standby;
					}
					else
						blackServer.state = stateIfRoger;

					SDL_SemPost(blackMsgDataLock);
					SDL_SemPost(blackServerDataLock);
		
					goto end_wait_for_roger_black;

				default:
						SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "unknown response to message!");
				case 'q':
						blackQuits();
			}
		}

		SDL_SemPost(blackMsgDataLock);
		SDL_SemPost(blackServerDataLock);

		SDL_Delay(25);

		SDL_SemWait(blackMsgDataLock);
		SDL_SemWait(blackServerDataLock);

		if(!msgToBlack.empty && msgToBlack.type == MSG_QUITTING) {
			SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "quitting command received from main thread!");

			strcpy(blackServer.out, convertMsgToString(msgToBlack));

			send(blackServer.socket, blackServer.out, strlen(blackServer.out), 30);

			msgToBlack.empty = true;

			SDL_SemPost(blackMsgDataLock);
			SDL_SemPost(blackServerDataLock);

			blackQuits();
		}

	}

end_wait_for_roger_black:
	return;
}

int whiteConnectionHandlingThread(void *data) {
	int r, err;

	for(;;) {
#ifdef _SOCKET_DEBUG
		puts("white sLoop...");
#endif

		SDL_SemWait(whiteServerDataLock);
		SDL_SemWait(whiteMsgDataLock);

		switch(whiteServer.state) {
			case waitingForWhite:
				r = read(whiteServer.socket, whiteServer.in, 30);
				err = errno;

				if(r < 0) {
					if(err == EAGAIN || err == EWOULDBLOCK)
#ifdef _SOCKET_DEBUG
						puts("would block (fyi waiting fo white)");
#else
						SDL_Delay(10);
#endif /* #ifdef _SOCKET_DEBUG */
					else
						SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "reading socket failed");

					break;
				}


				msgWhite = convertStringToMsg(whiteServer.in);

				if(msgWhite.type == MSG_QUITTING) {
					SDL_SemPost(whiteMsgDataLock);
					SDL_SemPost(whiteServerDataLock);

					whiteQuits();
				}

				SDL_SemPost(whiteMsgDataLock);
				SDL_SemPost(whiteServerDataLock);

				/* the main thread needs to check that the move was legal and did the move result in the game ending
				 * (draw or checkmate) */
				whiteWaitForReceivingMsg();

				SDL_SemWait(whiteMsgDataLock);
				SDL_SemWait(whiteServerDataLock);

				if(msgToWhite.type == MSG_GAME_ENDS) {
					whiteServer.state = standby;

					whiteWaitForRoger(standby);	
				}
				else {
					whiteServer.state = waitingForBlack;
				}

				SDL_Delay(10);
		
				SDL_SemPost(whiteMsgDataLock);
				SDL_SemPost(whiteServerDataLock);

				break;

			case waitingForBlack:
				if(msgToWhite.empty == false && msgToWhite.type != MSG_QUITTING) {
					strcpy(whiteServer.out, convertMsgToString(msgToWhite));

					msgToWhite.empty = true;
		
					send(whiteServer.socket, whiteServer.out, strlen(whiteServer.out), 30);
					whiteWaitForRoger(waitingForWhite);
				}

				break;

			case standby: /* this waits for the next game to start */
#ifdef _SOCKET_DEBUG
				SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "the connection thread is on standby!");
#endif

				if(!msgToWhite.empty && msgToWhite.type != MSG_QUITTING) {
					msgToWhite.empty = true;

					strcpy(whiteServer.out, convertMsgToString(msgToWhite));
		
					send(whiteServer.socket, whiteServer.out, strlen(whiteServer.out), 30);

					whiteWaitForRoger(waitingForWhite);
				}

				break;
			
			default:
				SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "unknown server state encountered!!");
				break;
		}

		if(!msgToWhite.empty && msgToWhite.type == MSG_QUITTING) {
			SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "quitting command received from main thread!");

			strcpy(whiteServer.out, convertMsgToString(msgToWhite));
			send(whiteServer.socket, whiteServer.out, strlen(whiteServer.out), 30);

			msgToWhite.empty = true;

#ifdef _SOCKET_DEBUG 
			printf("sending quit command to client: %s and len %d\n", whiteServer.out, strlen(whiteServer.out));
#endif

			whiteQuits();
		}

		SDL_SemPost(whiteMsgDataLock);

		SDL_SemPost(whiteServerDataLock);

		SDL_Delay(50);
	}

	whiteQuits();
}

int blackConnectionHandlingThread(void *data) {
	int r, err;

	for(;;) {
#ifdef _SOCKET_DEBUG
		puts("sLoop...");
#endif

		SDL_SemWait(blackServerDataLock);
		SDL_SemWait(blackMsgDataLock);

		switch(blackServer.state) {
			case waitingForWhite:
				if(msgToBlack.empty == false && msgToBlack.type != MSG_QUITTING) {
					strcpy(blackServer.out, convertMsgToString(msgToBlack));

					msgToBlack.empty = true;
		
					send(blackServer.socket, blackServer.out, strlen(blackServer.out), 30);
					blackWaitForRoger(waitingForBlack);
				}

				break;

			case waitingForBlack:
				msgToBlack.empty = true;
		
				r = read(blackServer.socket, blackServer.in, 30);
				err = errno;

				if(r < 0) {
					if(err == EAGAIN || err == EWOULDBLOCK)
#ifdef _SOCKET_DEBUG
						puts("would block (fyi waiting fo black)");
#else
						SDL_Delay(10);
#endif /* #ifdef _SOCKET_DEBUG */
					else
						SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "reading socket failed");

					break;
				}

				msgBlack = convertStringToMsg(blackServer.in);

				if(msgBlack.type == MSG_QUITTING) {
					SDL_SemPost(blackMsgDataLock);
					SDL_SemPost(blackServerDataLock);

					blackQuits();
				}

				SDL_SemPost(blackMsgDataLock);
				SDL_SemPost(blackServerDataLock);

				/* the main thread needs to check that the move was legal and did the move result in the game ending
				 * (draw or checkmate) 
				 * REMEMBER: The main thread currently doesn't check if the move is legal! */
				blackWaitForReceivingMsg();

				SDL_SemWait(blackMsgDataLock);
				SDL_SemWait(blackServerDataLock);

				if(msgToBlack.type == MSG_GAME_ENDS) {
					blackServer.state = standby;

					blackWaitForRoger(standby);
				}
				else {
					blackServer.state = waitingForWhite;
				}

				SDL_Delay(25);
		
				SDL_SemPost(blackMsgDataLock);
				SDL_SemPost(blackServerDataLock);

				break;

			case standby: /* this waits for the next game to start */
#ifdef _SOCKET_DEBUG
				SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "the connection thread is on standby!");
#endif

				if(!msgToBlack.empty && msgToBlack.type != MSG_QUITTING) {
					strcpy(blackServer.out, convertMsgToString(msgToBlack));
msgToBlack.empty = true;
		
					send(blackServer.socket, blackServer.out, strlen(blackServer.out), 30);

					blackWaitForRoger(waitingForWhite);
				}

				break;
			
			default:
				SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "unknown server state encountered!!");
				break;
		}

		if(!msgToBlack.empty && msgToBlack.type == MSG_QUITTING) {
			SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "quitting command received from main thread!");

			strcpy(blackServer.out, convertMsgToString(msgToBlack));
			send(blackServer.socket, blackServer.out, strlen(blackServer.out), 30);

			msgToBlack.empty = true;

#ifdef _SOCKET_DEBUG 
			printf("sending quit command to client: %s and len %d\n", blackServer.out, strlen(blackServer.out));
#endif

			blackQuits();
		}

		SDL_SemPost(blackMsgDataLock);

		SDL_SemPost(blackServerDataLock);

		SDL_Delay(50);
	}

	blackQuits();
}

void createBlackThread(void) {
	blackThreadID = SDL_CreateThread(blackConnectionHandlingThread, "Black connection", NULL);

	blackServerDataLock = SDL_CreateSemaphore(1);
	blackMsgDataLock = SDL_CreateSemaphore(1);

	blackOnPort = true;

	blackServer.state = waitingForWhite;
}

void createWhiteThread(void) {
	whiteThreadID = SDL_CreateThread(whiteConnectionHandlingThread, "White connection", NULL);

	whiteServerDataLock = SDL_CreateSemaphore(1);
	whiteMsgDataLock = SDL_CreateSemaphore(1);

	whiteOnPort = true;

	whiteServer.state = waitingForWhite;
}
