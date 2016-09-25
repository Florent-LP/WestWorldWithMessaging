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
	coutQueue->send(sentence, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
}