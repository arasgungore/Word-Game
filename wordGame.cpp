#include "wordGame.h"
#include <fstream>
#include <vector>
#include <cmath>
//#include <conio.h>
#include <sstream>

template<typename T> std::string to_string(const T &value) {
	std::ostringstream os;
	os << value;
	return os.str();
}

void MoveCursorToXY(const short &x, const short &y) {	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), (COORD){x,y});	}

template<class T> T GetRandomNumber(const T &min, const T &max) {	return min + T(double(rand()) / double(RAND_MAX+1.0) * (max-min+1));	}



Timer::Timer(const short &minutes, const double &seconds) : minutes(minutes), seconds(seconds) {	}

short Timer::GetMinutes() const {	return minutes;	}

double Timer::GetSeconds() const {	return seconds;	}

void Timer::SetTimer(const short &minutes, const double &seconds) {
	this->minutes = minutes;
	this->seconds = seconds;
}

void Timer::IncreaseTimer(const short &m_inc, const double &s_inc) {
	minutes += m_inc;
	seconds += s_inc;
	if(seconds>=60.0) {
		seconds-=60.0;
		minutes++;
	}
}

bool Timer::UpdateTimer(const clock_t &before, const clock_t &after) {
	seconds -= double(after - before) / double(CLOCKS_PER_SEC);
	if(seconds<0.0) {
		seconds += 60.0;
		if((--minutes)<0)
			return false;
	}
	return true;
}

void Timer::UpdateTimerDisplay(clock_t &begin, const clock_t &after, const short &x, const short &y) const {
	if(double(after - begin) / double(CLOCKS_PER_SEC) >= 0.9) {
		MoveCursorToXY(x, y);
		std::cout << "     ";
		MoveCursorToXY(x, y);
		std::cout << minutes << ":" << std::setfill('0') << std::setw(2) << short(seconds);
		begin = after;
	}
}



WordGame::WordGame(const short &minutes, const double &seconds, const short &benjamin_m, const double &benjamin_s) : current_time(minutes,seconds), benjamin(benjamin_m,benjamin_s) {	}

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

