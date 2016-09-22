#ifndef DRUNKARD_OWNED_STATES_H
#define DRUNKARD_OWNED_STATES_H
//------------------------------------------------------------------------
//
//  Name:   DrunkardOwnedStates.h
//
//  Desc:   All the states that can be assigned to the Drunkard class.
//
//  Author: Florent Le Prince 2016 (florent.le-prince1@uqac.ca)
//
//------------------------------------------------------------------------

#include "fsm/State.h"
class Drunkard;

class DrunkardsGlobalState : public State<Drunkard> {  
	private:
		DrunkardsGlobalState(){}

		DrunkardsGlobalState(const DrunkardsGlobalState&);
		DrunkardsGlobalState& operator=(const DrunkardsGlobalState&);
 
	public:
		static DrunkardsGlobalState* Instance();

		virtual void Enter(Drunkard* drunkard){}
		virtual void Execute(Drunkard* drunkard);
		virtual void Exit(Drunkard* drunkard){}

		virtual bool OnMessage(Drunkard* drunkard, const Telegram& msg);
};

//------------------------------------------------------------------------

class GetDrunk : public State<Drunkard> {
	private:
		GetDrunk(){}

		GetDrunk(const GetDrunk&);
		GetDrunk& operator=(const GetDrunk&);
 
	public:
		static GetDrunk* Instance();

		virtual void Enter(Drunkard* drunkard);
		virtual void Execute(Drunkard* drunkard);
		virtual void Exit(Drunkard* drunkard);

		virtual bool OnMessage(Drunkard* drunkard, const Telegram& msg);
};

//------------------------------------------------------------------------

class SeekFight : public State<Drunkard> {
	private:
		SeekFight(){}

		SeekFight(const SeekFight&);
		SeekFight& operator=(const SeekFight&);
 
	public:
		static SeekFight* Instance();

		virtual void Enter(Drunkard* drunkard);
		virtual void Execute(Drunkard* drunkard);
		virtual void Exit(Drunkard* drunkard);

		virtual bool OnMessage(Drunkard* drunkard, const Telegram& msg);
};

//------------------------------------------------------------------------

class GetInFight : public State<Drunkard> {
	private:
		GetInFight(){}

		GetInFight(const GetInFight&);
		GetInFight& operator=(const GetInFight&);
		
	public:
		static GetInFight* Instance();

		virtual void Enter(Drunkard* drunkard);
		virtual void Execute(Drunkard* drunkard);
		virtual void Exit(Drunkard* drunkard);

		virtual bool OnMessage(Drunkard* drunkard, const Telegram& msg);
};

//------------------------------------------------------------------------

class SleepTilRested : public State<Drunkard> {
	private:
		SleepTilRested(){}

		SleepTilRested(const SleepTilRested&);
		SleepTilRested& operator=(const SleepTilRested&);

		int m_iSleepDuration;
		
	public:
		static SleepTilRested* Instance();

		virtual void Enter(Drunkard* drunkard);
		virtual void Execute(Drunkard* drunkard);
		virtual void Exit(Drunkard* drunkard);

		virtual bool OnMessage(Drunkard* drunkard, const Telegram& msg);
};
#endif