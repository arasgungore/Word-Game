#include "word_game.h"
#include <fstream>
#include <vector>
#include <cmath>
#include <conio.h>

// moves the cursor to a different (x, y) location on the terminal
void MoveCursorToXY(const short &x, const short &y) {
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), (COORD){x, y});
}

// returns a random number between "min" and "max"
template<class T> T GetRandomNumber(const T &min, const T &max) {
	return min + T(double(rand()) / double(RAND_MAX + 1.0) * (max - min + 1));
}


// Timer constructor
Timer::Timer(const short &minutes, const double &seconds) : minutes(minutes), seconds(seconds) {	}

// getter method, returns minutes
short Timer::GetMinutes() const {
	return minutes;
}

// getter method, returns seconds
double Timer::GetSeconds() const {
	return seconds;
}

// setter method, sets minutes and seconds
void Timer::SetTimer(const short &minutes, const double &seconds) {
	this->minutes = minutes;
	this->seconds = seconds;
}

// increases the timer by an amount of "m_inc" minutes and "s_inc" seconds
void Timer::IncreaseTimer(const short &m_inc, const double &s_inc) {
	minutes += m_inc;
	seconds += s_inc;
	if(seconds >= 60.0) {
		seconds -= 60.0;
		minutes++;
	}
}

// updates the timer
bool Timer::UpdateTimer(const clock_t &before, const clock_t &after) {
	seconds -= double(after - before) / double(CLOCKS_PER_SEC);
	if(seconds < 0.0) {
		seconds += 60.0;
		if((--minutes) < 0)
			return false;
	}
	return true;
}

// updates the timer display on the terminal
void Timer::UpdateTimerDisplay(clock_t &begin, const clock_t &after, const short &x, const short &y) const {
	if(double(after - begin) / double(CLOCKS_PER_SEC) >= 0.9) {		// update the timer display once every 0.9 seconds
		MoveCursorToXY(x, y);
		std::cout << "     ";
		MoveCursorToXY(x, y);
		std::cout << minutes << ":" << std::setfill('0') << std::setw(2) << short(seconds);
		begin = after;
	}
}



// WordGame constructor
WordGame::WordGame(const short &minutes, const double &seconds, const short &benjamin_m, const double &benjamin_s) : current_time(minutes, seconds), benjamin(benjamin_m, benjamin_s) {	}

// starts the game
void WordGame::Start() {
	ShowWindow(GetConsoleWindow(), SW_MAXIMIZE);
	for(unsigned short i=MIN_NO_OF_LETTERS;i<=MAX_NO_OF_LETTERS;i++)
		for(unsigned short j=0;j<NO_OF_ANSWERS_FOR_EACH_NO_OF_LETTERS;j++)
			ReadQuestion(i);
	PrintOpeningScreen();
	PrintLoadingScreen();
	GetNextQuestion();
	Update();
}

// updates the frame
void WordGame::Update() {
	clock_t begin = clock(), before, after = clock();
	do {
		current_time.UpdateTimerDisplay(begin, after, 16, 14);
		if(kbhit())
			switch(toupper(getch())) {
				case BENJAMIN:					// if the player hits the Benjamin button (pun intended), stops the timer
					StopTime(before, after);	// and gives 30 seconds to the player to come up with an answer
					break;
				case HARF_ALAYIM:				// if the player requests a letter by pressing 'H', one letter of the answer
					GetLetter();				// is revealed on the display
					break;
				case PAUSE_KEY: {				// pause the game
					clock_t begin_temp = clock(), after_temp;
					char key;
					do {
						key = getch();
						if(key == ESCAPE_KEY)
							exit(0);
					} while((key = toupper(key)) != PAUSE_KEY);
					after_temp = clock();
					double past_time = double(after_temp - begin_temp) / double(CLOCKS_PER_SEC);
					current_time.IncreaseTimer(short(past_time)/60, fmod(past_time, 60.0));		// adds the time that has passed while the game has been paused to the timer
					break; }
				case ESCAPE_KEY:
					std::cout << "\n\n\n\n";
					exit(0);
			}
		before = after;
		after = clock();
	} while(current_time.UpdateTimer(before, after));
	current_time.SetTimer(0, 0.0);
	PrintEndGameScreen();
	RecordScore();
}

