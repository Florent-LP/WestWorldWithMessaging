#include "Drunkard.h"

void Drunkard::Update() {
  m_pStateMachine->Update();
}

void Drunkard::Say(std::string sentence) {
	consoleMx.lock();

	/*coloredText consoleData;
	consoleData.colors = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
	consoleData.text = sentence;
	consoleQueue->push(consoleData);*/
	SetTextColor(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	std::cout << sentence;

	consoleMx.unlock();
}

bool Drunkard::HandleMessage(const Telegram& msg) {
  return m_pStateMachine->HandleMessage(msg);
}

bool Drunkard::Fatigued() const {
  return (m_iFatigue > DrunkardTirednessThreshold);
}

int Drunkard::SleepDuration() const {
	return RandInt(0, DrunkardTirednessThreshold);
}

bool Drunkard::Drunk() const {
	return (m_iDrunkenness > DrunkennessThreshold);
}