#include "Drunkard.h"

void Drunkard::Update() {
  SetTextColor(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
  m_pStateMachine->Update();
}

bool Drunkard::HandleMessage(const Telegram& msg) {
  return m_pStateMachine->HandleMessage(msg);
}

bool Drunkard::Fatigued() const {
  return m_iFatigue > DrunkardTirednessThreshold;
}

bool Drunkard::Drunk() const {
	return m_iDrunkenness > DrunkennessThreshold;
}