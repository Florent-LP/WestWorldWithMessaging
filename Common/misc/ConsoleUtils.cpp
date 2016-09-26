#include "ConsoleUtils.h"

ConsoleQueue* ConsoleQueue::Instance() {
	static ConsoleQueue instance;
	return &instance;
}

void ConsoleQueue::send(std::string text, WORD color = FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN) {
	std::lock_guard<std::mutex> lock(m);

	coloredText consoleData;
	consoleData.text = text;
	consoleData.color = color;
	q.push(consoleData);

	c.notify_one();
}

std::string ConsoleQueue::getLine() {
	while (!q.empty());
	std::lock_guard<std::mutex> lock(m);

	std::string input;
	std::getline(std::cin, input);
	return input;
}

void ConsoleQueue::print() {
	std::unique_lock<std::mutex> lock(m);

	while (q.empty())
		c.wait(lock);

	coloredText consoleData = q.front();
	SetTextColor(consoleData.color);
	std::cout << consoleData.text;

	q.pop();
}

void ConsoleQueue::printLoop() {
	while (!quit) {
		print();
		Sleep(200);
	}
}

void ConsoleQueue::termLoop() {
	/*bool empty = false;
	while (!empty) {
		m.lock();
		empty = q.empty();
		m.unlock();
	}*/

	while (!q.empty());

	quit = true;
}