// reveals a letter of the answer (Harf alay�m)
// however by doing so, the player sacrifices 100 potential points
void WordGame::GetLetter() {
	unsigned short letter_index;
	do {
		letter_index = GetRandomNumber<unsigned short>(0, current_word.length() - 1);
	} while(word_on_display[letter_index] != '_');
	for(unsigned short i=0;i<30;i++) {		// display 30 random letters fast before revealing the letter
		word_on_display[letter_index] = GetRandomNumber<char>('A', 'Z');
		PrintQandAScreen();
		Sleep(20);
	}
	word_on_display[letter_index] = current_word[letter_index];
	PrintQandAScreen();
	if((--no_of_undisplayed_letters) <= 0) {
		Sleep(1000);
		current_time.IncreaseTimer(0, 1.1);
		GetNextQuestion();
		return;
	}
	current_time.IncreaseTimer(0, 0.6);
}

// stops the timer (Benjamin), gives the player time to think
// however by doing so, the player cannot request anymore letters and will lose points by the amount of unrevealed number of letters
void WordGame::StopTime(clock_t &before1, clock_t &after1) {
	benjamin.SetTimer(BENJAMIN_TIME_IN_SECONDS/60, BENJAMIN_TIME_IN_SECONDS%60);
	MoveCursorToXY(16, 19);
	std::cout << benjamin.GetMinutes() << ":" << std::setfill('0') << std::setw(2) << short(benjamin.GetSeconds()) << std::endl;
	std::cout << "\t\tTahmininizi giriniz: ";
	clock_t begin = clock(), before, after = clock();
	do {
		benjamin.UpdateTimerDisplay(begin, after, 16, 19);
		if(kbhit()) {
			MoveCursorToXY(37, 20);
			std::string guess;
			getline(std::cin, guess);
			if(CompareAnswers(guess)) {
				StopTimeUtil(before, after, before1, after1, true);
				return;
			}
			else {
				MoveCursorToXY(37, 20);
				std::cout << std::string(guess.length(), ' ');
				MoveCursorToXY(37, 20);
			}
		}
		before = after;
		after = clock();
	} while(benjamin.UpdateTimer(before, after));
	StopTimeUtil(before, after, before1, after1, false);
}

// an utility function of "StopTime", displays the answer, updates player's score, and gets the next question
void WordGame::StopTimeUtil(clock_t &before, clock_t &after, clock_t &before1, clock_t &after1, const bool &guessed_correct) {
	DisplayAnswer();
	MoveCursorToXY(46, 14);
	std::cout << std::string(std::to_string(score).length(), ' ');
	MoveCursorToXY(46, 14);
	std::cout << (score += (guessed_correct ? 1 : -1) * no_of_undisplayed_letters * POINTS_PER_LETTER);
	Sleep(1000);
	before = after;
	after = clock();
	benjamin.UpdateTimer(before, after);
	short minutes = BENJAMIN_TIME_IN_SECONDS/60 - benjamin.GetMinutes();
	double seconds = double(BENJAMIN_TIME_IN_SECONDS%60) - benjamin.GetSeconds();
	if(seconds < 0.0) {
		seconds += 60.0;
		minutes--;
	}
	current_time.IncreaseTimer(minutes, seconds);
	before1 = after1;
	after1 = clock();
	current_time.UpdateTimer(before1, after1);
	GetNextQuestion();
}

