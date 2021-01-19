#include <vgcp.h>

void cleanup(void) {
        SDL_DestroyRenderer(gameRenderer);
        SDL_DestroyWindow(gameWindow);

        SDL_Quit();
}

void die(char *fmt, ...) {
        va_list vargs;

        va_start(vargs, fmt);

        fputs(ANSI_ESCAPE_CODE(ESCAPE_CODE_RED) ANSI_ESCAPE_CODE(ESCAPE_CODE_BOLD), stderr);
        SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, fmt, vargs);
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "fatal error... dying...");
        fputs(ANSI_ESCAPE_CODE(ESCAPE_CODE_RESET), stderr);

        va_end(vargs);

        exit(EXIT_FAILURE);
}

