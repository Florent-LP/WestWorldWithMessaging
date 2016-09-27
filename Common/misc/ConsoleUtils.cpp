#include "ConsoleUtils.h"

ConsoleQueue* ConsoleQueue::Instance() {
	static ConsoleQueue instance;
	return &instance;
}

void ConsoleQueue::send(std::string text, WORD color) {
	// lock access (from other threads) until end of call
	std::lock_guard<std::mutex> lock(m);

	coloredText consoleData;
	consoleData.text = text;
	consoleData.color = color;
	q.push(consoleData);

	// resume print()
	c.notify_one();
}

std::string ConsoleQueue::getLine() {
	waitIdle();
	// lock access until end of call
	std::lock_guard<std::mutex> lock(m);

	std::string input;
	std::getline(std::cin, input);
	return input;
}

void ConsoleQueue::print() {
	// lock access
	std::unique_lock<std::mutex> lock(m);

	// wait until text is pushed in queue
	while (q.empty() && !quit)
		c.wait(lock);

	if (!q.empty()) { // in case quit == true
		coloredText consoleData = q.front();
		SetTextColor(consoleData.color);
		std::cout << consoleData.text;

		q.pop();
	}
}

void ConsoleQueue::printLoop(int period) {
	while (!quit) {
		print();
		Sleep(period);
	}
}

void ConsoleQueue::breakLoop() {
	waitIdle();
	quit = true;
}

void ConsoleQueue::waitIdle(int maxChecks, int period) {
	for (int checks = 0; checks < maxChecks; ) {
		if (q.empty()) checks++;
		else checks = 0;
		Sleep(period);
	}

	// if queue is empty, print() is blocked by c.lock()
	std::lock_guard<std::mutex> lock(m); // lock access until end of call
	c.notify_one(); // unblock print()
}