// prints the opening (aka title) screen, gives the rules and instructions on how to play the game
void WordGame::PrintOpeningScreen() {
	std::cout << " .----------------.  .----------------.  .----------------.  .----------------.  .----------------.  .----------------." << std::endl;
	std::cout << "| .--------------. || .--------------. || .--------------. || .--------------. || .--------------. || .--------------. |" << std::endl;
	std::cout << "| |  ___  ____   | || |  _________   | || |   _____      | || |     __O__    | || | ____    ____ | || |  _________   | |" << std::endl;
	std::cout << "| | |_  ||_  _|  | || | |_   ___  |  | || |  |_   _|     | || |    |_   _|   | || ||_   \\  /   _|| || | |_   ___  |  | |" << std::endl;
	std::cout << "| |   | |_/ /    | || |   | |_  \\_|  | || |    | |       | || |      | |     | || |  |   \\/   |  | || |   | |_  \\_|  | |" << std::endl;
	std::cout << "| |   |  __'.    | || |   |  _|  _   | || |    | |   _   | || |      | |     | || |  | |\\  /| |  | || |   |  _|  _   | |" << std::endl;
	std::cout << "| |  _| |  \\ \\_  | || |  _| |___/ |  | || |   _| |__/ |  | || |     _| |_    | || | _| |_\\/_| |_ | || |  _| |___/ |  | |" << std::endl;
	std::cout << "| | |____||____| | || | |_________|  | || |  |________|  | || |    |_____|   | || ||_____||_____|| || | |_________|  | |" << std::endl;
	std::cout << "| |              | || |              | || |              | || |              | || |              | || |              | |" << std::endl;
	std::cout << "| '--------------' || '--------------' || '--------------' || '--------------' || '--------------' || '--------------' |" << std::endl;
	std::cout << " '----------------'  '----------------'  '----------------'  '----------------'  '----------------'  '----------------'" << std::endl;
	std::cout << "          .----------------.  .----------------.  .----------------.  .-----------------. .----------------. " << std::endl;
	std::cout << "         | .--------------. || .--------------. || .--------------. || .--------------. || .--------------. |" << std::endl;
	std::cout << "         | |     ____     | || |  ____  ____  | || | _____  _____ | || | ____  _____  | || | _____  _____ | |" << std::endl;
	std::cout << "         | |   .'    `.   | || | |_  _||_  _| | || ||_   _||_   _|| || ||_   \\|_   _| | || ||_   _||_   _|| |" << std::endl;
	std::cout << "         | |  /  .--.  \\  | || |   \\ \\  / /   | || |  | |    | |  | || |  |   \\ | |   | || |  | |    | |  | |" << std::endl;
	std::cout << "         | |  | |    | |  | || |    \\ \\/ /    | || |  | '    ' |  | || |  | |\\ \\| |   | || |  | '    ' |  | |" << std::endl;
	std::cout << "         | |  \\  `--'  /  | || |    _|  |_    | || |   \\ `--' /   | || | _| |_\\   |_  | || |   \\ `--' /   | |" << std::endl;
	std::cout << "         | |   `.____.'   | || |   |______|   | || |    `.__.'    | || ||_____|\\____| | || |    `.__.'    | |" << std::endl;
	std::cout << "         | |              | || |              | || |              | || |              | || |              | |" << std::endl;
	std::cout << "         | '--------------' || '--------------' || '--------------' || '--------------' || '--------------' |" << std::endl;
	std::cout << "          '----------------'  '----------------'  '----------------'  '----------------'  '----------------' " << std::endl;
	std::cout << "\n\n\n\nKelime Oyunu'na ho� geldiniz!";
	std::cout << "\n\n\nKurallar:";
	std::cout << "\n\n-> " << NO_OF_ANSWERS_FOR_EACH_NO_OF_LETTERS << "'er taneden ";
	const unsigned short no_of_questions = NO_OF_ANSWERS_FOR_EACH_NO_OF_LETTERS * (MAX_NO_OF_LETTERS - MIN_NO_OF_LETTERS + 1);
	unsigned short total_no_of_letters = 0;
	for(unsigned short i=MIN_NO_OF_LETTERS;i<=MAX_NO_OF_LETTERS;i++) {
		total_no_of_letters += i * NO_OF_ANSWERS_FOR_EACH_NO_OF_LETTERS;
		std::cout << i << (i == MAX_NO_OF_LETTERS-1 ? " ve " : ", ");
	}
	std::cout << "\b\b harfli; toplam " << no_of_questions << " kelime ya da kelime grubunu verilen tan�mlara g�re ";
	if(GAME_TIME_IN_SECONDS/60 > 0)
		std::cout << GAME_TIME_IN_SECONDS/60 << " dakika ";
	if(GAME_TIME_IN_SECONDS%60 > 0)
		std::cout << GAME_TIME_IN_SECONDS%60 << " saniye ";
	std::cout << "i�erisinde bilmeye �al��acaks�n�z.";
	std::cout << "\n\n-> Bu " << no_of_questions << " kelimeden (" << total_no_of_letters << " harf) bildi�iniz her harf i�in " << POINTS_PER_LETTER << " puan kazanacaks�z.";
	std::cout << "\n\n-> Yard�m i�in '" << HARF_ALAYIM << "' tu�una basarak harf alabilir ama ald���n�z harflerden puan alamazs�n�z, s�reyi '";
	std::cout << BENJAMIN << "' tu�una (Benjamin) basarak durdurabilir ama durdurduktan sonra harf alamazs�n�z.";
	std::cout << "\n\n-> S�reyi durdurduktan itibaren kelimeyi bilmeniz i�in";
	if(BENJAMIN_TIME_IN_SECONDS/60 > 0)
		std::cout << " " << BENJAMIN_TIME_IN_SECONDS/60 << " dakika";
	BENJAMIN_TIME_IN_SECONDS%60 > 0		?	std::cout << " " << BENJAMIN_TIME_IN_SECONDS%60 << " saniyeniz"	:	std::cout << "n�z";
	std::cout << " olacak, bu s�re i�inde kelimeyi bilemezseniz bilemedi�iniz her harf i�in " << POINTS_PER_LETTER << " puan kaybedeceksiniz.";
	std::cout << "\n\n-> Oyunu istedi�iniz zaman durdurmak i�in '" << PAUSE_KEY << "' tu�una, ��kmak i�in de 'Escape' tu�una basabilirsiniz.";
	std::cout << "\n\n\nDiledi�iniz herhangi bir tu�a basarak oyunu ba�latabilirsiniz.";
	if(getch() == ESCAPE_KEY)
		exit(0);
	system("cls");
}

