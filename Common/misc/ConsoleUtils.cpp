#include "ConsoleUtils.h"

ConsoleQueue* ConsoleQueue::Instance() {
	static ConsoleQueue instance;
	return &instance;
}

void ConsoleQueue::send(std::string text, WORD color = FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN) {
	m.lock();

	coloredText consoleData;
	consoleData.text = text;
	consoleData.color = color;
	q.push(consoleData);

	m.unlock();
}

void ConsoleQueue::printAll() {
	coloredText consoleData;
	while (!q.empty()) {
		consoleData = q.front();

		SetTextColor(consoleData.color);
		std::cout << consoleData.text;

		q.pop();
	}
}