void WordGame::Update() {
	clock_t begin = clock(), before, after = clock();
	do {
		current_time.UpdateTimerDisplay(begin, after, 16, 14);
//		if(kbhit())
//			switch(toupper(getch())) {
			switch(toupper(getchar())) {
				case BENJAMIN:
					StopTime(before, after);
					break;
				case HARF_ALAYIM:
					GetLetter();
					break;
				case PAUSE_KEY: {
					clock_t begin_temp=clock(), after_temp;
					char key;
					do {
//						key = getch();
						key = getchar();
						if(key == ESCAPE_KEY)
							exit(0);
					} while((key = toupper(key)) != PAUSE_KEY);
					after_temp = clock();
					double past_time = double(after_temp - begin_temp) / double(CLOCKS_PER_SEC);
					current_time.IncreaseTimer(short(past_time)/60, fmod(past_time, 60.0));
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

void WordGame::GetLetter() {
	unsigned short letter_index;
	do {
		letter_index = GetRandomNumber<unsigned short>(0, current_word.length()-1);
	} while(word_on_display[letter_index] != '_');
	for(unsigned short i=0;i<30;i++) {
		word_on_display[letter_index] = GetRandomNumber<char>('A', 'Z');
		PrintQandAScreen();
		Sleep(20);
	}
	word_on_display[letter_index] = current_word[letter_index];
	PrintQandAScreen();
	if((--no_of_undisplayed_letters)<=0) {
		Sleep(1000);
		current_time.IncreaseTimer(0, 1.1);
		GetNextQuestion();
		return;
	}
	current_time.IncreaseTimer(0, 0.6);
}

void WordGame::StopTime(clock_t &before1, clock_t &after1) {
	benjamin.SetTimer(BENJAMIN_TIME_IN_SECONDS/60, BENJAMIN_TIME_IN_SECONDS%60);
	MoveCursorToXY(16, 19);
	std::cout << benjamin.GetMinutes() << ":" << std::setfill('0') << std::setw(2) << short(benjamin.GetSeconds()) << std::endl;
	std::cout << "\t\tTahmininizi giriniz: ";
	clock_t begin=clock(), before, after=clock();
	do {
		benjamin.UpdateTimerDisplay(begin, after, 16, 19);
//		if(kbhit()) {
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
//		}
		before = after;
		after = clock();
	} while(benjamin.UpdateTimer(before, after));
	StopTimeUtil(before, after, before1, after1, false);
}

void WordGame::StopTimeUtil(clock_t &before, clock_t &after, clock_t &before1, clock_t &after1, const bool &guessed_correct) {
	DisplayAnswer();
	MoveCursorToXY(46, 14);
	std::cout << std::string(to_string(score).length(), ' ');
	MoveCursorToXY(46, 14);
	std::cout << (score += (guessed_correct ? 1 : -1) * no_of_undisplayed_letters * POINTS_PER_LETTER);
	Sleep(1000);
	before = after;
	after = clock();
	benjamin.UpdateTimer(before, after);
	short minutes = BENJAMIN_TIME_IN_SECONDS/60 - benjamin.GetMinutes();
	double seconds = double(BENJAMIN_TIME_IN_SECONDS%60) - benjamin.GetSeconds();
	if(seconds<0.0) {
		seconds+=60.0;
		minutes--;
	}
	current_time.IncreaseTimer(minutes, seconds);
	before1 = after1;
	after1 = clock();
	current_time.UpdateTimer(before1, after1);
	GetNextQuestion();
}

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
	std::cout << "\n\n\n\nKelime Oyunu'na hoþ geldiniz!";
	std::cout << "\n\n\nKurallar:";
	std::cout << "\n\n-> " << NO_OF_ANSWERS_FOR_EACH_NO_OF_LETTERS << "'er taneden ";
	const unsigned short no_of_questions = NO_OF_ANSWERS_FOR_EACH_NO_OF_LETTERS * (MAX_NO_OF_LETTERS - MIN_NO_OF_LETTERS + 1);
	unsigned short total_no_of_letters=0;
	for(unsigned short i=MIN_NO_OF_LETTERS;i<=MAX_NO_OF_LETTERS;i++) {
		total_no_of_letters += i * NO_OF_ANSWERS_FOR_EACH_NO_OF_LETTERS;
		std::cout << i << (i == MAX_NO_OF_LETTERS-1 ? " ve " : ", ");
	}
	std::cout << "\b\b harfli; toplam " << no_of_questions << " kelime ya da kelime grubunu verilen tanýmlara göre ";
	if(GAME_TIME_IN_SECONDS/60>0)
		std::cout << GAME_TIME_IN_SECONDS/60 << " dakika ";
	if(GAME_TIME_IN_SECONDS%60>0)
		std::cout << GAME_TIME_IN_SECONDS%60 << " saniye ";
	std::cout << "içerisinde bilmeye çalýþacaksýnýz.";
	std::cout << "\n\n-> Bu " << no_of_questions << " kelimeden (" << total_no_of_letters << " harf) bildiðiniz her harf için " << POINTS_PER_LETTER << " puan kazanacaksýz.";
	std::cout << "\n\n-> Yardým için '" << HARF_ALAYIM << "' tuþuna basarak harf alabilir ama aldýðýnýz harflerden puan alamazsýnýz, süreyi '";
	std::cout << BENJAMIN << "' tuþuna (Benjamin) basarak durdurabilir ama durdurduktan sonra harf alamazsýnýz.";
	std::cout << "\n\n-> Süreyi durdurduktan itibaren kelimeyi bilmeniz için";
	if(BENJAMIN_TIME_IN_SECONDS/60>0)
		std::cout << " " << BENJAMIN_TIME_IN_SECONDS/60 << " dakika";
	BENJAMIN_TIME_IN_SECONDS%60>0		?	std::cout << " " << BENJAMIN_TIME_IN_SECONDS%60 << " saniyeniz"	:	std::cout << "nýz";
	std::cout << " olacak, bu süre içinde kelimeyi bilemezseniz bilemediðiniz her harf için " << POINTS_PER_LETTER << " puan kaybedeceksiniz.";
	std::cout << "\n\n-> Oyunu istediðiniz zaman durdurmak için '" << PAUSE_KEY << "' tuþuna, çýkmak için de 'Escape' tuþuna basabilirsiniz.";
	std::cout << "\n\n\nDilediðiniz herhangi bir tuþa basarak oyunu baþlatabilirsiniz.";
//	if(getch() == ESCAPE_KEY)
	if(getchar() == ESCAPE_KEY)
		exit(0);
	system("cls");
}

void WordGame::PrintLoadingScreen() {
	MoveCursorToXY(40, 10);
	std::cout << "Yükleniyor...";
	MoveCursorToXY(34, 11);
	for(unsigned short i=0;i<20;i++) {
		Sleep(50);
		std::cout << char(182);
	}
	system("cls");
}

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

void WordGame::PrintEndGameScreen() const {
	system("cls");
	MoveCursorToXY(40, 10);
	std::cout << "Nihai skorunuz: " << score << std::endl;
	std::cout << "\t\t\t\t\tArttýrdýðýnýz süre: " << current_time.GetMinutes() << " dakika " << short(current_time.GetSeconds()) << " saniye";
//	if(getch() == ESCAPE_KEY)
	if(getchar() == ESCAPE_KEY)
		exit(0);
	system("cls");
}

void WordGame::RecordScore() {
	time_t mytime = time(NULL);
	const std::string filename = "rekorlar.txt";
	std::string player_name;
	MoveCursorToXY(40, 10);
	std::cout << "Ýsminizi giriniz: ";
	getline(std::cin, player_name);
	std::ofstream myfile(filename, std::ios::app);
	myfile << "Oyuncu ismi: " << player_name << std::string(35 - player_name.length(), ' ');
	myfile << "Oyun tarihi: " << ctime(&mytime) << std::endl;
	myfile << "Skor: " << score << std::string(42 - to_string(score).length(), ' ');
	myfile << "Arttýrýlan süre: " << current_time.GetMinutes() << ":" << std::setfill('0') << std::setw(2) << short(current_time.GetSeconds()) << std::endl;
	myfile << "__________________________________________________" << std::endl;
	myfile.close();
	system("cls");
	MoveCursorToXY(32, 10);
	std::cout << "Eski kayýtlarý görmek için '" << VIEW_RECORDS << "' tuþuna basabilirsiniz." << std::endl;
	std::cout << "\t\t\t\tTekrar oynamak için '" << RESTART_KEY << "' tuþuna basabilirsiniz." << std::endl;
	std::cout << "\t\t\t\tProgramdan çýkmak için ise dilediðiniz herhangi bir tuþa basabilirsiniz.";
//	switch(toupper(getch())) {
	switch(toupper(getchar())) {
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

void WordGame::ReadQuestion(const unsigned short &no_of_letters) {
	const std::string filename = "Questions/" + to_string(no_of_letters) + " letter answers.txt";
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
	unsigned int question_index = GetRandomNumber<unsigned int>(0, atoi(answer.c_str())-1);
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

bool WordGame::CompareAnswers(const std::string &guess) const {
	if(current_word.length() != guess.length())
		return false;
	std::string answer = current_word;
	for(unsigned short i=0;i<answer.length();i++) {
		switch(answer[i]) {
			case 'Â':
				answer[i] = 'A';
				break;
			case 'Î':
				answer[i] = 'I';
				break;
			case 'Û':
				answer[i] = 'U';
		}
		if(answer[i] != TR_toupper(guess[i]))
			return false;
	}
	return true;
}

char WordGame::TR_toupper(const char &ch) {
	if(ch>='a' && ch<='z')
		return ch=='i' ? 'Ý' : ch-32;
	switch(ch) {
		case 'ç':	return 'Ç';
		case 'ð':	return 'Ð';
		case 'ý':	return 'I';
		case 'ö':	return 'Ö';
		case 'þ':	return 'Þ';
		case 'ü':	return 'Ü';
		default:	return ch;
	}
}