// prints a loading screen with a progress bar
void WordGame::PrintLoadingScreen() {
	MoveCursorToXY(40, 10);
	std::cout << "Y�kleniyor...";
	MoveCursorToXY(34, 11);
	for(unsigned short i=0;i<20;i++) {
		Sleep(50);
		std::cout << char(182);
	}
	system("cls");
}

// prints the question, its hidden answer, and also the score
void WordGame::PrintQandAScreen() const {
	MoveCursorToXY(0, 0);
	std::cout << "\n\n\t\t************************************";
	std::cout << "\n\n\t\t....................................\n\n\n\t\t\t";
	for(const auto &letter : word_on_display)
		std::cout << letter << " ";
	std::cout << "\n\n\n\t\t....................................";
	std::cout << "\n\n\t\t************************************\n\n\t\t";
	std::cout << current_time.GetMinutes() << ":" << std::setfill('0') << std::setw(2) << short(current_time.GetSeconds());
	std::cout << "\t\t\tSkor: " << score << "\n\n\t\t" << current_question;
}

// prints game over screen
void WordGame::PrintEndGameScreen() const {
	system("cls");
	MoveCursorToXY(40, 10);
	std::cout << "Nihai skorunuz: " << score << std::endl;
	std::cout << "\t\t\t\t\tArtt�rd���n�z s�re: " << current_time.GetMinutes() << " dakika " << short(current_time.GetSeconds()) << " saniye";
	if(getch() == ESCAPE_KEY)
		exit(0);
	system("cls");
}

