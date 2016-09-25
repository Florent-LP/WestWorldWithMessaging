#include "MinersWife.h"

bool MinersWife::HandleMessage(const Telegram& msg)
{
  return m_pStateMachine->HandleMessage(msg);
}


void MinersWife::Update()
{
  m_pStateMachine->Update();
}


void MinersWife::Say(std::string sentence) {
	consoleMx.lock();

	/*coloredText consoleData;
	consoleData.colors = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
	consoleData.text = sentence;
	consoleQueue->push(consoleData);*/
	SetTextColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
	std::cout << sentence;

	consoleMx.unlock();
}