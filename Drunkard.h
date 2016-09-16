#ifndef DRUNKARD_H
#define DRUNKARD_H
//------------------------------------------------------------------------
//
//  Name:   Drunkard.h
//
//  Desc:   A class defining a drunken goldminer.
//
//  Author: Florent Le Prince 2016 (florent.le-prince1@uqac.ca)
//
//------------------------------------------------------------------------
#include "Miner.h"

class Drunkard : public Miner {
	private:
	public:
		Drunkard(int id): Miner(id) {
			//set up state machine
			m_pStateMachine = new StateMachine<Miner>(this);
			m_pStateMachine->SetCurrentState(GoHomeAndSleepTilRested::Instance());
		}
		~Drunkard() {
			delete m_pStateMachine;
		}
};

#endif