#include <vgcp.h>

bool init(void) {
        if(SDL_Init(SDL_INIT_VIDEO) < 0) {
                SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Failed to initialize SDL: %s", SDL_GetError());
                return true;
        }

        gameWindow = SDL_CreateWindow(GAME_NAME,
                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                        GAME_WINDOW_WIDTH, GAME_WINDOW_HEIGHT,
                        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

        if(gameWindow == NULL) {
                SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Failed to create the window: %s", SDL_GetError());
                return true;
        }

        gameRenderer = SDL_CreateRenderer(gameWindow, -1, SDL_RENDERER_ACCELERATED);

        if(gameRenderer == NULL) {
                SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Failed to create the renderer: %s", SDL_GetError());
                return true;
        }

        SDL_RenderSetLogicalSize(gameRenderer, GAME_LOGICAL_WIDTH, GAME_LOGICAL_HEIGHT);

        if(!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to initialize SDL_Image: %s", IMG_GetError());
                return true;
        }

        if(TTF_Init() == -1) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to initialize SDL_ttf: %s", TTF_GetError());
                return true;
        }

	initPositionList();

        return loadMedia();
}

bool loadMedia(void) {
        uint8_t p, c;
        char suffixStr[128];
        char filename[128];

        for(p = pawn; p < empty; p++) {
                for(c = 0; c < 2; c++) {
                        strcpy(suffixStr, "_");
                        strcat(suffixStr, COLOR_INT_TO_STR(c));
                        strcat(suffixStr, ".png");

                        strcpy(filename, PIECES_IMG_PATH);
                        strcat(filename, pieceName[p]);
                        strcat(filename, suffixStr);

                        pieceTexture[p][c] = loadTexture(filename);
                }
        }

        SDL_Color textColor = {0xff, 0xff, 0xff};

        gameFont = TTF_OpenFont(FONT_PATH"steel.ttf", 50);
        textTexture[0].texture = renderText("CHECKMATE", textColor);
        textTexture[1].texture = renderText("White wins", textColor);
        textTexture[2].texture = renderText("Black wins", textColor);
	textTexture[3].texture = renderText("Draw", textColor);

	textTexture[0].rect.w = 400;
	textTexture[0].rect.h = 100;
	textTexture[0].rect.x = (GAME_LOGICAL_WIDTH / 2) - (textTexture[0].rect.w  / 2);
	textTexture[0].rect.y = 110;

	textTexture[0].background.w = 440;
	textTexture[0].background.h = 110;
	textTexture[0].background.x = (GAME_LOGICAL_WIDTH / 2) - (textTexture[0].background.w / 2);
	textTexture[0].background.y = 95;

	textTexture[1].rect.w = 200;
	textTexture[1].rect.h = 60;
	textTexture[1].rect.x = (GAME_LOGICAL_WIDTH / 2) - (textTexture[1].rect.w  / 2);
	textTexture[1].rect.y = 220;

	textTexture[1].background.w = 220;
	textTexture[1].background.h = 70;
	textTexture[1].background.x = (GAME_LOGICAL_WIDTH / 2) - (textTexture[1].background.w / 2);
	textTexture[1].background.y = 210;

	/* "black wins" rect & background is the same as
	 * "white wins" rect & background */

	textTexture[2].rect = textTexture[1].rect;
	textTexture[2].background = textTexture[1].background;

	textTexture[3].rect.w = 200;
	textTexture[3].rect.h = 100;
	textTexture[3].rect.x = (GAME_LOGICAL_WIDTH / 2) - (textTexture[3].rect.w  / 2);
	textTexture[3].rect.y = 110;

	textTexture[3].background.w = 240;
	textTexture[3].background.h = 110;
	textTexture[3].background.x = (GAME_LOGICAL_WIDTH / 2) - (textTexture[3].background.w  / 2);
	textTexture[3].background.y = 95;

        newGameButton.texture[normal] = renderText("New Game", textColor);

        textColor.r = 0x00;
        textColor.g = 0x00;
        textColor.b = 0x00;

        newGameButton.texture[mouseIn] = renderText("New Game", textColor);
        newGameButton.texture[clicked] = renderText("New Game", textColor);

        newGameButton.rect.w = 180;
        newGameButton.rect.h = 60;
        newGameButton.rect.x = (GAME_LOGICAL_WIDTH / 2) - (newGameButton.rect.w / 2);
        newGameButton.rect.y = 295;

        newGameButton.backgroundRect.w = 200;
        newGameButton.backgroundRect.h = 70;
        newGameButton.backgroundRect.x = (GAME_LOGICAL_WIDTH / 2) - (newGameButton.backgroundRect.w / 2);
        newGameButton.backgroundRect.y = 285;

        newGameButton.backgroundColor[normal].r = 0x00;
        newGameButton.backgroundColor[normal].g = 0x00;
        newGameButton.backgroundColor[normal].b = 0x00;

        newGameButton.backgroundColor[mouseIn].r = 0xff;
        newGameButton.backgroundColor[mouseIn].g = 0xff;
        newGameButton.backgroundColor[mouseIn].b = 0xff;

        newGameButton.backgroundColor[clicked].r = 0x00;
        newGameButton.backgroundColor[clicked].g = 0x00;
        newGameButton.backgroundColor[clicked].b = 0x00;

        newGameButton.state = normal;

        return false;
}

void initBoard(void) {
        uint8_t x, y;

        for(y = 0; y < 8; y++) {
                for(x = 0; x < 8; x++) {
                        position.board[y][x] = unusedBoard[y][x];
                }
        }
}