// records the player's score
void WordGame::RecordScore() {
	time_t mytime = time(NULL);
	const std::string filename = "rekorlar.txt";
	std::string player_name;
	MoveCursorToXY(40, 10);
	std::cout << "�sminizi giriniz: ";
	getline(std::cin, player_name);
	std::ofstream myfile(filename, std::ios::app);
	myfile << "Oyuncu ismi: " << player_name << std::string(35 - player_name.length(), ' ');
	myfile << "Oyun tarihi: " << ctime(&mytime) << std::endl;
	myfile << "Skor: " << score << std::string(42 - std::to_string(score).length(), ' ');
	myfile << "Artt�r�lan s�re: " << current_time.GetMinutes() << ":" << std::setfill('0') << std::setw(2) << short(current_time.GetSeconds()) << std::endl;
	myfile << "__________________________________________________" << std::endl;
	myfile.close();
	system("cls");
	MoveCursorToXY(32, 10);
	std::cout << "Eski kay�tlar� g�rmek i�in '" << VIEW_RECORDS << "' tu�una basabilirsiniz." << std::endl;
	std::cout << "\t\t\t\tTekrar oynamak i�in '" << RESTART_KEY << "' tu�una basabilirsiniz." << std::endl;
	std::cout << "\t\t\t\tProgramdan ��kmak i�in ise diledi�iniz herhangi bir tu�a basabilirsiniz.";
	switch(toupper(getch())) {
		case VIEW_RECORDS:
			try {
				std::ifstream myfile(filename);
				system("cls");
				if(myfile.is_open()) {
					std::cout << myfile.rdbuf();
					myfile.close();
					exit(0);
				}
				throw;
			}
			catch(...) {
				std::cerr << "!ERROR!\t\tUnable to open \"" << filename << "\".\t\t!ERROR!" << std::endl;
				exit(1);
			}
		case RESTART_KEY:
			while(!QandA.empty())
				QandA.pop();
			score = 0;
			current_time.SetTimer(GAME_TIME_IN_SECONDS/60, GAME_TIME_IN_SECONDS%60);
			system("cls");
			Start();
	}
}

// gets the next question once the previous question has passed and its answer has been revealed to the player
void WordGame::GetNextQuestion() {
	if(QandA.empty()) {
		PrintEndGameScreen();
		RecordScore();
		return;
	}
	current_word = QandA.front();
	QandA.pop();
	current_question = QandA.front();
	QandA.pop();
	no_of_undisplayed_letters = current_word.length();
	word_on_display = std::string(no_of_undisplayed_letters, '_');
	system("cls");
	PrintQandAScreen();
}

// reveals the answer to the player after time's up, all letters are requested, or the question is answered correctly
void WordGame::DisplayAnswer() {
	std::vector<unsigned short> undisplayed_letter_indexes;
	for(unsigned short i=0;i<word_on_display.length();i++)
		if(word_on_display[i] == '_')
			undisplayed_letter_indexes.push_back(i);
	for(unsigned short i=0;i<30;i++) {
		for(const auto &j : undisplayed_letter_indexes)
			word_on_display[j] = GetRandomNumber<char>('A', 'Z');
		PrintQandAScreen();
		Sleep(20);
	}
	word_on_display = current_word;
	PrintQandAScreen();
	Sleep(200);
}

// selects questions from question database randomly and loads them to memory
void WordGame::ReadQuestion(const unsigned short &no_of_letters) {
	const std::string filename = "Questions/" + std::to_string(no_of_letters) + " letter answers.txt";
	std::ifstream myfile(filename);
	try {
		if(!myfile.is_open())
			throw;
	}
	catch(...) {
		std::cerr << "!ERROR!\t\tUnable to open \"" << filename << "\".\t\t!ERROR!" << std::endl;
		exit(1);
	}
	std::string answer, question;
	getline(myfile, answer);
	unsigned int question_index = GetRandomNumber<unsigned int>(0, atoi(answer.c_str()) - 1);
	myfile.ignore(1);
	for(unsigned int i=0;i<question_index;i++)
		getline(myfile, answer);
	getline(myfile, answer, ':');
	QandA.push(answer);
	myfile.ignore(1);
	getline(myfile, question);
	QandA.push(question);
	myfile.close();
}

// checks whether the player's guess is correct or wrong
bool WordGame::CompareAnswers(const std::string &guess) const {
	if(current_word.length() != guess.length())
		return false;
	std::string answer = current_word;
	for(unsigned short i=0;i<answer.length();i++) {
		switch(answer[i]) {
			case '�':
				answer[i] = 'A';
				break;
			case '�':
				answer[i] = 'I';
				break;
			case '�':
				answer[i] = 'U';
		}
		if(answer[i] != TR_toupper(guess[i]))
			return false;
	}
	return true;
}

// returns the given Turkish character in lowercase
char WordGame::TR_toupper(const char &ch) {
	if(ch>='a' && ch<='z')
		return ch=='i' ? '�' : ch-32;
	switch(ch) {
		case '�':	return '�';
		case '�':	return '�';
		case '�':	return 'I';
		case '�':	return '�';
		case '�':	return '�';
		case '�':	return '�';
		default:	return ch;
	}
}
