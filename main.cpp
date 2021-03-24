#include "word_game.h"

int main() {
	srand((unsigned int)time(NULL));
	SetConsoleOutputCP(CP_TURKISH);
	SetConsoleCP(CP_TURKISH);
	WordGame AliIhsanVarol(GAME_TIME_IN_SECONDS/60, GAME_TIME_IN_SECONDS%60, BENJAMIN_TIME_IN_SECONDS/60, BENJAMIN_TIME_IN_SECONDS%60);
	AliIhsanVarol.Start();
	return 0;
}
