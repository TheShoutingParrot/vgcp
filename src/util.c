#include <vgcp.h>

void cleanup(void) {
	if(blackOnPort) {
		closeSocket(blackServer.socket);
	}
	else if(whiteOnPort) {
		closeSocket(whiteServer.socket);
	}

        SDL_DestroyRenderer(gameRenderer);
        SDL_DestroyWindow(gameWindow);

        SDL_Quit();
}

void usage(const char *name) {
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "usage: %s [-v] [-h] [-b PORT] [-w PORT]", name);
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "For more help use the -h command");

	exit(0);
}

/* prints a more detailed usage guide */
void help(const char *name) {
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Help message for vgcp version: "PROGRAM_VERSION"\n");
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "usage: %s [-v] [-h] [-b PORT]\n", name);

	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "command-line arguments:");
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "\t-v\t\tprints the version out and exits");
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "\t-h\t\tprints this help message out\n");
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "\t-w PORT\t\tgives control of the white player to another program that connects on the PORT*\n");
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "\t-b PORT\t\tgives control of the black player to another program that connects on the PORT*\n");

	putchar('\n');
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "* These features are very unstable and untested!");

	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "If you find a bug or another type of issue please report it on github (link: https://github.com/TheShoutingParrot/vgcp/issues) or send an email to \"theshoutingparrot@protonmail.com\"!");

	exit(0);
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
