#ifndef DRUNKARD_H
#define DRUNKARD_H
//------------------------------------------------------------------------
//
//  Name:   Drunkard.h
//
//  Desc:   A class defining a drunkard.
//
//  Authors: Florent Le Prince 2016 (florent.le-prince1@uqac.ca), Maxime Legrand
//
//------------------------------------------------------------------------

#include "BaseGameEntity.h"
#include "Locations.h"
#include "misc/ConsoleUtils.h"
#include "fsm/State.h"
#include "fsm/StateMachine.h"
#include "DrunkardOwnedStates.h"
#include "misc/Utils.h"

template <class entity_type> class State;
struct Telegram;

const int DrunkardTirednessThreshold = 5;
const int DrunkennessThreshold = 5;

class Drunkard : public BaseGameEntity {
	private:
		StateMachine<Drunkard>* m_pStateMachine;
		location_type m_Location;
		
		int m_iFatigue;
		int m_iDrunkenness;
		bool m_bRivalSpotted;

	public:
		// Constructor & Destructor
		Drunkard(int id): m_iFatigue(0),
                          m_iDrunkenness(0),
						  m_bRivalSpotted(false),
                          BaseGameEntity(id) {
			m_pStateMachine = new StateMachine<Drunkard>(this);

			m_pStateMachine->SetCurrentState(SleepTilRested::Instance());

			m_pStateMachine->SetGlobalState(DrunkardsGlobalState::Instance());
		}

		~Drunkard() {
			delete m_pStateMachine;
		}
		
		// Getters & Setters
		StateMachine<Drunkard>* GetFSM() const {
			return m_pStateMachine;
		}
		
		location_type Location() const {
			return m_Location;
		}

		void ChangeLocation(location_type loc) {
			m_Location = loc;
		}

		bool Fatigued() const;

		int getFatigue() const {
			return m_iFatigue;
		}

		void DecreaseFatigue() {
			m_iFatigue -= 1;
		}

		void IncreaseFatigue() {
			m_iFatigue += 1;
		}

		int SleepDuration() const;

		bool Drunk() const;

		int getDrunkenness() const {
			return m_iDrunkenness;
		}

		void ResetDrunkenness() {
			m_iDrunkenness = 0;
		}

		void DecreaseDrunkenness() {
			m_iDrunkenness -= 1;
		}

		void IncreaseDrunkenness() {
			m_iDrunkenness += 1;
		}

		bool RivalSpotted() const {
			return m_bRivalSpotted;
		}

		void SetRivalSpotted(bool val) {
			m_bRivalSpotted = val;
		}
		
		// Operators
		void Update();

		virtual bool HandleMessage(const Telegram& msg);

		virtual void Say(std::string sentence);
};
#endif