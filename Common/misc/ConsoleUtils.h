#ifndef CONSOLE_UTILS_H
#define CONSOLE_UTILS_H
//------------------------------------------------------------------------
//
//  Name:   ConsoleUtils.h
//
//  Desc:   Just a few handy utilities for dealing with consoles
//
//  Author: Mat Buckland (fup@ai-junkie.com)
//  Modified: Florent Le Prince (florent.le-prince1@uqac.fr), Maxime Legrand
//
//------------------------------------------------------------------------
#include <windows.h>
#include <conio.h>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <queue>
#include <string>

// This class enables thread concurrency over cout.
// Each thread enqueues its output when the queue is free.
// The class can print continuously the contents of the queue
// when the printLoop is launched in a separate thread.
#define coutQueue ConsoleQueue::Instance()

typedef struct {
	WORD color;
	std::string text;
} coloredText;

class ConsoleQueue {
	private:
		std::mutex m; // mutex for the queue
		std::condition_variable c; // used to block printLoop
		std::atomic<bool> quit; // used to break printLoop
		std::queue<coloredText> q; // std::cout queue
	public:

		ConsoleQueue() : m(), c(), quit(false), q() {}
		~ConsoleQueue() {}

		// singleton
		static ConsoleQueue* Instance();

		// replaces std::cout
		void send(std::string text, WORD color = FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN);

		// replaces std::cin
		std::string getLine();

		// dequeues a coloredText and prints it
		void print();

		// loops print() (use in a separate thread)
		void printLoop(int period = 200);

		// breaks the printLoop (must be called before joining its thread)
		void breakLoop();

		// ensures that the queue has been empty for a while (inactive)
		void waitIdle(int maxChecks = 5, int period = 40);
};

//default text colors can be found in wincon.h
inline void SetTextColor(WORD colors)
{
  HANDLE hConsole=GetStdHandle(STD_OUTPUT_HANDLE);
  
  SetConsoleTextAttribute(hConsole, colors);
}

inline void PressAnyKeyToContinue()
{
  //change text color to white
  SetTextColor(FOREGROUND_BLUE| FOREGROUND_RED | FOREGROUND_GREEN);

  std::cout << "\n\nPress any key to continue" << std::endl; 

  while (!_kbhit()){}

  return;
}

#endif