#ifndef WORD_GAME_H
#define WORD_GAME_H

#define CP_TURKISH 1254
#define BENJAMIN 'B'
#define HARF_ALAYIM 'H'
#define PAUSE_KEY 'P'
#define ESCAPE_KEY 27
#define RESTART_KEY 'R'
#define VIEW_RECORDS 'Y'
#define GAME_TIME_IN_SECONDS 240
#define BENJAMIN_TIME_IN_SECONDS 30
#define MIN_NO_OF_LETTERS 4
#define MAX_NO_OF_LETTERS 10
#define NO_OF_ANSWERS_FOR_EACH_NO_OF_LETTERS 2
#define POINTS_PER_LETTER 100

class Timer;
class WordGame;

#include <iostream>
#include <iomanip>
#include <queue>
#include <time.h>
#include <windows.h>

class Timer {
private:
	short minutes;
	double seconds;
public:
	Timer(const short &minutes, const double &seconds = 0.0);
	short GetMinutes() const;
	double GetSeconds() const;
	void SetTimer(const short &minutes, const double &seconds);
	void IncreaseTimer(const short &m_inc, const double &s_inc);
	bool UpdateTimer(const clock_t &before, const clock_t &after);
	void UpdateTimerDisplay(clock_t &begin, const clock_t &after, const short &x, const short &y) const;
};

class WordGame {
private:
	int score = 0;
	unsigned short no_of_undisplayed_letters = MIN_NO_OF_LETTERS;
	std::string current_word, current_question, word_on_display;
	std::queue<std::string> QandA;
	Timer current_time, benjamin;
	void Update();
	void GetLetter();
	void StopTime(clock_t &before1, clock_t &after1);
	void StopTimeUtil(clock_t &before, clock_t &after, clock_t &before1, clock_t &after1, const bool &guessed_correct);
	static void PrintOpeningScreen();
	static void PrintLoadingScreen();
	void PrintQandAScreen() const;
	void PrintEndGameScreen() const;
	void RecordScore();
	void GetNextQuestion();
	void DisplayAnswer();
	void ReadQuestion(const unsigned short &no_of_letters);
	bool CompareAnswers(const std::string &guess) const;
	static char TR_toupper(const char &ch);
public:
	WordGame(const short &minutes = 4, const double &seconds = 0.0, const short &benjamin_m = 0, const double &benjamin_s = 30.0);
	void Start();
};

void MoveCursorToXY(const short &x, const short &y);
template<class T> T GetRandomNumber(const T &min, const T &max);

#endif		//KELIME_OYUNU_H
