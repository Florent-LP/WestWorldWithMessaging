#include "Drunkard.h"

void Drunkard::Update() {
  m_pStateMachine->Update();
}

void Drunkard::Say(std::string sentence) {
	coutQueue->send(sentence, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
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